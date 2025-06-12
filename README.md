# Pointer Wars 2025: Linked List Edition Week 2

There are a few tasks this week, with the end goal being to move
beyond the microbenchmarks of last week and move into the performance
of real software (or at least, closer to real :) ).

# Task 1: Improve Linked List Implementations
Many of you received feedback about worst case time complexity of
various linked list functions being too high. In particular, 
linked_list_size() often was often implemented as a linear time
function as opposed to constant, as was linked_list_insert_end().

I strongly suggest improving these this week. The linked_list_size()
function is easy enough to derive a constant time implementation
for. The linked_list_insert_end() function is a bit tricky.

The typical solution is to implement a doublely linked list, and
there were submissions that did that from the beginning. Consequently,
they blew the pants off everyone else's code in terms of performance
results for linked_list_insert_end(), scaling nearly identitically 
to their implementations of linked_list_insert_front().

You certainly may choose to do this, but you can implement it 
another way, using an iterator. There's a nasty case that you
need to handle wherein you remove the 2nd to last element of
the linked list with this solution, however, you can use an
iterator to mimic a tail pointer.

# Task 2: Check For Memory Leaks
A rare few (more than one) of your submissions caused substantial 
memory thrashing during performance testing, because some functions
forgot to call free_fptr() on various pointers. 

This is actually a rather interesting finding: a memory leak
caused some pretty severe performance problems! Usually they
hide out in the background never to be found.

Performance testing is gated on having a memory-leak free
implementation going forward. Linux users can make use of
'make run_valgriend_checks' on the command line. I'm not sure of
a Windows equivalent tool. Send mail if you need help finding
one, or if you know of one.
 
# Task 3: Implement a queue
The new code to write this week is to make use of your linked list
code and implement a queue. A queue is a FIFO structure (First In
First Out) which often is implemented as a linked list in its
underlying implementation. The queue will hold data of type
"unsigned int", just like the previous week's linked list, although
you should put some thought into how you might extend your linked
list to hold any C data type. It's not required for this week,
but a good thing to noodle on.

The queue has the following functions specified in queue.h, see
the header file for the exact function signatures.
 x queue_register_malloc()
 x queue_register_free()
 x queue_create()
 x queue_delete()
 x queue_push()
 x queue_pop()
 x queue_size()
 x queue_has_next()
 x queue_next()

Most of these behave exactly as one would expect, with a similar style
API as the linked_list of the previous week. The queue_push() 
inserts a node at the end of the linked_list. The queue_pop() function
removes the node at the end and provides its data to programmer, 
if the queue is non-empty. Note the change in function signature, 
return value is success/failure, and the pointer passed in is where
the value is updated on success.

The queue_has_next() function specifies whether a node can be popped,
and the queue_next() function specifies that value with the pointer
passed into it, subject to its existance. Importantly, queue_next()
does not pop the node off of the queue on success.

# Optional Task: Custom Memory Allocator
This is only required if you want optimal performance.

Thus far, ALL of the submissions received call malloc_fptr() each time 
a node is inserted, and (ideally) call free_fptr() when a node is 
removed. This adds around 20 or so nanoseconds to each insertion,
or around 60% of the time for each call to linked_list_insert_front().
When optimized to have a worst case time complexity of O(k), it will
also occur for linked_list_insert_end() (iteration time dominated
nearly all of your solutions for week 1).

A means of performing this, and this is by no means optimal, is
roughly the following:
 0. Create a structure called "free_node", that contains a pointer
    to a struct node, and a pointer to another free_node structure.
 1. Create a "free list" (perhaps called __free_list) within your
    linked_list structure and initialize it to NULL. This list
    consists of 
 2. On insertion of a linked_list, remove a pointer from the 
    free_list.
 4. If no pointers are in the list, call malloc_fptr() and
    allocate an array of struct node pointers. Size of the array
    is your call. Upon success, add those newly allocated pointers
    to the free_list.
 4. Upon removal of a node, return the pointer to the free list.
 5. Upon deletion of the linked list, free_fptr() all of the 
    pointers you allocated. The tricky part here is that you
    allocate multiple at once.

Please send your questions about this to pointerwars2025@gmail.com.

# The Workload
Your queue is going to be used in a to-be-written Breadth First Search
program. The tentative plan is to have you search as to whether there
is a path (via hyperlinks) over a 2007 version of Wikipedia.

There will be some additional microbenchmark testing as well.
