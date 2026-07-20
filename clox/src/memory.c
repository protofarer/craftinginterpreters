#include <stdlib.h>
#include <sys/mman.h>
#include "memory.h"
#include "k99_allocator.h"
#define K99_FREELIST_IMPLEMENTATION
#include "k99_freelist.h"

static Allocator g_allocator;
static FreeList fl;

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	void *result = g_allocator.reallocate(g_allocator.data, pointer, oldSize, newSize);
	if (result == NULL && newSize > 0) exit(1);
	return result;
}

void initMemory(size_t size) {
	void *buf = mmap(0, size, PROT_READ | PROT_WRITE,
					 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (buf == MAP_FAILED) {
		printf("Failed alloc program memory");
		exit(1);
	}
	g_allocator = make_freelist_allocator(&fl, buf, size, PlacementPolicyFindBest);
}
