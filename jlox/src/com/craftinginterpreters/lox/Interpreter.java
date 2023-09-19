package com.craftinginterpreters.lox;

class Interpreter implements Expr.Visitor<Object> {
	@Override
	public Object visitLiteralExpr(Expr.Literal expr) {
		return expr.value;
	}

	@Override
	public Object visitGroupingExpr(Expr.Grouping expr) {
		return evaluate(expr.expression);
	}

	private Object evaluate(Expr expr) {
		return expr.accept(this);
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

	void interpret(Expr expression) {
		try {
			Object value = evaluate(expression);
			System.out.println(stringify(value));
		} catch (RuntimeError error) {
			Lox.runtimeError(error);
		}
	}
}
