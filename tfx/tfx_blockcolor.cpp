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

#define TFX_COLORMAP_DEPTH 6
#define TFX_COLMAPDIM (1<<TFX_COLORMAP_DEPTH)
#define TFX_TRUCOLBITS (8 - TFX_COLORMAP_DEPTH)
#define TFX_COLMAP(r,g,b) *(mBlockMap + ((r)<<(TFX_COLORMAP_DEPTH * 2)) + ((g)<<TFX_COLORMAP_DEPTH) + (b))


TFX_BlockColor::TFX_BlockColor()
{
    mBlockMap = NULL;
    mActualColors = NULL;
    mErrorMap = NULL;
}


TFX_BlockColor::~TFX_BlockColor()
{
    delete[] mBlockMap;
    delete[] mActualColors;
    delete[] mErrorMap;
}


void TFX_BlockColor::CalcColor(int red, int green, int blue, int offset) 
{
    int a, b, c, d, ch, co, R, G, B;
    int lastdist, dist;

    lastdist = 0x7fffffff;

    for (c = 0, d = 0; c < 16; c++, d += 3) 
    {
        dist = SQUARE(TFX_Palette8[d + 0] - red) +
            SQUARE(TFX_Palette8[d + 1] - green) +
            SQUARE(TFX_Palette8[d + 2] - blue);
        if (dist < lastdist) 
        {
            lastdist = dist;
            co = c;
            ch = 219; // 100% block in IBMSCII 
            R = TFX_Palette8[d + 0];
            G = TFX_Palette8[d + 1];
            B = TFX_Palette8[d + 2];
        }
    }
    c = co;
    d = c * 3;
    for (b = 0, a = 0; b < 16; b++, a += 3) 
    {
        dist = SQUARE(((TFX_Palette8[a + 0] + TFX_Palette8[d + 0]) / 2) - red) +
            SQUARE(((TFX_Palette8[a + 1] + TFX_Palette8[d + 1]) / 2) - green) +
            SQUARE(((TFX_Palette8[a + 2] + TFX_Palette8[d + 2]) / 2) - blue);
        if (dist < lastdist) 
        {
            lastdist = dist;
            if (b > c)
                co = b + (c << 4);
            else
                co = c + (b << 4);
            ch = 177; // 50% block in IBMSCII 

            R = (TFX_Palette8[a + 0] + TFX_Palette8[d + 0]) / 2;
            G = (TFX_Palette8[a + 1] + TFX_Palette8[d + 1]) / 2;
            B = (TFX_Palette8[a + 2] + TFX_Palette8[d + 2]) / 2;
        }
        dist = SQUARE((TFX_Palette8[a + 0] * 2 / 3 + TFX_Palette8[d + 0] / 3) - red) +
            SQUARE((TFX_Palette8[a + 1] * 2 / 3 + TFX_Palette8[d + 1] / 3) - green) +
            SQUARE((TFX_Palette8[a + 2] * 2 / 3 + TFX_Palette8[d + 2] / 3) - blue);
        if (dist < lastdist) 
        {
            lastdist = dist;
            if (b > c)
            {
                co = b + (c << 4);
                ch = 178; // 75% block in IBMSCII 
            }
            else
            {
                co = c + (b << 4);
                ch = 176; // 25% block in IBMSCII 
            }

            R = (TFX_Palette8[a + 0] * 2 + TFX_Palette8[d + 0]) / 3;
            G = (TFX_Palette8[a + 1] * 2 + TFX_Palette8[d + 1]) / 3;
            B = (TFX_Palette8[a + 2] * 2 + TFX_Palette8[d + 2]) / 3;
        }
        dist = SQUARE((TFX_Palette8[a + 0] / 3 + TFX_Palette8[d + 0] * 2 / 3) - red) +
            SQUARE((TFX_Palette8[a + 1] / 3 + TFX_Palette8[d + 1] * 2 / 3) - green) +
            SQUARE((TFX_Palette8[a + 2] / 3 + TFX_Palette8[d + 2] * 2 / 3) - blue);
        if (dist < lastdist) 
        {
            lastdist = dist;
            if (c > b)
            {
                co = c + (b << 4);
                ch = 178; // 75% block in IBMSCII 
            }
            else
            {
                co = b + (c <<4 );
                ch = 176; // 25% block in IBMSCII 
            }

            R = (TFX_Palette8[a + 0] + TFX_Palette8[d + 0] * 2 ) / 3;
            G = (TFX_Palette8[a + 1] + TFX_Palette8[d + 1] * 2 ) / 3;
            B = (TFX_Palette8[a + 2] + TFX_Palette8[d + 2] * 2 ) / 3;
        }
    }

    mBlockMap[offset] = (co << 8) + ch;
    mActualColors[offset * 3 + 0] = R;
    mActualColors[offset * 3 + 1] = G;
    mActualColors[offset * 3 + 2] = B;
}


void TFX_BlockColor::BuildLUT() 
{
    int r, g, b;
    int f;
    f = 256 / TFX_COLMAPDIM;
    mBlockMap = new short[TFX_COLMAPDIM * TFX_COLMAPDIM * TFX_COLMAPDIM];
    mActualColors = new unsigned char[TFX_COLMAPDIM * TFX_COLMAPDIM * TFX_COLMAPDIM * 3];
    mErrorMap = new signed char[80 * 50 * 3];
    memset(mErrorMap, 0, 80 * 50 * 3);
    for (r = 0; r < TFX_COLMAPDIM; r++) 
    {
        for (g = 0; g < TFX_COLMAPDIM; g++)
        {
            for (b = 0; b < TFX_COLMAPDIM; b++)
            {
                CalcColor(r * f, g * f, b * f, ((r) << (TFX_COLORMAP_DEPTH * 2)) + ((g) << TFX_COLORMAP_DEPTH) + (b));
            }
        }
    }
}


void TFX_BlockColor::Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    assert(mBlockMap!=NULL);
    int xsize = aSrcQuad.x1 - aSrcQuad.x0;
    int ysize = aSrcQuad.y1 - aSrcQuad.y0;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos++)
        {
            int color = aSource[sourcepos];
            aTarget[targetpos] = TFX_COLMAP(((color) & 0xff) >> TFX_TRUCOLBITS,
                ((color >> 8) & 0xff) >> TFX_TRUCOLBITS,
                ((color >> 16) & 0xff) >> TFX_TRUCOLBITS);
        }
    }
}



void TFX_BlockColor::Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    assert(mBlockMap!=NULL);
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 2;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 2;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 2)
    {
        for (int x = 0, targetpos=(aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 2)
        {
            int redblue;
            int green;
            redblue  = aSource[sourcepos] & 0xff00ff;
            green    = aSource[sourcepos] & 0x00ff00;
            redblue += aSource[sourcepos + 1] & 0xff00ff;
            green   += aSource[sourcepos + 1] & 0x00ff00;
            redblue += aSource[sourcepos + aSrcPitch] & 0xff00ff;
            green   += aSource[sourcepos + aSrcPitch] & 0x00ff00;
            redblue += aSource[sourcepos + aSrcPitch + 1] & 0xff00ff;
            green   += aSource[sourcepos + aSrcPitch + 1] & 0x00ff00;
            int color = ((redblue>>2) & 0xff00ff) + ((green>>2) & 0x00ff00);

            int r = color & 0xff;
            int g = (color >> 8) & 0xff;
            int b = (color >> 16) & 0xff;

            if (mOptions & TFX_TEMPORAL_DITHER)
            {
                r += mErrorMap[targetpos*3+0];
                g += mErrorMap[targetpos*3+1];
                b += mErrorMap[targetpos*3+2];

                if (r < 0) r = 0;
                if (r > 0xff) r = 0xff;
                if (g < 0) g = 0;
                if (g > 0xff) g = 0xff;
                if (b < 0) b = 0;
                if (b > 0xff) b = 0xff;
            }

            int offset = ((r >> TFX_TRUCOLBITS)<<(TFX_COLORMAP_DEPTH * 2)) + 
                         ((g >> TFX_TRUCOLBITS)<<TFX_COLORMAP_DEPTH) + 
                         (b >> TFX_TRUCOLBITS);

            if (mOptions & TFX_TEMPORAL_DITHER)
            {
                int err_r = r - mActualColors[offset*3+0];
                int err_g = g - mActualColors[offset*3+1];
                int err_b = b - mActualColors[offset*3+2];

                mErrorMap[targetpos*3+0] = err_r;
                mErrorMap[targetpos*3+1] = err_g;
                mErrorMap[targetpos*3+2] = err_b;
            }

            aTarget[targetpos] = mBlockMap[offset];
        }
    }
}


void TFX_BlockColor::Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{

    assert(mBlockMap!=NULL);
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 4;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 4;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 4)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 4)
        {
            int redblue = 0;
            int green = 0;
            for (int iny = 0, inyp = sourcepos; iny < 4; iny++, inyp += aSrcPitch)
            {
                for (int inx = 0, inpos = inyp; inx < 4; inx++, inpos++)
                {
                    redblue += aSource[inpos] & 0xff00ff;
                    green   += aSource[inpos] & 0x00ff00;
                }
            }

            int color = ((redblue>>4) & 0xff00ff) + ((green>>4) & 0x00ff00);

            aTarget[targetpos] = TFX_COLMAP(((color    ) & 0xff)>>TFX_TRUCOLBITS,
                ((color>> 8) & 0xff)>>TFX_TRUCOLBITS,
                ((color>>16) & 0xff)>>TFX_TRUCOLBITS);
        }
    }
}
