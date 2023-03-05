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

# define RGB(r,g,b) ((uint32_t)((uint32_t)(0xFF)<<24)|((uint32_t)(b)<<16)|((uint32_t)(g)<<8)|r)

// blargg's palette: https://www.nesdev.org/wiki/PPU_palettes
const uint32_t ppu_palette[] =
{
    RGB( 84,  84,  84), RGB( 0,   30, 116), RGB(  8,  16, 144), RGB( 48,   0, 136), RGB( 68,   0, 100), RGB( 92,   0,  48), RGB( 84,   4,   0), RGB( 60,  24,   0),
    RGB(32,   42,   0), RGB(  8,  58,   0), RGB(  0,  64,   0), RGB(  0,  60,   0), RGB(  0,  50,  60), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(152, 150, 152), RGB( 8,   76, 196), RGB( 48,  50, 236), RGB( 92,  30, 228), RGB(136,  20, 176), RGB(160,  20, 100), RGB(152,  34,  32), RGB(120,  60,   0),
    RGB(84,   90,   0), RGB( 40, 114,   0), RGB(  8, 124,   0), RGB(  0, 118,  40), RGB(  0, 102, 120), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(236, 238, 236), RGB( 76, 154, 236), RGB(120, 124, 236), RGB(176,  98, 236), RGB(228,  84, 236), RGB(236,  88, 180), RGB(236, 106, 100), RGB(212, 136,  32),
    RGB(160, 170,   0), RGB(116, 196,   0), RGB( 76, 208,  32), RGB( 56, 204, 108), RGB( 56, 180, 204), RGB( 60,  60,  60), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(236, 238, 236), RGB(168, 204, 236), RGB(188, 188, 236), RGB(212, 178, 236), RGB(236, 174, 236), RGB(236, 174, 212), RGB(236, 180, 176), RGB(228, 196, 144),
    RGB(204, 210, 120), RGB(180, 222, 120), RGB(168, 226, 144), RGB(152, 226, 180), RGB(160, 214, 228), RGB(160, 162, 160), RGB(  0,   0,   0), RGB(  0,   0,   0)
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
    void (*set_pixels)(uint8_t* buffer, void* user_data);
    void* user_data;

    uint8_t scanline_sprites[8];
    int scanline_sprites_num;
    uint8_t sprite_memory[64*4];
    uint8_t picture_buffer[PictureBufferSize];

    enum State
    {
        PreRender,
        Render,
        PostRender,
        VerticalBlank
    } pipelineState;

    int cycle;
    int scanline;
    bool even_frame;

    bool vblank;
    bool spr_zero_hit;
    bool sprite_overflow;

    //Registers
    uint16_t data_address;
    uint16_t temp_address;
    uint8_t fine_x_scroll;
    bool first_write;
    uint8_t data_buffer;

    uint8_t sprite_data_address;

    //Setup flags and variables
    bool long_sprites;
    bool generate_interrupt;
    bool request_nmi;

    bool greyscale_mode;
    bool show_sprites;
    bool show_background;
    bool hide_edge_sprites;
    bool hide_edge_background;

    enum CharacterPage
    {
        Low,
        High,
    } bg_page, spr_page;

    uint8_t data_addr_increment;
} r2c02_t;

typedef struct {
    uint8_t (*read)(uint16_t addr, void* user_data);
    void (*write)(uint16_t addr, uint8_t data, void* user_data);
    void (*set_pixels)(uint8_t* buffer, void* user_data);
    void* user_data;
} r2c02_desc_t;

/* initialize a new r2c02_t instance */
void r2c02_init(r2c02_t* sys, const r2c02_desc_t* desc);
/* reset r2c02_t instance */
void r2c02_reset(r2c02_t* sys);
/* tick r2c02_t instance */
uint64_t r2c02_tick(r2c02_t* sys, uint64_t pins);

uint8_t r2c02_read(r2c02_t* sys, uint8_t addr);
void r2c02_write(r2c02_t* sys, uint8_t addr, uint8_t data);

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

inline static void _swap(uint8_t* d1, uint8_t* d2);

void r2c02_init(r2c02_t* sys, const r2c02_desc_t* desc) {
    CHIPS_ASSERT(sys);
    memset(sys, 0, sizeof(*sys));
    sys->read = desc->read;
    sys->write = desc->write;
    sys->set_pixels = desc->set_pixels;
    sys->user_data = desc->user_data;
}

void r2c02_reset(r2c02_t* sys) {
    CHIPS_ASSERT(sys);
    sys->long_sprites = sys->generate_interrupt = sys->greyscale_mode = sys->vblank = sys->sprite_overflow = false;
    sys->show_background = sys->show_sprites = sys->even_frame = sys->first_write = true;
    sys->bg_page = sys->spr_page = Low;
    sys->data_address = sys->cycle = sys->scanline = sys->sprite_data_address = sys->fine_x_scroll = sys->temp_address = 0;
    sys->data_addr_increment = 1;
    sys->pipelineState = PreRender;
    sys->scanline_sprites_num = 0;
}

uint8_t r2c02_read(r2c02_t* sys, uint8_t addr) {
    switch(addr) {
        case 0x02: {
            // get PPU status
            sys->request_nmi = false;
            uint8_t status = sys->sprite_overflow << 5 | sys->spr_zero_hit << 6 | sys->vblank << 7;
            // Reading status clears vblank!
            sys->vblank = false;
            sys->first_write = true;
            return status;
        }
        case 0x04:
            // get OAM data
            return sys->sprite_memory[addr];
        case 0x07: {
            // get PPU data
            uint8_t data = sys->read(sys->data_address, sys->user_data);
            sys->data_address += sys->data_addr_increment;
            //Reads are delayed by one byte/read when address is in this range
            if (sys->data_address < 0x3f00) {
                //Return from the data buffer and store the current value in the buffer
                _swap(&data, &sys->data_buffer);
            }
            return data;
        }
        default: break;
    }
    return 0;
}

void r2c02_write(r2c02_t* sys, uint8_t addr, uint8_t data) {
    switch(addr) {
        case 0x00: {
            // set control
            sys->generate_interrupt = data & 0x80;
            sys->long_sprites = data & 0x20;
            sys->bg_page = (!!(data & 0x10));
            sys->spr_page = (!!(data & 0x8));
            if (data & 0x4)
                sys->data_addr_increment = 32;
            else
                sys->data_addr_increment = 1;

            //Set the nametable in the temp address, this will be reflected in the data address during rendering
            sys->temp_address &= ~0xc00;                 //Unset
            sys->temp_address |= (data & 0x3) << 10;     //Set according to ctrl bits
        } break;
        case 0x01: {
            // set mask
            sys->greyscale_mode = data & 0x1;
            sys->hide_edge_background = !(data & 0x2);
            sys->hide_edge_sprites = !(data & 0x4);
            sys->show_background = data & 0x8;
            sys->show_sprites = data & 0x10;
        } break;
        case 0x03: {
            // set OAM address
            sys->sprite_data_address = data;
        } break;
        case 0x04: {
            // set OAM data
            sys->sprite_memory[sys->sprite_data_address++] = data;
        } break;
        case 0x05: {
            // set scroll
            if (sys->first_write) {
                sys->temp_address &= ~0x1f;
                sys->temp_address |= (data >> 3) & 0x1f;
                sys->fine_x_scroll = data & 0x7;
                sys->first_write = false;
            } else {
                sys->temp_address &= ~0x73e0;
                sys->temp_address |= ((data & 0x7) << 12) |
                                 ((data & 0xf8) << 2);
                sys->first_write = true;
            }
        } break;
        case 0x06: {
            // set PPU address
            CHIPS_ASSERT(sys);
            if (sys->first_write) {
                sys->temp_address &= 0x00ff;
                sys->temp_address |= (data & 0x3f) << 8;
                sys->first_write = false;
            } else {
                sys->temp_address &= 0xff00;
                sys->temp_address |= data;
                sys->data_address = sys->temp_address;
                sys->first_write = true;
            }
        } break;
        case 0x07: {
            // set PPU data
            sys->write(sys->data_address, data, sys->user_data);
            sys->data_address += sys->data_addr_increment;
        } break;
        default: break;
    }
}

uint64_t r2c02_tick(r2c02_t* sys, uint64_t pins) {
    CHIPS_ASSERT(sys);
    switch (sys->pipelineState)
    {
        case PreRender:
            if (sys->cycle == 1)
                sys->vblank = sys->spr_zero_hit = false;
            else if (sys->cycle == ScanlineVisibleDots + 2 && sys->show_background && sys->show_sprites) {
                //Set bits related to horizontal position
                sys->data_address &= ~0x41f; //Unset horizontal bits
                sys->data_address |= sys->temp_address & 0x41f; //Copy
            }
            else if (sys->cycle > 280 && sys->cycle <= 304 && sys->show_background && sys->show_sprites) {
                //Set vertical bits
                sys->data_address &= ~0x7be0; //Unset bits related to horizontal
                sys->data_address |= sys->temp_address & 0x7be0; //Copy
            }
            //if rendering is on, every other frame is one cycle shorter
            if (sys->cycle >= ScanlineEndCycle - (!sys->even_frame && sys->show_background && sys->show_sprites)) {
                sys->pipelineState = Render;
                sys->cycle = sys->scanline = 0;
            }

            // add IRQ support for MMC3
            if(sys->cycle==260 && sys->show_background && sys->show_sprites) {
                // TODO: sys->scanline_irq(sys->user_data);
            }
            break;
        case Render:
            if (sys->cycle > 0 && sys->cycle <= ScanlineVisibleDots) {
                uint8_t bgColor = 0, sprColor = 0;
                bool bgOpaque = false, sprOpaque = true;
                bool spriteForeground = false;

                int x = sys->cycle - 1;
                int y = sys->scanline;

                if (sys->show_background) {
                    int x_fine = (sys->fine_x_scroll + x) % 8;
                    if (!sys->hide_edge_background || x >= 8) {
                        //fetch tile
                        uint16_t addr = 0x2000 | (sys->data_address & 0x0FFF); //mask off fine y
                        uint8_t tile = sys->read(addr, sys->user_data);

                        //fetch pattern
                        //Each pattern occupies 16 bytes, so multiply by 16
                        addr = (tile * 16) + ((sys->data_address >> 12) & 0x7); //Add fine y
                        addr |= sys->bg_page << 12; //set whether the pattern is in the high or low page
                        //Get the corresponding bit determined by (8 - x_fine) from the right
                        bgColor = (sys->read(addr, sys->user_data) >> (7 ^ x_fine)) & 1; //bit 0 of palette entry
                        bgColor |= ((sys->read(addr + 8, sys->user_data) >> (7 ^ x_fine)) & 1) << 1; //bit 1

                        bgOpaque = bgColor; //flag used to calculate final pixel with the sprite pixel

                        //fetch attribute and calculate higher two bits of palette
                        addr = 0x23C0 | (sys->data_address & 0x0C00) | ((sys->data_address >> 4) & 0x38)
                                    | ((sys->data_address >> 2) & 0x07);
                        uint8_t attribute = sys->read(addr, sys->user_data);
                        int shift = ((sys->data_address >> 4) & 4) | (sys->data_address & 2);
                        //Extract and set the upper two bits for the color
                        bgColor |= ((attribute >> shift) & 0x3) << 2;
                    }
                    //Increment/wrap coarse X
                    if (x_fine == 7) {
                        if ((sys->data_address & 0x001F) == 31) {  // if coarse X == 31
                            sys->data_address &= ~0x001F;          // coarse X = 0
                            sys->data_address ^= 0x0400;           // switch horizontal nametable
                        }
                        else {
                            sys->data_address++;                // increment coarse X
                        }
                    }
                }

                 if (sys->show_sprites && (!sys->hide_edge_sprites || x >= 8)) {
                     for (uint8_t ii = 0; ii<sys->scanline_sprites_num; ++ii) {
                         uint8_t i = sys->scanline_sprites[ii];
                         uint8_t spr_x = sys->sprite_memory[i * 4 + 3];

                         if (0 > x - spr_x || x - spr_x >= 8)
                             continue;

                         uint8_t spr_y     = sys->sprite_memory[i * 4 + 0] + 1,
                                 tile      = sys->sprite_memory[i * 4 + 1],
                                 attribute = sys->sprite_memory[i * 4 + 2];

                         int length = (sys->long_sprites) ? 16 : 8;
                         int x_shift = (x - spr_x) % 8, y_offset = (y - spr_y) % length;

                         if ((attribute & 0x40) == 0) //If NOT flipping horizontally
                             x_shift ^= 7;
                         if ((attribute & 0x80) != 0) //IF flipping vertically
                             y_offset ^= (length - 1);

                         uint16_t addr = 0;

                         if (!sys->long_sprites) {
                             addr = tile * 16 + y_offset;
                             if (sys->spr_page == High) addr += 0x1000;
                         }
                         else { //8x16 sprites
                             //bit-3 is one if it is the bottom tile of the sprite, multiply by two to get the next pattern
                             y_offset = (y_offset & 7) | ((y_offset & 8) << 1);
                             addr = (tile >> 1) * 32 + y_offset;
                             addr |= (tile & 1) << 12; //Bank 0x1000 if bit-0 is high
                         }

                         sprColor |= (sys->read(addr, sys->user_data) >> (x_shift)) & 1; //bit 0 of palette entry
                         sprColor |= ((sys->read(addr + 8, sys->user_data) >> (x_shift)) & 1) << 1; //bit 1

                         if (!(sprOpaque = sprColor)) {
                             sprColor = 0;
                             continue;
                         }

                         sprColor |= 0x10; //Select sprite palette
                         sprColor |= (attribute & 0x3) << 2; //bits 2-3

                         spriteForeground = !(attribute & 0x20);

                         //Sprite-0 hit detection
                         if (!sys->spr_zero_hit && sys->show_background && i == 0 && sprOpaque && bgOpaque) {
                             sys->spr_zero_hit = true;
                         }

                         break; //Exit the loop now since we've found the highest priority sprite
                     }
                 }

                uint8_t paletteAddr = bgColor;
                if ((!bgOpaque && sprOpaque) || (bgOpaque && sprOpaque && spriteForeground))
                    paletteAddr = sprColor;
                else if (!bgOpaque && !sprOpaque)
                    paletteAddr = 0;

                sys->picture_buffer[x+y*256] = sys->read(0x3f00 + paletteAddr, sys->user_data);
            }
            else if (sys->cycle == ScanlineVisibleDots + 1 && sys->show_background) {
                //Shamelessly copied from nesdev wiki
                if ((sys->data_address & 0x7000) != 0x7000)  // if fine Y < 7
                    sys->data_address += 0x1000;              // increment fine Y
                else {
                    sys->data_address &= ~0x7000;             // fine Y = 0
                    int y = (sys->data_address & 0x03E0) >> 5;    // let y = coarse Y
                    if (y == 29)
                    {
                        y = 0;                                // coarse Y = 0
                        sys->data_address ^= 0x0800;              // switch vertical nametable
                    }
                    else if (y == 31)
                        y = 0;                                // coarse Y = 0, nametable not switched
                    else
                        y += 1;                               // increment coarse Y
                    sys->data_address = (sys->data_address & ~0x03E0) | (y << 5);
                                                            // put coarse Y back into sys->dataAddress
                }
            }
            else if (sys->cycle == ScanlineVisibleDots + 2 && sys->show_background && sys->show_sprites) {
                //Copy bits related to horizontal position
                sys->data_address &= ~0x41f;
                sys->data_address |= sys->temp_address & 0x41f;
            }

            // add IRQ support for MMC3
            if(sys->cycle==260 && sys->show_background && sys->show_sprites) {
                // TODO: sys->scanline_irq(sys->user_data);
            }

            if (sys->cycle >= ScanlineEndCycle) {
                //Find and index sprites that are on the next Scanline
                //This isn't where/when this indexing, actually copying in 2C02 is done
                //but (I think) it shouldn't hurt any games if this is done here

                sys->scanline_sprites_num = 0;
                int range = 8;
                if (sys->long_sprites) {
                    range = 16;
                }

                size_t j = 0;
                for (size_t i = sys->sprite_data_address / 4; i < 64; ++i) {
                    int diff = (sys->scanline - sys->sprite_memory[i * 4]);
                    if (0 <= diff && diff < range) {
                        if (j >= 8) {
                            sys->sprite_overflow = true;
                            break;
                        }
                        sys->scanline_sprites[sys->scanline_sprites_num++] = i;
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
            if (sys->cycle >= ScanlineEndCycle) {
                ++sys->scanline;
                sys->cycle = 0;
                sys->pipelineState = VerticalBlank;
                sys->set_pixels(sys->picture_buffer, sys->user_data);
            }

            break;
        case VerticalBlank:
            if (sys->cycle == 1 && sys->scanline == VisibleScanlines + 1) {
                sys->vblank = true;
                if (sys->generate_interrupt)
                    sys->request_nmi = true;
            }

            if (sys->cycle >= ScanlineEndCycle) {
                ++sys->scanline;
                sys->cycle = 0;
            }

            if (sys->scanline >= FrameEndScanline) {
                sys->pipelineState = PreRender;
                sys->scanline = 0;
                sys->even_frame = !sys->even_frame;
            }

            break;
        default:
            // Well, this shouldn't have happened.
            assert(false);
            break;
    }

    ++sys->cycle;
    return pins;
}

inline static void _swap(uint8_t* d1, uint8_t* d2) {
    uint8_t tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;
}

#endif /* CHIPS_IMPL */
