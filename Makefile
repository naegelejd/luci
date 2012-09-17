CC = gcc
CFLAGS =

TARGET = luci
LIBS =

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INSTALLDIR = /usr/local/bin/

EXECUTABLE = $(BINDIR)/$(TARGET)

SOURCES := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS := $(addprefix $(OBJDIR)/,ast.o env.o common.o functions.o parser.tab.o lexer.yy.o)

#all: $(EXECUTABLE)
all: debug

debug: CFLAGS += -DDEBUG -g
debug: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(EXECUTABLE): | $(BINDIR)

$(BINDIR):
	test -d $(BINDIR) || mkdir $(BINDIR)

$(OBJDIR)/%.tab.o: $(SRCDIR)/%.y
	bison -d -o $(SRCDIR)/$*.tab.c $<
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.tab.c

$(OBJDIR)/%.yy.o: $(SRCDIR)/%.l
	flex -o $(SRCDIR)/$*.yy.c $<
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.yy.c

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

install:
	test -d $(INSTALLDIR) || mkdir $(INSTALLDIR)
	cp $(EXECUTABLE) $(INSTALLDIR)

doc: Doxyfile $(SRCDIR)/*.c
	doxygen Doxyfile >> /dev/null

clean:
	rm -f $(BINDIR)/* $(OBJDIR)/* $(SRCDIR)/*.yy.c $(SRCDIR)/*.tab.c $(SRCDIR)/*.tab.h
