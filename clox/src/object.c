#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// to use FAM, i suppose
// copyString: rm chars alloc, pass chars thru to allocateString
// allocateString: ALLOCATE_OBJ accepts fam_bytes

// takeString: no change, but now won't use an existing chars, which is what happens in concatenate. it will always copy


#define ALLOCATE_OBJ(type, objectType) \
	(type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE_OBJ_FLEX(type, objectType, extraSize) \
	(type*)allocateObject(sizeof(type) + extraSize, objectType)

static Obj* allocateObject(size_t size, ObjType type) {
	Obj* object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;

	object->next = vm.objects;
	vm.objects = object;
	return object;
}

static ObjString* allocateString(const char* chars, int length) {
	ObjString* string = makeString(length);
	memcpy(string->chars, chars, length);
	string->chars[length] = '\0';
	return string;
}

ObjString* takeString(char* chars, int length) {
	ObjString* string = allocateString(chars, length);
	FREE_ARRAY(char, chars, length + 1);
	return allocateString(chars, length);
}

ObjString* makeString(int length) {
	ObjString* string = ALLOCATE_OBJ_FLEX(ObjString, OBJ_STRING, length + 1);
	string->length = length;
	return string;
}

ObjString* copyString(const char* chars, int length) {
	/* char* heapChars = ALLOCATE(char, length + 1); */
	/* memcpy(heapChars, chars, length); */
	/* heapChars[length] = '\0'; */
	return allocateString(chars, length);
}

void printObject(Value value) {
	switch (OBJ_TYPE(value)) {
		case OBJ_STRING:
			printf("%s", AS_CSTRING(value));
			break;
	}
}
