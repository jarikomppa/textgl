#include <stdarg.h>
#include <GL/zgl.h>

void gl_fatal_error(char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  fprintf(stderr,"TinyGL: fatal error: ");
  vfprintf(stderr,format,ap);
  fprintf(stderr,"\n");
  __asm int 3;
  exit(1);

  va_end(ap);
}
