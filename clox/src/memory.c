#include <stdlib.h>
#include <sys/mman.h>
#include "memory.h"
#include "k99_allocator.h"
#include "object.h"
#include "vm.h"
#define K99_FREELIST_IMPLEMENTATION
#include "k99_freelist.h"

static Allocator g_allocator;
static FreeList fl;

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	void *result = g_allocator.reallocate(g_allocator.data, pointer, oldSize, newSize);
	if (result == NULL && newSize > 0) exit(1);
	return result;
}

static void freeObject(Obj* object) {
	switch (object->type) {
		case OBJ_STRING: {
			ObjString* string = (ObjString*)object;

			/* FREE_ARRAY(char, string->chars, string->length + 1); */
			/* FREE(ObjString, object); */
			reallocate(object, sizeof(ObjString) + string->length + 1, 0);
			break;
		}
	}
}

void freeObjects() {
	Obj* object = vm.objects;
	while (object != NULL) {
		Obj* next = object->next;
		freeObject(object);
		object = next;
	}
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
