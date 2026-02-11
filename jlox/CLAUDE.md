# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **jlox**, a tree-walk interpreter for the Lox programming language, implemented in Java. It's part of the "Crafting Interpreters" book project.

## Build and Run Commands

The project uses `just` (a command runner similar to make):

- **Build**: `just build` - Compiles all Java files to `bin/`
- **Run REPL**: `just run` - Starts interactive REPL mode
- **Build and run**: `just go` - Compiles and starts REPL in one command
- **Run file**: `just runfile <filename>` - Executes a Lox script
- **Build and run file**: `just gofile <filename>` - Compiles and executes a Lox script
- **Generate AST classes**: `just gen-ast` - Regenerates Expr.java and Stmt.java from definitions
- **Clean**: `just clean` - Removes the bin/ directory

All compiled classes go to `bin/` directory. The main entry point is `com.craftinginterpreters.lox.Lox`.

## Architecture

### Pipeline: Source → Tokens → AST → Execution

The interpreter follows a classic three-stage pipeline:

1. **Scanning** (`Scanner.java`) - Converts source text into tokens
2. **Parsing** (`Parser.java`) - Builds an Abstract Syntax Tree from tokens
3. **Interpretation** (`Interpreter.java`) - Evaluates the AST using the Visitor pattern

### Key Components

**Main Entry Point (`Lox.java`)**
- Handles both REPL mode (no args) and file execution (single arg)
- Maintains global error state (`hadError`, `hadRuntimeError`)
- Creates a single shared `Interpreter` instance for the session
- Exit codes: 64 (usage), 65 (syntax error), 70 (runtime error)

**Lexical Analysis (`Scanner.java`)**
- Converts raw source into `List<Token>`
- Tracks line numbers for error reporting
- Handles keywords via static HashMap lookup

**Syntax Analysis (`Parser.java`)**
- Recursive descent parser implementing the Lox grammar
- Operator precedence (lowest to highest): comma → conditional → equality → comparison → term → factor → unary → primary
- Includes error productions for better error messages (e.g., detecting missing left-hand operands)
- Synchronizes on statement boundaries after parse errors

**Runtime Execution (`Interpreter.java`)**
- Implements Visitor pattern for both `Expr.Visitor<Object>` and `Stmt.Visitor<Void>`
- Expression evaluation returns `Object` (dynamic typing)
- Statement execution returns `Void`
- Type checking happens at runtime (e.g., `checkNumberOperands()`)

### AST Generation System

**Code Generation Tool (`GenerateAst.java`)**
- Generates `Expr.java` and `Stmt.java` from type definitions
- Each AST node type is defined as a static nested class
- Automatically generates Visitor interfaces and accept() methods
- **Important**: When adding new expression or statement types, modify the type lists in `GenerateAst.java` and run `just gen-ast`

**Current AST Types:**
- Expressions: `Binary`, `Grouping`, `Literal`, `Unary`, `Conditional`
- Statements: `Expression`, `Print`

### Visitor Pattern Usage

All AST traversal uses the Visitor pattern:
- Each AST base class (`Expr`, `Stmt`) defines a `Visitor<R>` interface
- Visitors implement methods like `visitBinaryExpr()`, `visitPrintStmt()`
- AST nodes have an `accept(Visitor<R>)` method that dispatches to the correct visit method
- `AstPrinter.java` demonstrates this pattern for debugging (pretty-prints AST)
- `Interpreter.java` uses it for evaluation and execution

### Error Handling

Two error types:
1. **Parse errors** - Reported via `Lox.error(Token, String)`, sets `hadError = true`
2. **Runtime errors** - Thrown as `RuntimeError`, caught in `Interpreter.interpret()`, reported via `Lox.runtimeError()`

## Development Workflow

**Adding New Language Features:**

1. Add tokens to `TokenType.java` if needed
2. Update `Scanner.java` to recognize new tokens
3. Modify AST definitions in `GenerateAst.java`
4. Run `just gen-ast` to regenerate AST classes
5. Update `Parser.java` with new grammar rules
6. Implement visitor methods in `Interpreter.java`
7. Test via REPL or create test files

**Testing:**

Create `.lox` files in the project root and run with `just gofile test.lox`
