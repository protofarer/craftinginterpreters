compile:
	mkdir -p bin
	javac -d bin/ src/com/craftinginterpreters/lox/*.java

run:
	java -cp bin com.craftinginterpreters.lox.Lox

print-ast:
	java -cp bin/ com.craftinginterpreters.lox.AstPrinter

clean:
	rm -rf bin/ *.class