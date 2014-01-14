/*
 * Texture Manager
 */

#include <GL/zgl.h>

static GLTexture *find_texture(GLContext *c,int h)
{
  GLTexture *t;

  t=c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
  while (t!=NULL) {
    if (t->handle == h) return t;
    t=t->next;
  }
  return NULL;
}



void *glGetTexdataPtr(int target, int texture, int level)
{
	GLContext *c = gl_get_context();
	GLTexture * t = find_texture(c, texture);
	if (t)
		return t->images[level].pixmap;
	return NULL;
}

int glGetTexdataLevels(int target, int texture)
{
	GLContext *c = gl_get_context();
	GLTexture * t = find_texture(c, texture);
	if (t)
		return t->highest_miplevel;
	return 0;
}

void glGetTexdataInfo(int target, int texture, int level, int *width, int *height)
{
	GLContext *c = gl_get_context();
	GLTexture * t = find_texture(c, texture);
	*width = 0;
	*height = 0;
	if (!t)
		return;
	*width = t->images[level].xsize;
	*height = t->images[level].xsize;
}


static void free_texture(GLContext *c,int h)
{
  GLTexture *t,**ht;
  GLImage *im;
  int i;

  t=find_texture(c,h);
  if (t->prev==NULL) {
    ht=&c->shared_state.texture_hash_table
      [t->handle % TEXTURE_HASH_TABLE_SIZE];
    *ht=t->next;
  } else {
    t->prev->next=t->next;
  }
  if (t->next!=NULL) t->next->prev=t->prev;

  for(i=0;i<MAX_TEXTURE_LEVELS;i++) {
    im=&t->images[i];
    if (im->pixmap != NULL) gl_free(im->pixmap);
  }

  gl_free(t);
}

GLTexture *alloc_texture(GLContext *c,int h)
{
  GLTexture *t,**ht;
  
  t=(GLTexture *)gl_zalloc(sizeof(GLTexture));

  ht=&c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

  t->next=*ht;
  t->prev=NULL;
  if (t->next != NULL) t->next->prev=t;
  *ht=t;

  t->handle=h;
  
  return t;
}


void glInitTextures(GLContext *c)
{
  /* textures */

  c->texture_2d_enabled=0;
  c->current_texture=find_texture(c,0);
}

void glGenTextures(int n, unsigned int *textures)
{
  GLContext *c=gl_get_context();
  int max,i;
  GLTexture *t;

  max=0;
  for(i=0;i<TEXTURE_HASH_TABLE_SIZE;i++) {
    t=c->shared_state.texture_hash_table[i];
    while (t!=NULL) {
      if (t->handle>max) max=t->handle;
      t=t->next;
    }

  }
  for(i=0;i<n;i++) {
    textures[i]=max+i+1;
  }
}


void glDeleteTextures(int n, const unsigned int *textures)
{
  GLContext *c=gl_get_context();
  int i;
  GLTexture *t;

  for(i=0;i<n;i++) {
    t=find_texture(c,textures[i]);
    if (t!=NULL && t!=0) {
      if (t==c->current_texture) {
	glBindTexture(GL_TEXTURE_2D,0);
      }
      free_texture(c,textures[i]);
    }
  }
}


void glopBindTexture(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int texture=p[2].i;
  GLTexture *t;

  assert(target == GL_TEXTURE_2D && texture >= 0);

  t=find_texture(c,texture);
  if (t==NULL) {
    t=alloc_texture(c,texture);
  }
  c->current_texture=t;
}

void glopTexImage2D(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int level=p[2].i;
  int components=p[3].i;
  int width=p[4].i;
  int height=p[5].i;
  int border=p[6].i;
  int format=p[7].i;
  int type=p[8].i;
  void *pixels=p[9].p;
  GLImage *im;
  unsigned char *pixels1;

  if (!(target == GL_TEXTURE_2D && 
//	  level == 0 && 
	  (components == 3 || components == GL_RGBA) && 
      border == 0 && 
	  (format == GL_RGB || format == GL_RGBA) &&
      type == GL_UNSIGNED_BYTE)) 
  {
    gl_fatal_error("glTexImage2D: combinaison of parameters not handled");
  }
  
  pixels1=(unsigned char*)pixels;
  if (c->current_texture->highest_miplevel < level)
	  c->current_texture->highest_miplevel = level;
  im=&c->current_texture->images[level];
  im->xsize=width;
  im->ysize=height;
  if (im->pixmap!=NULL) gl_free(im->pixmap);
  im->pixmap=gl_malloc(width*height*4);
  if(im->pixmap && pixels1) 
  {
	  if (components == 3)
	  {
		  int i;
		  for (i = 0; i < width*height; i++)
			  ((unsigned int*)im->pixmap)[i] = 
			  (pixels1[i*3+0] << 16) |
			  (pixels1[i*3+1] << 8) |
			  (pixels1[i*3+2] << 0);
	  }
	  else
	  {
		  memcpy(im->pixmap,pixels1,width*height*4);
	  }
  }
}


/* TODO: not all tests are done */
void glopTexEnv(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int pname=p[2].i;
  int param=p[3].i;

  if (target != GL_TEXTURE_ENV) {
  error:
    gl_fatal_error("glTexParameter: unsupported option");
  }

  if (pname != GL_TEXTURE_ENV_MODE) goto error;

  if (param != GL_DECAL) goto error;
}

/* TODO: not all tests are done */
void glopTexParameter(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int pname=p[2].i;
  int param=p[3].i;
  
  if (target != GL_TEXTURE_2D) {
  error:
    gl_fatal_error("glTexParameter: unsupported option");
  }

  switch(pname) {
  case GL_TEXTURE_WRAP_S:
  case GL_TEXTURE_WRAP_T:
    if (param != GL_REPEAT) goto error;
    break;
  }
}

void glopPixelStore(GLContext *c,GLParam *p)
{
  int pname=p[1].i;
  int param=p[2].i;

  if (pname != GL_UNPACK_ALIGNMENT ||
      param != 1) {
    gl_fatal_error("glPixelStore: unsupported option");
  }
}
