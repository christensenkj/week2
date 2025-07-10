/*
MIT License

Copyright (c) 2025 pointerwars2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SLAB_ALLOCATOR_H_
#define SLAB_ALLOCATOR_H_

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

/* My use case 
Node Size: All my allocations will be for list nodes, which are fixed-size.
Allocation Pattern: Frequent allocations and deallocations, but always for the same size.
No need for general-purpose malloc/free: I can optimize for the specific linked_list pattern.

For fixed-size allocations, the free-list allocator is ideal:
Pre-allocate a large block of memory (an array of nodes).
Maintain a free list of available nodes.
Allocate: Pop a node from the free list.
Free: Push the node back onto the free list.

One caveat: must provide a way to allocate additional chunk memory if our
initial malloc wasn't large enough. A simple slab allocation scheme should
suffice for this simple example.

Notes for slab sizing
    Arm Cortex A72 specs:
    L1 dcache: 32 KB, L2 cache: 512 KB
    L1 dcache line size: 64 bytes
*/

/* Each slab should take up about 1/4 of the cache */
#define SLAB_SIZE   (32 * 1024)
#define MAX_SLABS   (128 * 1024)

/* Supported allocation sizes in bytes as an X macro.
   Add new sizes to support here, no need to update 
   elsewhere. */
#define SUPPORTED_SIZES_DEF(_func, ...) \
    _func(16, ##__VA_ARGS__), \
    _func(24, ##__VA_ARGS__), \
    _func(32, ##__VA_ARGS__),

/* Define an array of supported sizes. Automatically
   resizes with the above X macro. */
#define SUPPORTED_SIZES_ELEM(_size) _size
#define SUPPORTED_SIZES_ARRAY() \
    uint32_t supported_sizes[MAX_SUPPORTED_SIZES] = { \
        SUPPORTED_SIZES_DEF(SUPPORTED_SIZES_ELEM) \
    }

/* Enum of supported sizes */
#define SUPPORTED_SIZES_ENUM(_size)   SIZE_##_size
typedef enum {
    SUPPORTED_SIZES_DEF(SUPPORTED_SIZES_ENUM)
    MAX_SUPPORTED_SIZES,
} slab_supported_sizes_t;

/* A slab is a fixed-size chunk of memory that's allocated using 
   stdlib malloc. The chunk exists as a list of nodes, sized according
   to allocatable sizes. */

/* Slab node struct. Represents a single allocatable node. */
struct free_node {
    uint32_t alloc_size;
    struct free_node *next;
};

/* Slab struct. Represents a slab of nodes, allocated
   infrequently. */
struct slab {
    struct free_node *pool;
    struct free_node *free_list;
    uint32_t size; // size of the whole slab in bytes
    uint32_t num_nodes;
    uint32_t used;
    struct slab *next;
    struct slab *prev;
};

/* Slab allocator struct. Comprised of multiple slabs and
   accompanying meta info. */
struct slab_allocator {
    uint64_t num_total_slabs;
    /* Multiple slab lists, one for each supported alloc size */
    struct slab *slabs[MAX_SUPPORTED_SIZES];  
    uint32_t supported_sizes[MAX_SUPPORTED_SIZES];
    uint32_t num_slabs[MAX_SUPPORTED_SIZES];
    uint32_t slab_size;
    bool init;
};

/* Public functions */
void *slab_allocator_malloc(uint32_t size);
void slab_allocator_free(void* ptr);

#endif