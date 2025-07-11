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
#include "stdio.h"

// Function pointers to (potentially) custom malloc() and
// free() functions.
static void * (*malloc_fptr)(size_t size) = NULL;
static void   (*free_fptr)(void* addr)    = NULL; 

// Implement your queue functions here.
//

/* Register a user-specified malloc method */
bool queue_register_malloc(void * (*malloc)(size_t)) {
    INVALID_PTR_CHECK(malloc, false);
    malloc_fptr = malloc;
    linked_list_register_malloc(malloc);
    return true;
}

/* Register a user-specified free method */
bool queue_register_free(void (*free)(void*)) {
    INVALID_PTR_CHECK(free, false);
    free_fptr = free; 
    linked_list_register_free(free);
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

/* Delete a queue */
bool queue_delete(struct queue * queue) {
    INVALID_PTR_CHECK(queue, false);
    linked_list_delete(queue->ll);
    free_fptr(queue);
    return true;
}

/* Push new data onto the end of the queue */
bool queue_push(struct queue * queue, unsigned int data) {
    INVALID_PTR_CHECK(queue, false);

    if (!linked_list_insert_end(queue->ll, data)) {
        return false;
    }

    ++queue->len;
    return true;
}

/* Pop data from the head of the queue */
bool queue_pop(struct queue * queue, unsigned int * popped_data __attribute__((unused))) {
    INVALID_PTR_CHECK(queue, false);

    /* Return early if there is no data to pop */
    if (!queue->len) {
        popped_data = NULL;
        return false;
    }

    /* Set the pointer to the data to pop */
    *popped_data = queue->ll->head->data;

    /* Remove the first element of the queue. */
    bool ret = linked_list_remove(queue->ll, 0);
    --queue->len;

    return ret;
}

/* Get the size of the queue */
size_t queue_size(struct queue * queue) {
    INVALID_PTR_CHECK(queue, SIZE_MAX);
    return queue->len;
}

/* Check if the queue contains readable data */
bool queue_has_next(struct queue * queue) {
    INVALID_PTR_CHECK(queue, false);
    return queue->len ? true : false;
}

/* Return the head of the queue in a passed parameter */
bool queue_next(struct queue * queue, unsigned int * popped_data __attribute__((unused))){
    INVALID_PTR_CHECK(queue, false);
    if (!queue->len) {
        popped_data = NULL;
        return false;
    }
    
    *popped_data = queue->ll->head->data;
    return true;
}
