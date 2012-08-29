CC=gcc -g

all: luci

luci: ast.o astexec.o parser.o lexer.o
	$(CC) -o luci *.o

astexec.o: astexec.c
	$(CC) -c astexec.c

ast.o: ast.c
	$(CC) -c ast.c

parser.o: parser.c
	$(CC) -c parser.c

lexer.o: lexer.c
	$(CC) -c lexer.c

parser.c: parser.y
	bison -d -o parser.c parser.y

lexer.c: lexer.l
	flex -o lexer.c lexer.l

clean:
	rm -rf *.o luci
