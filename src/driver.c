#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "driver.h"
#include "ast.h"
#include "env.h"

extern FILE *yyin;
extern yyparse();
extern yydebug();

int VERBOSE;

struct ASTNode *root_node;
struct ExecContext *root_env;

const char *options[] =
{
    "-v",
    0
};

int is_option(const char *arg)
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



int main(int argc, char *argv[])
{
    /* initialize options */
    VERBOSE = 0;
    if (argc < 2) {
	yyin = stdin;
    }
    else {
	int i;
	char *arg;
	for (i = 1; i < argc; i++) {
	    arg = argv[i];
	    if (strcmp(arg, "-v") == 0) {
		VERBOSE = 1;
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

    /*yydebug(1);*/
    root_node = 0;

    yyparse(&root_node);

    if (root_node)
    {
	root_env = create_env();
	exec_AST(root_env, root_node);
	cleanup();
    }

    return EXIT_SUCCESS;
}

/*  Safely allocate memory, and quit on failure */
void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result)
    {
	die("alloc failed\n");
    }
    return result;
}

/* AKA babble */
void yak(const char *format, ... )
{
    if (VERBOSE)
    {
	va_list arglist;

	printf("INFO: ");

	va_start(arglist, format);
	vprintf(format, arglist);
	va_end(arglist);
    }
}

void die(const char* format, ... )
{
    va_list arglist;

    fprintf(stderr, "Fatal Error: ");

    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    va_end (arglist);

    cleanup();
    exit(1);
}

void cleanup(void)
{
    destroy_env(root_env);
    destroy_AST(root_node);
}
