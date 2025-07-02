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

#define MAX_SLABS   1024
#define SLAB_SIZE   ((32 * 1024) / sizeof(struct free_node))

/* A slab is a sixed-size chunk of memory that's allocated using 
   stdlib malloc. The chunk exists as a node of a list of 
   allocated chunks. Each chunk is formatted as a list of 
   linked list nodes, representing free nodes available for
   use in a general-purpose linked list.*/

struct free_node {
    struct free_node *next;
    unsigned int padding[6];
};

struct slab {
    struct free_node *nodes;
    struct free_node *free_list;
    size_t size;
    size_t used;
    struct slab *next;
    struct slab *prev;
};

struct slab_allocator {
    struct slab *slabs;  /* pointer to list of slabs in allocator */
    size_t num_slabs;
    size_t slab_size;
    size_t max_slabs;
    bool init;
};

/* Helper macro to initialize a slab allocator container */
#define SLAB_ALLOCATOR_CONFIG(_slabs, _num_slabs, _slab_size, _max_slabs, _init) \
{ \
    .slabs = _slabs, \
    .num_slabs = _num_slabs, \
    .slab_size = _slab_size, \
    .max_slabs = _max_slabs, \
    .init = _init, \
}

/* Function prototypes */
void *slab_allocator_malloc(size_t size);
void slab_allocator_free(void* ptr);

#endif