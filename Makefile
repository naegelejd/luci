CC = gcc
CFLAGS =

TARGET = luci
#OBJECTS = driver.o ast.o env.o functions.o parser.tab.o lexer.yy.o
LIBS = 

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INSTALLDIR = /usr/local/bin/

EXECUTABLE = $(BINDIR)/$(TARGET)

SOURCES := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJECTS += $(OBJDIR)/parser.tab.o $(OBJDIR)/lexer.yy.o

all: $(TARGET)

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p obj/
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(SRCDIR)/%.yy.c: $(SRCDIR)/%.l
	flex -o $@ $<

$(SRCDIR)/%.tab.c: $(SRCDIR)/%.y
	bison -d -o $@ $<

$(OBJDIR)/%.yy.o: $(SRCDIR)/%.yy.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.tab.o: $(SRCDIR)/%.tab.y
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	mkdir -p $(INSTALLDIR)
	cp $(TARGET) $(INSTALLDIR)

doc: Doxyfile *.c
	doxygen Doxyfile >> /dev/null

clean:
	rm -f $(TARGET) $(OBJDIR)/* doc/*
