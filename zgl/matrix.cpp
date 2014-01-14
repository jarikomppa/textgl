#define M_PI 3.14159265358979323846
#include <GL/zgl.h>

void gl_print_matrix( const GLfloat *m)
{
#ifndef GP32
   int i;

   for (i=0;i<4;i++) {
      fprintf(stderr,"%f %f %f %f\n", (float)(m[i]), (float)(m[4+i]), (float)(m[8+i]), (float)(m[12+i]) );
   }
#endif
}

static inline void gl_matrix_update(GLContext *c)
{
  c->matrix_model_projection_updated=(c->matrix_mode<=1);
}


void glopMatrixMode(GLContext *c,GLParam *p)
{
  int mode=p[1].i;
  switch(mode) {
  case GL_MODELVIEW:
    c->matrix_mode=0;
    break;
  case GL_PROJECTION:
    c->matrix_mode=1;
    break;
  case GL_TEXTURE:
    c->matrix_mode=2;
    break;
  default:
    assert(0);
  }
}

void glopLoadMatrix(GLContext *c,GLParam *p)
{
  glm::mat4 *m;
  int i;
  
  GLParam *q;

  m=c->matrix_stack_ptr[c->matrix_mode];
  q=p+1;

  for(i=0;i<4;i++) {
    m[0][0][i]=q[0].f;
    m[0][1][i]=q[1].f;
    m[0][2][i]=q[2].f;
    m[0][3][i]=q[3].f;
    q+=4;
  }

  gl_matrix_update(c);
}

void glopLoadIdentity(GLContext *c,GLParam *p)
{

  //gl_M4_Id(c->matrix_stack_ptr[c->matrix_mode]);
	c->matrix_stack_ptr[c->matrix_mode][0] = glm::mat4();
  gl_matrix_update(c);
}

void glopMultMatrix(GLContext *c,GLParam *p)
{
  glm::mat4 m;
  int i;

  GLParam *q;
  q=p+1;

  for(i=0;i<4;i++) {
    m[0][i]=q[0].f;
    m[1][i]=q[1].f;
    m[2][i]=q[2].f;
    m[3][i]=q[3].f;
    q+=4;
  }

//  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);
  c->matrix_stack_ptr[c->matrix_mode][0] = c->matrix_stack_ptr[c->matrix_mode][0] * m;

  gl_matrix_update(c);
}


void glopPushMatrix(GLContext *c,GLParam *p)
{
  int n=c->matrix_mode;
  glm::mat4 *m;

  assert( (c->matrix_stack_ptr[n] - c->matrix_stack[n] + 1 )
	   < c->matrix_stack_depth_max[n] );

  m=++c->matrix_stack_ptr[n];
  
  m[0] = m[-1];
//  gl_M4_Move(&m[0],&m[-1]);

  gl_matrix_update(c);
}

void glopPopMatrix(GLContext *c,GLParam *p)
{
  int n=c->matrix_mode;

  assert( c->matrix_stack_ptr[n] > c->matrix_stack[n] );
  c->matrix_stack_ptr[n]--;
  gl_matrix_update(c);
}


void glopRotate(GLContext *c,GLParam *p)
{
	glm::mat4 m;
	float rot = p[1].f;
	m = glm::rotate(m, rot, glm::vec3(p[2].f,p[3].f,p[4].f));
	c->matrix_stack_ptr[c->matrix_mode][0] = m * c->matrix_stack_ptr[c->matrix_mode][0];
	gl_matrix_update(c);  
}

void glopScale(GLContext *c,GLParam *p)
{
	glm::mat4 m;
	m = glm::scale(m,glm::vec3(p[1].f,p[2].f,p[3].f));
	c->matrix_stack_ptr[c->matrix_mode][0] = m * c->matrix_stack_ptr[c->matrix_mode][0];
    gl_matrix_update(c);
}

void glopTranslate(GLContext *c,GLParam *p)
{
	glm::mat4 m;
	m = glm::translate(m, glm::vec3(p[1].f,p[2].f,p[3].f));
	m = glm::transpose(m); // <- wtfbbq
	c->matrix_stack_ptr[c->matrix_mode][0] = m * c->matrix_stack_ptr[c->matrix_mode][0];
	gl_matrix_update(c);  
}


void glopFrustum(GLContext *c,GLParam *p)
{
  float *r;
  glm::mat4 m;
  GLfloat left=p[1].f;
  GLfloat right=p[2].f;
  GLfloat bottom=p[3].f;
  GLfloat top=p[4].f;
  GLfloat near=p[5].f;
  GLfloat farp=p[6].f;
  GLfloat x,y,A,B,C,D,tmp2=(float)(2);

  x = ((tmp2*near)/ (right-left));
  y = ((tmp2*near)/ (top-bottom));
  A = ((right+left)/ (right-left));
  B = ((top+bottom)/ (top-bottom));
  C = (-((farp+near))/ (farp-near));
  D = (-(((tmp2*farp)*near))/ (farp-near));

  r=glm::value_ptr(m);
  r[0]= x; r[1]=(float)(0);  r[2]=A;            r[3]=(float)(0);
  r[4]= (float)(0); r[5]=y;  r[6]=B;            r[7]=(float)(0);
  r[8]= (float)(0); r[9]=(float)(0);  r[10]=C;           r[11]=D;
  r[12]=(float)(0); r[13]=(float)(0); r[14]=(float)(-1); r[15]=(float)(0);

  c->matrix_stack_ptr[c->matrix_mode][0] = c->matrix_stack_ptr[c->matrix_mode][0] * m;
//  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}
  
