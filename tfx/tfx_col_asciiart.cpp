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


TFX_ColAsciiArt::TFX_ColAsciiArt()
{
}


TFX_ColAsciiArt::~TFX_ColAsciiArt()
{
}


static int findcol(int r, int g, int b, int except)
{
    int dist = 0xffffff;
    int col = 0;
    int i;

    for (i = 0; i < 16; i++)
    {
        if (i != except)
        {
            int d = SQUARE(TFX_Palette8[i * 3 + 2] - r) +
                SQUARE(TFX_Palette8[i * 3 + 1] - g) +
                SQUARE(TFX_Palette8[i * 3 + 0] - b);

            if (d < dist)
            {
                col = i;
                dist = d;
            }
        }
    }

    return col;
}


void TFX_ColAsciiArt::Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    assert(mAsciiMap != NULL);
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 2;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 2;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch * 2)
    {
        for (int x = 0, targetpos = (aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 2)
        {
            int c1 = aSource[sourcepos];
            int c2 = aSource[sourcepos + 1];
            int c3 = aSource[sourcepos + aSrcPitch];
            int c4 = aSource[sourcepos + aSrcPitch + 1];

            if (mOptions & TFX_TEMPORAL_DITHER)
            {
                int tr, tg, tb;

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

            int avgcolor = ((c1 & 0xfcfcfc) + (c2 & 0xfcfcfc) + (c3 & 0xfcfcfc) + (c4 & 0xfcfcfc)) >> 2;

            int primarycolor = findcol((avgcolor >> 16) & 0xff,
                (avgcolor >> 8) & 0xff,
                (avgcolor >> 0) & 0xff, -1);
            int secondarycolor = findcol((((avgcolor >> 16) & 0xff) - TFX_Palette8[primarycolor * 3 + 2]) * 2,
                (((avgcolor >> 8) & 0xff) - TFX_Palette8[primarycolor * 3 + 1]) * 2,
                (((avgcolor >> 0) & 0xff) - TFX_Palette8[primarycolor * 3 + 0]) * 2, primarycolor);

#define CAP(a) abs(a)*2

            int c1a, c2a, c3a, c4a;

            c1a = (CAP(((c1 >> 0) & 0xff) - TFX_Palette8[primarycolor * 3 + 0]) << 0) |
                (CAP(((c1 >> 8) & 0xff) - TFX_Palette8[primarycolor * 3 + 1]) << 8) |
                (CAP(((c1 >> 16) & 0xff) - TFX_Palette8[primarycolor * 3 + 2]) << 16);
            c2a = (CAP(((c2 >> 0) & 0xff) - TFX_Palette8[primarycolor * 3 + 0]) << 0) |
                (CAP(((c2 >> 8) & 0xff) - TFX_Palette8[primarycolor * 3 + 1]) << 8) |
                (CAP(((c2 >> 16) & 0xff) - TFX_Palette8[primarycolor * 3 + 2]) << 16);
            c3a = (CAP(((c3 >> 0) & 0xff) - TFX_Palette8[primarycolor * 3 + 0]) << 0) |
                (CAP(((c3 >> 8) & 0xff) - TFX_Palette8[primarycolor * 3 + 1]) << 8) |
                (CAP(((c3 >> 16) & 0xff) - TFX_Palette8[primarycolor * 3 + 2]) << 16);
            c4a = (CAP(((c4 >> 0) & 0xff) - TFX_Palette8[primarycolor * 3 + 0]) << 0) |
                (CAP(((c4 >> 8) & 0xff) - TFX_Palette8[primarycolor * 3 + 1]) << 8) |
                (CAP(((c4 >> 16) & 0xff) - TFX_Palette8[primarycolor * 3 + 2]) << 16);

            int c1b = MAX(((c1a >> 0) & 0xff),
                MAX(((c1a >> 8) & 0xff),
                ((c1a >> 16) & 0xff)));
            int c2b = MAX(((c2a >> 0) & 0xff),
                MAX(((c2a >> 8) & 0xff),
                ((c2a >> 16) & 0xff)));
            int c3b = MAX(((c3a >> 0) & 0xff),
                MAX(((c3a >> 8) & 0xff),
                ((c3a >> 16) & 0xff)));
            int c4b = MAX(((c4a >> 0) & 0xff),
                MAX(((c4a >> 8) & 0xff),
                ((c4a >> 16) & 0xff)));

            int c = (((c1b >> 4) & 0xf) << 12) +
                (((c2b >> 4) & 0xf) << 8) +
                (((c3b >> 4) & 0xf) << 4) +
                (((c4b >> 4) & 0xf));

            aTarget[targetpos] = (mAsciiMap[c] & 0xff) | (primarycolor << 12) | (secondarycolor << 8);

            if (mOptions & TFX_TEMPORAL_DITHER)
            {
#define GATHERERROR(cc,p) \
    mErrorMap[targetpos * 4 * 3 + 0] = \
    ( \
    (mActualColors[c * 4 + (p)] * TFX_Palette8[secondarycolor * 3 + 0] + \
    (0xff - mActualColors[c * 4 + (p)]) * TFX_Palette8[primarycolor * 3 + 0]) \
    -(((cc) >> 0) & 0xff)); \
    mErrorMap[targetpos * 4 * 3 + 1] = \
    ((mActualColors[c * 4 + (p)] * TFX_Palette8[secondarycolor * 3 + 1] + \
    (0xff - mActualColors[c * 4 + (p)]) * TFX_Palette8[primarycolor * 3 + 1]) \
    -(((cc) >> 8) & 0xff)); \
    mErrorMap[targetpos * 4 * 3 + 2] = \
    ((mActualColors[c * 4 + (p)] * TFX_Palette8[secondarycolor * 3 + 2] + \
    (0xff - mActualColors[c * 4 + (p)]) * TFX_Palette8[primarycolor * 3 + 2]) \
    -(((cc) >> 16) & 0xff));

                GATHERERROR(c1, 0);
                GATHERERROR(c2, 1);
                GATHERERROR(c3, 2);
                GATHERERROR(c4, 3);
            }
        }
    }
}

