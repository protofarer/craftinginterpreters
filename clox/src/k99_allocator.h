#include <stddef.h>

#ifndef K99_ALLOCATOR_H
#define K99_ALLOCATOR_H

typedef struct {
    void *data;
    // interface:
    // old_mem      old_size    new_size    effect
    // any          any         0           free - return NULL
    // NULL         0           >0          fresh alloc
    // valid        >0          >0          resize

    // NB:
    // any          0           any         caller bug
    void *(*reallocate)(void *data, void *ptr_block,  size_t old_size, size_t new_size);
} Allocator;

#endif
