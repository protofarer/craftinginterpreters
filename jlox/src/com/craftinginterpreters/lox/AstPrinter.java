package com.craftinginterpreters.lox;

class AstPrinter implements Expr.Visitor<String>, Stmt.Visitor<String> {
	String print(Expr expr) {
		return expr.accept(this);
	}

	String print(Stmt stmt) {
		return stmt.accept(this);
	}

	@Override
	public String visitBinaryExpr(Expr.Binary expr) {
		return parenthesize(expr.operator.lexeme, expr.left, expr.right);
	}

	@Override
	public String visitGroupingExpr(Expr.Grouping expr) {
		return parenthesize("group", expr.expression);
	}

	@Override
	public String visitLiteralExpr(Expr.Literal expr) {
		if (expr.value == null) return "nil";
		return expr.value.toString();
	}

	@Override
	public String visitUnaryExpr(Expr.Unary expr) {
		return parenthesize(expr.operator.lexeme, expr.right);
	}

	@Override
	public String visitVariableExpr(Expr.Variable expr) {
		return expr.name.lexeme;
	}

	@Override
	public String visitConditionalExpr(Expr.Conditional expr) {
		// TODO: should pass lexeme? "?:" ?
		return parenthesize("?:", expr.condition, expr.elseBranch, expr.thenBranch);
	}

	@Override
	public String visitExpressionStmt(Stmt.Expression stmt) {
		return parenthesize("expression-statement:", stmt.expression);
	}

	@Override
	public String visitPrintStmt(Stmt.Print stmt) {
		return parenthesize("print-statement:", "'", stmt.expression, "'");
	}

	@Override
	public String visitVarStmt(Stmt.Var stmt) {
		return parenthesize("var-statement:", stmt.name.lexeme, " = ", stmt.initializer);
	}

	@Override
	public String visitAssignExpr(Expr.Assign expr) {
		return parenthesize("assign-expr:", expr.name.lexeme, " = ", expr.value);
	}

	@Override
	public String visitLogicalExpr(Expr.Logical expr) {
		return parenthesize(expr.operator.lexeme, expr.left, expr.right);
	}

	@Override
	public String visitCallExpr(Expr.Call expr) {
		return parenthesize("call", expr.callee, expr.arguments);
	}

	// TODO: what should below be???

	@Override
	public String visitBlockStmt(Stmt.Block stmt) {
		// return parenthesize("assign-expr:", expr.name.lexeme, " = ", expr.value);
		return "";
	}
	@Override
	public String visitIfStmt(Stmt.If stmt) {
		return "";
	}
	@Override
	public String visitWhileStmt(Stmt.While stmt) {
		return "";
	}
	@Override
	public String visitFunctionStmt(Stmt.Function stmt) {
		return "";
	}
	@Override
	public String visitReturnStmt(Stmt.Return stmt) {
		return "";
	}

	///////////////////////////////////////

	private String parenthesize(String name, Object... parts) {
		StringBuilder builder = new StringBuilder();

		builder.append("(").append(name);
		for (Object part : parts) {
			builder.append(" ");
			if (part instanceof Expr) {
				builder.append(((Expr) part).accept(this));
			} else if (part instanceof Stmt) {
				builder.append(((Stmt) part).accept(this));
			} else {
				builder.append(part.toString());
			}
		}
		builder.append(")");
		return builder.toString();
	}

	public static void main(String[] args) {
		Expr expression = new Expr.Binary(
				new Expr.Unary(
					new Token(TokenType.MINUS, "-", null, 1),
					new Expr.Literal(123)),
				new Token(TokenType.STAR, "*", null, 1),
				new Expr.Grouping(
					new Expr.Literal(45.67)));

		System.out.println(new AstPrinter().print(expression));
	}

}
