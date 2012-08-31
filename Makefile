CC=gcc -g

all: luci

luci: ast.o env.o functions.o parser.o lexer.o
	$(CC) -o luci *.o

env.o: env.c
	$(CC) -c env.c

functions.o: functions.c
	$(CC) -c functions.c

ast.o: ast.c
	$(CC) -c ast.c

parser.o: parser.c
	$(CC) -c parser.c

lexer.o: lexer.c
	$(CC) -c lexer.c

#graph: parser.dot
#	dot -Tpng parser.dot > graph.png
#parser.dot: parser.c

parser.c: parser.y
	bison -g -d -o parser.c parser.y

lexer.c: lexer.l
	flex -o lexer.c lexer.l

clean:
	rm -rf *.o luci
