#ifndef K99_FREELIST_H
#define K99_FREELIST_H

#include <stddef.h>
#include <stdlib.h>
#include "k99_allocator.h"

// a block is defined as the space "used" up by either a free node or an allocation
// the header is located inside the block

// Free Block:

// X (node)
// |<------------- block size ------------------>|
// |-- header --|-- (block_size - header_size) --|-- next block --|
//  ^
//  node ptr / block start


// Allocated Block

// X (node)
//                           >|<-- aligned!
// |<----------------- block size ------------------->|
// |-- padding --|-- header --|----- (user data) -----|-- next block --|
//  ^             ^            ^
//  block start   header_ptr  ptr (returned to caller)

typedef struct {
    size_t block_size; // entire space used by this allocation, includes padding and header
    size_t padding;    // gap between node and the start of header, aka alignment_padding
} FreeListAllocationHeader;

typedef struct FreeListNode FreeListNode;
struct FreeListNode {
    FreeListNode *next;
    size_t block_size; // alloc.padding + alloc.block_size
};

typedef enum PlacementPolicy {
    PlacementPolicyFindFirst,
    PlacementPolicyFindBest
} PlacementPolicy;

typedef struct {
    void *          data;
    size_t          size;
    size_t          used;

    FreeListNode *  head;
    PlacementPolicy policy;
} FreeList;

extern Allocator make_freelist_allocator(FreeList *fl, unsigned char * buf, size_t size, PlacementPolicy policy);

#ifdef K99_FREELIST_IMPLEMENTATION

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static void freelist_init(FreeList *fl, void *data, size_t size, PlacementPolicy policy);
static void freelist_free_all(FreeList *fl);
extern void *freelist_reallocate(void *data, void *old_memory, size_t old_size, size_t new_size);
static FreeListNode *freelist_find_first(
    FreeList *fl,
    size_t size,
    size_t alignment,
    size_t *padding_,
    FreeListNode **prev_node_
);
static FreeListNode *freelist_find_best(
    FreeList *fl,
    size_t size,
    size_t alignment,
    size_t *padding_, // used by caller
    FreeListNode **prev_node_ // caller has access to prev node since traverse happens here
);
static void *freelist_alloc_align(FreeList *fl, size_t size, size_t alignment);
static void freelist_coalescence(FreeList *fl, FreeListNode *prev_node, FreeListNode *free_node);
static void freelist_free(FreeList *fl, void *ptr);
static void freelist_node_insert(FreeListNode **phead, FreeListNode *prev_node, FreeListNode *new_node);
static void freelist_node_remove(FreeListNode **phead, FreeListNode *prev_node, FreeListNode *del_node);
static void *freelist_resize_align(FreeList *fl, void *old_memory, size_t old_size, size_t new_size, size_t align);
static void *freelist_resize(FreeList *fl, void *old_memory, size_t old_size, size_t new_size);
static void freelist_display(FreeList fl);

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

void freelist_free_all(FreeList *fl) {
    fl->used = 0;
    FreeListNode *first_node = (FreeListNode *)fl->data;
    first_node->block_size = fl->size;
    first_node->next = NULL;
    fl->head = first_node;
}

void freelist_init(FreeList *fl, void *data, size_t size, PlacementPolicy policy) {
    fl->data = data;
    fl->size = size;
    fl->policy = policy;
    freelist_free_all(fl);
}

extern Allocator make_freelist_allocator(FreeList *fl, unsigned char * buf, size_t size, PlacementPolicy policy) {
    freelist_init(fl, buf, size, policy);
    Allocator a = {
        .data = (void *)fl,
        .reallocate = &freelist_reallocate,
    };
    return a;
}

static bool freelist__is_power_of_two(uintptr_t x) {
    return (x & (x - 1)) == 0;
}

// padding between node and user data, which encompasses the header!
static size_t freelist__calc_padding_with_header(
    uintptr_t ptr,
    uintptr_t alignment,
    size_t header_size
) {
    uintptr_t p, a, modulo, padding, needed_space;

    assert(freelist__is_power_of_two(alignment));

    p = ptr;
    a = alignment;
    modulo = p & (a - 1);

    padding = 0;
    needed_space = 0;

    if (modulo != 0) {
        padding = a - modulo;
    }

    needed_space = (uintptr_t)header_size;

    if (padding < needed_space) {
        needed_space -= padding;
        // round needed_space up to nearest multiple of a
        // ((needed_space + a - 1) / a) * a
        if ((needed_space & (a - 1)) != 0) {
            padding += a * (1 + (needed_space / a));
        } else {
            padding += a * (needed_space / a);
        }
    }
    return (size_t)padding;
}

static FreeListNode *freelist_find_first(
    FreeList *fl,
    size_t size,
    size_t alignment,
    size_t *padding_, // used by caller
    FreeListNode **prev_node_ // caller has access to prev node since traverse happens here
) {
    // iterate free list for first free block w/ enough space
    FreeListNode *node = fl->head;
    FreeListNode *prev_node = NULL;

    size_t padding = 0;

    while (node != NULL) {
        padding = freelist__calc_padding_with_header(
            (uintptr_t)node,
            (uintptr_t)alignment,
            sizeof(FreeListAllocationHeader)
        );
        size_t required_space = size + padding;
        if (node->block_size >= required_space) {
            break;
        }
        prev_node = node;
        node = node->next;
    }
    if (padding_) *padding_ = padding;
    if (prev_node_) *prev_node_ = prev_node;
    return node;
}

static FreeListNode *freelist_find_best(
    FreeList *fl,
    size_t size,
    size_t alignment,
    size_t *padding_, // used by caller
    FreeListNode **prev_node_ // caller has access to prev node since traverse happens here
) {
    // iterate entire free list to find best fit
    // O(n)
    size_t smallest_diff = ~(size_t)0;

    FreeListNode *node = fl->head;
    FreeListNode *prev_node = NULL;

    FreeListNode *best_node = NULL;
    FreeListNode *prev_best_node = NULL;
    size_t best_padding = 0;

    size_t padding = 0;

    while (node != NULL) {
        padding = freelist__calc_padding_with_header(
            (uintptr_t)node,
            (uintptr_t)alignment,
            sizeof(FreeListAllocationHeader)
        );
        size_t required_space = size + padding;
        if (node->block_size >= required_space && (node->block_size - required_space < smallest_diff)) {
            smallest_diff = node->block_size - required_space;
            // TODO: bug, node and prev_node change, so problematically best_node and prev_best_node change
            best_node = node;
            prev_best_node = prev_node;
            best_padding = padding;
        }
        prev_node = node;
        node = node->next;
    }

    if (padding_) *padding_ = best_padding;
    if (prev_node_) *prev_node_ = prev_best_node;
    return best_node;
}

static void *freelist_alloc_align(FreeList *fl, size_t size, size_t alignment) {
    size_t total_padding = 0; // from node to **start of user data**
    FreeListNode *prev_node = NULL;
    FreeListNode *node = NULL;
    size_t preheader_gap, required_space, remaining;
    FreeListAllocationHeader *header_ptr;

    // Enforce minimum values
    if (size < sizeof(FreeListNode)) {
        size = sizeof(FreeListNode);
    }
    if (alignment < 8) {
        alignment = 8;
    }

    // Get the free node
    if (fl->policy == PlacementPolicyFindBest) {
        node = freelist_find_best(fl, size, alignment, &total_padding, &prev_node);
    } else {
        node = freelist_find_first(fl, size, alignment, &total_padding, &prev_node);
    }
    if (node == NULL) {
        assert(0 && "Free list has no free memory");
        return NULL;
    }

    // from node to **start of header**
    preheader_gap = total_padding - sizeof(FreeListAllocationHeader);
    required_space = size + total_padding;
    remaining = node->block_size - required_space;

    // Insert a new free node with any remaining space
    if (remaining > 0) {
        FreeListNode *new_node = (FreeListNode *)((char *)node + required_space);
        new_node->block_size = remaining;
        freelist_node_insert(&fl->head, node, new_node);
    }

    // Remove the now-used free node
    freelist_node_remove(&fl->head, prev_node, node);

    // Setup header
    header_ptr = (FreeListAllocationHeader *)((char *)node + preheader_gap);
    header_ptr->block_size = required_space;
    header_ptr->padding = preheader_gap;

    fl->used += required_space;

    return (void *)((char *)header_ptr + sizeof(FreeListAllocationHeader));
}

void freelist_node_insert(FreeListNode **phead, FreeListNode *prev_node, FreeListNode *new_node) {
    if (prev_node == NULL) {
        if (*phead != NULL) {
            new_node->next = *phead;
        }
        // NOTE: add to gb mistakes list
        *phead = new_node;
    } else {
        if (prev_node->next == NULL) {
            prev_node->next = new_node;
            new_node->next = NULL;
        } else {
            new_node->next = prev_node->next;
            prev_node->next = new_node;
        }
    }
}

static void freelist_node_remove(FreeListNode **phead, FreeListNode *prev_node, FreeListNode *del_node) {
    if (prev_node == NULL) {
        *phead = del_node->next;
    } else {
        prev_node->next = del_node->next;
    }
}

static void freelist_free(FreeList *fl, void *ptr) {
    FreeListAllocationHeader *header;
    FreeListNode *free_node;
    FreeListNode *node;
    FreeListNode *prev_node = NULL;

    if (ptr == NULL) {
        return;
    }

    // why ptr cast to char* isntead of uintptr_t?
    // :: because char * stays in "pointer land", uintptr_t treats as integer, then needs cast back to a pointer type

    header = (FreeListAllocationHeader *)((char *)ptr - sizeof(FreeListAllocationHeader));
    // why is freenode header set to same address as alloc header? What if they vary in size?
    free_node = (FreeListNode *)((char *)header - header->padding);
    free_node->block_size = header->block_size;
    free_node->next = NULL;

    node = fl->head;
    while (node != NULL) {
        // CSDR: should compare with addr of free_node! not ptr!.
        if (free_node < node) {
            freelist_node_insert(&fl->head, prev_node, free_node);
            break;
        }
        prev_node = node;
        node = node->next;
    }
    // handle when ptr is tail free node or free list is empty (head NULL)
    if (node == NULL) {
        freelist_node_insert(&fl->head, prev_node, free_node);
    }

    fl->used -= free_node->block_size;

    freelist_coalescence(fl, prev_node, free_node);
}

static void freelist_coalescence(FreeList *fl, FreeListNode *prev_node, FreeListNode *free_node) {
    // Why does ref code:
    // (void *)((char *)free_node + free_node->block_size) == free_node->next
    // because free_node is a struct, compiler will scale the addition with block_size by sizeof(struct)!
    // void * isn't necessary but makes intent explicit and suppresses any compiler warnings
    if (free_node->next != NULL &&
        ((void *)((char *)free_node + free_node->block_size) == free_node->next)) {
        free_node->block_size += free_node->next->block_size;
        freelist_node_remove(&fl->head, free_node, free_node->next);
    }
    if (prev_node != NULL && prev_node->next != NULL &&
        ((void *)((char *)prev_node + prev_node->block_size) == free_node)) {
        prev_node->block_size += free_node->block_size;
        freelist_node_remove(&fl->head, prev_node, free_node);
    }
}

static void *freelist_resize_align(FreeList *fl, void *old_memory, size_t old_size, size_t new_size, size_t align) {
    unsigned char *old_mem = (unsigned char *)old_memory;

    assert(freelist__is_power_of_two(align));

    if (old_mem == NULL || old_size == 0) {
        return freelist_alloc_align(fl, new_size, align);

    } else if ((unsigned char *)fl->data <= old_mem && old_mem < (unsigned char *)fl->data + fl->size) {
        if (new_size > old_size) {
            FreeListAllocationHeader *old_mem_header = (FreeListAllocationHeader *)((char *)old_mem - sizeof(FreeListAllocationHeader));
            char *end_old_mem = (char *)old_mem + old_size;

            // find big enough contiguous-to-allocated-block free node
            FreeListNode *node = fl->head;
            FreeListNode *prev_node = NULL;
            FreeListNode *valid_adjacent_node = NULL;
            size_t grow_size = new_size - old_size;
            while (node != NULL) {
                if ((char *)node == (char *)end_old_mem) {
                    if (node->block_size >= grow_size) {
                        valid_adjacent_node = node;
                        break;
                    }
                }
                prev_node = node;
                node = node->next;
            }

            // grow in place
            if (valid_adjacent_node != NULL) {
                // simple extension of old_mem
                old_mem_header->block_size += grow_size;

                // Either:
                // A. remove node entirely if space remaining is too small for a free node
                if ((valid_adjacent_node->block_size - grow_size) < sizeof(FreeListNode)) {
                    old_mem_header->block_size += valid_adjacent_node->block_size - grow_size;
                    fl->used += valid_adjacent_node->block_size;
                    freelist_node_remove(&fl->head, prev_node, valid_adjacent_node);

                // B. move node start and shrink it
                } else {
                    char *end_new_mem = end_old_mem + grow_size;
                    FreeListNode old_free_node = *valid_adjacent_node;
                    size_t shrunken_size = old_free_node.block_size - grow_size;
                    FreeListNode *next = old_free_node.next;
                    valid_adjacent_node = (FreeListNode *)end_new_mem;
                    valid_adjacent_node->block_size = shrunken_size;
                    valid_adjacent_node->next = next;
                    if (prev_node != NULL) {
                        prev_node->next = valid_adjacent_node;
                    } else {
                        fl->head = valid_adjacent_node;
                    }
                    fl->used += grow_size;
                }
                return old_memory;

            // alloc and move
            } else {
                void *new_memory = freelist_alloc_align(fl, new_size, align);
                size_t copy_size = old_size < new_size ? old_size : new_size;
                memmove(new_memory, old_memory, copy_size);
                freelist_free(fl, old_memory);
                return new_memory;
            }

        // NB: new_size == old_size handled by the interface
        // shrink in place
        } else {
            FreeListAllocationHeader *old_mem_header = (FreeListAllocationHeader *)((char *)old_mem - sizeof(FreeListAllocationHeader));

            size_t shrink_size = old_size - new_size;
            old_mem_header->block_size -= shrink_size;

            char *end_old_mem = (char *)old_mem + old_size;
            char *end_modified_mem = (char *)old_mem + new_size;

            // is there a contiguous free node?
            FreeListNode *node = fl->head;
            FreeListNode *prev_node = NULL;
            FreeListNode *adjacent_node = NULL;

            // do work for creating a free node
            FreeListNode *prev_new_node = NULL;

            while (node != NULL) {
                if ((char *)node == (char *)end_old_mem) {
                    adjacent_node = node;
                    break;
                }
                if ((char *)node < (char *)end_old_mem) {
                    prev_new_node = node;
                }
                prev_node = node;
                node = node->next;
            }

            // yes, move to end of modified memory and grow it
            if (adjacent_node != NULL) {
                FreeListNode old_free_node = *adjacent_node;
                size_t grown_size = old_free_node.block_size + shrink_size;
                FreeListNode *next = old_free_node.next;
                adjacent_node = (FreeListNode *)end_modified_mem;
                adjacent_node->block_size = grown_size;
                adjacent_node->next = next;
                if (prev_node != NULL) {
                    prev_node->next = adjacent_node;
                } else {
                    fl->head = adjacent_node;
                }

            // no, create a free node
            } else {
                FreeListNode *new_node = (FreeListNode *)end_modified_mem;
                new_node->block_size = shrink_size;
                freelist_node_insert(&fl->head, prev_new_node, new_node);
            }

            fl->used -= shrink_size;
            return old_memory;
        }
    } else {
        assert(0 && "Memory is out of bounds of the buffer in this free list");
        return NULL;
    }
}

static void *freelist_resize(FreeList *fl, void *old_memory, size_t old_size, size_t new_size) {
    return freelist_resize_align(fl, old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}

// TODO: should reallocate handle these cases or pass down to resize_alloc???
// interface:
// ptr_block    old_size    new_size    effect

// any          any         0           free - return NULL
// NULL         0           >0          fresh alloc
// valid        >0          >0          resize

// NB:
// any          0           any         caller bug

extern void *freelist_reallocate(void *data, void *old_memory, size_t old_size, size_t new_size) {
    // invalid
    if (old_memory == NULL && new_size == 0) {
        return NULL;
    }

    // free
    if (new_size == 0) {
        freelist_free((FreeList *)data, old_memory);
        return NULL;
    }


    // no-op
    if (old_size == new_size) {
        return old_memory;
    }

#ifdef DEBUG
    assert(!(old_memory != NULL && old_size == 0) && "non-NULL ptr_block requires old_size > 0");
#endif

    // fresh or modified alloc
    return freelist_resize((FreeList *)data, old_memory, old_size, new_size);
}

static void freelist_display(FreeList fl) {
	printf("FreeList:: data:%p size:%zu used:%zu head:%p policy:%s\n",
        fl.data,
        fl.size,
        fl.used,
        fl.head,
        fl.policy == 0 ? "find first" : "find best"
    );
}

#endif
#endif
