///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// TextFX7
// Copyright (c) 1995-2008 Jari Komppa
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


TFX_HalfBlockColor::TFX_HalfBlockColor()
{
    mErrorMap = 0;
}


TFX_HalfBlockColor::~TFX_HalfBlockColor()
{
    delete[] mErrorMap;
}


short int TFX_HalfBlockColor::CalcColor(int red, int green, int blue) 
{
    int c,d,ch,co;
    int lastdist, dist;

    lastdist = 0x7fffffff;

    for (c = 0, d = 0; c < 16; c++, d += 3) 
    {
        dist = SQUARE(TFX_Palette6[d + 0] - red) +
            SQUARE(TFX_Palette6[d + 1] - green) +
            SQUARE(TFX_Palette6[d + 2] - blue);
        if (dist < lastdist) 
        {
            lastdist = dist;
            co = c;
            ch = 32;
        }
    }
    co = co | (co << 4);
    return((co << 8) + ch);
}


void TFX_HalfBlockColor::BuildLUT() 
{
    mErrorMap = new signed char[160 * 100 * 3];
    memset(mErrorMap, 0, 160 * 100 * 3);
}


int TFX_HalfBlockColor::findcol(int incol)
{
    int dist = 0xffffff;
    int col = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        int d = SQUARE(TFX_Palette8[i * 3 + 0]-((incol >> 0) & 0xff)) +
                SQUARE(TFX_Palette8[i * 3 + 1]-((incol >> 8) & 0xff)) +
                SQUARE(TFX_Palette8[i * 3 + 2]-((incol >> 16) & 0xff));

        if (d < dist)
        {
            col = i;
            dist = d;
        }
    }

    return col;
}


void TFX_HalfBlockColor::Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
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

            if (mOptions & TFX_TEMPORAL_DITHER)
            {
                int tr,tg,tb;
#define APPLYERROR(cc,p) \
    tr = ((cc) >> 0) & 0xff; \
    tg = ((cc) >> 8) & 0xff; \
    tb = ((cc) >> 16) & 0xff; \
    tr += mErrorMap[targetpos * 4 * 3 + (p) * 3 + 0]; \
    tg += mErrorMap[targetpos * 4 * 3 + (p) * 3 + 1]; \
    tb += mErrorMap[targetpos * 4 * 3 + (p) * 3 + 2]; \
    if (tr < 0) tr = 0; \
    if (tr > 0xff) tr = 0xff; \
    if (tg < 0) tg = 0; \
    if (tg > 0xff) tg = 0xff; \
    if (tb < 0) tb = 0; \
    if (tb > 0xff) tb = 0xff; \
    (cc) = (tr << 0) | (tg << 8) | (tb << 16);

                APPLYERROR(c1, 0);
                APPLYERROR(c2, 1);
                APPLYERROR(c3, 2);
                APPLYERROR(c4, 3);
            }

            int redblue;
            int green;
            redblue  = c1 & 0xff00ff;
            green    = c1 & 0x00ff00;
            redblue += c3 & 0xff00ff;
            green   += c3 & 0x00ff00;
            int color1w = ((redblue>>1) & 0xff00ff) + ((green>>1) & 0x00ff00);
            redblue = green = 0;
            redblue += c2 & 0xff00ff;
            green   += c2 & 0x00ff00;
            redblue += c4 & 0xff00ff;
            green   += c4 & 0x00ff00;
            int color2w = ((redblue>>1) & 0xff00ff) + ((green>>1) & 0x00ff00);
            redblue  = c1 & 0xff00ff;
            green    = c1 & 0x00ff00;
            redblue += c2 & 0xff00ff;
            green   += c2 & 0x00ff00;
            int color1h = ((redblue>>1) & 0xff00ff) + ((green>>1) & 0x00ff00);
            redblue = green = 0;
            redblue += c3 & 0xff00ff;
            green   += c3 & 0x00ff00;
            redblue += c4 & 0xff00ff;
            green   += c4 & 0x00ff00;
            int color2h = ((redblue>>1) & 0xff00ff) + ((green>>1) & 0x00ff00);

            int dw = SQUARE(((color1w >> 0) & 0xff)-((color2w >> 0) & 0xff)) +
                SQUARE(((color1w >> 8) & 0xff)-((color2w >> 8) & 0xff)) +
                SQUARE(((color1w >> 16) & 0xff)-((color2w >> 16) & 0xff));

            int dh = SQUARE(((color1h >> 0) & 0xff)-((color2h >> 0) & 0xff)) +
                SQUARE(((color1h >> 8) & 0xff)-((color2h >> 8) & 0xff)) +
                SQUARE(((color1h >> 16) & 0xff)-((color2h >> 16) & 0xff));

            if (dw < dh)
            {
                int col1 = findcol(color2h);
                int col2 = findcol(color1h);

                aTarget[targetpos] = 220 | (col1 << 8) | (col2 << 12);

                if (mOptions & TFX_TEMPORAL_DITHER)
                {
#define GATHERERROR(cc,p,ci) \
    mErrorMap[targetpos * 4 * 3 + 0] = ((TFX_Palette8[(ci) * 3 + 0]-((cc) >> 0) & 0xff)); \
    mErrorMap[targetpos * 4 * 3 + 1] = ((TFX_Palette8[(ci) * 3 + 1]-((cc) >> 8) & 0xff)); \
    mErrorMap[targetpos * 4 * 3 + 2] = ((TFX_Palette8[(ci) * 3 + 2]-((cc) >> 16) & 0xff)); 

                    GATHERERROR(c1, 0, col2);
                    GATHERERROR(c2, 1, col2);
                    GATHERERROR(c3, 2, col1);
                    GATHERERROR(c4, 3, col1);
                }
            }
            else
            {
                int col1 = findcol(color2w);
                int col2 = findcol(color1w);

                aTarget[targetpos] = 221 | (col2 << 8) | (col1 << 12);

                if (mOptions & TFX_TEMPORAL_DITHER)
                {
                    GATHERERROR(c1, 0, col2);
                    GATHERERROR(c2, 1, col1);
                    GATHERERROR(c3, 2, col2);
                    GATHERERROR(c4, 3, col1);
                }
            }
        }
    }
}

