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
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "textfx.h"

extern void demomain();
static HANDLE gScreenBuffer;
short TFX_FrameBuffer[TFX_ConsoleHeight * TFX_ConsoleWidth];
HWND TFX_ConsoleWindowHandle;
int TFX_Paramc;
char **TFX_Params;

void TFX_SetTitle(char * aTitle)
{
    SetConsoleTitleA(aTitle);
}

void TFX_Present()
{
    // Convert textmode screen map to CHAR_INFO array
    CHAR_INFO ci[TFX_ConsoleHeight * TFX_ConsoleWidth];
    for (int i = 0; i < TFX_ConsoleHeight * TFX_ConsoleWidth; i++)
    {
        ci[i].Char.AsciiChar = TFX_FrameBuffer[i] & 0xff;
        ci[i].Attributes = (TFX_FrameBuffer[i] >> 8) & 0xff;
    }
    // set up size structs
    COORD max, src;
    max.X = TFX_ConsoleWidth;
    max.Y = TFX_ConsoleHeight;
    src.X = 0;
    src.Y = 0;
    SMALL_RECT outrect;
    outrect.Top = 0;
    outrect.Left = 0;
    outrect.Right = TFX_ConsoleWidth - 1;
    outrect.Bottom = TFX_ConsoleHeight - 1;     
    // write result to the console
    WriteConsoleOutputA(gScreenBuffer, ci, max, src, &outrect);
}

static void setupConsole()
{
    gScreenBuffer = CreateConsoleScreenBuffer(GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(gScreenBuffer);
    COORD size;
    size.X = TFX_ConsoleWidth;
    size.Y = TFX_ConsoleHeight;
    SetConsoleScreenBufferSize(gScreenBuffer, size);
    // Disable processing of data & wrapping: 
    SetConsoleMode(gScreenBuffer, 0);

    // Attempt to set the "normal" code page
    SetConsoleOutputCP(437);

    // Find console window by setting the title to a random string
    // and asking windows to find this window..
    srand(GetTickCount());
    char title[81];
    int i = sprintf(title,"[detecting window] ");
    for (; i < 80 ; i++)    
        title[i] = (rand() & 0x31) + 'A';
    title[80] = 0;
    SetConsoleTitleA(title);
    TFX_ConsoleWindowHandle = NULL;
    i = 0;
    while (TFX_ConsoleWindowHandle == NULL && i < 100)
    {
        // Sleep a bit to make the title change    
        Sleep(10);
        TFX_ConsoleWindowHandle = FindWindowA(NULL, title);
        i++;
    }
    // Set the window title to something more sensible..
    SetConsoleTitleA("TextFX7 console window - press alt-enter for fullscreen");
    // ..and maximize the window.
    ShowWindow(TFX_ConsoleWindowHandle, SW_MAXIMIZE);

	// try to hide the cursor:
    CONSOLE_CURSOR_INFO cci;
    cci.bVisible = FALSE;
    cci.dwSize = 1;   
    SetConsoleCursorInfo(gScreenBuffer, &cci);
}

// main functions - one for console app, one for windows app. 

#ifdef _CONSOLE
int main(int paramc, char **params)
{
    TFX_Paramc = paramc;
    TFX_Params = params;
    setupConsole();
    demomain();
    return 0;
}
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
    TFX_Paramc = 0;
    TFX_Params = 0;
    if (!AllocConsole()) return 0;
    setupConsole();
    demomain();
    return 0;
}
#endif