#include "chunk.h"
#include "debug.h"
#include "common.h"
#include "memory.h"
#include "vm.h"

#include "k99_defines.h"

int main(int argc, const char *argv[]) {
	initMemory(Megabytes(1));

	initVM();

	Chunk chunk;
	initChunk(&chunk);

	// -((1.2 + 3.4) / 5.6)
	/* writeConstant(&chunk, 1.2, 123); */
	/* writeConstant(&chunk, 3.4, 123); */
	/* writeChunk(&chunk, OP_ADD, 123); */
	/**/
	/* writeConstant(&chunk, 5.6, 123); */
	/* writeChunk(&chunk, OP_DIVIDE, 123); */
	/**/
	/* writeChunk(&chunk, OP_NEGATE, 123); */

	/* 1 + 2 * 3 - 4 / -5 */
	writeConstant(&chunk, 1, 123);
	writeConstant(&chunk, 2, 123);
	writeConstant(&chunk, 3, 123);
	writeChunk(&chunk, OP_MULTIPLY, 123);
	writeChunk(&chunk, OP_ADD, 123);

	writeConstant(&chunk, 4, 123);
	writeConstant(&chunk, 5, 123);
	writeChunk(&chunk, OP_NEGATE, 123);
	writeChunk(&chunk, OP_DIVIDE, 123);
	writeChunk(&chunk, OP_SUBTRACT, 123);

	writeChunk(&chunk, OP_RETURN, 10000);

	disassembleChunk(&chunk, "test chunk");
	interpret(&chunk);
	freeVM();
	freeChunk(&chunk);
	return 0;
}

/* 1 + 2 * 3 - 4 / -5 */
/**/
/* push 1 */
/* push 2 */
/* push 3 */
/* mul */
/* add */
/* push 4 */
/* push 5 */
/* negate */
/* divide */
/* minus */

// 4 - 3 * -2

// NO NEGATE! -1 opcode BUT +2 instructions per negate
/* push 4 */
/**/
/* push 3 */
/* push 2 */
/**/
/* push 2 */
/* push 2 */
/* subtract */
/* subtract */
/**/
/* mul */
/* sub */

// NO SUBTRACT! -1 opcode BUT +1 instruction per subtract
/* push 4 */
/* push 3 */
/* push 2 */
/* neg */
/* mul */
/* neg */
/* add */
