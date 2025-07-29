#ifndef SLAB_ALLOCATOR_TEST_H
#define SLAB_ALLOCATOR_TEST_H

#include "slab_allocator.h"

void test_basic_alloc_free(void);
void test_double_alloc_free(void);
void test_exhaust_slab_and_allocate_new(void);
void test_free_and_reuse(void);

#endif // SLAB_ALLOCATOR_TEST_H 