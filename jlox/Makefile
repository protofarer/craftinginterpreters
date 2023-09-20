all:
	mkdir -p bin
	javac -d bin/ src/com/craftinginterpreters/lox/*.java

run:
	java -cp bin com.craftinginterpreters.lox.Lox

# Declare a phony target to silence Make's 'Nothing to be done' message
.PHONY: run-file

run-file: always_run
	java -cp bin com.craftinginterpreters.lox.Lox $(filter-out $@,$(MAKECMDGOALS))

run-dbg:
	java -cp bin com.craftinginterpreters.lox.Lox script.lox

always_run:
	@:

# Catch-all target to silence Make's 'Nothing to be done' message for other arguments
%: always_run
	@:

dbg-ast:
	javac -d bin/ src/com/craftinginterpreters/lox/*.java
	java -cp bin/ com.craftinginterpreters.lox.AstPrinter

cpl-genast:
	mkdir -p bin
	javac -d bin/ src/com/craftinginterpreters/lox/tool/GenerateAst.java

run-genast:
	java -cp bin/ com.craftinginterpreters.lox.tool.GenerateAst src/com/craftinginterpreters/lox

genast:
	make cpl-genast
	make run-genast

clean:
	rm -rf bin/ *.class
	