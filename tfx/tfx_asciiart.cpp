///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// TextFX7
// Copyright (c) 1995-2009 Jari Komppa
//
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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "textfx.h"

TFX_AsciiArt::TFX_AsciiArt()
{
    mAsciiMap = NULL;
    mActualColors = NULL;
    mErrorMap = NULL;
}

TFX_AsciiArt::~TFX_AsciiArt()
{
    delete[] mAsciiMap;
    delete[] mActualColors;
    delete[] mErrorMap;
}


void TFX_AsciiArt::BuildLUT()
{
    int startchar = 32;
    int endchar = 126;
    int a, b, c, d, e, f, g, h, i;
    int * reg;
    reg = new int[256 * 4]; // allocate memory for font analyse data
    unsigned char * pchar = (unsigned char *)TFX_AsciiFontdata;
    mAsciiMap = new unsigned char[16 * 16 * 16 * 16]; // allocate the look-up table
    mActualColors = new unsigned char[16 * 16 * 16 * 16 * 4];
    mErrorMap = new signed char[160 * 100 * 4];
    memset(mErrorMap, 0, 160 * 100 * 4);

    // Step one: analyse font

    int chars;
    for (chars = startchar; chars <= endchar; chars++)
    {
        int charoffset = chars * 12;
        int i, j, c;
        int scratch[8 * 12];
        for (i = 0, c = 0; i < 12; i++)
        {
            for (j = 0; j < 8; j++, c++)
            {
#define IS_BIT(a,b) (((a) & (1 << (b))) ? 1 : 0)
                int cc = IS_BIT(*(pchar + i + charoffset), j) ? 255 : 0;
                scratch[i * 8 + j] = cc;
            }
        }

        int blurred[12 * 8];

        for (i = 0, c = 0; i < 12; i++)
        {
            for (j = 0; j < 8; j++, c++)
            {
                blurred[c] =
                    (
                    scratch[((i + 0) % 12) * 8 + ((j + 0) % 8)] * 4 +
                    scratch[((i + 1) % 12) * 8 + ((j + 0) % 8)] * 2 * ((i == 11) ? 0 : 1) +
                    scratch[((i + 11) % 12) * 8 + ((j + 0) % 8)] * 2 * ((i == 0) ? 0 : 1) +
                    scratch[((i + 0) % 12) * 8 + ((j + 1) % 8)] * 2 * ((j == 7) ? 0 : 1) +
                    scratch[((i + 1) % 12) * 8 + ((j + 1) % 8)] * ((i == 11 || j == 7) ? 0 : 1) +
                    scratch[((i + 11) % 12) * 8 + ((j + 1) % 8)] * ((i == 0 || j == 7) ? 0 : 1) +
                    scratch[((i + 0) % 12) * 8 + ((j + 7) % 8)] * 2 * ((j == 0) ? 0 : 1) +
                    scratch[((i + 1) % 12) * 8 + ((j + 7) % 8)] * ((i == 11 || j == 0) ? 0 : 1) +
                    scratch[((i + 11) % 12) * 8 + ((j + 7) % 8)] * ((i == 0 || j == 0) ? 0 : 1)
                    ) / 8;
            }
        }

        e = 0;
        for (b = 0; b < 6; b++)
            for (c = 0; c < 4; c++)
                e += blurred[b * 8 + c];
        *(reg + chars * 4 + 0) = e;
        e = 0;
        for (b = 0; b < 6; b++)
            for (c = 0; c < 4; c++)
                e += blurred[b * 8 + c + 4];
        *(reg + chars * 4 + 1) = e;
        e = 0;
        for (b = 0; b < 6; b++)
            for (c = 0; c < 4; c++)
                e += blurred[b * 8 + c + 6 * 8];
        *(reg + chars * 4 + 2) = e;
        e = 0;
        for (b = 0; b < 6; b++)
            for (c = 0; c < 4; c++)
                e += blurred[b * 8 + c + 6 * 8 + 4];
        *(reg + chars * 4 + 3) = e;
    }

    // Step two: build lookup table with the good old nearest-value mess
    for (a = 0; a < 16; a++)
    {
        for (b = 0; b < 16; b++)
        {
            for (c = 0; c < 16; c++)
            {
                for (d = 0; d < 16; d++)
                {
                    f = 0x7fffffff;
                    g = 0;
                    int a1, b1, c1, d1;
                    for (h = startchar;h < (endchar + 1);h++)
                    {
                        i = SQUARE(*(reg + h * 4 + 0) - a * 16 * 24) +
                            SQUARE(*(reg + h * 4 + 1) - b * 16 * 24) +
                            SQUARE(*(reg + h * 4 + 2) - c * 16 * 24) +
                            SQUARE(*(reg + h * 4 + 3) - d * 16 * 24);
                        if (i < f)
                        {
                            f = i;
                            g = h;
                            a1 = *(reg + h * 4 + 0) / 24;
                            b1 = *(reg + h * 4 + 1) / 24;
                            c1 = *(reg + h * 4 + 2) / 24;
                            d1 = *(reg + h * 4 + 3) / 24;
                        }
                    }
                    mAsciiMap[(a << 12) + (b << 8) + (c << 4) + d] = g;
                    mActualColors[((a << 12) + (b << 8) + (c << 4) + d) * 4 + 0] = a1;
                    mActualColors[((a << 12) + (b << 8) + (c << 4) + d) * 4 + 1] = b1;
                    mActualColors[((a << 12) + (b << 8) + (c << 4) + d) * 4 + 2] = c1;
                    mActualColors[((a << 12) + (b << 8) + (c << 4) + d) * 4 + 3] = d1;
                }
            }
        }
    }
    delete[] reg;
}



void TFX_AsciiArt::Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    assert(mAsciiMap!=NULL);
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 2;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 2;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 2)
    {
        for (int x = 0, targetpos=(aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 2)
        {
            int c1 = aSource[sourcepos];
            int c2 = aSource[sourcepos + 1];
            int c3 = aSource[sourcepos + aSrcPitch];
            int c4 = aSource[sourcepos + aSrcPitch + 1];
/*
            int c1a = (((c1 >>  0) & 0xff) * 1 + 
                       ((c1 >>  8) & 0xff) * 6 + 
                       ((c1 >> 16) & 0xff) * 3) / 10;

            int c2a = (((c2 >>  0) & 0xff) * 1 + 
                       ((c2 >>  8) & 0xff) * 6 + 
                       ((c2 >> 16) & 0xff) * 3) / 10;

            int c3a = (((c3 >>  0) & 0xff) * 1 + 
                       ((c3 >>  8) & 0xff) * 6 + 
                       ((c3 >> 16) & 0xff) * 3) / 10;

            int c4a = (((c4 >>  0) & 0xff) * 1 + 
                       ((c4 >>  8) & 0xff) * 6 + 
                       ((c4 >> 16) & 0xff) * 3) / 10;
*/
            int c1a = ((c1 >>  8) & 0xff);
            int c2a = ((c2 >>  8) & 0xff);
            int c3a = ((c3 >>  8) & 0xff);
            int c4a = ((c4 >>  8) & 0xff);

            if (mOptions & TFX_TEMPORAL_DITHER)
            {

#define APPLYERROR(cc,p) \
                (cc) += mErrorMap[targetpos*4+(p)]; \
                if ((cc) < 0) (cc) = 0; \
                if ((cc) > 0xff) (cc) = 0xff; 
        
                APPLYERROR(c1a, 0);
                APPLYERROR(c2a, 1);
                APPLYERROR(c3a, 2);
                APPLYERROR(c4a, 3);
            }

            int c=(((c1a >> 4) & 0xf) << 12)+
                  (((c2a >> 4) & 0xf) << 8)+
                  (((c3a >> 4) & 0xf) << 4)+
                  (((c4a >> 4) & 0xf));

            aTarget[targetpos] = mAsciiMap[c] | 0x0700;                                 

            if (mOptions & TFX_TEMPORAL_DITHER)
            {

#define GATHERERROR(cc,p) \
                mErrorMap[targetpos * 4 + (p)] = mActualColors[c * 4 + (p)] - (cc);
        
                GATHERERROR(c1a, 0);
                GATHERERROR(c2a, 1);
                GATHERERROR(c3a, 2);
                GATHERERROR(c4a, 3);
            }
        }
    }
}
