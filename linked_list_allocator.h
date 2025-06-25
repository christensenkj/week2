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

#ifndef LINKED_LIST_ALLOCATOR_H_
#define LINKED_LIST_ALLOCATOR_H_

/* Brainstorming a custom allocator

    Arm Cortex A72 specs:
    L1 dcache: 32 KB, L2 cache: 512 KB
    L1 dcache line size: 64 bytes

    There are a few different ideas:

    - Pre-fetch a large chunk of unused data (16 KB) at linked list creation
      Also initialize two empty linked lists: one for free nodes and one for occupied nodes
      insert() removes the head of the free list and adds it to the occupied list.
      remove() removes a node from the occupied list and adds it to the free list. 

    - Since the sizes of various items to malloc and free are preknown, I can hardcode 
      allocation and freeing of only a few different sized memory blocks (sizeof node, linked_list, queue).

    - When inserting, allocate 64 bytes worth of nodes at a time. This ensures fast iteration 
      (all within a single cache line) for at least a few nodes.
*/

#endif