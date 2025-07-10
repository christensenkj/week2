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

#include "slab_allocator.h"
#include "signal.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#ifdef FEATURE_MULTITHREADED
#include "pthread.h"
/* Recursive mutex to protect multithreaded accesses to the above global object */
pthread_mutex_t g_allocator_rmutex;

/* Initialize the recursive mutex used to lock 
    accesses to the glo#bal allocator instance */
static void slab_allocator_mutex_init(void) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_allocator_rmutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
#endif

/* Size of the node header, which contains the data size */
#define NODE_HEADER_SIZE   (sizeof(uint32_t))

/* Global allocator instance */
static struct slab_allocator g_allocator = {0};

/* Inititalize global allocator parameters on first use */
static inline void allocator_init(void) {
    /* Define an array of supported sizes */
    SUPPORTED_SIZES_ARRAY();
    for (int size_idx = 0; size_idx < MAX_SUPPORTED_SIZES; size_idx++) {
        g_allocator.slabs[size_idx] = NULL;
        g_allocator.num_slabs[size_idx] = 0;
        g_allocator.supported_sizes[size_idx] = supported_sizes[size_idx];
    }
    g_allocator.num_total_slabs = 0;
    g_allocator.slab_size = SLAB_SIZE;
    g_allocator.init = true;
}

/* Map of allocation block size to slab list index.
   Fatally errors on failure. */
static inline int supported_alloc_size_map(uint32_t alloc_size) {
    int idx = -1;
    for (int i = 0; i < MAX_SUPPORTED_SIZES; i++) {
        if (alloc_size == g_allocator.supported_sizes[i]) {
           idx = i;
           break; 
        }
    }
    if (idx == -1) {
        printf("Unsupported allocation size: %u.\n", alloc_size);
        exit(1);
    }
    return idx;
}

/* Create a slab. Initialize its parameters and partition 
   its nodes according to the required node size. */
static struct slab *create_slab(uint32_t size_idx) {

/* Helper macros to improve in-line readability. */
#define NODE_SIZE(_alloc_size)  (_alloc_size + NODE_HEADER_SIZE)
#define NEXT_NODE_ADDR(_node, _node_size) \
    (struct free_node *) ((uint8_t *) _node + _node_size)

    /* Create a new slab */
    struct slab *new_slab = malloc(sizeof(struct slab));
    if (new_slab == NULL) {
        printf("Unable to malloc space for a new slab.\n");
        return NULL;
    }

    new_slab->pool = malloc(g_allocator.slab_size);
    if (new_slab->pool == NULL) {
        printf("Unable to malloc space for a new pool.\n");
        free(new_slab);
        return NULL;
    }

    new_slab->size = g_allocator.slab_size;
    new_slab->used = 0;
    new_slab->next = NULL;
    new_slab->prev = NULL;

    /* Calculate the number of nodes in the slab */
    uint32_t alloc_size = g_allocator.supported_sizes[size_idx];
    uint32_t node_size = NODE_SIZE(alloc_size);
    new_slab->num_nodes = new_slab->size / node_size;

    /* Split the malloc'd chunk into nodes of desired size */
    struct free_node *tmp = new_slab->pool;
    int nodes = new_slab->num_nodes;
    while (nodes > 1) {
        tmp->next = NEXT_NODE_ADDR(tmp, node_size);
        tmp->alloc_size = alloc_size;
        tmp = tmp->next;
        nodes--;
    }
    tmp->next = NULL;
    tmp->alloc_size = alloc_size;

    new_slab->free_list = new_slab->pool;
    return new_slab;

#undef NODE_SIZE
#undef NEXT_NODE_ADDR
}

/* Add a new slab to the allocator's list of managed memory slabs. */
static struct slab *allocator_add_slab(uint32_t size_idx) {
    /* Check to make sure we're not over-allocating */
    if (g_allocator.num_total_slabs + 1 > MAX_SLABS) {
        printf("Allocated too many slabs: %lu\n", g_allocator.num_total_slabs);
        return NULL;
    }
    /* Performs a malloc() */
    struct slab *new_slab = create_slab(size_idx);

    if (new_slab == NULL) {
         printf("Unable to add new slab to allocator.\n");
         return NULL;
    }

    /* Add the new slab to the allocator's slab list */
    if (g_allocator.slabs[size_idx] != NULL) {
        g_allocator.slabs[size_idx]->prev = new_slab;
    }
    new_slab->next = g_allocator.slabs[size_idx];
    g_allocator.slabs[size_idx] = new_slab;
    g_allocator.num_slabs[size_idx]++;
    g_allocator.num_total_slabs++;

    return new_slab;
}

/* Remove a slab. Free the slab and all of its allocatable
   nodes. */
static void allocator_remove_slab(struct slab *slab) {

    uint32_t alloc_size = slab->pool->alloc_size;
    int size_idx = supported_alloc_size_map(alloc_size);

    // Removing the only slab in the slab list
    if (g_allocator.num_slabs[size_idx] == 1) {
        g_allocator.slabs[size_idx] = NULL;
        free(slab->pool);
        free(slab);
    }
    // Removing the head of the slab list
    else if (slab == g_allocator.slabs[size_idx]) {
        g_allocator.slabs[size_idx] = slab->next;
        g_allocator.slabs[size_idx]->prev = NULL;
        free(slab->pool);
        free(slab);
    }
    // Removing a body node in the slab list
    else {
        if (slab->next != NULL) {
            slab->next->prev = slab->prev;
        }
        slab->prev->next = slab->next;
        free(slab->pool);
        free(slab);
    }

    g_allocator.num_slabs[size_idx]--;
    g_allocator.num_total_slabs--;

    if (!g_allocator.num_total_slabs) {
        g_allocator.init = false;
    }
}

/* Specialized malloc implementation for linked_list node-sized 
   allocations. Searches for a free node in the slab list 
   containing nodes of the allocation size. */
void* slab_allocator_malloc(uint32_t alloc_size) {
    /* Initialize global allocator instance on first malloc */
    if (!g_allocator.init) {
        allocator_init();
    }

    /* Find size idx */
    int size_idx = supported_alloc_size_map(alloc_size);

    /* Find a slab with free space */
    struct slab *slab = g_allocator.slabs[size_idx];
    while (slab != NULL) {
        if (slab->free_list != NULL) {
            struct free_node *node = slab->free_list;
            slab->free_list = node->next;
            slab->used++;

            /* The first word of a block stores its size */
            void *ret = (void *) ((uint8_t *) node + NODE_HEADER_SIZE);
            return ret;
        }
        slab = slab->next;
    }

    /* No free space in any of the allocator's slabs, so add a new slab */
    if (allocator_add_slab(size_idx) != NULL) {
        /* Recursive call to get a new slab with a full free_list */
        return slab_allocator_malloc(alloc_size); 
    }

    return NULL;
}

/* Free allocated memory. */
void slab_allocator_free(void* ptr) {

/* Helper macros for better in-line readability */
#define SLAB_START_ADDR(_slab_ptr) (_slab_ptr->pool)
#define SLAB_END_ADDR(_slab_ptr) \
    ((struct free_node *) ((uint8_t *)_slab_ptr->pool + _slab_ptr->size))

    /* Get the size of the block to be freed from the provided pointer */
    struct free_node *node = (struct free_node *) ((uint32_t *) ptr - 1);

    /* Find size idx */
    int size_idx = supported_alloc_size_map(node->alloc_size);

    /* Iterate over slabs to find the slab that owns ptr */
    struct slab *slab = g_allocator.slabs[size_idx];

    while (slab != NULL) {
        /* Determine if provided ptr belongs to the current slab */
        if (node >= SLAB_START_ADDR(slab) && 
                node < SLAB_END_ADDR(slab)) {

            /* Return the pointer to the slab's free list */
            node->next = slab->free_list;
            slab->free_list = node;
            slab->used--;

            /* If the slab has no more used nodes, free the whole slab */
            if (!slab->used) {
                allocator_remove_slab(slab);
            }
            return;
        }
        slab = slab->next;
    }


    /* The provided pointer was never allocated by this slab allocator */
    printf("Unable to free linked_list node back to slab.");
    exit(1);

#undef SLAB_START_ADDR
#undef SLAB_END_ADDR
}