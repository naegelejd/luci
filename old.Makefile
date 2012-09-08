
$(OBJDIR)/%.yy.o: $(SRCDIR)/%.l
	flex -o $*.yy.c $<
	$(CC) $(CFLAGS) -c $*.yy.c

$(OBJDIR)/%.tab.o: $(SRCDIR)/%.y
	bison -d -o $(SRCDIR)/$*.tab.c $<
	$(CC) $(CFLAGS) -c $(SRCDIR)/$*.tab.c

