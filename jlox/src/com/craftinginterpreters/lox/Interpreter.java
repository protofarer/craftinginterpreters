package com.craftinginterpreters.lox;

import java.util.List;

class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {
	private Environment environment = new Environment();

	void interpret(List<Stmt> statements) {
		try {
			for (Stmt statement : statements) {
				execute(statement);
			}
		} catch (RuntimeError error) {
			Lox.runtimeError(error);
		}
	}

	@Override
	public Object visitLiteralExpr(Expr.Literal expr) {
		return expr.value;
	}

	@Override
	public Object visitGroupingExpr(Expr.Grouping expr) {
		return evaluate(expr.expression);
	}

	@Override
	public Void visitVarStmt(Stmt.Var stmt) {
		Object value = null;
		if (stmt.initializer != null) {
			value = evaluate(stmt.initializer);
		}

		environment.define(stmt.name.lexeme, value);
		return null;
	}

	@Override
	public Object visitAssignExpr(Expr.Assign expr) {
		Object value = evaluate(expr.value);
		environment.assign(expr.name, value);
		return value;
	}

	private Object evaluate(Expr expr) {
		return expr.accept(this);
	}

	private void execute(Stmt stmt) {
		stmt.accept(this);
	}

	@Override
	public Void visitExpressionStmt(Stmt.Expression stmt) {
		evaluate(stmt.expression);
		return null;
	}

	@Override
	public Void visitPrintStmt(Stmt.Print stmt) {
		Object value = evaluate(stmt.expression);
		System.out.println(stringify(value));
		return null;
	}

	@Override
	public Object visitUnaryExpr(Expr.Unary expr) {
		Object right = evaluate(expr.right);

		switch (expr.operator.type) {
			case BANG:
				return !isTruthy(right);
			case MINUS:
			checkNumberOperand(expr.operator, right);
				return -(double)right;
		}

		// Unreachable
		return null;
	}

	@Override
	public Object visitVariableExpr(Expr.Variable expr) {
		return environment.get(expr.name);
	}

	@Override
	public Object visitBinaryExpr(Expr.Binary expr) {
		Object left = evaluate(expr.left);
		Object right = evaluate(expr.right);

		switch (expr.operator.type) {
			case GREATER:
				checkNumberOperand(expr.operator, left, right);
				return (double)left > (double)right;
			case GREATER_EQUAL:
				checkNumberOperand(expr.operator, left, right);
				return (double)left >= (double)right;
			case LESS:
				checkNumberOperand(expr.operator, left, right);
				return (double)left < (double)right;
			case LESS_EQUAL:
				checkNumberOperand(expr.operator, left, right);
				return (double)left <= (double)right;
			case BANG_EQUAL: return !isEqual(left, right);
			case EQUAL_EQUAL: return isEqual(left, right);
			case MINUS:
				checkNumberOperand(expr.operator, left, right);
				return (double)left - (double)right;
			case PLUS:
				if (left instanceof Double && right instanceof Double) {
					return (double)left + (double)right;
				}

				if (left instanceof String && right instanceof String) {
					return (String)left + (String)right;
				}

				if (left instanceof String && right instanceof Double) {
					String text = doubleToString((double)right);
					return (String)left + text;
				}

				if (left instanceof Double && right instanceof String) {
					String text = doubleToString((double)left);
					return text + (String)right;
				}

				if (left instanceof String && right == null) {
					return (String)left + "nil";
				}

				if (left == null && right instanceof String) {
					return "nil" + (String)right;
				}

				if (left instanceof String) {
					// right should be an object, boolean
					String text = right.toString();
					// System.out.println(text);
					return (String)left + text;
				}
				if (right instanceof String) {
					// left should be an object, boolean
					String text = left.toString();
					return text + (String)right;
				}

				throw new RuntimeError(expr.operator, 
					"At least one operand must be a string.");
			case SLASH:
				checkNumberOperand(expr.operator, left, right);
				if ((double)right == 0) throw new RuntimeError(expr.operator, "Cannot divide by zero." );
				return (double)left / (double)right;
			case STAR:
				checkNumberOperand(expr.operator, left, right);
				return (double)left * (double)right;
		}

		// Unreachable
		return null;
	}

	private boolean isTruthy(Object object) {
		if (object == null) return false;
		if (object instanceof Boolean) return (boolean)object;
		return true;
	}

	private boolean isEqual(Object a, Object b) {
		if (a == null && b == null) return true;
		if (a == null) return false;

		return a.equals(b);
	}

	private String stringify(Object object) {
		if (object == null) return "nil";

		if (object instanceof Double) {
			String text = object.toString();
			if (text.endsWith(".0")) {
				text = text.substring(0, text.length() - 2);
			}
			return text;
		}

		return object.toString();
	}

	private void checkNumberOperand(Token operator, Object operand) {
		if (operand instanceof Double) return;
		throw new RuntimeError(operator, "Operand must be a number.");
	}

	private void checkNumberOperand(Token operator, Object left, Object right) {
		if (left instanceof Double && right instanceof Double) return;
		throw new RuntimeError(operator, "Operands must be numbers.");
	}

	private String doubleToString(Double x) {
		String text = Double.toString(x);
		// truncate for numbers
		if (text.endsWith(".0")) {
			text = text.substring(0, text.length() - 2);
		}
		return text;
	}
}