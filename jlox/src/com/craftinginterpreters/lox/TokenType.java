package com.craftinginterpreters.lox;

enum TokenType {
	// single-char
	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

	// pair of single-chars (ternary operator)
	QUESTION, COLON,

	// one or two char
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,

	// literals
	IDENTIFIER, STRING, NUMBER,

	// keywords
	AND, CLASS, ELSE, FALSE, FOR, FUN, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

	EOF
}
