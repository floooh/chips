#pragma once
/*#
    # mem.h

    memory system for emulated 8-bit computers

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macro with your own implementation
    (default: assert(c))

    ~~~C
    CHIPS_ASSERT(c)
    ~~~

    ## Feature Overview

    - maps 16-bit addresses to host system addresses with 1 KByte page-size
      granularity
    - memory pages can be mapped as RAM, ROM or RAM-behind-ROM (where
      read accesses are mapped to a different memory page then write accesses)
    - 4 independent page-table layers to simplify bank-switching implementations

    ## Usage

    1. call **mem_init()** to initialize a mem_t instance
    2. call **mem_map_ram()**, **mem_map_rom()** and **mem_map_rw()** to
       initialize the mapping from the 16-bit address space to host memory
       locations.
    3. call **mem_rd()** and **mem_wr()** to read and write bytes from and
       to the 16-bit address space
    4. if needed, call the functions from step (2) to change the memory
       mapping (for instance to switch memory banks in and out of the
       16-bit address space)

    ## Layers, Pages and mapping to CPU-visible addresses

    ****************************************************************************
    *               Page 0   Page 1   Page 2   Page 3   Page 4  ...            *
    *             +--------+--------+                                          *
    *     Layer 3 |   30   |   31   |                                          *
    *             +--------+--------+--------+--------+--------+----           *
    *     Layer 2 |   20   |        |   22   |   23   |   24   |               *
    *             +--------+        +--------+--------+--------+----           *
    *     Layer 1 |   10   |        |   12   |   13   |                        *
    *             +--------+        +--------+--------+                        *
    *     Layer 0 |   00   |        |   02   |   03   |                        *
    *             +--------+        +--------+--------+                        *
    *                                                                          *
    *             +--------+--------+--------+--------+--------+----           *
    * CPU Visible |   00   |   31   |   02   |   03   |   24   |               *
    *             +--------+--------+--------+--------+--------+----           *
    *             0x0000   0x0400   0x0800   0x1000   0x1400   0x1800          *
    ****************************************************************************


    Each layer is an array of 64 page items (one page item covers 1 KByte of memory).

    The CPU sees the highest priority valid page items (where layer 0 is
    highest priority and layer 3 is lowest priority).

    Each page item consists of two host system pointers, one for read access,
    and one for write access.

    There are 2 internal special 'junk pages', one for write accesses to
    read-only-memory or unmapped memory, and one for read-access from unmapped
    memory. A read access from unmapped memory always returns 0xFF.

    The different page-mapping scenarios are then implemented as follows:

    - **RAM**: both read- and write-pointers point to the same host-memory
      location
    - **ROM**: the read-pointer points to a host-memory-location, and the
      write-pointer points to the junk-write-page
    - **RAM-behind-ROM**: the read- and write-pointers point to _different_
      host-memory locations
    - **unmapped page**: the read-pointer points to the internal junk-read-page, and
      the write-pointer to the internal junk-write-page

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* address range (64 KByte) */
#define MEM_ADDR_RANGE (1U<<16)
#define MEM_ADDR_MASK (MEM_ADDR_RANGE-1)

/* page size (1 KByte) */
#define MEM_PAGE_SHIFT (10U)
#define MEM_PAGE_SIZE (1U<<MEM_PAGE_SHIFT)
#define MEM_PAGE_MASK (MEM_PAGE_SIZE-1)

#define MEM_NUM_PAGES (MEM_ADDR_RANGE / MEM_PAGE_SIZE)
#define MEM_NUM_LAYERS (4U)

/* a memory page item maps a chunk of emulator memory to host memory */
typedef struct {
    uint8_t* read_ptr;
    uint8_t* write_ptr;
} mem_page_t;

/* a memory instance is a 2-dimensional table of memory pages */
typedef struct {
    /* the pages that are actually visible to the emulated CPU */
    mem_page_t page_table[MEM_NUM_PAGES];
    /* memory-mapped layers, layer 0 is highest priority */
    mem_page_t layers[MEM_NUM_LAYERS][MEM_NUM_PAGES];
} mem_t;

/* initialize a new mem instance */
void mem_init(mem_t* mem);
/* map a range of RAM */
void mem_map_ram(mem_t* mem, size_t layer, uint16_t addr, uint32_t size, uint8_t* ptr);
/* map a range of ROM */
void mem_map_rom(mem_t* mem, size_t layer, uint16_t addr, uint32_t size, const uint8_t* ptr);
/* map a range of memory to different read/write pointers (e.g. for RAM behind ROM) */
void mem_map_rw(mem_t* mem, size_t layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr);
/* unmap all memory pages in a layer, also updates the CPU-visible page-table */
void mem_unmap_layer(mem_t* mem, size_t layer);
/* unmap all memory pages in all layers, also updates the CPU-visible page-table */
void mem_unmap_all(mem_t* mem);
/* get the host-memory read-ptr of an emulator memory address */
uint8_t* mem_readptr(mem_t* mem, uint16_t addr);
/* copy a range of bytes into memory via mem_wr() */
void mem_write_range(mem_t* mem, uint16_t addr, const uint8_t* src, uint32_t num_bytes);

/* read a byte at 16-bit address */
static inline uint8_t mem_rd(mem_t* mem, uint16_t addr) {
    return mem->page_table[addr>>MEM_PAGE_SHIFT].read_ptr[addr & MEM_PAGE_MASK];
}
/* write a byte to 16-bit address */
static inline void mem_wr(mem_t* mem, uint16_t addr, uint8_t data) {
    mem->page_table[addr>>MEM_PAGE_SHIFT].write_ptr[addr & MEM_PAGE_MASK] = data;
}
/* helper method to write a 16-bit value, does 2 mem_wr() */
static inline void mem_wr16(mem_t* mem, uint16_t addr, uint16_t data) {
    mem_wr(mem, addr, (uint8_t)data);
    mem_wr(mem, addr+1, (uint8_t)(data>>8));
}
/* helper method to read a 16-bit value, does 2 mem_rd() */
static inline uint16_t mem_rd16(mem_t* mem, uint16_t addr) {
    uint8_t l = mem_rd(mem, addr);
    uint8_t h = mem_rd(mem, addr+1);
    return (h<<8)|l;
}

/* read a byte from a specific layer (slow!) */
uint8_t mem_layer_rd(mem_t* mem, size_t layer, uint16_t addr);
/* write a byte to a specific layer (slow!) */
void mem_layer_wr(mem_t* mem, size_t layer, uint16_t addr, uint8_t data);

/* convert any internal pointers to offsets (helper function for serialization) */
void mem_snapshot_onsave(mem_t* snapshot, void* base);
/* ...and the reverse */
void mem_snapshot_onload(mem_t* snapshot, void* base);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

// a dummy page for currently unmapped memory
static uint8_t _mem_unmapped_page[MEM_PAGE_SIZE];
// a write-only 'junk table' for writes to ROM areas
static uint8_t _mem_junk_page[MEM_PAGE_SIZE];

void mem_init(mem_t* m) {
    CHIPS_ASSERT(m);
    *m = (mem_t){0};
    memset(_mem_unmapped_page, 0xFF, sizeof(_mem_unmapped_page));
    mem_unmap_all(m);
}

/* this sets the CPU-visible mapping of a page in the page-table */
static void _mem_update_page_table(mem_t* m, size_t page_index) {
    /* find highest priority layer which maps this memory page */
    size_t layer_index;
    for (layer_index = 0; layer_index < MEM_NUM_LAYERS; layer_index++) {
        if (m->layers[layer_index][page_index].read_ptr) {
            /* found highest priority layer with valid mapping */
            break;
        }
    }
    if (layer_index != MEM_NUM_LAYERS) {
        /* found a valid mapping */

        /*
            FIXME FIXME FIXME

            The following lines triggers a code generation problem in Xcode 14.2
            (also 14.0 and 14.1) resulting in a nullptr access because this line
            doesn't seem to be executed resulting in all read_ptr/write_ptr to be
            zero. Happens when called from within _kc85_update_memory_map() with
            at least -O2. After 2 days of investigation it's still unclear whether
            this is a bug in Clang or actual UB. A few facts from the investigation:

            - compiling with -fwrapv fixes the problem (yet no case of
              signed integer overflow could be found going up the callstack,
              UBSAN is also clean) - frwrapv seems to disable any loop unrolling
              optimizations
            - replacing the struct assignment below with memcpy fixes the problem
            - __attribute__((noinline)) fixes the problem
            - compiling with UBSAN fixes the problem(!!!) (but neither UBSAN nor
              ASAN trigger anything)
            - compiling with more recent clang versions (e.g. to WASM with Emscripten
              or 'zig cc' also fixes the problem (but in the case of 'zig cc' it might
              also be because zig uses different default compilation options?

            TODO: check again once Xcode 15 is released (assuming this finally updated
            clang)
        */
        // m->page_table[page_index] = m->layers[layer_index][page_index];

        m->page_table[page_index].read_ptr = m->layers[layer_index][page_index].read_ptr;
        m->page_table[page_index].write_ptr = m->layers[layer_index][page_index].write_ptr;
    }
    else {
        /* no mapping exists for this page, set to special 'unmapped page' */
        m->page_table[page_index].read_ptr = _mem_unmapped_page;
        m->page_table[page_index].write_ptr = _mem_junk_page;
    }
}

static void _mem_map(mem_t* m, size_t layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr) {
    CHIPS_ASSERT(m);
    CHIPS_ASSERT(layer < MEM_NUM_LAYERS);
    CHIPS_ASSERT((addr & MEM_PAGE_MASK) == 0);
    CHIPS_ASSERT((size & MEM_PAGE_MASK) == 0);
    CHIPS_ASSERT(size <= MEM_ADDR_RANGE);
    const size_t num = size>>MEM_PAGE_SHIFT;
    CHIPS_ASSERT(num <= MEM_NUM_PAGES);
    for (size_t i = 0; i < num; i++) {
        const uint16_t offset = i * MEM_PAGE_SIZE;
        // the page_index will wrap-around
        const uint16_t page_index = ((addr+offset) & MEM_ADDR_MASK) >> MEM_PAGE_SHIFT;
        CHIPS_ASSERT(page_index <= MEM_NUM_PAGES);
        mem_page_t* page = &m->layers[layer][page_index];
        page->read_ptr = (uint8_t*)read_ptr + offset;
        if (0 != write_ptr) {
            page->write_ptr = write_ptr + offset;
        }
        else {
            page->write_ptr = _mem_junk_page;
        }
        _mem_update_page_table(m, page_index);
    }
}

void mem_map_ram(mem_t* m, size_t layer, uint16_t addr, uint32_t size, uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, ptr);
}

void mem_map_rom(mem_t* m, size_t layer, uint16_t addr, uint32_t size, const uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, 0);
}

void mem_map_rw(mem_t* m, size_t layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr) {
    CHIPS_ASSERT(read_ptr && write_ptr);
    _mem_map(m, layer, addr, size, read_ptr, write_ptr);
}

void mem_unmap_layer(mem_t* m, size_t layer) {
    CHIPS_ASSERT(m);
    CHIPS_ASSERT(layer < MEM_NUM_LAYERS);
    for (size_t page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
        mem_page_t* page = &m->layers[layer][page_index];
        page->read_ptr = 0;
        page->write_ptr = 0;
        _mem_update_page_table(m, page_index);
    }
}

void mem_unmap_all(mem_t* m) {
    for (size_t layer_index = 0; layer_index < MEM_NUM_LAYERS; layer_index++) {
        for (size_t page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
            mem_page_t* page = &m->layers[layer_index][page_index];
            page->read_ptr = 0;
            page->write_ptr = 0;
        }
    }
    for (size_t page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
        _mem_update_page_table(m, page_index);
    }
}

uint8_t* mem_readptr(mem_t* m, uint16_t addr) {
    CHIPS_ASSERT(m);
    return (uint8_t*) &(m->page_table[addr>>MEM_PAGE_SHIFT].read_ptr[addr&MEM_PAGE_MASK]);
}

void mem_write_range(mem_t* m, uint16_t addr, const uint8_t* src, uint32_t num_bytes) {
    for (size_t i = 0; i < num_bytes; i++) {
        mem_wr(m, addr++, src[i]);
    }
}

uint8_t mem_layer_rd(mem_t* mem, size_t layer, uint16_t addr) {
    CHIPS_ASSERT(layer < MEM_NUM_LAYERS);
    if (mem->layers[layer][addr>>MEM_PAGE_SHIFT].read_ptr) {
        return mem->layers[layer][addr>>MEM_PAGE_SHIFT].read_ptr[addr&MEM_PAGE_MASK];
    }
    else {
        return 0xFF;
    }
}

void mem_layer_wr(mem_t* mem, size_t layer, uint16_t addr, uint8_t data) {
    CHIPS_ASSERT(layer < MEM_NUM_LAYERS);
    if (mem->layers[layer][addr>>MEM_PAGE_SHIFT].write_ptr) {
        mem->layers[layer][addr>>MEM_PAGE_SHIFT].write_ptr[addr&MEM_PAGE_MASK] = data;
    }
}

#define MEM_SPECIAL_OFFSET_NULLPTR (-1)
#define MEM_SPECIAL_OFFSET_UNMAPPED_PAGE (-2)
#define MEM_SPECIAL_OFFSET_JUNK_PAGE (-3)

static void mem_ptr_to_offset(uint8_t** ptr_ptr, uint8_t* base) {
    uint8_t* ptr = *ptr_ptr;
    if (ptr == 0) {
        *ptr_ptr = (uint8_t*)(intptr_t)MEM_SPECIAL_OFFSET_NULLPTR;
    }
    else if (ptr == _mem_unmapped_page) {
        *ptr_ptr = (uint8_t*)(intptr_t)MEM_SPECIAL_OFFSET_UNMAPPED_PAGE;
    }
    else if (ptr == _mem_junk_page) {
        *ptr_ptr = (uint8_t*)(intptr_t)MEM_SPECIAL_OFFSET_JUNK_PAGE;
    }
    else {
        CHIPS_ASSERT(base <= *ptr_ptr);
        *ptr_ptr = (uint8_t*) (*ptr_ptr - base);
    }
}

static void mem_offset_to_ptr(uint8_t** ptr_ptr, uint8_t* base) {
    intptr_t offset = (intptr_t)*ptr_ptr;
    switch (offset) {
        case MEM_SPECIAL_OFFSET_NULLPTR:
            *ptr_ptr = 0;
            break;
        case MEM_SPECIAL_OFFSET_UNMAPPED_PAGE:
            *ptr_ptr = _mem_unmapped_page;
            break;
        case MEM_SPECIAL_OFFSET_JUNK_PAGE:
            *ptr_ptr = _mem_junk_page;
            break;
        default:
            *ptr_ptr = (base + offset);
            break;
    }
}

void mem_snapshot_onsave(mem_t* snapshot, void* base) {
    uint8_t* base8 = (uint8_t*)base;
    for (size_t page = 0; page < MEM_NUM_PAGES; page++) {
        mem_ptr_to_offset(&snapshot->page_table[page].read_ptr, base8);
        mem_ptr_to_offset(&snapshot->page_table[page].write_ptr, base8);
    }
    for (size_t layer = 0; layer < MEM_NUM_LAYERS; layer++) {
        for (size_t page = 0; page < MEM_NUM_PAGES; page++) {
            mem_ptr_to_offset(&snapshot->layers[layer][page].read_ptr, base8);
            mem_ptr_to_offset(&snapshot->layers[layer][page].write_ptr, base8);
        }
    }
}

void mem_snapshot_onload(mem_t* snapshot, void* base) {
    uint8_t* base8 = (uint8_t*)base;
    for (size_t page = 0; page < MEM_NUM_PAGES; page++) {
        mem_offset_to_ptr(&snapshot->page_table[page].read_ptr, base8);
        mem_offset_to_ptr(&snapshot->page_table[page].write_ptr, base8);
    }
    for (size_t layer = 0; layer < MEM_NUM_LAYERS; layer++) {
        for (size_t page = 0; page < MEM_NUM_PAGES; page++) {
            mem_offset_to_ptr(&snapshot->layers[layer][page].read_ptr, base8);
            mem_offset_to_ptr(&snapshot->layers[layer][page].write_ptr, base8);
        }
    }
}

#endif /* CHIPS_IMPL */
