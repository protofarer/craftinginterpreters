# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

This is clox — the C implementation of the Lox language from [Crafting Interpreters](https://craftinginterpreters.com/). The codebase follows the book chapter by chapter.

## Build & Run

```sh
just dev        # compile all src/*.c with clang and run build/main
```

Or manually:

```sh
clang src/*.c -o build/main && ./build/main
```

No test suite exists yet — correctness is verified by running `main.c` and inspecting stdout.

## Architecture

The interpreter is a bytecode VM. Components so far:

- **`chunk`** — a dynamic array of `uint8_t` bytecode plus a `ValueArray` of constants and a `LineEntriesArray` for source-line tracking. The central data structure everything else touches.
- **`value`** — `Value` is currently a plain `double`. `ValueArray` is a growable array of `Value`.
- **`debug`** — `disassembleChunk` / `disassembleInstruction` for human-readable bytecode dumps. Also owns `LineEntry` / `LineEntriesArray` and the run-length encoding of source lines (`updateLineEntriesArray` / `getLine`).
- **`memory`** — single `reallocate()` function backing three macros (`GROW_CAPACITY`, `GROW_ARRAY`, `FREE_ARRAY`) used everywhere for dynamic arrays.
- **`common`** — shared stdint/stdbool/stddef includes.
- **`main.c`** — scratch harness; manually builds a chunk and calls the disassembler.

### Line-number encoding

Rather than a parallel `int lines[]` array (book ch14 baseline), the code uses run-length encoding: `LineEntriesArray` stores `{line, count}` pairs. `getLine(chunk, offset)` walks entries summing counts to find which line an instruction offset falls on.

## Code Style

- C99 (`-std=c99`) — set in both the `justfile` build command and `.clangd` for the LSP
- Compiled with `-Wall -Wextra -Wshadow`
- Formatting: LLVM style, 4-space tabs, braces attached (`BreakBeforeBraces: Attach`) — enforced by `.clang-format`
- All dynamic arrays follow the same init/write/free pattern (see `ValueArray` as the canonical example)
