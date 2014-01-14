#include <stdlib.h>
#include <GL/gl.h>
#include <GL/zgl.h>
#include <GL/zbuffer.h>

#define ZCMP(z,zpix) ((z) >= (zpix))

void ZB_fillTriangleFlat(ZBuffer *zb,
			 ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
    int color;

#define INTERP_Z

#define DRAW_INIT()				\
{						\
  color=RGB_TO_PIXEL(p2->r,p2->g,p2->b);	\
}
  
#define PUT_PIXEL(_a)				\
{						\
    zz=z >> ZB_POINT_Z_FRAC_BITS;		\
    if (ZCMP(zz,pz[_a])) {				\
      pp[_a]=color;				\
      pz[_a]=zz;				\
    }						\
    z+=dzdx;					\
}

#include <GL/ztriangle.h>
}

/*
 * Smooth filled triangle.
 * The code below is very tricky :)
 */

void ZB_fillTriangleSmooth(ZBuffer *zb,
			   ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
        int _drgbdx;

#define INTERP_Z
#define INTERP_RGB

#define SAR_RND_TO_ZERO(v,n) (v / (1<<n))

#define DRAW_INIT() 				\
{						\
  _drgbdx=(SAR_RND_TO_ZERO(drdx,6) << 22) & 0xFFC00000;		\
  _drgbdx|=SAR_RND_TO_ZERO(dgdx,5) & 0x000007FF;		\
  _drgbdx|=(SAR_RND_TO_ZERO(dbdx,7) << 12) & 0x001FF000; 	\
}


#define PUT_PIXEL(_a)				\
{						\
    zz=z >> ZB_POINT_Z_FRAC_BITS;		\
    if (ZCMP(zz,pz[_a])) {				\
      tmp=rgb & 0xF81F07E0;			\
      pp[_a]=tmp | (tmp >> 16);			\
      pz[_a]=zz;				\
    }						\
    z+=dzdx;					\
    rgb=(rgb+drgbdx) & ( ~ 0x00200800);		\
}

#define DRAW_LINE()							   \
{									   \
  register unsigned short *pz;					   \
  register PIXEL *pp;					   \
  register unsigned int tmp,z,zz,rgb,drgbdx;				   \
  register int n;							   \
  n=(x2 >> 16) - x1;							   \
  pp=pp1+x1;								   \
  pz=pz1+x1;								   \
  z=z1;									   \
  rgb=(r1 << 16) & 0xFFC00000;						   \
  rgb|=(g1 >> 5) & 0x000007FF;						   \
  rgb|=(b1 << 5) & 0x001FF000;						   \
  drgbdx=_drgbdx;							   \
  while (n>=3) {							   \
    PUT_PIXEL(0);							   \
    PUT_PIXEL(1);							   \
    PUT_PIXEL(2);							   \
    PUT_PIXEL(3);							   \
    pz+=4;								   \
    pp+=4;								   \
    n-=4;								   \
  }									   \
  while (n>=0) {							   \
    PUT_PIXEL(0);							   \
    pz+=1;								   \
    pp+=1;								   \
    n-=1;								   \
  }									   \
}

#include <GL/ztriangle.h>
}

void ZB_setTexture(ZBuffer *zb,GLTexture *texture)
{
    zb->current_texture=texture;
}

void ZB_fillTriangleMapping(ZBuffer *zb,
			    ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
    PIXEL *texture;

#define INTERP_Z
#define INTERP_ST

#define DRAW_INIT()				\
{						\
  texture=(PIXEL*)zb->current_texture->images[0].pixmap;			\
}

#define PUT_PIXEL(_a)				\
{						\
   zz=z >> ZB_POINT_Z_FRAC_BITS;		\
     if (ZCMP(zz,pz[_a])) {				\
       /*pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];*/	\
       pz[_a]=zz;				\
    }						\
    z+=dzdx;					\
    s+=dsdx;					\
    t+=dtdx;					\
}


#include <GL/ztriangle.h>
}


__inline int find_mip(float s, float t, int cap)
{
	//return 0;

	int step = fabs(s);
	int it = fabs(t);

	// should we prefer a higher or a lower mip?
	if (step > it) step = it;

	int v = 0;
	// not optimal, but on the other hand, content should be optimized for
	// higher mip levels..
	while (step > 2) { step /= 2; v++; }

	if (v>cap) return cap;

	return v;
}

/*
 * Texture mapping with perspective correction.
 * We use the gradient method to make less divisions.
 * TODO: pipeline the division
 */

void ZB_fillTriangleMappingPerspective(ZBuffer *zb,
                            ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
    PIXEL *pixels;
	GLImage *texture;
    float fdzdx,fndzdx;
    float ndszdx,ndtzdx;
	int mip;
	float top_x = zb->current_texture->images[0].xsize;
	float top_y = zb->current_texture->images[0].ysize;
	int cap = zb->current_texture->highest_miplevel;

	mip = 0;

#define INTERP_Z
#define INTERP_STZ

#define NB_INTERP 8
#define SLL_NB_INTERP (float)(8)
#define SLLLL_NB_INTERP (int)(8)

#define DRAW_INIT()				\
{						\
  texture=&zb->current_texture->images[mip];\
  pixels=(PIXEL*)zb->current_texture->images[mip].pixmap;\
  fdzdx=(float)(dzdx);\
  fndzdx=(SLL_NB_INTERP* fdzdx);\
  ndszdx=(SLLLL_NB_INTERP* dszdx);\
  ndtzdx=(SLLLL_NB_INTERP* dtzdx);\
}


#define PUT_PIXEL(_a)				\
{						\
   zz=z >> ZB_POINT_Z_FRAC_BITS;		\
     if (ZCMP(zz,pz[_a])) {				\
	   int ss = (int)floor((s - floor(s)) * texture->xsize); \
	   int tt = (int)floor((t - floor(t)) * texture->ysize)  * texture->xsize; \
       pp[_a]=*(PIXEL *)(pixels+ss+tt); \
       pz[_a]=zz;				\
    }						\
    z+=dzdx;					\
    s+=dsdx;					\
    t+=dtdx;					\
}


#define DRAW_LINE()				\
{						\
  register unsigned short *pz;		\
  register PIXEL *pp;		\
  register unsigned int z,zz;	\
  float s, t; \
  register int n;		\
  float sz,tz, dsdx,dtdx; \
  GLfloat fz,zinv; \
  n=(x2>>16)-x1;                             \
  fz=(float)(z1);\
  zinv=((float)(1)/ fz);\
  pp=(PIXEL *)((char *)pp1 + x1 * PSZB); \
  pz=pz1+x1;					\
  z=z1;						\
  sz=sz1;\
  tz=tz1;\
  while (n>=(NB_INTERP-1)) {						   \
    {\
      s=(sz* (float)(zinv));\
      t=(tz* (float)(zinv));\
      dsdx= (((dszdx- (s*(float)(fdzdx)))* (float)(zinv)));\
      dtdx= (((dtzdx- (t*(float)(fdzdx)))* (float)(zinv)));\
	  mip = find_mip(dsdx*top_y,dtdx*top_x,cap); \
	  texture=&zb->current_texture->images[mip]; \
	  pixels=(PIXEL*)texture->pixmap;\
      fz=(fz+ fndzdx);\
      zinv=((float)(1)/ fz);\
    }\
    PUT_PIXEL(0);							   \
    PUT_PIXEL(1);							   \
    PUT_PIXEL(2);							   \
    PUT_PIXEL(3);							   \
    PUT_PIXEL(4);							   \
    PUT_PIXEL(5);							   \
    PUT_PIXEL(6);							   \
    PUT_PIXEL(7);							   \
    pz+=NB_INTERP;							   \
    pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);\
    n-=NB_INTERP;							   \
    sz=(sz+ ndszdx);\
    tz=(tz+ ndtzdx);\
  }									   \
    {\
      s=(sz* (float)(zinv));\
      t=(tz* (float)(zinv));\
      dsdx= (((dszdx- (s*(float)(fdzdx)))* (float)(zinv)));\
      dtdx= (((dtzdx- (t*(float)(fdzdx)))* (float)(zinv)));\
    }\
  while (n>=0) {							   \
    PUT_PIXEL(0);							   \
    pz+=1;								   \
    pp=(PIXEL *)((char *)pp + PSZB);\
    n-=1;								   \
  }									   \
}
  
#include <GL/ztriangle.h>
}

