package com.craftinginterpreters.lox;

class VisitorRPN implements Expr.Visitor<String> {
	@Override
	public String visitBinaryExpr(Expr.Binary expr) {
		String left = expr.left.accept(this);
		String right = expr.right.accept(this);
		return left + " " + right + " " + expr.operator.lexeme;
	}

	@Override
	public String visitGroupingExpr(Expr.Grouping expr) {
		return expr.expression.accept(this);
	}

	@Override
	public String visitUnaryExpr(Expr.Unary expr) {
		return expr.right + " " + expr.operator;
	}

	@Override
	public String visitLiteralExpr(Expr.Literal expr) {
		return expr.value.toString();
	}

	public static void main(String[] args) {
		Expr expr = new Expr.Binary(
      new Expr.Grouping(
        new Expr.Binary(
          new Expr.Literal(1),
          new Token(TokenType.PLUS, "+", null, 1),
          new Expr.Literal(2)
        )
      ),
      new Token(TokenType.STAR, "*", null, 1),
      new Expr.Literal(4)
    );

    VisitorRPN visitor = new VisitorRPN();
    String rpn = expr.accept(visitor);
    System.out.println(rpn); // Should print "1 2 + 4 *"
	}
}
