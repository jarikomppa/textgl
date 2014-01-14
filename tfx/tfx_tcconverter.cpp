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
#include "textfx.h"


TFX_TCConverter::TFX_TCConverter() 
{ 
    mOptions = 0; 
}


TFX_TCConverter::~TFX_TCConverter() 
{
}


void TFX_TCConverter::SetOptions(int option) 
{ 
    mOptions = option; 
}


void TFX_TCConverter::Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0);
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0);
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;
    TFXQuad nq;
    nq.x0 = 0;
    nq.y0 = 0;
    nq.x1 = xsize * 2;
    nq.y1 = ysize * 2;
    int *temp = new int[xsize * ysize * 2 * 2];

    for (int y = 0; y < ysize; y++, sourceyofs += aSrcPitch)
    {
        for (int x = 0, targetpos=(aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos ++)
        {
            int c1 = aSource[sourcepos];

            temp[y * xsize * 2 * 2 + x * 2 + 0] = c1;
            temp[y * xsize * 2 * 2 + x * 2 + 1] = c1;
            temp[y * xsize * 2 * 2 + x * 2 + 0 + xsize * 2] = c1;
            temp[y * xsize * 2 * 2 + x * 2 + 1 + xsize * 2] = c1;
        }
    }

    Dump2x(temp, nq, xsize * 2, aTgtX0, aTgtY0, aTarget);
    delete[] temp;
}


void TFX_TCConverter::Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch, int aTgtX0, int aTgtY0, short *aTarget)
{
    int xsize = (aSrcQuad.x1 - aSrcQuad.x0) / 2;
    int ysize = (aSrcQuad.y1 - aSrcQuad.y0) / 2;
    int sourceyofs = aSrcQuad.y0 * aSrcPitch;
    TFXQuad nq;
    nq.x0 = 0;
    nq.y0 = 0;
    nq.x1 = xsize;
    nq.y1 = ysize;
    int *temp = new int[xsize * ysize];

    for (int y = 0, c = 0; y < ysize; y++, sourceyofs += aSrcPitch * 2)
    {
        for (int x = 0, targetpos=(aTgtY0 + y) * TFX_ConsoleWidth + aTgtX0, sourcepos = sourceyofs + aSrcQuad.x0; x < xsize; x++, targetpos++, sourcepos += 2, c++)
        {
            int c1 = aSource[sourcepos];
            int c2 = aSource[sourcepos + 1];
            int c3 = aSource[sourcepos + aSrcPitch];
            int c4 = aSource[sourcepos + aSrcPitch + 1];

            int avgcolor = ((c1 & 0xfcfcfc) + (c2 & 0xfcfcfc) + (c3 & 0xfcfcfc) + (c4 & 0xfcfcfc)) >> 2;

            temp[c] = avgcolor;
        }
    }

    Dump2x(temp, nq, xsize, aTgtX0, aTgtY0, aTarget);
    delete[] temp;
}

