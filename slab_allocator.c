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
//#include "pthread.h"
#include "signal.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* Global allocator instance */
static struct slab_allocator g_alloc = SLAB_ALLOCATOR_CONFIG(NULL, 0, 0, 0, false);

#ifdef FEATURE_MULTITHREADED
/* Recursive mutex to protect multithreaded accesses to the above global object */
pthread_mutex_t g_alloc_rmutex;

/* Initialize the recursive mutex used to lock 
    accesses to the global allocator instance */
static void slab_allocator_mutex_init(void) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_alloc_rmutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
#endif

static struct slab *create_slab() {
    /* Create a new slab */
    struct slab *new_slab = malloc(sizeof(struct slab));
    new_slab->nodes = malloc(g_alloc.slab_size * sizeof(struct free_node));
    new_slab->size = g_alloc.slab_size;
    new_slab->used = 0;
    new_slab->next = NULL;
    new_slab->prev = NULL;

    /* Initialize the free list for the new slab */
    new_slab->free_list = new_slab->nodes;
    for (size_t i = 0; i < g_alloc.slab_size - 1; i++) {
        new_slab->nodes[i].next = &new_slab->nodes[i+1];
    }
    new_slab->nodes[g_alloc.slab_size-1].next = NULL;

    return new_slab;
}

/* Add a new slab to the allocator's list of managed memory slabs. */
static struct slab *allocator_add_slab() {
    /* Initialize global allocator instance on first malloc */
    if (!g_alloc.init) {
        g_alloc.slabs = NULL;
        g_alloc.num_slabs = 0;
        g_alloc.slab_size = SLAB_SIZE;
        g_alloc.max_slabs = MAX_SLABS;
        g_alloc.init = true;
    }

    /* Ensure we're not allocating too many slabs */
    if (g_alloc.num_slabs >= MAX_SLABS) {
        printf("Allocating too many slabs.\n");
        return NULL; 
    }

    /* Performs a malloc() */
    struct slab *new_slab = create_slab();

    if (new_slab == NULL) {
         printf("Unable to add new slab to allocator.\n");
         exit(1);
    }

    /* Add the new slab to the allocator's slab list */
    if (g_alloc.slabs != NULL) {
        g_alloc.slabs->prev = new_slab;
    }
    new_slab->next = g_alloc.slabs;
    g_alloc.slabs = new_slab;
    g_alloc.num_slabs++;

    return new_slab;
}

void allocator_remove_slab(struct slab *slab) {
    // Removing the only slab in the slab list
    if (g_alloc.num_slabs == 1) {
        g_alloc.slabs = NULL;
        free(slab->nodes);
        free(slab);
    }

    // Removing the head of the slab list
    else if (slab == g_alloc.slabs) {
        g_alloc.slabs = slab->next;
        g_alloc.slabs->prev = NULL;
        free(slab->nodes);
        free(slab);
    }

    else {
        if (slab->next != NULL) {
            slab->next->prev = slab->prev;
        }
        slab->prev->next = slab->next;
        free(slab->nodes);
        free(slab);
    }

    g_alloc.num_slabs--;

    if (!g_alloc.num_slabs) {
        g_alloc.init = false;
    }
}

/* Specialized malloc implementation for linked_list node-sized 
   allocations */
void* slab_allocator_malloc(size_t size) {
    if (size > sizeof(struct free_node)) {
        return NULL;
    }

    /* Find a slab with free space */
    struct slab *slab = g_alloc.slabs;
    while (slab != NULL) {
        if (slab->free_list != NULL) {
            struct free_node *node = slab->free_list;
            slab->free_list = node->next;
            slab->used++;
            return node;
        }
        slab = slab->next;
    }

    /* No free space in any of the allocator's slabs, so add a new slab */
    if (allocator_add_slab() != NULL) {
        /* Recursive call to get a new slab with a full free_list */
        return slab_allocator_malloc(size); 
    }

    return NULL;
}

void slab_allocator_free(void* ptr) {
    /* Iterate over slabs to find the slab that owns ptr */
    struct slab *slab = g_alloc.slabs;

    while (slab != NULL) {
        /* Determine if provided ptr belongs to the current slab */
        if (ptr >= (void *) slab->nodes && 
                ptr < (void *) (slab->nodes + slab->size)) {

            /* Return the pointer to the slab's free list */
            struct free_node *node = (struct free_node *) ptr;
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
}