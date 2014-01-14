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
#include <string.h>
#include <assert.h>
#include "textfx.h"


int TFX_BruteForce::scan(int *data)
{
    unsigned char * pchar = (unsigned char *)TFX_AsciiFontdata;

    int best = 0;
    int bestdiff = 0xfffffff;
    int chars;

    // brute-force matcher, generating each character/color set separately and applies blur to
    // them, then calculates "bit exact" difference

#define MINCHAR 32
#define MAXCHAR 127 
    //#define MINCHAR 0
    //#define MAXCHAR 255

    static unsigned char *lut = NULL;

    if (lut == NULL)
    {
        lut = new unsigned char[(MAXCHAR - MINCHAR) * 12 * 8];
        for (chars = MINCHAR; chars < MAXCHAR; chars++)
        {
            int charoffset = chars * 12;
            int i, j, c;
            int scratch[8 * 12];
            for (i = 0, c = 0; i < 12; i++)
            {
                for (j = 0; j < 8; j++, c++)
                {
                    int cc = IS_BIT(*(pchar + i + charoffset), j) ? 255 : 0;
                    scratch[i * 8 + j] = cc;
                }
            }

            for (i = 0, c = 0; i < 12; i++)
            {
                for (j = 0; j < 8; j++, c++)
                {
                    lut[(chars - MINCHAR) * 12 * 8 + c] =
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
                        ) / 16;
                }
            }
        }
    }

    for (chars = MINCHAR; chars < MAXCHAR; chars++)
    {
        int charoffset = chars * 12;
        int bg, fg;
        for (bg = 0; bg < 16; bg++)
        {
            for (fg = 0; fg < 16; fg++)
            {
                if (bg != fg)
                {
                    int i, j, c;
                    int diff = 0;

                    for (i = 0, c = 0; i < 12; i++)
                    {
                        for (j = 0; j < 8; j++, c++)
                        {
                            int lv = lut[(chars - MINCHAR) * 12 * 8 + c];
                            int vr = (lv * ((TFX_Palette32[fg] >> 0) & 0xff) + (0xff - lv) * ((TFX_Palette32[bg] >> 0) & 0xff)) >> 8;
                            int vg = (lv * ((TFX_Palette32[fg] >> 8) & 0xff) + (0xff - lv) * ((TFX_Palette32[bg] >> 8) & 0xff)) >> 8;
                            int vb = (lv * ((TFX_Palette32[fg] >> 16) & 0xff) + (0xff - lv) * ((TFX_Palette32[bg] >> 16) & 0xff)) >> 8;


                            diff += SQUARE((vr) - ((data[c] >> 0) & 0xff)) +
                                SQUARE((vg) - ((data[c] >> 8) & 0xff)) +
                                SQUARE((vb) - ((data[c] >> 16) & 0xff));
                        }
                    }


                    if (diff < bestdiff)
                    {
                        bestdiff = diff;
                        best = (bg << 12) | (fg << 8) | chars;
                        if (!bestdiff)
                            return best;
                    }
                }
            }
        }
    }

    return best;
}

TFX_BruteForce::TFX_BruteForce()
{}

TFX_BruteForce::~TFX_BruteForce()
{}



void TFX_BruteForce::BuildLUT()
{}


static int bilerp(int c0, int c1, int c2, int c3, int x, int y, int w, int h)
{
    // some preparation
    int c0r = (c0 >> 0) & 0xff;
    int c0g = (c0 >> 8) & 0xff;
    int c0b = (c0 >> 16) & 0xff;

    int c1r = (c1 >> 0) & 0xff;
    int c1g = (c1 >> 8) & 0xff;
    int c1b = (c1 >> 16) & 0xff;

    int c2r = (c2 >> 0) & 0xff;
    int c2g = (c2 >> 8) & 0xff;
    int c2b = (c2 >> 16) & 0xff;

    int c3r = (c3 >> 0) & 0xff;
    int c3g = (c3 >> 8) & 0xff;
    int c3b = (c3 >> 16) & 0xff;

    // horizontal lerp

    int c01r = (c0r * x + c1r * (w - x)) / w;
    int c01g = (c0g * x + c1g * (w - x)) / w;
    int c01b = (c0b * x + c1b * (w - x)) / w;

    int c23r = (c2r * x + c3r * (w - x)) / w;
    int c23g = (c2g * x + c3g * (w - x)) / w;
    int c23b = (c2b * x + c3b * (w - x)) / w;

    // vertical lerp

    int cr = (c23r * y + c01r * (h - y)) / h;
    int cg = (c23g * y + c01g * (h - y)) / h;
    int cb = (c23b * y + c01b * (h - y)) / h;

    assert(cr >= 0 && cr < 0x100);
    assert(cg >= 0 && cg < 0x100);
    assert(cb >= 0 && cb < 0x100);

    return (cr << 0) | (cg << 8) | (cb << 16);
}

void TFX_BruteForce::Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    int grid[8*14];
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0);
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0);
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 1)
        {
            int i, j;

            for (i = 0; i < 6; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    grid[i*8 + j] = aSource[sourcepos];
                    grid[i*8 + j + 4] = aSource[sourcepos];
                    grid[i*8 + j + 6*8] = aSource[sourcepos];
                    grid[i*8 + j + 6*8 + 4] = aSource[sourcepos];
                }
            }

            aTarget[targetpos] = scan(grid);
            TFX_Present();
        }
    }

}

void TFX_BruteForce::Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    int grid[8*14];
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 2;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 2;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 2)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 2)
        {
            int i, j;
#if 1 // bilerp

            int c0 = aSource[sourcepos];
            int c1 = aSource[sourcepos + 1];
            int c2 = aSource[sourcepos + aSrcPitch];
            int c3 = aSource[sourcepos + aSrcPitch + 1];
            for (i = 0; i < 12; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    grid[i*8 + j] = bilerp(c0, c1, c2, c3, j, i, 8, 12);
                }
            }
#else // pointsample

            for (i = 0; i < 6; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    grid[i*8 + j] = aSource[sourcepos];
                    grid[i*8 + j + 4] = aSource[sourcepos + 1];
                    grid[i*8 + j + 6*8] = aSource[sourcepos + aSrcPitch];
                    grid[i*8 + j + 6*8 + 4] = aSource[sourcepos + aSrcPitch + 1];
                }
            }
#endif
            aTarget[targetpos] = scan(grid);
            TFX_Present();
        }
    }

}

void TFX_BruteForce::Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    int grid[8*14];
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 4;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 4;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 4)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 4)
        {
#if 1 // bilerp
            int i, j, k, l;
            for (k = 0; k < 2; k++) // horiz
            {
                for (l = 0; l < 2; l++) // vert
                {
                    int c0 = aSource[sourcepos + k * 2 + l * 2 * aSrcPitch];
                    int c1 = aSource[sourcepos + k * 2 + l * 2 * aSrcPitch + 1];
                    int c2 = aSource[sourcepos + k * 2 + l * 2 * aSrcPitch + aSrcPitch];
                    int c3 = aSource[sourcepos + k * 2 + l * 2 * aSrcPitch + aSrcPitch + 1];
                    for (i = 0; i < 6; i++)
                    {
                        for (j = 0; j < 4; j++)
                        {
                            grid[(i + l*6)*8 + j + k*4] = bilerp(c0, c1, c2, c3, j, i, 4, 6);
                        }
                    }
                }
            }
#else // pointsample
            int i, j;

            for (i = 0; i < 6; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    grid[i*8 + j] = aSource[sourcepos];
                    grid[i*8 + j + 4] = aSource[sourcepos + 1];
                    grid[i*8 + j + 6*8] = aSource[sourcepos + aSrcPitch];
                    grid[i*8 + j + 6*8 + 4] = aSource[sourcepos + aSrcPitch + 1];
                }
            }
#endif
            aTarget[targetpos] = scan(grid);
            TFX_Present();
        }
    }
}
