#include "slab_allocator_test.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "queue.h"

#define TEST_PRINT

void test_basic_alloc_free() {
    printf("Test: Basic alloc/free...\n");
    void *ptr1 = slab_allocator_malloc(16);
    assert(ptr1 != NULL);
    void *ptr2 = slab_allocator_malloc(24);
    assert(ptr2 != NULL);
    slab_allocator_free(ptr1);
    slab_allocator_free(ptr2);
    printf("  Passed.\n");
}

void test_double_alloc_free() {
    printf("Test: Double alloc/free...\n");
    void *ptr1 = slab_allocator_malloc(32);
    void *ptr2 = slab_allocator_malloc(32);
    assert(ptr1 != NULL && ptr2 != NULL);
    assert(ptr1 != ptr2);
    slab_allocator_free(ptr1);
    slab_allocator_free(ptr2);
    printf("  Passed.\n");
}

void test_exhaust_slab_and_allocate_new() {
    printf("Test: Exhaust slab and allocate new...\n");
    size_t alloc_size = 16;
    size_t nodes_per_slab = SLAB_SIZE / alloc_size;
    void *ptrs[nodes_per_slab + 2];
    // Allocate enough to fill one slab
    for (size_t i = 0; i < nodes_per_slab; ++i) {
        ptrs[i] = slab_allocator_malloc(alloc_size);
        assert(ptrs[i] != NULL);
    }
    // Next allocation should force a new slab
    ptrs[nodes_per_slab] = slab_allocator_malloc(alloc_size);
    assert(ptrs[nodes_per_slab] != NULL);
    // Free all
    for (size_t i = 0; i <= nodes_per_slab; ++i) {
        slab_allocator_free(ptrs[i]);
    }
    printf("  Passed.\n");
}

void test_free_and_reuse() {
    printf("Test: Free and reuse...\n");
    void *ptr1 = slab_allocator_malloc(24);
    void *ptr2 = slab_allocator_malloc(24);
    assert(ptr1 != NULL && ptr2 != NULL);
    slab_allocator_free(ptr1);
    void *ptr3 = slab_allocator_malloc(24);
    assert(ptr3 == ptr1); // Should reuse freed block
    slab_allocator_free(ptr2);
    slab_allocator_free(ptr3);
    printf("  Passed.\n");
   #if 0 
    // Test OOM condition
    uint64_t alloc_size = 256;
    uint64_t num_allocs = 0;
    while(1) {
        uint64_t *ptr = malloc(alloc_size);
        if (ptr == NULL) {
            printf("Reached memory limit of allocator\n");
            break;
        }

        num_allocs++;
#ifdef TEST_PRINT
        printf("bytes allocated: %lu\n", num_allocs * alloc_size);
#endif
    }
    printf("num allocs: %lu\n", num_allocs);
    #endif
}