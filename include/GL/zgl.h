#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <GL/gl.h>
#include <GL/zbuffer.h>
#include <GL/zfeatures.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG
/* #define NDEBUG */

enum {

#define ADD_OP(a,b,c) OP_ ## a ,

#include <GL/opinfo.h>

};

/* initially # of allocated GLVertexes (will grow when necessary) */
#define POLYGON_MAX_VERTEX 16

/* Max # of specular light pow buffers */
#ifndef MAX_SPECULAR_BUFFERS
#define MAX_SPECULAR_BUFFERS 8
#endif
/* # of entries in specular buffer */
#ifndef SPECULAR_BUFFER_SIZE
#define SPECULAR_BUFFER_SIZE 1024
#endif
/* specular buffer granularity */
#ifndef SPECULAR_BUFFER_RESOLUTION
#define SPECULAR_BUFFER_RESOLUTION 1024
#endif


#define MAX_MODELVIEW_STACK_DEPTH  32
#define MAX_PROJECTION_STACK_DEPTH 8
#define MAX_TEXTURE_STACK_DEPTH    8
#define MAX_NAME_STACK_DEPTH       64
#define MAX_TEXTURE_LEVELS         11
#define MAX_LIGHTS                 16

#define VERTEX_HASH_SIZE 1031

#define MAX_DISPLAY_LISTS 1024
#define OP_BUFFER_MAX_SIZE 512

#define TGL_OFFSET_FILL    0x1
#define TGL_OFFSET_LINE    0x2
#define TGL_OFFSET_POINT   0x4

typedef struct GLSpecBuf {
  int shininess_i;
  int last_used;
  GLfloat buf[SPECULAR_BUFFER_SIZE+1];
  struct GLSpecBuf *next;
} GLSpecBuf;

typedef struct GLLight {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 position;	
  glm::vec3 spot_direction;
  GLfloat spot_exponent;
  GLfloat spot_cutoff;
  GLfloat attenuation[3];
  /* precomputed values */
  GLfloat cos_spot_cutoff;
  glm::vec3 norm_spot_direction;
  glm::vec3 norm_position;
  /* we use a linked list to know which are the enabled lights */
  int enabled;
  struct GLLight *next,*prev;
} GLLight;

typedef struct GLMaterial {
  glm::vec4 emission;
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  GLfloat shininess;

  /* computed values */
  int shininess_i;
  int do_specular;  
} GLMaterial;


typedef struct GLViewport {
  int xmin,ymin,xsize,ysize;
  glm::vec3 scale;
  glm::vec3 trans;
  int updated;
} GLViewport;

typedef union {
  int op;
  GLfloat f;
  int i;
  unsigned int ui;
  void *p;
} GLParam;

typedef struct GLParamBuffer {
  GLParam ops[OP_BUFFER_MAX_SIZE];
  struct GLParamBuffer *next;
} GLParamBuffer;

typedef struct GLList {
  GLParamBuffer *first_op_buffer;
  /* TODO: extensions for an hash table or a better allocating scheme */
} GLList;

typedef struct GLVertex {
  int edge_flag;
  glm::vec3 normal;
  glm::vec4 coord;
  glm::vec4 tex_coord;
  glm::vec4 color;
  
  /* computed values */
  glm::vec4 ec;                /* eye coordinates */
  glm::vec4 pc;                /* coordinates in the normalized volume */
  int clip_code;        /* clip code */
  ZBufferPoint zp;      /* integer coordinates for the rasterization */
} GLVertex;

typedef struct GLImage {
  void *pixmap;
  int xsize,ysize;
} GLImage;

/* textures */

#define TEXTURE_HASH_TABLE_SIZE 256

typedef struct GLTexture {
  GLImage images[MAX_TEXTURE_LEVELS];
  int highest_miplevel;
  int handle;
  struct GLTexture *next,*prev;
} GLTexture;


/* shared state */

typedef struct GLSharedState {
  GLList **lists;
  GLTexture **texture_hash_table;
} GLSharedState;

struct GLContext;

typedef void (*gl_draw_triangle_func)(struct GLContext *c,
                                      GLVertex *p0,GLVertex *p1,GLVertex *p2);

/* display context */

typedef struct GLContext {
  /* Z buffer */
  ZBuffer *zb;

  /* lights */
  GLLight lights[MAX_LIGHTS];
  GLLight *first_light;
  glm::vec4 ambient_light_model;
  int local_light_model;
  int lighting_enabled;
  int light_model_two_side;

  /* materials */
  GLMaterial materials[2];
  int color_material_enabled;
  int current_color_material_mode;
  int current_color_material_type;

  /* textures */
  GLTexture *current_texture;
  int texture_2d_enabled;

  /* shared state */
  GLSharedState shared_state;

  /* current list */
  GLParamBuffer *current_op_buffer;
  int current_op_buffer_index;
  int exec_flag,compile_flag,print_flag;

  /* matrix */

  int matrix_mode;
  glm::mat4 *matrix_stack[3];
  glm::mat4 *matrix_stack_ptr[3];
  int matrix_stack_depth_max[3];

  glm::mat4 matrix_model_view_inv;
  glm::mat4 matrix_model_projection;
  int matrix_model_projection_updated;
  int matrix_model_projection_no_w_transform; 
  int apply_texture_matrix;

  /* viewport */
  GLViewport viewport;

  /* current state */
  int polygon_mode_back;
  int polygon_mode_front;

  int current_front_face;
  int current_shade_model;
  int current_cull_face;
  int cull_face_enabled;
  int normalize_enabled;
  gl_draw_triangle_func draw_triangle_front,draw_triangle_back;

  /* selection */
  int render_mode;
  unsigned int *select_buffer;
  int select_size;
  unsigned int *select_ptr,*select_hit;
  int select_overflow;
  int select_hits;

  /* names */
  unsigned int name_stack[MAX_NAME_STACK_DEPTH];
  int name_stack_size;

  /* clear */
  GLfloat clear_depth;
  glm::vec4 clear_color;

  /* current vertex state */
  glm::vec4 current_color;
  unsigned int longcurrent_color[3]; /* precomputed integer color */
  glm::vec4 current_normal;
  glm::vec4 current_tex_coord;
  int current_edge_flag;

  /* glBegin / glEnd */
  int in_begin;
  int begin_type;
  int vertex_n,vertex_cnt;
  int vertex_max;
  GLVertex *vertex;

  /* opengl 1.1 arrays  */
  GLfloat *vertex_array;
  int vertex_array_size;
  int vertex_array_stride;
  GLfloat *normal_array;
  int normal_array_stride;
  GLfloat *color_array;
  int color_array_size;
  int color_array_stride;
  GLfloat *texcoord_array;
  int texcoord_array_size;
  int texcoord_array_stride;
  int client_states;
  
  /* opengl 1.1 polygon offset */
  GLfloat offset_factor;
  GLfloat offset_units;
  int offset_states;
  
  /* specular buffer. could probably be shared between contexts, 
    but that wouldn't be 100% thread safe */
  GLSpecBuf *specbuf_first;
  int specbuf_used_counter;
  int specbuf_num_buffers;

  /* opaque structure for user's use */
  void *opaque;
  /* resize viewport function */
  int (*gl_resize_viewport)(struct GLContext *c,int *xsize,int *ysize);

  /* depth test */
  int depth_test;
} GLContext;

extern GLContext *gl_ctx;

void gl_add_op(GLParam *p);

/* clip.c */
void gl_transform_to_viewport(GLContext *c,GLVertex *v);
void gl_draw_triangle(GLContext *c,GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_line(GLContext *c,GLVertex *p0,GLVertex *p1);
void gl_draw_point(GLContext *c,GLVertex *p0);

void gl_draw_triangle_point(GLContext *c,
                            GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_triangle_line(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_triangle_fill(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_triangle_select(GLContext *c,
                             GLVertex *p0,GLVertex *p1,GLVertex *p2);

/* matrix.c */
void gl_print_matrix(const GLfloat *m);
/*
void glopLoadIdentity(GLContext *c,GLParam *p);
void glopTranslate(GLContext *c,GLParam *p);*/

/* light.c */
void gl_add_select(GLContext *c,unsigned int zmin,unsigned int zmax);
void gl_enable_disable_light(GLContext *c,int light,int v);
void gl_shade_vertex(GLContext *c,GLVertex *v);

void glInitTextures(GLContext *c);
void glEndTextures(GLContext *c);
GLTexture *alloc_texture(GLContext *c,int h);

/* image_util.c */
void gl_convertRGB_to_5R6G5B(unsigned short *pixmap,unsigned char *rgb,
                             int xsize,int ysize);
void gl_convertRGB_to_8A8R8G8B(unsigned int *pixmap, unsigned char *rgb,
                               int xsize, int ysize);
void gl_resizeImage(unsigned char *dest,int xsize_dest,int ysize_dest,
                    unsigned char *src,int xsize_src,int ysize_src);
void gl_resizeImageNoInterpolate(unsigned char *dest,int xsize_dest,int ysize_dest,
                                 unsigned char *src,int xsize_src,int ysize_src);

GLContext *gl_get_context(void);

void gl_fatal_error(char *format, ...);


/* specular buffer "api" */
GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, 
                              const GLfloat shininess);

#if defined(__BEOS__) || defined(_MSC_VER)
void dprintf(const char *, ...);

#else /* !BEOS */

#ifdef DEBUG

#define dprintf(format, args...)  \
  fprintf(stderr,"In '%s': " format "\n",__FUNCTION__, ##args);

#else

#define dprintf(format, args...)

#endif
#endif /* !BEOS */

/* glopXXX functions */

#define ADD_OP(a,b,c) void glop ## a (GLContext *,GLParam *);
#include <GL/opinfo.h>

/* this clip epsilon is needed to avoid some rounding errors after
   several clipping stages */


#define CLIP_EPSILON (1E-5)
#define CLIP_EPSILON1 (CLIP_EPSILON + 1.0)

#ifdef _MSC_VER
#define inline __inline
#endif

static inline int gl_clipcode(GLfloat x,GLfloat y,GLfloat z,GLfloat w1)
{
  GLfloat w;

  w=(w1* (float)(CLIP_EPSILON1));

#ifndef SLL_DEBUG
  return (x<-(w)) |
    ((x>w)<<1) |
    ((y<-(w))<<2) |
    ((y>w)<<3) |
    ((z<-(w))<<4) | 
    ((z>w)<<5) ;
#else
  	return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* _tgl_zgl_h_ */
