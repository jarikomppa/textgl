///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// Example of the TextGL library
// Copyright (c) 2014 Jari Komppa
//
// TextGL is based on TinyGL (C) 1997-1998 Fabrice Bellard
//
// Both TextGL and TinyGL are under the same ZLIB license as this source:
//
///////////////////////////////////////////////
// License
///////////////////////////////////////////////
// 
//     This software is provided 'as-is', without any express or implied
//     warranty.    In no event will the authors be held liable for any damages
//     arising from the use of this software.
// 
//     Permission is granted to anyone to use this software for any purpose,
//     including commercial applications, and to alter it and redistribute it
//     freely, subject to the following restrictions:
// 
//     1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would be
//        appreciated but is not required.
//     2. Altered source versions must be plainly marked as such, and must not be
//        misrepresented as being the original software.
//     3. This notice may not be removed or altered from any source distribution.
// 
// (eg. same as ZLIB license)
// 
//
///////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include "textfx.h"
#include "conio.h"

#include "gl/gl.h"
#include "gl/glu.h"
#include "gl/tfxswgl.h"
#include "stb_image.h"

GLuint texture[3];
int * fb;

void loadtexture()
{
	int i, j;
	glGenTextures(3, &texture[0]);

	// Load texture using stb
	int x, y, n;
	unsigned char *data = stbi_load("tex.png", &x, &y, &n, 4);

	// in case someone runs the exe from the release dir..
	if (data == NULL)
		data = stbi_load("../tex.png", &x, &y, &n, 4);

	if (data == NULL)
		return;

	int l, w, h;
	w = x;
	h = y;
	l = 0;
	unsigned int * mip = new unsigned int[w * h * 5];
	unsigned int * src = (unsigned int*)data;

	memset(mip, 0, w * h * 4);

	// mark all pixels with alpha = 0 to black
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			if ((src[i * w + j] & 0xff000000) == 0)
				src[i * w + j] = 0;
		}
	}
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // NULL should be ok, just allocate
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // NULL should be ok, just allocate

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Tell OpenGL to read the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)src);

	if (mip)
	{
		// precalculate summed area tables
		// it's a box filter, which isn't very good, but at least it's fast =)
		int ra = 0, ga = 0, ba = 0, aa = 0;
		int i, j, c;
		unsigned int * rbuf = mip + (w * h * 1);
		unsigned int * gbuf = mip + (w * h * 2);
		unsigned int * bbuf = mip + (w * h * 3);
		unsigned int * abuf = mip + (w * h * 4);

		for (j = 0, c = 0; j < h; j++)
		{
			ra = ga = ba = aa = 0;
			for (i = 0; i < w; i++, c++)
			{
				ra += (src[c] >>  0) & 0xff;
				ga += (src[c] >>  8) & 0xff;
				ba += (src[c] >> 16) & 0xff;
				aa += (src[c] >> 24) & 0xff;
				if (j == 0)
				{
					rbuf[c] = ra;
					gbuf[c] = ga;
					bbuf[c] = ba;
					abuf[c] = aa;
				}
				else
				{
					rbuf[c] = ra + rbuf[c - w];
					gbuf[c] = ga + gbuf[c - w];
					bbuf[c] = ba + bbuf[c - w];
					abuf[c] = aa + abuf[c - w];
				}
			}
		}

		while (w > 1 || h > 1)
		{
			l++;
			w /= 2;
			h /= 2;
			if (w == 0) w = 1;
			if (h == 0) h = 1;

			int dw = x / w;
			int dh = y / h;

			for (j = 0, c = 0; j < h; j++)
			{
				for (i = 0; i < w; i++, c++)
				{
					int x1 = i * dw;
					int y1 = j * dh;
					int x2 = x1 + dw - 1;
					int y2 = y1 + dh - 1;
					int div = (x2 - x1) * (y2 - y1);
					y1 *= x;
					y2 *= x;
					int r = rbuf[y2 + x2] - rbuf[y1 + x2] - rbuf[y2 + x1] + rbuf[y1 + x1];
					int g = gbuf[y2 + x2] - gbuf[y1 + x2] - gbuf[y2 + x1] + gbuf[y1 + x1];
					int b = bbuf[y2 + x2] - bbuf[y1 + x2] - bbuf[y2 + x1] + bbuf[y1 + x1];
					int a = abuf[y2 + x2] - abuf[y1 + x2] - abuf[y2 + x1] + abuf[y1 + x1];

					r /= div;
					g /= div;
					b /= div;
					a /= div;

					if (a == 0)
						mip[c] = 0;
					else
						mip[c] = ((r & 0xff) <<  0) | 
						((g & 0xff) <<  8) | 
						((b & 0xff) << 16) | 
						((a & 0xff) << 24); 
				}
			}
			glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)mip);
		}
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
		delete[] mip;
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	}

	// and cleanup.
	stbi_image_free(data);
}



void init()
{
	loadtexture();
	glEnable( GL_TEXTURE_2D );
	glClearColor(0,0,0,0);
	glClearDepth(1);
	glEnable( GL_DEPTH_TEST );

	glViewport(0, 0, 160, 100);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1f, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void draw(int tick)
{

	// Set up some rotation values..
	float xrot = tick * 1.6f * 0.051f;
	float yrot = tick * 1.4f * 0.051f;
	float zrot = tick * 1.8f * 0.051f;
	float zoom = (float)sin(tick * 0.001f) * 2 - 5;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glLoadIdentity();

	glBindTexture( GL_TEXTURE_2D, texture[0] );

	// Render one quad in the background

	glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex3f(-1,-1, -2.3f);
		glTexCoord2f(1, 1); glVertex3f( 1,-1, -2.3f);
		glTexCoord2f(1, 0); glVertex3f( 1, 1, -2.3f);
		glTexCoord2f(0, 0); glVertex3f(-1, 1, -2.3f);
	glEnd();

	// clear zbuffer so that the above doesn't clip with the rest
	glClear( GL_DEPTH_BUFFER_BIT );

	glTranslatef(0, 0, zoom);

	glRotatef(xrot * 0.5f, 0, 0, 1); 
	glRotatef(yrot * 0.5f, 0, 1, 0); 
	glRotatef(zrot * 0.5f, 1, 0, 0); 

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// render the whacky extra quad
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex3f(-2,-2, 0);
		glTexCoord2f(1, 1); glVertex3f( 2,-2, 0);
		glTexCoord2f(1, 0); glVertex3f( 2, 2, 0);
		glTexCoord2f(0, 0); glVertex3f(-2, 2, 0);
	glEnd();

	glLoadIdentity();
	glTranslatef(0, 0, zoom);

	glRotatef(xrot, 1, 0, 0); 
	glRotatef(yrot, 0, 1, 0); 
	glRotatef(zrot, 0, 0, 1); 

	// finally, render our cube, with different textures in some faces
	glBegin(GL_QUADS);
		// Front 
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexCoord2f(0,1); glVertex3f(-1,-1,1);
		glTexCoord2f(1,1); glVertex3f(1,-1,1);
		glTexCoord2f(1,0); glVertex3f(1,1,1);
		glTexCoord2f(0,0); glVertex3f(-1,1,1);

		// Back 
		glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
		glTexCoord2f(0,1); glVertex3f(-1,1,-1);
		glTexCoord2f(1,1); glVertex3f(1,1,-1);
		glTexCoord2f(1,0); glVertex3f(1,-1,-1);

		// Top 
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexCoord2f(1,1); glVertex3f(-1, 1,-1);
		glTexCoord2f(1,0); glVertex3f(-1, 1, 1);
		glTexCoord2f(0,0); glVertex3f( 1, 1, 1);
		glTexCoord2f(0,1); glVertex3f( 1, 1,-1);

		// Bottom 
		glTexCoord2f(0,1); glVertex3f(-1,-1,-1);
		glTexCoord2f(1,1); glVertex3f( 1,-1,-1);
		glTexCoord2f(1,0); glVertex3f( 1,-1, 1);
		glTexCoord2f(0,0); glVertex3f(-1,-1, 1);

		// Right
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexCoord2f(0,0); glVertex3f(1,-1,-1);
		glTexCoord2f(0,1); glVertex3f(1, 1,-1);
		glTexCoord2f(1,1); glVertex3f(1, 1, 1);
		glTexCoord2f(1,0); glVertex3f(1,-1, 1);

		// Left
		glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
		glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
		glTexCoord2f(0,1); glVertex3f(-1, 1, 1);
		glTexCoord2f(1,1); glVertex3f(-1, 1,-1);
	glEnd();

	// Render and copy the result to bitmap framebuffer
	tfx_swgl_SwapBuffers();
}


void moirepattern(int * ptr, int w, int h, int tick)
{
	int i, j, c;

	for (i = 0, c = 0; i < h; i++)
		for (j = 0; j < w; j++, c++)
		{
			int x = j-w/2;
			int y = i-w/2;
			ptr[c] = (y*x + tick*10) << 4;
		}
}

void framebuffercopy(int * ptr, int w, int h, int tick)
{
	int i, j, c;

	for (i = 0, c = 0; i < h; i++)
		for (j = 0; j < w; j++, c++)
		{
			ptr[c] = fb[j*160/256+(i*100/256)*160];
		}
}

void render(int tick)
{
	// Do note that glGetTexdataPtr is DEFINITELY not standard gl, but something I
	// whipped together for TextGL for demo use

	// Fill one texture with animated moire pattern
	moirepattern((int*)glGetTexdataPtr(GL_TEXTURE_2D, texture[1],0), 256, 256, tick);
	// Fill one texture with a copy of the framebuffer (i.e, the previous frame)
	framebuffercopy((int*)glGetTexdataPtr(GL_TEXTURE_2D, texture[2],0), 256, 256, tick);
	// Draw the 3d stuff
	draw(tick);

	// we can also render directly to the bitmap framebuffer if we want. 
	// Let's have a blinking square in the top corner.
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			fb[i*160+j] = ((tick >> 9) & 1) ? 0xffffffff:0;
}

void demomain()
{
	// The bitmap-ascii art converter object
	TFX_AsciiArt caa;

	TFX_SetTitle("Wait, precalculating..");
	caa.BuildLUT(); // You know, when I originally wrote TextFX, this step took SECONDS

	// Allocate our bitmap framebuffer (32 bit, 4x ascii resolution)
	fb=new int[160*100];

	// Set up GL context that points at the above framebuffer
	tfx_swgl_Context *ctx = tfx_swgl_CreateContext();
	tfx_swgl_MakeCurrent(fb, ctx);

	// initialize our demo (gl setup, load texture, etc)
	init();

	// Change console title
	TFX_SetTitle("TextGL example");

	unsigned int starttick = GetTickCount();
	int key = 0;

	while(key != 0x1b) // while not esc
	{
		// read key if one is pending. _getch() alone would block.
		if (_kbhit()) key = _getch();

		// tick for the frame. Without -starttick we'd get some wacky floating point
		// values down the line - we need to keep tick values relatively small.
		unsigned int tick = GetTickCount() - starttick;

		// Perform render
		render(tick);

		//////
		////// Everything above this line deals with bitmap framebuffer
		//////

		// Use TextFX7 to convert bitmap framebuffer to ascii
		caa.Dump2x(fb, TFXQuad(0, 0, 160, 100), 160, 0, 0);
		
		//////
		////// Everything below this line deals with text mode framebuffer
		//////

		// We can mess around with the text mode framebuffer (TFX_FrameBuffer) here.
		// The buffer is 80x50 shorts, where each "pixel" is a short, with 8 bits
		// of character data and 8 bits of color data, where low 4 bits are foreground
		// and 4 high bits are background color.

		// Replace the color attribute of a span of rows:
		int i, j;
		for (i = 0; i < 80; i++)
			for (j = 20; j < 30; j++)
				TFX_FrameBuffer[j*80+i] = (TFX_FrameBuffer[j*80+i] & 0xff) | (4 << 8);

		// Draw some text at the bottom, and make it cOLoRFul:
		char sometext[15] = "TextGL example";
		for (i = 0; i < 14; i++)
			TFX_FrameBuffer[50*80-14+i] = sometext[i] | (((i + (tick >> 4)) & 0xf) << 8);

		// dump the text mode framebuffer to the console
		TFX_Present();
	}
}
