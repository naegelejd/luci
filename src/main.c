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
    puts("\nHELP\n");
}


int main(int argc, char *argv[])
{
    /* initialize options */
    int verbose = 0;
    int graph = 0;

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
                exit(0);
            }
	    if (strcmp(arg, "-v") == 0)
		verbose = 1;
            if (strcmp(arg, "-g") == 0)
                graph = 1;
	    if (strcmp(arg, "-V") == 0) {
		fprintf(stdout, "%s\n", version_string);
		exit(0);
	    }
	}

	char *filename = argv[i-1];
	if (!is_option(filename)) {
	    if (!(yyin = fopen(filename, "r"))) {
		fprintf(stderr, "Error: Can't open file %s\n", filename);
		exit(1);
	    }
	}
	else {
	    yyin = stdin;
	}
    }

    if (begin(verbose, graph, 1))
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

