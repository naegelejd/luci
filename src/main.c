#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* from parser */
extern FILE *yyin;

static const char const * version_string = "Luci v0.2";


static const char *options[] =
{
    "-h",
    "-v",
    "-c",
    "-g",
    "-V",
    0
};

static int is_option(const char *arg)
{
    int i = 0;
    while (options[i] != 0) {
	if (strcmp(arg, options[i]) == 0) {
	    return 1;
	}
	++i;
    }
    return 0;
}

static void help()
{
    puts("Usage: luci [options] filename\n");
    puts("Options:");
    puts("    -h\t\tShow help and exit");
    puts("    -v\t\tVerbose mode");
    puts("    -g\t\tPrint a Graphviz dot spec for the parsed AST");
    puts("    -c\t\tCompile the source to bytecode (dev)");
    printf("\n%s\n", version_string);
}


int main(int argc, char *argv[])
{
    /* initialize options */
    int verbose = 0;
    int execute = 1;
    int compile = 0;
    int graph = 0;

    char *infilename = NULL;

    if (argc < 2) {
	yyin = stdin;
    }
    else {
	int i;
	char *arg;
	for (i = 1; i < argc; i++) {
	    arg = argv[i];
            if (strcmp(arg, "-h") == 0) {
                help();
                goto finish;
            }
            else if (strcmp(arg, "-v") == 0)
		verbose = 1;
            else if (strcmp(arg, "-c") == 0) {
                compile = 1;
                execute = 0;
            }
            else if (strcmp(arg, "-g") == 0) {
                graph = 1;
                execute = 0;
            }
            else if (strcmp(arg, "-V") == 0) {
		fprintf(stdout, "%s\n", version_string);
                goto finish;
	    }
            else if (i == (argc - 1))
                infilename = arg;
            else
                die("Invalid options: %s\n", arg);
	}
    }

    if (infilename == NULL)
        yyin = stdin;
    else if (!(yyin = fopen(infilename, "r")))
        die("Can't read from file %s\n", infilename);
    yak("Reading from %s\n", infilename? infilename : "stdin");

    if (!(begin(verbose, execute, compile, graph)))
        return EXIT_FAILURE;

finish:
        return EXIT_SUCCESS;
}
