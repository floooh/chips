#pragma once
/*
    mem.h   -- memory implementation for 8-bit systems

    TODO: documentation.
*/
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

/* a callback pointer for memory-mapped-I/O areas */
typedef uint8_t (*memio_t)(bool write, uint16_t addr, uint8_t inval);

/* a memory page item maps a chunk of emulator memory to host memory */
typedef struct {
    union {
        const uint8_t* read_ptr;
        memio_t memio_cb;
    };
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
extern void mem_init(mem_t* mem);
/* map a range of RAM */
extern void mem_map_ram(mem_t* mem, int layer, uint16_t addr, uint32_t size, uint8_t* ptr);
/* map a range of ROM */
extern void mem_map_rom(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr);
/* map a range of memory to different read/write pointers (e.g. for RAM behind ROM) */
extern void mem_map_rw(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr);
/* map a range of memory to a memory-mapped-io callback function */
extern void mem_map_io(mem_t* mem, int layer, uint16_t addr, uint32_t size, memio_t cb);
/* unmap all memory pages in a layer, also updates the CPU-visible page-table */
extern void mem_unmap_layer(mem_t* mem, int layer);
/* unmap all memory pages in all layers, also updates the CPU-visible page-table */
extern void mem_unmap_all(mem_t* mem);
/* get the host-memory read-ptr of a emulator memory address */
extern const uint8_t* mem_readptr(mem_t* mem, uint16_t addr);
/* copy a range of bytes into memory via mem_wr() */
extern void mem_write_range(mem_t* mem, uint16_t addr, const uint8_t* src, int num_bytes);

/* read a byte at address without memory-mapped-io support */
static inline uint8_t mem_rd(const mem_t* mem, uint16_t addr) {
    return mem->page_table[addr>>MEM_PAGE_SHIFT].read_ptr[addr & MEM_PAGE_MASK];
}
/* read a byte at address with memory-mapped-io support */
static inline uint8_t mem_rdio(const mem_t* mem, uint16_t addr) {
    const mem_page_t* page = &mem->page_table[addr>>MEM_PAGE_SHIFT];
    if (page->write_ptr) {
        /* a regular memory page */
        return page->read_ptr[addr & MEM_PAGE_MASK];
    }
    else {
        /* a memory-mapped-io page, invoke callback */
        return page->memio_cb(false, addr, 0);
    }
}
/* write a byte at address without memory-mapped-io support */
static inline void mem_wr(mem_t* mem, uint16_t addr, uint8_t data) {
    mem->page_table[addr>>MEM_PAGE_SHIFT].write_ptr[addr & MEM_PAGE_MASK] = data;
}
/* write a byte at address with memory-mapped-io support */
static inline void mem_wrio(mem_t* mem, uint16_t addr, uint8_t data) {
    mem_page_t* page = &mem->page_table[addr>>MEM_PAGE_SHIFT];
    if (page->write_ptr) {
        /* a regular memory page */
        page->write_ptr[addr&MEM_PAGE_MASK] = data;
    }
    else {
        /* a memory-mapped-io page, invoke callback */
        page->memio_cb(true, addr, data);
    }
}
/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
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

static void _mem_map(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr, memio_t cb) {
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
        if (cb) {
            CHIPS_ASSERT(!read_ptr && !write_ptr);
            page->memio_cb = cb;
            page->write_ptr = 0;
        }
        else {
            page->read_ptr = read_ptr + offset;
            if (0 != write_ptr) {
                page->write_ptr = write_ptr + offset;
            }
            else {
                page->write_ptr = m->junk_page;
            }
        }
        _mem_update_page_table(m, page_index);
    }
}

void mem_map_ram(mem_t* m, int layer, uint16_t addr, uint32_t size, uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, ptr, 0);
}

void mem_map_rom(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr) {
    CHIPS_ASSERT(ptr);
    _mem_map(m, layer, addr, size, ptr, 0, 0);
}

void mem_map_rw(mem_t* m, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr) {
    CHIPS_ASSERT(read_ptr && write_ptr);
    _mem_map(m, layer, addr, size, read_ptr, write_ptr, 0);
}

void mem_map_io(mem_t* m, int layer, uint16_t addr, uint32_t size, memio_t cb) {
    CHIPS_ASSERT(cb);
    _mem_map(m, layer, addr, size, 0, 0, cb);
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

const uint8_t* mem_readptr(mem_t* m, uint16_t addr) {
    CHIPS_ASSERT(m);
    return &(m->page_table[addr>>MEM_PAGE_SHIFT].read_ptr[addr&MEM_PAGE_MASK]);
} 

void mem_write_range(mem_t* m, uint16_t addr, const uint8_t* src, int num_bytes) {
    for (int i = 0; i < num_bytes; i++) {
        mem_wr(m, addr++, src[i]);
    }
}
#endif /* CHIPS_IMPL */
#ifdef __cplusplus
} /* extern "C" */
#endif
