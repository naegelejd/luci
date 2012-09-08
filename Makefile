CC = gcc
CFLAGS =

EXECUTABLE = luci
OBJECTS = driver.o ast.o env.o functions.o parser.tab.o lexer.yy.o
LIBRARIES = 

INSTALL_DIR = /usr/local/bin/

all: $(EXECUTABLE)

debug: CFLAGS += -DDEBUG -g
debug: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)

%.yy.o: %.l
	flex -o $*.yy.c $<
	$(CC) $(CFLAGS) -c $*.yy.c

%.tab.o: %.y
	bison -d $<
	$(CC) $(CFLAGS) -c $*.tab.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS) *.yy.c *.tab.c

install:
	mkdir -p $(INSTALL_DIR)
	cp $(EXECUTABLE) $(INSTALL_DIR)

