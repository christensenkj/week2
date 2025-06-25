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

#include "queue.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

// Function pointers to (potentially) custom malloc() and
// free() functions.
// TODO @karston: look into a custom malloc() and free() for linked_list
// to avoid fragmentation and improve performance.
static void * (*malloc_fptr)(size_t size) = NULL;
static void   (*free_fptr)(void* addr)    = NULL; 

// Implement your queue functions here.
//

/* Register a user-specified malloc method */
bool queue_register_malloc(void * (*malloc)(size_t)) {
    INVALID_PTR_CHECK(malloc, false);
    malloc_fptr = malloc;
    return true;
}

/* Register a user-specified free method */
bool queue_register_free(void (*free)(void*)) {
    INVALID_PTR_CHECK(free, false);
    free_fptr = free; 
    return true;
}

/* Create a queue */
struct queue * queue_create(void) {
    struct queue *q = (struct queue *) malloc_fptr(sizeof(struct queue));
    INVALID_PTR_CHECK(q, NULL);
    q->ll = linked_list_create();
    q->len = 0;
    return q;
}

bool queue_delete(struct queue * queue) {
    INVALID_PTR_CHECK(queue, false);
    free_fptr(queue->ll);
    free_fptr(queue);
    return true;
}

bool queue_push(struct queue * queue, unsigned int data) {
    INVALID_PTR_CHECK(queue, false);

    if (!linked_list_insert_front(queue->ll, data)) {
        return false;
    }

    ++queue->len;
    return true;
}

bool queue_pop(struct queue * queue, unsigned int * popped_data __attribute__((unused))) {
    INVALID_PTR_CHECK(queue, false);

    /* Return early if there is no data to pop */
    if (!queue->len) {
        popped_data = NULL;
        return false;
    }

    /* Set the pointer to the data to pop */
    popped_data = &(queue->ll->tail->data);

    /* Remove the last element of the queue. */
    bool ret = linked_list_remove(queue->ll, queue->len - 1);
    --queue->len;

    return ret;
}

size_t queue_size(struct queue * queue) {
    INVALID_PTR_CHECK(queue, false);
    return queue->len;
}

bool queue_has_next(struct queue * queue) {
    INVALID_PTR_CHECK(queue, false);
    return queue->len ? true : false;
}

bool queue_next(struct queue * queue, unsigned int * popped_data __attribute__((unused))){
    if (!queue->len) {
        popped_data = NULL;
        return false;
    }
    
    popped_data = &(queue->ll->tail->data);
    return true;
}