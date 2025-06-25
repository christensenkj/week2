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

#ifndef _QUEUE_H
#define _QUEUE_H

#include "linked_list.h"

// Some rules for Pointer Wars 2025 week 2:
// 0. Implement all functions in queue.c
// 1. Feel free to add members to the structures, but please do not remove 
//    any or rename any. Doing so will cause test infrastructure to fail
//    to link against your shared library.
// 2. Same goes for the function declarations.
// 3. In your implementation, make use of malloc_fptr() and free_fptr()
//    functions for memory allocation and memory freeing. This allows
//    test infrastructure a bit more flexility. See linked_list.c for
//    declarations of those function pointers.

// Definition of the queue.
// 
struct queue {
    struct linked_list* ll;
    size_t len;
};


// Creates a new queue.
// PRECONDITION: Register malloc() and free() functions via the
//               queue_register_malloc() and 
//               queue_register_free() functions.
// Returns a new linked_list on success, NULL on failure.
//
struct queue * queue_create(void);

// Deletes a linked_list.
// \param queue : Pointer to queue to delete
// Returns TRUE on success, FALSE otherwise.
//
bool queue_delete(struct queue * queue);

// Pushes an unsigned int onto the queue.
// \param queue : Pointer to queue.
// \param data  : Data to insert.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_push(struct queue * queue, unsigned int data);

// Pops an unsigned int from the queue, if one exists.
// \param queue       : Pointer to queue.
// \param popped_data : Pointer to popped data (provided by caller), if pop occurs.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_pop(struct queue * queue, unsigned int * popped_data); 

// Returns the size of the queue.
// \param queue : Pointer to queue.
// Returns size on success, SIZE_MAX otherwise.
//
size_t queue_size(struct queue * queue);

// Returns whether an entry exists to be popped.
// \param queue: Pointer to queue.
// Returns TRUE if an entry can be popped, FALSE otherwise.
//
bool queue_has_next(struct queue * queue);

// Returns the value at the head of the queue, but does
// not pop it.
// \param queue       : Pointer to queue.
// \param popped_data : Pointer to popped data (provided by caller), if pop occurs.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_next(struct queue * queue, unsigned int * popped_data);

// Registers malloc() function.
// \param malloc : Function pointer to malloc()-like function.
// POSTCONDITION: Initializes malloc() function pointer in linked_list.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_register_malloc(void * (*malloc)(size_t));

// Registers free() function.
// \param free : Function pointer to free()-like function.
// POSTCONDITION: Initializes free() functional pointer in linked_list.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_register_free(void (*free)(void*));

#endif 
