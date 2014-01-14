#ifndef _tgl_specbuf_h_
#define _tgl_specbuf_h_

#include <GL/gl.h>

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

typedef struct GLSpecBuf {
  int shininess_i;
  int last_used;
  GLfloat buf[SPECULAR_BUFFER_SIZE+1];
  struct GLSpecBuf *next;
} GLSpecBuf;

GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, 
                              const GLfloat shininess);
void specbuf_cleanup(GLContext *c); /* free all memory used */

#endif /* _tgl_specbuf_h_ */
