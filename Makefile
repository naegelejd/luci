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
OBJECTS := $(OBJDIR)/ast.o $(OBJDIR)/env.o $(OBJDIR)/driver.o $(OBJDIR)/functions.o $(OBJDIR)/parser.tab.o $(OBJDIR)/lexer.yy.o

#all: $(TARGET)
all: debug

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p obj/
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

#$(SRCDIR)/%.yy.c: $(SRCDIR)/%.l

#$(SRCDIR)/%.tab.c: $(SRCDIR)/%.y

$(OBJDIR)/%.tab.o: $(SRCDIR)/%.y
	bison -d -o $(SRCDIR)/$*.tab.c $<
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.tab.c

$(OBJDIR)/%.yy.o: $(SRCDIR)/%.l
	flex -o $(SRCDIR)/$*.yy.c $<
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.yy.c

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	mkdir -p $(INSTALLDIR)
	cp $(TARGET) $(INSTALLDIR)

doc: Doxyfile $(SRCDIR)/*.c
	doxygen Doxyfile >> /dev/null

clean:
	rm -f $(TARGET) $(OBJDIR)/*
	rm -rf doc/*
