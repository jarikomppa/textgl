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

#include "tfx_fontdata.h"

int TFX_Palette8[16*3]={ // 'clean' RGB palette 
    0x00,0x00,0x00,  0x00,0x00,0x80,  0x00,0x80,0x00,  0x00,0x80,0x80, 0x80,0x00,0x00, 0x80,0x00,0x80, 0x80,0x80,0x00, 0xc0,0xc0,0xc0,
    0x80,0x80,0x80,  0x00,0x00,0xff,  0x00,0xff,0x00,  0x00,0xff,0xff, 0xff,0x00,0x00, 0xff,0x00,0xff, 0xff,0xff,0x00, 0xff,0xff,0xff
};

int TFX_Palette6[16*3]={ // 'clean' RGB palette 
    0, 0, 0,  0, 0,32,  0,32, 0,  0,32,32, 32, 0, 0, 32, 0,32, 32,32, 0, 48,48,48,
    32,32,32,  0, 0,63,  0,63, 0,  0,63,63, 63, 0, 0, 63, 0,63, 63,63, 0, 63,63,63
};

int TFX_PaletteIBM[16*3]={ // IBM classic palette, 16c 
    0, 0, 0,  0, 0,42,  0,42, 0,  0,42,42, 42, 0, 0, 42, 0,42, 42,21, 0, 42,42,42,
    9999,9999,9999, 21,21,63, 21,63,21, 21,63,63, 63,21,21, 63,21,63, 63,63,21, 63,63,63
};

int TFX_Palette32[16] = 
{
    0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,
    0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
};
