
#ifndef TEDDY__TINY_GL__TFXSWGL_H
#define TEDDY__TINY_GL__TFXSWGL_H



#include <GL/gl.h>
#include <GL/zgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLContext   *gl_context;
	int *surface;
} tfx_swgl_Context;


extern tfx_swgl_Context *tfx_swgl_CreateContext ();
extern void              tfx_swgl_DestroyContext( tfx_swgl_Context *ctx );
extern int               tfx_swgl_MakeCurrent   ( int *surface, tfx_swgl_Context *ctx );
extern void              tfx_swgl_SwapBuffers   ();


#ifdef __cplusplus
}
#endif


#endif  /*  TEDDY_TINYGL_TFXSWGL_H  */


