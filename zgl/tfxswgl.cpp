
#include <GL/tfxswgl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*  Prototype  */
static int tfx_swgl_resize_viewport( GLContext *c, int *xsize_ptr, int *ysize_ptr );


/*  Create context  */
tfx_swgl_Context *tfx_swgl_CreateContext(){
	tfx_swgl_Context *ctx;

	ctx = (tfx_swgl_Context*)malloc( sizeof(tfx_swgl_Context) );
	if( ctx == NULL ){
		return NULL;
	}
	ctx->gl_context = NULL;
	return ctx;
}


/*!  Destroy context  */
void tfx_swgl_DestroyContext( tfx_swgl_Context *ctx ){
	if( ctx->gl_context != NULL ){
		glClose();
	}
	free( ctx );
}


/*!  Connect surface to context  */
int tfx_swgl_MakeCurrent( int *surface, tfx_swgl_Context *ctx ){
	int	mode;
	int	xsize;
	int	ysize;
	int	n_colors = 0;
	ZBuffer *zb;

	if( ctx->gl_context == NULL ){
		/* create the TinyGL context */

		xsize = 160;
		ysize = 100;
		mode = ZB_MODE_RGBA;
		zb = ZB_open( xsize, ysize, mode, n_colors, NULL, NULL, NULL);
		
		if( zb == NULL )
		{
			return 0;
		}
		
		/* initialisation of the TinyGL interpreter */
		glInit( zb );
		ctx->gl_context                     = gl_get_context();
		ctx->gl_context->opaque             = (void *) ctx;
		ctx->gl_context->gl_resize_viewport = tfx_swgl_resize_viewport;

		/* set the viewport */
		/*  TIS: !!! HERE SHOULD BE -1 on both to force reshape  */
		/*  which is needed to make sure initial reshape is  */
		/*  called, otherwise it is not called..  */
		ctx->gl_context->viewport.xsize = xsize;
		ctx->gl_context->viewport.ysize = ysize;
		glViewport( 0, 0, xsize, ysize );
	}
	ctx->surface = surface;
	return 1;
}


/*!  Swap buffers  */     
void tfx_swgl_SwapBuffers(){
	GLContext        *gl_context;
	tfx_swgl_Context *ctx;

    
    /* retrieve the current sdl_swgl_Context */
    gl_context = gl_get_context();
    ctx = (tfx_swgl_Context *)gl_context->opaque;

	ZB_copyFrameBuffer(ctx->gl_context->zb, ctx->surface, 160*2);

	// note: does not actually output - let's leave that to the app
}

void *glGetFramebufferPtr()
{
	GLContext        *gl_context;
	tfx_swgl_Context *ctx;
    gl_context = gl_get_context();
    ctx = (tfx_swgl_Context *)gl_context->opaque;
	return ctx->gl_context->zb->pbuf;
}


/*!  Resize context  */
static int tfx_swgl_resize_viewport( GLContext *c, int *xsize_ptr, int *ysize_ptr ){
	tfx_swgl_Context *ctx;
	int               xsize;
	int               ysize;
  
	ctx = (tfx_swgl_Context *)c->opaque;

	xsize = *xsize_ptr;
	ysize = *ysize_ptr;

	/* we ensure that xsize and ysize are multiples of 2 for the zbuffer. 
	   TODO: find a better solution */
	xsize &= ~3;
	ysize &= ~3;

	if (xsize == 0 || ysize == 0) return -1;

	*xsize_ptr = xsize;
	*ysize_ptr = ysize;

	/* resize the Z buffer */
	ZB_resize( c->zb, ctx->surface, xsize, ysize );
	return 0;
}

