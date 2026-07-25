#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	initValueArray(&chunk->constants);
	initLineEntriesArray(&chunk->lineEntries);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code =
			GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->count++;

	updateLineEntriesArray(&chunk->lineEntries, line);
}

void freeChunk(Chunk *chunk) {
	freeLineEntriesArray(&chunk->lineEntries);
	freeValueArray(&chunk->constants);
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value) {
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void writeConstant(Chunk *chunk, Value value, int line) {
	int idx = addConstant(chunk, value);
	if (idx < 256) {
		writeChunk(chunk, OP_CONSTANT, line);
		writeChunk(chunk, idx, line); // int truncated to uint8_t
	} else {
		writeChunk(chunk, OP_CONSTANT_LONG, line);
		writeChunk(chunk, idx, line); // int truncated to uint8_t
		writeChunk(chunk, idx >> 8, line);
		writeChunk(chunk, idx >> 16, line);
	}
}

void initLineEntriesArray(LineEntriesArray *array) {
	array->entries = NULL;
	array->capacity = 0;
	array->count = 0;
}

void updateLineEntriesArray(LineEntriesArray *array, int line) {
	if (array->count > 0 && array->entries[array->count - 1].line == line) {
		LineEntry *entry = &array->entries[array->count - 1];
		entry->count++;
	} else {
		if (array->capacity < array->count + 1) {
			int oldCapacity = array->capacity;
			array->capacity = GROW_CAPACITY(oldCapacity);
			array->entries = GROW_ARRAY(LineEntry, array->entries, oldCapacity,
										array->capacity);
		}
		array->entries[array->count++] = (LineEntry){line, 1};
	}
}

void freeLineEntriesArray(LineEntriesArray *array) {
	FREE_ARRAY(LineEntry, array->entries, array->capacity);
	initLineEntriesArray(array);
}

int getLine(Chunk *chunk, int offset) {
	int sum = 0;
	for (int i = 0; i < chunk->lineEntries.count; i++) {
		LineEntry entry = chunk->lineEntries.entries[i];
		sum += entry.count;
		if (offset + 1 <= sum) { // because offset == 0 is sum == 1
			return entry.line;
		}
	}
	return -1;
}
