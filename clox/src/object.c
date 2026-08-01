#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
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

static ObjString* allocateString(const char* chars, int length, uint32_t hash) {
	ObjString* string = makeString(length);
	memcpy(string->chars, chars, length);
	string->chars[length] = '\0';
	string->hash = hash;
	tableSet(&vm.strings, string, NIL_VAL);
	return string;
}

static uint32_t hashString(const char* key, int length) {
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; i++) {
		hash ^= (uint8_t)key[i];
		hash *= 16777619;
	}
	return hash;
}

// FAM approach uses makeString instead
/* ObjString* takeString(char* chars, int length) { */
/* 	ObjString* string = allocateString(chars, length); */
/* 	FREE_ARRAY(char, chars, length + 1); */
/* 	uint32_t hash = hashString(chars, length); */
/* 	return allocateString(chars, length, hash); */
/* } */

ObjString* makeString(int length) {
	ObjString* string = ALLOCATE_OBJ_FLEX(ObjString, OBJ_STRING, length + 1);
	string->length = length;
	return string;
}

ObjString* copyString(const char* chars, int length) {
	uint32_t hash = hashString(chars, length);
	ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
	if (interned != NULL) return interned;

	/* char* heapChars = ALLOCATE(char, length + 1); */
	/* memcpy(heapChars, chars, length); */
	/* heapChars[length] = '\0'; */

	return allocateString(chars, length, hash);
}

void printObject(Value value) {
	switch (OBJ_TYPE(value)) {
		case OBJ_STRING:
			printf("%s", AS_CSTRING(value));
			break;
	}
}

ObjString* intern(char* chars, int length) {
	uint32_t hash = hashString(chars, length);
	ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
	if (interned != NULL) {
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}
	return NULL;
}
