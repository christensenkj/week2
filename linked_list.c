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

#include "linked_list.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

// Function pointers to (potentially) custom malloc() and
// free() functions.
// TODO @karston: look into a custom malloc() and free() for linked_list
// to avoid fragmentation and improve performance.
static void * (*malloc_fptr)(size_t size) = NULL;
static void   (*free_fptr)(void* addr)    = NULL; 

/* Create a new linked list node */
static inline struct node * create_node(unsigned int data) {
    struct node * new = (struct node *) malloc_fptr(sizeof(struct node));
    new->data = data;
    new->next = NULL;
    new->prev = NULL;
    return new;
}

/* Determine if it's quicker to reach the desired index from the head or the tail and
   return a pointer to the node at the provided index */
static inline struct node * linked_list_traverse_to_index(struct linked_list * ll, unsigned int index) {
    // Check edges and bad inputs
    if (index == 0) {
        return ll->head;
    }
    else if (index == ll->len-1) {
        return ll->tail;
    }
    else if (index >= ll->len) {
        return NULL;
    }

    // Determine if it's quicker to iterate to the desired index from the head or tail
    struct node * current;
    if (index >= ll->len/2) {  // If the index is in the tail half of the list, iterate from the tail
        current = ll->tail->prev; // iterate from tail->prev rather than tail to save some cycles
        for (unsigned int i = ll->len-2; i > index && current != NULL; i--) {
            current = current->prev;
        }
    }
    else {  // If the index is in the head half of the list, iterate from the head
        current = ll->head->next; // iterate from tail->prev rather than tail to save some cycles
        for (unsigned int i = 1; i < index && current != NULL; i++) {
            current = current->next;
        }
    }

    // Index was not in the list
    if (current == NULL) {
        return false;
    }

    return current;
}

/* Create a new linked list */
struct linked_list * linked_list_create(void) {
    struct linked_list * ll = (struct linked_list *) malloc_fptr(sizeof(struct linked_list));
    if (ll != NULL) {
        ll->head = NULL;
        ll->tail = NULL;
        ll->len = 0;
    }
    return ll;
}

/* Delete an entire linked list */
bool linked_list_delete(struct linked_list * ll) {
    INVALID_PTR_CHECK(ll, false);

    // Iterate through the list, freeing each node. Faster than calling linked_list_remove() 
    // repeatedly since we avoid jumping and populating new stack frames
    struct node * current = ll->head;
    struct node * next;
    while(current != NULL) {
        next = current->next;
        free_fptr(current);
        current = next;
    }

    // Free the containing ll struct
    ll->head = NULL;
    free_fptr(ll);
    return true;
}

/* Return the size of the linked list. Cache the size as part of the linked_list struct 
   to improve performance */
size_t linked_list_size(struct linked_list * ll) {
    // Invalid list
    INVALID_PTR_CHECK(ll, SIZE_MAX);
    return ll->len;
}

/* Insert a new node at the tail of the list */
bool linked_list_insert_end(struct linked_list * ll, unsigned int data) {
    INVALID_PTR_CHECK(ll, false);

    // Create a new node with the provided data
    struct node * new = create_node(data);

    // Handle empty linked list case
    if (ll->head == NULL) {
        ll->head = new;
        ll->tail = ll->head;
        ++ll->len;
        return true;
    }
    new->prev = ll->tail;
    ll->tail->next = new;
    ll->tail = new;
    ++ll->len;
    return true; 
}

/* Insert a new node at the head of the list */
bool linked_list_insert_front(struct linked_list * ll, unsigned int data) {
    INVALID_PTR_CHECK(ll, false);

    // Create a new node with the provided data
    struct node * new = create_node(data);
    new->next = ll->head;

    // Handle empty linked list case
    if (ll->head == NULL) {
        ll->head = new;
        ll->tail = ll->head;
        ++ll->len;
        return true;
    }

    ll->head->prev = new;
    ll->head = new;
    ++ll->len;
    return true; 
}

/* Insert a new node at the specified index */
bool linked_list_insert(struct linked_list * ll, size_t index, unsigned int data) {
    // Perform checks
    INVALID_PTR_CHECK(ll, false);

    // Handle special cases up front 
    if (index == 0) {
        return linked_list_insert_front(ll, data);
    } 
    else if (index == ll->len) { 
        return linked_list_insert_end(ll, data);
    } 

    struct node * current = linked_list_traverse_to_index(ll, index);
    if (current == NULL) {
        return NULL;
    }

    // Create a new node
    struct node * new = create_node(data);

    // Insert the new node before current, tying up all prev and next pointers
    new->prev = current->prev;
    new->next = current;
    current->prev->next = new;
    current->prev = new;
    ++ll->len;
    return true;
}

/* Find the first occurrence of a value in the list */
size_t linked_list_find(struct linked_list * ll, unsigned int data) {
    INVALID_PTR_CHECK(ll, SIZE_MAX);

    // Iterate through the list
    int index = 0;
    struct node * current = ll->head;
    while(current != NULL) {
        if (current->data == data) {
            return index;
        }
        ++index;
        current = current->next;
    }
    return SIZE_MAX;
}

/* Remove a node at the specified index */
bool linked_list_remove(struct linked_list * ll, size_t index) {
    INVALID_PTR_CHECK(ll, false);
    if (index >= ll->len) {
        return false;
    }

    // Deleting the last node in the list
    if (ll->len == 1) {
        struct node * tmp = ll->head;
        ll->head = NULL;
        free_fptr(tmp);
        --ll->len;
        return true;
    }

    // Deleting the head of the linked list
    if (index == 0) {
        struct node * tmp = ll->head;
        ll->head = tmp->next;
        ll->head->prev = NULL;
        free_fptr(tmp);
        --ll->len;
        return true;
    }

    // Deleting the tail of the linked list
    if (index == ll->len-1) {
        struct node * tmp = ll->tail;
        ll->tail = tmp->prev;
        ll->tail->next = NULL;
        free_fptr(tmp);
        --ll->len;
        return true;
    }

    struct node * current = linked_list_traverse_to_index(ll, index);
    if (current == NULL) {
        return NULL;
    }

    // otherwise, current points to the index for deletion
    current->prev->next = current->next;
    current->next->prev = current->prev;
    free_fptr(current);
    --ll->len;
    return true;
}

/* Create an iterator to conveniently traverse nodes and access their members */
struct iterator * linked_list_create_iterator(struct linked_list * ll, size_t index) {
    INVALID_PTR_CHECK(ll, NULL);

    // Traverse to the specified node
    struct node * current = linked_list_traverse_to_index(ll, index);
    if (current == NULL) {
        return NULL;
    }
    else {
        struct iterator * it = (struct iterator *) malloc_fptr(sizeof(struct iterator));
        it->ll = ll;
        it->current_index = index;
        it->current_node = current;
        it->data = current->data;
        return it;
    }
}

/* Delete an iterator */
bool linked_list_delete_iterator(struct iterator * iter) {
    INVALID_PTR_CHECK(iter, false);
    free_fptr(iter);
    return true;
}

/* Iterate forward through the list */
bool linked_list_iterate(struct iterator * iter) {
    INVALID_PTR_CHECK(iter, false);
    if (iter->current_node->next == NULL) {
        return false;
    } 
    iter->current_index++;
    iter->current_node = iter->current_node->next;
    iter->data = iter->current_node->data;    
    return true;
}

/* Register a malloc function */
bool linked_list_register_malloc(void * (*malloc)(size_t)) {
    INVALID_PTR_CHECK(malloc, false);
    malloc_fptr = malloc;
    return true;
}

/* Register a free function */
bool linked_list_register_free(void (*free)(void*)) {
    INVALID_PTR_CHECK(free, false);
    free_fptr = free; 
    return true;
}
