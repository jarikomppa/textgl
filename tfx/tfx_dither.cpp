///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// TextFX7
// Copyright (c) 1995-2001 Jari Komppa
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
#include "textfx.h"

/* WELL512 algorithm implementation
   by Chris Lomont, from Game Programming Gems 7, page 120-121
   public domain 
 */
 
/* initialize state to random bits */
static unsigned long state[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
/* init should also reset this to 0 */
static unsigned int index = 0;
/* return 32 bit random number */
unsigned long WELLRNG512(void)
{
  unsigned long a, b, c, d;
  a = state[index];
  c = state[(index + 13) & 15];
  b = a ^ c ^ (a << 16) ^ (c << 15);
  c = state[(index + 9) & 15];
  c ^= (c >> 11);
  a = state[index] = b ^ c;
  d = a ^ ((a << 5) & 0xDA442D20UL);
  index = (index + 15) & 15;
  a = state[index];
  state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return state[index];
}


void TFX_RandomDither(int * aBuffer, int aWidth, int aHeight)
{
    int i, j, c;
    for (i = 0, c = 0; i < aHeight; i++)
    {
        for (j = 0; j < aWidth; j++, c++)
        {
            int r = (aBuffer[c] >> 16) & 0xff;
            int g = (aBuffer[c] >> 8) & 0xff;
            int b = (aBuffer[c] >> 0) & 0xff;

            r += (WELLRNG512() & 31) - 16;
            g += (WELLRNG512() & 31) - 16;
            b += (WELLRNG512() & 31) - 16;

            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            aBuffer[c] = (r << 16) | (g << 8) | b;
        }
    }
}

// from wikipedia ordered dither page
static int orderedpattern[4 * 4] =
{
    1, 9, 3, 11,
    13, 5, 15, 7,
    4, 12, 2, 10,
    16, 8, 14, 6
};


void TFX_OrderedDither(int * aBuffer, int aWidth, int aHeight)
{
    int i, j, c;
    for (i = 0, c = 0; i < aHeight; i++)
    {
        for (j = 0; j < aWidth; j++, c++)
        {
            int r = (aBuffer[c] >> 16) & 0xff;
            int g = (aBuffer[c] >> 8) & 0xff;
            int b = (aBuffer[c] >> 0) & 0xff;

            r += orderedpattern[(j & 3) * 4 + (i & 3)] * 2 - 16;
            g += orderedpattern[(j & 3) * 4 + (i & 3)] * 2 - 16;
            b += orderedpattern[(j & 3) * 4 + (i & 3)] * 2 - 16;

            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            aBuffer[c] = (r << 16) | (g << 8) | b;
        }
    }
}
