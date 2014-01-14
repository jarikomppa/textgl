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

#ifndef TEXTFX_H_INCLUDED
#define TEXTFX_H_INCLUDED

// Console parameters
static const int TFX_ConsoleHeight = 50;
static const int TFX_ConsoleWidth = 80;

// The raw text mode buffer that will be shown at the next TFX_Preset()
extern short TFX_FrameBuffer[];

// Main function parameters.
// If 0, app was built as windows application, not a console one
extern int TFX_Paramc; 
extern char **TFX_Params;

extern unsigned char TFX_AsciiFontdata[12 * 256];
extern int TFX_Palette8[16 * 3];
extern int TFX_Palette6[16 * 3];
extern int TFX_PaletteIBM[16 * 3];
extern int TFX_Palette32[16];

#define IS_BIT(a,b) (((a) & (1 << (b))) ? 1 : 0)
#define SQUARE(a) ((a) * (a))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// Present framebuffer to user
extern void TFX_Present();

// Set the title of the window
extern void TFX_SetTitle(char *aTitle);

// Some dithers
extern void TFX_RandomDither(int * aBuffer, int aWidth, int aHeight);
extern void TFX_OrderedDither(int * aBuffer, int aWidth, int aHeight);

class TFXQuad
{
public:
    int x0, y0, x1, y1;
    TFXQuad() {};
    TFXQuad(int aX0, int aY0, int aX1, int aY1) { x0 = aX0; x1 = aX1; y0 = aY0; y1 = aY1; }
};

enum TFX_OPTIONS
{
    TFX_TEMPORAL_DITHER = 1,
    TFX_CHARSWAP_DITHER = 2
};

class TFX_TCConverter
{
public:
    // Build look-up table. 
    virtual void BuildLUT() = 0;
    // Dump source buffer to target, 1x1->1x1 sampling
    virtual void Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch =  80, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer) = 0;
    // Dump source buffer to target, 4x4->1x1 sampling 
    virtual void Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 320, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    TFX_TCConverter();
    virtual ~TFX_TCConverter();
    virtual void SetOptions(int option);
protected:
    int mOptions;
};



class TFX_BlockColor : public TFX_TCConverter
{
public:
    TFX_BlockColor();
    // Build look-up table. 
    virtual void BuildLUT();
    // Dump source buffer to target, 1x1->1x1 sampling
    virtual void Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch =  80, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    // Dump source buffer to target, 4x4->1x1 sampling 
    virtual void Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 320, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    virtual ~TFX_BlockColor();
private:
    void CalcColor(int aRed, int aGreen, int aBlue, int offset);
    short int *mBlockMap;
    unsigned char *mActualColors;
    signed char *mErrorMap;
};

class TFX_HalfBlockColor : public TFX_TCConverter
{
public:
    TFX_HalfBlockColor();
    // Build look-up table. 
    virtual void BuildLUT();
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    virtual ~TFX_HalfBlockColor();
private:
    // Calculate single color. Colors are in 0..255 range.
    short int CalcColor(int aRed, int aGreen, int aBlue);
    int findcol(int incol);
    signed char *mErrorMap;
};


class TFX_AsciiArt : public TFX_TCConverter
{
public:
    TFX_AsciiArt();
    // Build look-up table. 
    virtual void BuildLUT();
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    virtual ~TFX_AsciiArt();
protected:
    unsigned char *mAsciiMap;
    unsigned char *mActualColors;
    signed char *mErrorMap;
};

class TFX_ColAsciiArt : public TFX_AsciiArt
{
public:
    TFX_ColAsciiArt();
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    virtual ~TFX_ColAsciiArt();
};


class TFX_BruteForce : public TFX_TCConverter
{
public:
    TFX_BruteForce();
    // Build look-up table. 
    virtual void BuildLUT();
    // Dump source buffer to target, 1x1->1x1 sampling
    virtual void Dump1x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch =  80, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    // Dump source buffer to target, 2x2->1x1 sampling 
    virtual void Dump2x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 160, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    // Dump source buffer to target, 4x4->1x1 sampling 
    virtual void Dump4x(int *aSource, TFXQuad &aSrcQuad, int aSrcPitch = 320, int aTgtX0 = 0, int aTgtY0 = 0, short *aTarget = TFX_FrameBuffer);
    virtual ~TFX_BruteForce();
private:
    int scan(int *data);
};

#endif // TEXTFX_H_INCLUDED
