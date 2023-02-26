#pragma once
/*#
    # r2c02.h

    Ricoh 2C02 or NES PPU (Picture Processing Unit).

    Do this:
    ~~~~C
    #define CHIPS_IMPL
    ~~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide
    ~~~C
    #define CHIPS_ASSERT(x) your_own_asset_macro(x)
    ~~~

    ## Real Pins
          ___  ___
         |*  \/   |
 R/W  >01]        [40<  VCC
  D0  [02]        [39>  ALE
  D1  [03]        [38]  AD0
  D2  [04]        [37]  AD1
  D3  [05]        [36]  AD2
  D4  [06]        [35]  AD3
  D5  [07]        [34]  AD4
  D6  [08]        [33]  AD5
  D7  [09]        [32]  AD6
  A2  >10]  2C02  [31]  AD7
  A1  >11]        [30>  A8
  A0  >12]        [29>  A9
 /CS  >13]        [28>  A10
EXT0  [14]        [27>  A11
EXT1  [15]        [26>  A12
EXT2  [16]        [25>  A13
EXT3  [17]        [24>  /R
 CLK  >18]        [23>  /W
/VBL  <19]        [22<  /SYNC
 VEE  >20]        [21>  VOUT
         |________|

    ## zlib/libpng license

    Copyright (c) 2023 Scemino
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution.
#*/

#define PPU_DISPLAY_WIDTH  (256)
#define PPU_DISPLAY_HEIGHT (240)
#define PPU_FRAMEBUFFER_SIZE_BYTES (PPU_DISPLAY_WIDTH * PPU_DISPLAY_HEIGHT)

const uint32_t ppu_palette[] =
{
    0xff666666, 0xff002a88, 0xff1412a7, 0xff3b00a4, 
    0xff5c007e, 0xff6e0040, 0xff6c0600, 0xff561d00, 
    0xff333500, 0xff0b4800, 0xff005200, 0xff004f08, 
    0xff00404d, 0xff000000, 0xff000000, 0xff000000, 
    0xffadadad, 0xff155fd9, 0xff4240ff, 0xff7527fe, 
    0xffa01acc, 0xffb71e7b, 0xffb53120, 0xff994e00, 
    0xff6b6d00, 0xff388700, 0xff0c9300, 0xff008f32, 
    0xff007c8d, 0xff000000, 0xff000000, 0xff000000, 
    0xfffffeff, 0xff64b0ff, 0xff9290ff, 0xffc676ff, 
    0xfff36aff, 0xfffe6ecc, 0xfffe8170, 0xffea9e22, 
    0xffbcbe00, 0xff88d800, 0xff5ce430, 0xff45e082, 
    0xff48cdde, 0xff4f4f4f, 0xff000000, 0xff000000, 
    0xfffffeff, 0xffc0dfff, 0xffd3d2ff, 0xffe8c8ff, 
    0xfffbc2ff, 0xfffec4ea, 0xfffeccc5, 0xfff7d8a5, 
    0xffe4e594, 0xffcfef96, 0xffbdf4ab, 0xffb3f3cc, 
    0xffb5ebf2, 0xffb8b8b8, 0xff000000, 0xff000000, 
};

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#endif /* CHIPS_IMPL */
