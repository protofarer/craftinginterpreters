package com.craftinginterpreters.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.Optional;

public class Lox {
	private static final Interpreter interpreter = new Interpreter();
	static boolean hadError = false;
	static boolean hadRuntimeError = false;

	// NOTE: strictly for not throwing the semicolon parse error when REPL evaluates an expression without an ending semicolon. Feels hacky.
	static boolean suppressErrors = false;

	public static void main(String[] args) throws IOException {
		if (args.length > 1) {
			System.out.println("Usage: jlox [script]");
			System.exit(64);
		} else if (args.length == 1) {
			runFile(args[0]);
		} else {
			runPrompt();
		}
	}

	private static void runFile(String path) throws IOException {
		byte[] bytes = Files.readAllBytes(Paths.get(path));
		run(new String(bytes, Charset.defaultCharset()));

		if (hadError) System.exit(65);
		if (hadRuntimeError) System.exit(70);
	}

	private static void runPrompt() throws IOException {
		InputStreamReader input = new InputStreamReader(System.in);
		BufferedReader reader = new BufferedReader(input);
		boolean printAst = false;

		for (;;) {
			System.out.print("> ");
			String line = reader.readLine();
			if (line == null) break;

			if (line.equals(".ast")) {
				printAst = !printAst;
				System.out.println("AST printing " + (printAst ? "enabled" : "disabled"));
				continue;
			}

			// Try parsing as an expression first (silently)
			Scanner scanner = new Scanner(line);
			List<Token> tokens = scanner.scanTokens();
			Parser parser = new Parser(tokens);

			suppressErrors = true;
			Expr expr = parser.parseExpression();
			suppressErrors = false;

			if (!hadError && expr != null) {
				// Successfully parsed as expression
				hadError = false;

				if (printAst) {
					System.out.println("AST: " + new AstPrinter().print(expr));
				}

				try {
					Object value = interpreter.evaluate(expr);
					System.out.println(interpreter.stringify(value));
				} catch (RuntimeError error) {
					Lox.runtimeError(error);
				}
				continue;
			}

			// Expression parse failed, try as statements
			hadError = false;
			scanner = new Scanner(line);
			tokens = scanner.scanTokens();
			parser = new Parser(tokens);
			List<Stmt> statements = parser.parse();

			if (hadError) {
				hadError = false;
				continue;
			}

			for (Stmt statement : statements) {
				if (printAst) {
					System.out.println("AST: " + new AstPrinter().print(statement));
				}

				if (statement instanceof Stmt.Expression) {
					Object value = interpreter.evaluateExpressionStmt((Stmt.Expression) statement);
					System.out.println(interpreter.stringify(value));
				} else {
					interpreter.execute(statement);
				}
			}
		}
	}

	private static void run(String source) {
		Scanner scanner = new Scanner(source);
		List<Token> tokens = scanner.scanTokens();
		Parser parser = new Parser(tokens);
		List<Stmt> statements = parser.parse();

		// Stop if there was a syntax error
		if (hadError) return;

		Resolver resolver = new Resolver(interpreter);
		resolver.resolve(statements);

		// Stop if there was a resolution error
		if (hadError) return;

		interpreter.interpret(statements);
	}

	static void error(int line, String message) {
		report(line, "", message);
	}

	private static void report(int line, String where, String message) {
		System.err.println("[line " + line + "] Error" + where + ": " + message);
		hadError = true;
	}

	static void error(Token token, String message) {
		if (suppressErrors) return;
		if (token.type == TokenType.EOF) {
			report(token.line, " at end", message);
		} else {
			report(token.line, " at '" + token.lexeme + "'", message);
		}
	}

	static void runtimeError(RuntimeError error) {
		System.err.println(error.getMessage() + "\n[line " + error.token.line + "]");
		hadRuntimeError = true;
	}
}
