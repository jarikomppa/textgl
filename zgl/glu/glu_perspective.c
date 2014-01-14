#include <math.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define M_PI           3.14159265358979323846f  /* pi */

void gluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = (zNear* (float)tan( (fovy* (M_PI/360.0)) ));
   ymin = -(ymax);

   xmin = (ymin* aspect);
   xmax = (ymax* aspect);

   glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

