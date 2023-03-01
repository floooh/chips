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
#define PPU_RAM_SIZE (0x800)

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

// data bus pins shared with CPU
#define I8255_PIN_D0    (0)
#define I8255_PIN_D1    (1)
#define I8255_PIN_D2    (2)
#define I8255_PIN_D3    (3)
#define I8255_PIN_D4    (4)
#define I8255_PIN_D5    (4)
#define I8255_PIN_D6    (5)
#define I8255_PIN_D7    (7)

#define I8255_PIN_A0    (8)
#define I8255_PIN_A1    (9)
#define I8255_PIN_A2    (10)

static const int PictureBufferSize = (256*240);

static const int ScanlineCycleLength = 341;
static const int ScanlineEndCycle = 340;
static const int VisibleScanlines = 240;
static const int ScanlineVisibleDots = 256;
static const int FrameEndScanline = 261;

static const int AttributeOffset = 0x3C0;

typedef struct {
    uint8_t (*read)(uint16_t addr, void* user_data);
    void (*write)(uint16_t addr, uint8_t data, void* user_data);
    void (*scanlineIRQ)(void* user_data);
    uint8_t (*readPalette)(uint8_t paletteAddr, void* user_data);
    void (*vblankCallback)(void* user_data);
    void (*setPixel)(int x, int y, uint8_t color, void* user_data);
    void* user_data;

    uint8_t scanlineSprites[8];
    uint8_t spriteMemory[64*4];
    uint8_t pictureBuffer[PictureBufferSize];
    int nScanlineSprites;

    enum State
    {
        PreRender,
        Render,
        PostRender,
        VerticalBlank
    } pipelineState;

    int cycle;
    int scanline;
    bool evenFrame;

    bool vblank;
    bool sprZeroHit;
    bool spriteOverflow;

    //Registers
    uint16_t dataAddress;
    uint16_t tempAddress;
    uint8_t fineXScroll;
    bool firstWrite;
    uint8_t dataBuffer;

    uint8_t spriteDataAddress;

    //Setup flags and variables
    bool longSprites;
    bool generateInterrupt;

    bool greyscaleMode;
    bool showSprites;
    bool showBackground;
    bool hideEdgeSprites;
    bool hideEdgeBackground;

    enum CharacterPage
    {
        Low,
        High,
    } bgPage, sprPage;
    
    uint16_t dataAddrIncrement;
} r2c02_t;

typedef struct {
    uint8_t (*read)(uint16_t addr, void* user_data);
    void (*write)(uint16_t addr, uint8_t data, void* user_data);
    void (*scanlineIRQ)(void* user_data);
    uint8_t (*readPalette)(uint8_t paletteAddr, void* user_data);
    void (*vblankCallback)(void* user_data);
    void (*setPixel)(int x, int y, uint8_t color, void* user_data);
    void* user_data;
} r2c02_desc_t;

/* initialize a new r2c02_t instance */
void r2c02_init(r2c02_t* sys, const r2c02_desc_t* desc);
/* reset r2c02_t instance */
void r2c02_reset(r2c02_t* sys);
/* tick r2c02_t instance */
uint64_t r2c02_tick(r2c02_t* sys, uint64_t pins);
uint8_t r2c02_status(r2c02_t* sys);
uint8_t r2c02_data(r2c02_t* sys);
uint8_t r2c02_oam_data(r2c02_t* sys, uint16_t addr);
void r2c02_set_mask(r2c02_t* sys, uint8_t mask);
void r2c02_set_data(r2c02_t* sys, uint8_t data);

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void r2c02_init(r2c02_t* sys, const r2c02_desc_t* desc) {
    CHIPS_ASSERT(sys);
    memset(sys, 0, sizeof(*sys));
    sys->read = desc->read;
    sys->write = desc->write;
    sys->scanlineIRQ = desc->scanlineIRQ;
    sys->readPalette = desc->readPalette;
    sys->vblankCallback = desc->vblankCallback;
    sys->setPixel = desc->setPixel;
    sys->user_data = desc->user_data;
}

uint8_t r2c02_status(r2c02_t* sys) {
    uint8_t status = sys->spriteOverflow << 5 | sys->sprZeroHit << 6 | sys->vblank << 7;
    // Reading status clears vblank!
    sys->vblank = false;
    sys->firstWrite = true;
    return status;
}

inline static void _swap(uint8_t* d1, uint8_t* d2) {
    uint8_t tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;
}

uint8_t r2c02_data(r2c02_t* sys) {
    uint8_t data = sys->read(sys->dataAddress, sys->user_data);
    sys->dataAddress += sys->dataAddrIncrement;

    //Reads are delayed by one byte/read when address is in this range
    if (sys->dataAddress < 0x3f00)
    {
        //Return from the data buffer and store the current value in the buffer
        _swap(&data, &sys->dataBuffer);
    }

    return data;
}

void r2c02_control(r2c02_t* sys, uint8_t ctrl) {
    sys->generateInterrupt = ctrl & 0x80;
    sys->longSprites = ctrl & 0x20;
    sys->bgPage = (!!(ctrl & 0x10));
    sys->sprPage = (!!(ctrl & 0x8));
    if (ctrl & 0x4)
        sys->dataAddrIncrement = 0x20;
    else
        sys->dataAddrIncrement = 1;
    //m_baseNameTable = (ctrl & 0x3) * 0x400 + 0x2000;

    //Set the nametable in the temp address, this will be reflected in the data address during rendering
    sys->tempAddress &= ~0xc00;                 //Unset
    sys->tempAddress |= (ctrl & 0x3) << 10;     //Set according to ctrl bits
}

uint8_t r2c02_oam_data(r2c02_t* sys, uint16_t addr) {
    return sys->spriteMemory[addr];
}

void r2c02_set_mask(r2c02_t* sys, uint8_t mask) {
    sys->greyscaleMode = mask & 0x1;
    sys->hideEdgeBackground = !(mask & 0x2);
    sys->hideEdgeSprites = !(mask & 0x4);
    sys->showBackground = mask & 0x8;
    sys->showSprites = mask & 0x10;
}

void r2c02_set_scroll(r2c02_t* sys, uint8_t scroll) {
    if (sys->firstWrite)
    {
        sys->tempAddress &= ~0x1f;
        sys->tempAddress |= (scroll >> 3) & 0x1f;
        sys->fineXScroll = scroll & 0x7;
        sys->firstWrite = false;
    }
    else
    {
        sys->tempAddress &= ~0x73e0;
        sys->tempAddress |= ((scroll & 0x7) << 12) |
                         ((scroll & 0xf8) << 2);
        sys->firstWrite = true;
    }
}

void r2c02_set_data(r2c02_t* sys, uint8_t data) {
    sys->write(sys->dataAddress, data, sys->user_data);
    sys->dataAddress += sys->dataAddrIncrement;
}

uint64_t r2c02_tick(r2c02_t* sys, uint64_t pins) {
    CHIPS_ASSERT(sys);
    switch (sys->pipelineState)
    {
        case PreRender:
            if (sys->cycle == 1)
                sys->vblank = sys->sprZeroHit = false;
            else if (sys->cycle == ScanlineVisibleDots + 2 && sys->showBackground && sys->showSprites)
            {
                //Set bits related to horizontal position
                sys->dataAddress &= ~0x41f; //Unset horizontal bits
                sys->dataAddress |= sys->tempAddress & 0x41f; //Copy
            }
            else if (sys->cycle > 280 && sys->cycle <= 304 && sys->showBackground && sys->showSprites)
            {
                //Set vertical bits
                sys->dataAddress &= ~0x7be0; //Unset bits related to horizontal
                sys->dataAddress |= sys->tempAddress & 0x7be0; //Copy
            }
//                 if (sys->cycle > 257 && sys->cycle < 320)
//                     sys->spriteDataAddress = 0;
            //if rendering is on, every other frame is one cycle shorter
            if (sys->cycle >= ScanlineEndCycle - (!sys->evenFrame && sys->showBackground && sys->showSprites))
            {
                sys->pipelineState = Render;
                sys->cycle = sys->scanline = 0;
            }

            // add IRQ support for MMC3
            if(sys->cycle==260 && sys->showBackground && sys->showSprites){
                sys->scanlineIRQ(sys->user_data);
            }
            break;
        case Render:
            if (sys->cycle > 0 && sys->cycle <= ScanlineVisibleDots)
            {
                uint8_t bgColor = 0, sprColor = 0;
                bool bgOpaque = false, sprOpaque = true;
                bool spriteForeground = false;

                int x = sys->cycle - 1;
                int y = sys->scanline;

                if (sys->showBackground)
                {
                    int x_fine = (sys->fineXScroll + x) % 8;
                    if (!sys->hideEdgeBackground || x >= 8)
                    {
                        //fetch tile
                        uint16_t addr = 0x2000 | (sys->dataAddress & 0x0FFF); //mask off fine y
                        //auto addr = 0x2000 + x / 8 + (y / 8) * (ScanlineVisibleDots / 8);
                        uint8_t tile = sys->read(addr, sys->user_data);

                        //fetch pattern
                        //Each pattern occupies 16 bytes, so multiply by 16
                        addr = (tile * 16) + ((sys->dataAddress >> 12/*y % 8*/) & 0x7); //Add fine y
                        addr |= sys->bgPage << 12; //set whether the pattern is in the high or low page
                        //Get the corresponding bit determined by (8 - x_fine) from the right
                        bgColor = (sys->read(addr, sys->user_data) >> (7 ^ x_fine)) & 1; //bit 0 of palette entry
                        bgColor |= ((sys->read(addr + 8, sys->user_data) >> (7 ^ x_fine)) & 1) << 1; //bit 1

                        bgOpaque = bgColor; //flag used to calculate final pixel with the sprite pixel

                        //fetch attribute and calculate higher two bits of palette
                        addr = 0x23C0 | (sys->dataAddress & 0x0C00) | ((sys->dataAddress >> 4) & 0x38)
                                    | ((sys->dataAddress >> 2) & 0x07);
                        uint8_t attribute = sys->read(addr, sys->user_data);
                        int shift = ((sys->dataAddress >> 4) & 4) | (sys->dataAddress & 2);
                        //Extract and set the upper two bits for the color
                        bgColor |= ((attribute >> shift) & 0x3) << 2;
                    }
                    //Increment/wrap coarse X
                    if (x_fine == 7)
                    {
                        if ((sys->dataAddress & 0x001F) == 31) // if coarse X == 31
                        {
                            sys->dataAddress &= ~0x001F;          // coarse X = 0
                            sys->dataAddress ^= 0x0400;           // switch horizontal nametable
                        }
                        else
                        {
                            sys->dataAddress += 1;                // increment coarse X
                        }
                    }
                }

                if (sys->showSprites && (!sys->hideEdgeSprites || x >= 8))
                {
                    for (uint8_t ii = 0; ii<sys->nScanlineSprites; ++ii)
                    {
                        uint8_t i = sys->scanlineSprites[ii];
                        uint8_t spr_x =     sys->spriteMemory[i * 4 + 3];

                        if (0 > x - spr_x || x - spr_x >= 8)
                            continue;

                        uint8_t spr_y     = sys->spriteMemory[i * 4 + 0] + 1,
                                tile      = sys->spriteMemory[i * 4 + 1],
                                attribute = sys->spriteMemory[i * 4 + 2];

                        int length = (sys->longSprites) ? 16 : 8;

                        int x_shift = (x - spr_x) % 8, y_offset = (y - spr_y) % length;

                        if ((attribute & 0x40) == 0) //If NOT flipping horizontally
                            x_shift ^= 7;
                        if ((attribute & 0x80) != 0) //IF flipping vertically
                            y_offset ^= (length - 1);

                        uint16_t addr = 0;

                        if (!sys->longSprites)
                        {
                            addr = tile * 16 + y_offset;
                            if (sys->sprPage == High) addr += 0x1000;
                        }
                        else //8x16 sprites
                        {
                            //bit-3 is one if it is the bottom tile of the sprite, multiply by two to get the next pattern
                            y_offset = (y_offset & 7) | ((y_offset & 8) << 1);
                            addr = (tile >> 1) * 32 + y_offset;
                            addr |= (tile & 1) << 12; //Bank 0x1000 if bit-0 is high
                        }

                        sprColor |= (sys->read(addr, sys->user_data) >> (x_shift)) & 1; //bit 0 of palette entry
                        sprColor |= ((sys->read(addr + 8, sys->user_data) >> (x_shift)) & 1) << 1; //bit 1

                        if (!(sprOpaque = sprColor))
                        {
                            sprColor = 0;
                            continue;
                        }

                        sprColor |= 0x10; //Select sprite palette
                        sprColor |= (attribute & 0x3) << 2; //bits 2-3

                        spriteForeground = !(attribute & 0x20);

                        //Sprite-0 hit detection
                        if (!sys->sprZeroHit && sys->showBackground && i == 0 && sprOpaque && bgOpaque)
                        {
                            sys->sprZeroHit = true;
                        }

                        break; //Exit the loop now since we've found the highest priority sprite
                    }
                }

                uint8_t paletteAddr = bgColor;

                if ( (!bgOpaque && sprOpaque) ||
                        (bgOpaque && sprOpaque && spriteForeground) )
                    paletteAddr = sprColor;
                else if (!bgOpaque && !sprOpaque)
                    paletteAddr = 0;
                //else bgColor

                sys->pictureBuffer[x+y*256] = sys->readPalette(paletteAddr, sys->user_data);
            }
            else if (sys->cycle == ScanlineVisibleDots + 1 && sys->showBackground)
            {
                //Shamelessly copied from nesdev wiki
                if ((sys->dataAddress & 0x7000) != 0x7000)  // if fine Y < 7
                    sys->dataAddress += 0x1000;              // increment fine Y
                else
                {
                    sys->dataAddress &= ~0x7000;             // fine Y = 0
                    int y = (sys->dataAddress & 0x03E0) >> 5;    // let y = coarse Y
                    if (y == 29)
                    {
                        y = 0;                                // coarse Y = 0
                        sys->dataAddress ^= 0x0800;              // switch vertical nametable
                    }
                    else if (y == 31)
                        y = 0;                                // coarse Y = 0, nametable not switched
                    else
                        y += 1;                               // increment coarse Y
                    sys->dataAddress = (sys->dataAddress & ~0x03E0) | (y << 5);
                                                            // put coarse Y back into sys->dataAddress
                }
            }
            else if (sys->cycle == ScanlineVisibleDots + 2 && sys->showBackground && sys->showSprites)
            {
                //Copy bits related to horizontal position
                sys->dataAddress &= ~0x41f;
                sys->dataAddress |= sys->tempAddress & 0x41f;
            }

//                 if (sys->cycle > 257 && sys->cycle < 320)
//                     sys->spriteDataAddress = 0;

            // add IRQ support for MMC3
            if(sys->cycle==260 && sys->showBackground && sys->showSprites){
                sys->scanlineIRQ(sys->user_data);
            }

            if (sys->cycle >= ScanlineEndCycle)
            {
                //Find and index sprites that are on the next Scanline
                //This isn't where/when this indexing, actually copying in 2C02 is done
                //but (I think) it shouldn't hurt any games if this is done here

                sys->nScanlineSprites = 0;

                int range = 8;
                if (sys->longSprites)
                {
                    range = 16;
                }

                size_t j = 0;
                for (size_t i = sys->spriteDataAddress / 4; i < 64; ++i)
                {
                    int diff = (sys->scanline - sys->spriteMemory[i * 4]);
                    if (0 <= diff && diff < range)
                    {
                        if (j >= 8)
                        {
                            sys->spriteOverflow = true;
                            break;
                        }
                        sys->scanlineSprites[sys->nScanlineSprites++] = i;
                        ++j;
                    }
                }

                ++sys->scanline;
                sys->cycle = 0;
            }

            if (sys->scanline >= VisibleScanlines)
                sys->pipelineState = PostRender;

            break;
        case PostRender:
            if (sys->cycle >= ScanlineEndCycle)
            {
                ++sys->scanline;
                sys->cycle = 0;
                sys->pipelineState = VerticalBlank;

                for (int x = 0; x < 256; ++x)
                {
                    for (int y = 0; y < 240; ++y)
                    {
                        sys->setPixel(x, y, sys->pictureBuffer[x+y*256], sys->user_data);
                    }
                }

            }

            break;
        case VerticalBlank:
            if (sys->cycle == 1 && sys->scanline == VisibleScanlines + 1)
            {
                sys->vblank = true;
                if (sys->generateInterrupt) sys->vblankCallback(sys->user_data);
            }

            if (sys->cycle >= ScanlineEndCycle)
            {
                ++sys->scanline;
                sys->cycle = 0;
            }

            if (sys->scanline >= FrameEndScanline)
            {
                sys->pipelineState = PreRender;
                sys->scanline = 0;
                sys->evenFrame = !sys->evenFrame;
            }

            break;
        default:
            //LOG(Error) << "Well, this shouldn't have happened." << std::endl;
            break;
    }

    ++sys->cycle;
    return pins;
}

#endif /* CHIPS_IMPL */
