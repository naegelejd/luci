#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "ast.h"
#include "compile.h"
#include "env.h"

extern yyparse();
extern yydebug();

static int VERBOSE = 0;

static struct AstNode *root_node = NULL;
static struct ExecContext *root_env = NULL;


static void cleanup(void)
{
    if (root_env)
        destroy_context(root_env);
    if(root_node)
        destroy_tree(root_node);
}


int begin(int verbose, int execute, int compile, int graph)
{
    VERBOSE = verbose;

    /*yydebug(1);*/

    root_node = 0;

    /* parse yyin and build and AST */
    yyparse(&root_node);

    if (!root_node)
        return EXIT_SUCCESS;

    if (compile) {
        Program *prog = compile_ast(root_node);
        print_instructions(prog);
        puts("");
        eval(prog);
        destroy_program(prog);
    }

    if (graph) {
        puts("digraph hierarchy {");
        puts("node [color=Green,fontcolor=Blue]");
        print_ast_graph(root_node, 0);
        puts("}");
    }

    if (execute) {
	root_env = create_context("global", NULL);
	initialize_context(root_env);
	exec_AST(root_env, root_node);
    }

    /* destroy the AST and the ExecEnviron */
    cleanup();

    return 1;
}

/* provide read-only access to the root of the AST */
struct ExecContext *get_root_env()
{
    return root_env;
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
