#define M_PI 3.14159265358979323846
#include <GL/zgl.h>
#include "msghandling.h"

void glopMaterial(GLContext *c,GLParam *p)
{
  int mode=p[1].i;
  int type=p[2].i;
  GLfloat *v=&p[3].f;
  int i;
  GLMaterial *m;

  if (mode == GL_FRONT_AND_BACK) {
    p[1].i=GL_FRONT;
    glopMaterial(c,p);
    mode=GL_BACK;
  }
  if (mode == GL_FRONT) m=&c->materials[0];
  else m=&c->materials[1];

  switch(type) {
  case GL_EMISSION:
    for(i=0;i<4;i++)
      m->emission[i]=v[i];
    break;
  case GL_AMBIENT:
    for(i=0;i<4;i++)
      m->ambient[i]=v[i];
    break;
  case GL_DIFFUSE:
    for(i=0;i<4;i++)
      m->diffuse[i]=v[i];
    break;
  case GL_SPECULAR:
    for(i=0;i<4;i++)
      m->specular[i]=v[i];
    break;
  case GL_SHININESS:
    m->shininess=v[0];
    m->shininess_i = (int)(((v[0]/ (float)(128))* (float)(SPECULAR_BUFFER_RESOLUTION)));
    break;
  case GL_AMBIENT_AND_DIFFUSE:
    for(i=0;i<4;i++)
      m->diffuse[i]=v[i];
    for(i=0;i<4;i++)
      m->ambient[i]=v[i];
    break;
  default:
    assert(0);
  }
}

void glopColorMaterial(GLContext *c,GLParam *p)
{
  int mode=p[1].i;
  int type=p[2].i;

  c->current_color_material_mode=mode;
  c->current_color_material_type=type;
}

void glopLight(GLContext *c,GLParam *p)
{
  int light=p[1].i;
  int type=p[2].i;
  glm::vec4 v;
  GLLight *l;
  int i;
  
  assert(light >= GL_LIGHT0 && light < GL_LIGHT0+MAX_LIGHTS );

  l=&c->lights[light-GL_LIGHT0];

  for(i=0;i<4;i++) v[i]=p[3+i].f;

  switch(type) {
  case GL_AMBIENT:
    l->ambient=v;
    break;
  case GL_DIFFUSE:
    l->diffuse=v;
    break;
  case GL_SPECULAR:
    l->specular=v;
    break;
  case GL_POSITION:
    {
      glm::vec4 pos;
	  pos = c->matrix_stack_ptr[0][0] * v;
//      gl_M4_MulV4(&pos,c->matrix_stack_ptr[0],&v);

      l->position=pos;

      if ((float)(l->position.w) == (float)((float)(0))) {
        l->norm_position.x=pos.x;
        l->norm_position.y=pos.y;
        l->norm_position.z=pos.z;
        
        //gl_V3_Norm(&l->norm_position);
		l->norm_position = glm::normalize(l->norm_position);
      }
    }
    break;
  case GL_SPOT_DIRECTION:
	  for (i = 0; i < 3; i++)
	  {
	    l->spot_direction[i]=v[i];
		l->norm_spot_direction[i]=v[i];
	  }
	  l->norm_spot_direction = glm::normalize(l->norm_spot_direction);
    //gl_V3_Norm(&l->norm_spot_direction);
    break;
  case GL_SPOT_EXPONENT:
    l->spot_exponent=v.x;
    break;
  case GL_SPOT_CUTOFF:
    {
#define SLL_M_PI (M_PI)
      GLfloat a=v.x, tmp180=(float)(180);
      assert((float)(a) == (float)(tmp180) || ((float)(a)>=(float)((float)(0)) && (float)(a)<=(float)((float)(90))));
      l->spot_cutoff=a;
      if ((float)(a) != (float)(tmp180)) l->cos_spot_cutoff=(float)cos(((a* SLL_M_PI)/ tmp180));
    }
    break;
  case GL_CONSTANT_ATTENUATION:
    l->attenuation[0]=v.x;
    break;
  case GL_LINEAR_ATTENUATION:
    l->attenuation[1]=v.x;
    break;
  case GL_QUADRATIC_ATTENUATION:
    l->attenuation[2]=v.x;
    break;
  default:
    assert(0);
  }
}
  

void glopLightModel(GLContext *c,GLParam *p)
{
  int pname=p[1].i;
  GLfloat *v=&p[2].f;
  int i;

  switch(pname) {
  case GL_LIGHT_MODEL_AMBIENT:
    for(i=0;i<4;i++) 
      c->ambient_light_model[i]=v[i];
    break;
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    c->local_light_model=(int)(v[0]);
    break;
  case GL_LIGHT_MODEL_TWO_SIDE:
    c->light_model_two_side = (int)(v[0]);
    break;
  default:
    tgl_warning("glopLightModel: illegal pname: 0x%x\n", pname);
    //assert(0);
    break;
  }
}


static inline GLfloat clampf(GLfloat a,GLfloat min,GLfloat max)
{
  if ((float)(a)<(float)(min)) return min;
  else if ((float)(a)>(float)(max)) return max;
  else return a;
}

void gl_enable_disable_light(GLContext *c,int light,int v)
{
  GLLight *l=&c->lights[light];
  if (v && !l->enabled) {
    l->enabled=1;
    l->next=c->first_light;
    c->first_light=l;
    l->prev=NULL;
  } else if (!v && l->enabled) {
    l->enabled=0;
    if (l->prev == NULL) c->first_light=l->next;
    else l->prev->next=l->next;
    if (l->next != NULL) l->next->prev=l->prev;
  }
}

/* non optimized lightening model */
void gl_shade_vertex(GLContext *c,GLVertex *v)
{
  GLfloat R,G,B,A;
  GLMaterial *m;
  GLLight *l;
  glm::vec3 n,s,d;
  GLfloat dist,tmp,att,dot,dot_spot,dot_spec;
  int twoside = c->light_model_two_side;

  m=&c->materials[0];

  n.x=v->normal.x;
  n.y=v->normal.y;
  n.z=v->normal.z;

  R=(m->emission.x+ (m->ambient.x* c->ambient_light_model.x));
  G=(m->emission.y+ (m->ambient.y* c->ambient_light_model.y));
  B=(m->emission.z+ (m->ambient.z* c->ambient_light_model.z));
  A=clampf(m->diffuse.w, (float)(0), (float)(1));

  for(l=c->first_light;l!=NULL;l=l->next) {
    GLfloat lR,lB,lG;
    
    /* ambient */
    lR=(l->ambient.x* m->ambient.x);
    lG=(l->ambient.y* m->ambient.y);
    lB=(l->ambient.z* m->ambient.z);

    if ((float)(l->position.w) == (float)((float)(0))) {
      /* light at infinity */
      d.x=l->position.x;
      d.y=l->position.y;
      d.z=l->position.z;
      att=(float)(1);
    } else {
      /* distance attenuation */
      d.x=(l->position.x- v->ec.x);
      d.y=(l->position.y- v->ec.y);
      d.z=(l->position.z- v->ec.z);
      dist=(float)sqrt((((d.x*d.x)+ (d.y*d.y))+ (d.z*d.z)));
      if ((float)(dist)>(float)((1E-3))) {
        tmp=((float)(1)/dist);
        d.x=(d.x* tmp);
        d.y=(d.y* tmp);
        d.z=(d.z* tmp);
      }
      att=((float)(1)/ (l->attenuation[0]+(dist* (l->attenuation[1]+(dist* l->attenuation[2])))));
    }
    dot=(((d.x*n.x)+ (d.y*n.y))+ (d.z*n.z));
    if (twoside && (float)(dot) < (float)((float)(0))) dot = -(dot);
    if ((float)(dot)>(float)((float)(0))) {
      /* diffuse light */
      lR = (lR+ ((dot* l->diffuse.x)* m->diffuse.x));
      lG = (lG+ ((dot* l->diffuse.y)* m->diffuse.y));
      lB = (lB+ ((dot* l->diffuse.z)* m->diffuse.z));

      /* spot light */
      if ((float)(l->spot_cutoff) != (float)((float)(180))) {
        dot_spot=-((((d.x*l->norm_spot_direction.x)+(d.y*l->norm_spot_direction.y))+(d.z*l->norm_spot_direction.z)));
        if (twoside && (float)(dot_spot) < (float)((float)(0))) dot_spot = -(dot_spot);
        if ((float)(dot_spot) < (float)(l->cos_spot_cutoff)) {
          /* no contribution */
          continue;
        } else {
          /* TODO: optimize */
          if ((float)(l->spot_exponent) > (float)((float)(0))) {
            att=(att* (float)pow(dot_spot,l->spot_exponent));
          }
        }
      }

      /* specular light */
      
      if (c->local_light_model) {
        glm::vec3 vcoord;
        vcoord.x=v->ec.x;
        vcoord.y=v->ec.y;
        vcoord.z=v->ec.z;
		vcoord = glm::normalize(vcoord);
        //gl_V3_Norm(&vcoord);
        s.x=(d.x- vcoord.x);
        s.y=(d.y- vcoord.x);
        s.z=(d.z- vcoord.x);
      } else {
        s.x=d.x;
        s.y=d.y;
        s.z=(d.z+ (float)(1));
      }
      dot_spec=(((n.x*s.x)+ (n.y*s.y))+(n.z*s.z));
      if (twoside && (float)(dot_spec) < (float)((float)(0))) dot_spec = -(dot_spec);
      if ((float)(dot_spec)>(float)((float)(0))) {
        GLSpecBuf *specbuf;
        int idx;
        tmp=(float)sqrt((((s.x*s.x)+(s.y*s.y))+(s.z*s.z)));
        if ((float)(tmp) > (float)((1E-3))) {
          dot_spec=(dot_spec/ tmp);
        }
      
        /* TODO: optimize */
        /* testing specular buffer code */
        /* dot_spec= pow(dot_spec,m->shininess);*/
        specbuf = specbuf_get_buffer(c, m->shininess_i, m->shininess);
        idx = (int)((dot_spec* (float)(SPECULAR_BUFFER_SIZE)));
        if (idx > SPECULAR_BUFFER_SIZE) idx = SPECULAR_BUFFER_SIZE;
        dot_spec = specbuf->buf[idx];
        lR=(lR+ ((dot_spec* l->specular.x)* m->specular.x));
        lG=(lG+ ((dot_spec* l->specular.y)* m->specular.y));
        lB=(lB+ ((dot_spec* l->specular.z)* m->specular.z));
      }
    }

    R=(R+ (att* lR));
    G=(G+ (att* lG));
    B=(B+ (att* lB));
  }

  v->color.x=clampf(R, (float)(0), (float)(1));
  v->color.y=clampf(G, (float)(0), (float)(1));
  v->color.z=clampf(B, (float)(0), (float)(1));
  v->color.w=A;
}

