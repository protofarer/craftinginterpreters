compile:
	mkdir -p bin
	javac -d bin/ src/com/craftinginterpreters/lox/*.java

print-ast:
	java -cp bin/ com.craftinginterpreters.lox.AstPrinter

clean:
	rm -rf bin/ *.class