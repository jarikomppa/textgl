#include <GL/zgl.h>

GLContext *gl_ctx;


void initSharedState(GLContext *c)
{
  GLSharedState *s=&c->shared_state;
  s->lists=(GLList **)gl_zalloc(sizeof(GLList *) * MAX_DISPLAY_LISTS);
  s->texture_hash_table=(GLTexture **)
      gl_zalloc(sizeof(GLTexture *) * TEXTURE_HASH_TABLE_SIZE);

  alloc_texture(c,0);
}

void endSharedState(GLContext *c)
{
  GLSharedState *s=&c->shared_state;
  int i;

  for(i=0;i<MAX_DISPLAY_LISTS;i++) {
    /* TODO */
  }
  gl_free(s->lists);

  gl_free(s->texture_hash_table);
}


void glInit(void *zbuffer1)
{
  ZBuffer *zbuffer=(ZBuffer *)zbuffer1;
  GLContext *c;
  GLViewport *v;
  int i;

  c=(GLContext *)gl_zalloc(sizeof(GLContext));
  gl_ctx=c;

  c->zb=zbuffer;

  /* allocate GLVertex array */
  c->vertex_max = POLYGON_MAX_VERTEX;
  c->vertex =(GLVertex *)gl_malloc(POLYGON_MAX_VERTEX*sizeof(GLVertex));
  
  /* viewport */
  v=&c->viewport;
  v->xmin=0;
  v->ymin=0;
  v->xsize=zbuffer->xsize;
  v->ysize=zbuffer->ysize;
  v->updated=1;

  /* shared state */
  initSharedState(c);

  /* lists */

  c->exec_flag=1;
  c->compile_flag=0;
  c->print_flag=0;

  c->in_begin=0;

  /* lights */
  for(i=0;i<MAX_LIGHTS;i++) {
    GLLight *l=&c->lights[i];
    l->ambient=glm::vec4((float)(0),(float)(0),(float)(0),(float)(1));
    l->diffuse=glm::vec4((float)(1),(float)(1),(float)(1),(float)(1));
    l->specular=glm::vec4((float)(1),(float)(1),(float)(1),(float)(1));
    l->position=glm::vec4((float)(0),(float)(0),(float)(1),(float)(0));
    l->norm_position=glm::vec3((float)(0),(float)(0),(float)(1));
    l->spot_direction=glm::vec3((float)(0),(float)(0),(float)(-1));
    l->norm_spot_direction=glm::vec3((float)(0),(float)(0),(float)(-1));
    l->spot_exponent=(float)(0);
    l->spot_cutoff=(float)(180);
    l->attenuation[0]=(float)(1);
    l->attenuation[1]=(float)(0);
    l->attenuation[2]=(float)(0);
    l->enabled=0;
  }
  c->first_light=NULL;
  c->ambient_light_model=glm::vec4((0.2f),(0.2f),(0.2f),(float)(1));
  c->local_light_model=0;
  c->lighting_enabled=0;
  c->light_model_two_side = 0;

  /* default materials */
  for(i=0;i<2;i++) {
    GLMaterial *m=&c->materials[i];
    m->emission=glm::vec4((float)(0),(float)(0),(float)(0),(float)(1));
    m->ambient=glm::vec4((0.2f),(0.2f),(0.2f),(float)(1));
    m->diffuse=glm::vec4((0.8f),(0.8f),(0.8f),(float)(1));
    m->specular=glm::vec4((float)(0),(float)(0),(float)(0),(float)(1));
    m->shininess=(float)(0);
  }
  c->current_color_material_mode=GL_FRONT_AND_BACK;
  c->current_color_material_type=GL_AMBIENT_AND_DIFFUSE;
  c->color_material_enabled=0;

  /* textures */
  glInitTextures(c);

  /* default state */
  c->current_color.x=(float)(1);
  c->current_color.y=(float)(1);
  c->current_color.z=(float)(1);
  c->current_color.w=(float)(1);
  c->longcurrent_color[0] = 65535;
  c->longcurrent_color[1] = 65535;
  c->longcurrent_color[2] = 65535;

  c->current_normal.x=(float)(1);
  c->current_normal.y=(float)(0);
  c->current_normal.z=(float)(0);
  c->current_normal.w=(float)(0);

  c->current_edge_flag=1;
  
  c->current_tex_coord.x=(float)(0);
  c->current_tex_coord.y=(float)(0);
  c->current_tex_coord.z=(float)(0);
  c->current_tex_coord.w=(float)(1);

  c->polygon_mode_front=GL_FILL;
  c->polygon_mode_back=GL_FILL;

  c->current_front_face=0; /* 0 = GL_CCW  1 = GL_CW */
  c->current_cull_face=GL_BACK;
  c->current_shade_model=GL_SMOOTH;
  c->cull_face_enabled=0;
  
  /* clear */
  c->clear_color.r=(float)(0);
  c->clear_color.g=(float)(0);
  c->clear_color.b=(float)(0);
  c->clear_color.a=(float)(0);
  c->clear_depth=(float)(0);

  /* selection */
  c->render_mode=GL_RENDER;
  c->select_buffer=NULL;
  c->name_stack_size=0;

  /* matrix */
  c->matrix_mode=0;
  
  c->matrix_stack_depth_max[0]=MAX_MODELVIEW_STACK_DEPTH;
  c->matrix_stack_depth_max[1]=MAX_PROJECTION_STACK_DEPTH;
  c->matrix_stack_depth_max[2]=MAX_TEXTURE_STACK_DEPTH;

  for(i=0;i<3;i++) {
    c->matrix_stack[i]=(glm::mat4 *)gl_zalloc(c->matrix_stack_depth_max[i] * sizeof(glm::mat4));
    c->matrix_stack_ptr[i]=c->matrix_stack[i];
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  c->matrix_model_projection_updated=1;

  /* opengl 1.1 arrays */
  c->client_states = 0;
  
  /* opengl 1.1 polygon offset */
  c->offset_states = 0;
  
  /* clear the resize callback function pointer */
  c->gl_resize_viewport = NULL;
  
  /* specular buffer */
  c->specbuf_first = NULL;
  c->specbuf_used_counter = 0;
  c->specbuf_num_buffers = 0;

  /* depth test */
  c->depth_test = 0;
}

void glClose(void)
{
  GLContext *c=gl_get_context();
  endSharedState(c);
  gl_free(c);
}
