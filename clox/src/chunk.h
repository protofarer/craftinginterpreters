#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL,
	OP_SET_GLOBAL,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_NEGATE,
	OP_PRINT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_RETURN,
} OpCode;

typedef struct {
	int line;  // source line number
	int count; // how many instructions share this line number
} LineEntry;

typedef struct {
	int capacity;
	int count;
	LineEntry *entries;
} LineEntriesArray;

typedef struct {
	int count;
	int capacity;
	uint8_t *code;
	ValueArray constants;
	LineEntriesArray lineEntries;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);
int addConstant(Chunk *chunk, Value value);

void writeConstant(Chunk *chunk, Value value, int line);

void initLineEntriesArray(LineEntriesArray *array);
void updateLineEntriesArray(LineEntriesArray *array, int line);
void freeLineEntriesArray(LineEntriesArray *array);
int getLine(Chunk *chunk, int offset);

#endif
