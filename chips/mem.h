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

    ## Functions
    ~~~C
    void mem_init(mem_t* mem);
    ~~~
    Initialize a new mem_t instance.

    ~~~C
    void mem_map_ram(mem_t* mem, int layer, uint16_t addr, uint32_t size, uint8_t* ptr)
    ~~~
    Map a range of host memory to a 16-bit address for RAM access in a
    given layer (0..3, 0 being the highest priority layer). Size is in bytes,
    must be a multiple of 0x0400 (decimal: 1024), and must be <= 0x10000 (decimal: 65536)

    ~~~C
    void mem_map_rom(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr)
    ~~~
    Map a range of host memory to a 16-bit address for ROM access.
    See mem_map_ram() for more details.

    ~~~C
    void mem_map_rw(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr)
    ~~~
    Map two host memory ranges to a 16-bit address for RAM-behind-ROM access.
    Read accesses will come from _read_ptr_, and write accesses will go
    to _write_ptr_. See mem_map_ram() for more details.

    ~~~C
    void mem_unmap_layer(mem_t* mem, int layer)
    ~~~
    Unmap all memory pages in a layer.

    ~~~C
    void mem_unmap_all(mem_t* mem)
    ~~~
    Unmap all memory pages in all layers.

    ~~~C
    uint8_t mem_rd(mem_t* mem, uint16_t addr)
    ~~~
    Read a byte from a 16-bit memory address from the CPU-visible
    memory page at that location. If the location is unmapped the read will
    come from the internal read-junk-page and  0xFF will be returned.

    ~~~C
    void mem_wr(mem_t* mem, uint16_t addr, uint8_t data)
    ~~~
    Write a byte to a 16-bit memory address to the CPU-visible memory
    page at that location. If the location is unmapped or ROM, the write
    will go the internal write-junk-page.

    ~~~C
    uint8_t* mem_readptr(mem_t* mem, uint16_t addr)
    ~~~
    A helper-function which returns the host-memory location of a 16-bit
    address for a read-access. Careful, this will return a pointer into the 
    internal read-junk-page if the page item is unmapped.

    ~~~C
    void mem_write_range(mem_t* mem, uint16_t addr, const uint8_t* src, int num_bytes)
    ~~~
    A helper function to copy a range of bytes from host memory to a 16-bit
    address range. This will do a series of mem_wr() calls. 

    ~~~C
    void mem_wr16(mem_t* mem, uint16_t addr, uint16_t data)
    ~~~
    A helper function to write a 16-bit value in little-endian format.
    This will do 2 calls to mem_wr().

    ~~~C
    uint16_t mem_rd16(mem_t* mem, uint16_t addr)
    ~~~
    A helper function to read a 16-bit value in little-endian format.
    This will do 2 calls to mem_rd().

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

#ifdef __cplusplus
extern "C" {
#endif

/* address range (64 KByte) */
#define MEM_ADDR_RANGE (1<<16)
#define MEM_ADDR_MASK (MEM_ADDR_RANGE-1)

/* page size (1 KByte) */
#define MEM_PAGE_SHIFT (10)
#define MEM_PAGE_SIZE (1<<MEM_PAGE_SHIFT)
#define MEM_PAGE_MASK (MEM_PAGE_SIZE-1)

#define MEM_NUM_PAGES (MEM_ADDR_RANGE / MEM_PAGE_SIZE)
#define MEM_NUM_LAYERS (4)

/* a memory page item maps a chunk of emulator memory to host memory */
typedef struct {
    const uint8_t* read_ptr;
    uint8_t* write_ptr;
} mem_page_t;

/* a memory instance is a 2-dimensional table of memory pages */
typedef struct {
    /* memory-mapped layers, layer 0 is highest priority */
    mem_page_t layers[MEM_NUM_LAYERS][MEM_NUM_PAGES];
    /* the pages that are actually visible to the emulated CPU */
    mem_page_t page_table[MEM_NUM_PAGES];
    /* a dummy page for currently unmapped memory */
    uint8_t unmapped_page[MEM_PAGE_SIZE];
    /* a write-only 'junk table' for writes to ROM areas */
    uint8_t junk_page[MEM_PAGE_SIZE];
} mem_t;

/* initialize a new mem instance */
void mem_init(mem_t* mem);
/* map a range of RAM */
void mem_map_ram(mem_t* mem, int layer, uint16_t addr, uint32_t size, uint8_t* ptr);
/* map a range of ROM */
void mem_map_rom(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr);
/* map a range of memory to different read/write pointers (e.g. for RAM behind ROM) */
void mem_map_rw(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr);
/* unmap all memory pages in a layer, also updates the CPU-visible page-table */
void mem_unmap_layer(mem_t* mem, int layer);
/* unmap all memory pages in all layers, also updates the CPU-visible page-table */
void mem_unmap_all(mem_t* mem);
/* get the host-memory read-ptr of an emulator memory address */
uint8_t* mem_readptr(mem_t* mem, uint16_t addr);
/* copy a range of bytes into memory via mem_wr() */
void mem_write_range(mem_t* mem, uint16_t addr, const uint8_t* src, int num_bytes);

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
uint8_t mem_layer_rd(mem_t* mem, int layer, uint16_t addr);
/* write a byte to a specific layer (slow!) */
void mem_layer_wr(mem_t* mem, int layer, uint16_t addr, uint8_t data);

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

void mem_init(mem_t* m) {
    CHIPS_ASSERT(m);
    memset(m, 0, sizeof(*m));
    memset(&m->unmapped_page, 0xFF, sizeof(m->unmapped_page));
    mem_unmap_all(m);
}

/* this sets the CPU-visible mapping of a page in the page-table */
static void _mem_update_page_table(mem_t* m, int page_index) {
    /* find highest priority layer which maps this memory page */
    int layer_index;
    for (layer_index = 0; layer_index < MEM_NUM_LAYERS; layer_index++) {
        if (m->layers[layer_index][page_index].read_ptr) {
            /* found highest priority layer with valid mapping */
            break;
        }
    }
    if (layer_index != MEM_NUM_LAYERS) { 
        /* found a valid mapping */
        m->page_table[page_index] = m->layers[layer_index][page_index];
    }
    else {
        /* no mapping exists for this page, set to special 'unmapped page' */
        m->page_table[page_index].read_ptr = m->unmapped_page;
        m->page_table[page_index].write_ptr = m->junk_page;
    }
}

static void _mem_map(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr) {
    CHIPS_ASSERT(m);
    CHIPS_ASSERT((layer >= 0) && (layer < MEM_NUM_LAYERS));
    CHIPS_ASSERT((addr & MEM_PAGE_MASK) == 0);
    CHIPS_ASSERT((size & MEM_PAGE_MASK) == 0);
    CHIPS_ASSERT(size <= MEM_ADDR_RANGE);
    const int num = size>>MEM_PAGE_SHIFT;
    CHIPS_ASSERT(num <= MEM_NUM_PAGES);
    for (int i = 0; i < num; i++) {
        const uint16_t offset = i * MEM_PAGE_SIZE;
        /* the page_index will wrap-around */
        const uint16_t page_index = ((addr+offset) & MEM_ADDR_MASK) >> MEM_PAGE_SHIFT;
        CHIPS_ASSERT(page_index <= MEM_NUM_PAGES);
        mem_page_t* page = &m->layers[layer][page_index];
        page->read_ptr = read_ptr + offset;
        if (0 != write_ptr) {
            page->write_ptr = write_ptr + offset;
        }
        else {
            page->write_ptr = m->junk_page;
        }
        _mem_update_page_table(m, page_index);
    }
}

void mem_map_ram(mem_t* m, int layer, uint16_t addr, uint32_t size, uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, ptr);
}

void mem_map_rom(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, 0);
}

void mem_map_rw(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr) {
    CHIPS_ASSERT(read_ptr && write_ptr);
    _mem_map(m, layer, addr, size, read_ptr, write_ptr);
}

void mem_unmap_layer(mem_t* m, int layer) {
    CHIPS_ASSERT(m);
    CHIPS_ASSERT((layer >= 0) && (layer < MEM_NUM_LAYERS));
    for (int page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
        mem_page_t* page = &m->layers[layer][page_index];
        page->read_ptr = 0; 
        page->write_ptr = 0;
        _mem_update_page_table(m, page_index);
    }
}

void mem_unmap_all(mem_t* m) {
    for (int layer_index = 0; layer_index < MEM_NUM_LAYERS; layer_index++) {
        for (int page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
            mem_page_t* page = &m->layers[layer_index][page_index];
            page->read_ptr = 0; 
            page->write_ptr = 0;
        }
    }
    for (int page_index = 0; page_index < MEM_NUM_PAGES; page_index++) {
        _mem_update_page_table(m, page_index);
    }
}

uint8_t* mem_readptr(mem_t* m, uint16_t addr) {
    CHIPS_ASSERT(m);
    return (uint8_t*) &(m->page_table[addr>>MEM_PAGE_SHIFT].read_ptr[addr&MEM_PAGE_MASK]);
} 

void mem_write_range(mem_t* m, uint16_t addr, const uint8_t* src, int num_bytes) {
    for (int i = 0; i < num_bytes; i++) {
        mem_wr(m, addr++, src[i]);
    }
}

uint8_t mem_layer_rd(mem_t* mem, int layer, uint16_t addr) {
    CHIPS_ASSERT((layer >= 0) && (layer < MEM_NUM_LAYERS));
    if (mem->layers[layer][addr>>MEM_PAGE_SHIFT].read_ptr) {
        return mem->layers[layer][addr>>MEM_PAGE_SHIFT].read_ptr[addr&MEM_PAGE_MASK];
    }
    else {
        return 0xFF;
    }
}

void mem_layer_wr(mem_t* mem, int layer, uint16_t addr, uint8_t data) {
    CHIPS_ASSERT((layer >= 0) && (layer < MEM_NUM_LAYERS));
    if (mem->layers[layer][addr>>MEM_PAGE_SHIFT].write_ptr) {
        mem->layers[layer][addr>>MEM_PAGE_SHIFT].write_ptr[addr&MEM_PAGE_MASK] = data;
    }
}

#endif /* CHIPS_IMPL */
