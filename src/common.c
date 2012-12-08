/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "ast.h"
#include "compile.h"
#include "interpret.h"
/*
#include "env.h"
*/

extern yyparse();
extern yydebug();

static AstNode *root_node = NULL;
static CompileState *cs = NULL;
static Frame *gf = NULL;

static void cleanup(void)
{
    /* If global compile state still exists */
    if (cs) {
        CompileState_delete(cs);
    }

    /* If global Frame still exists */
    if (gf) {
        Frame_delete(gf);
    }

    /* destroy the AST */
    if (root_node) {
        destroy_tree(root_node);
    }
}


int begin(int verbose, int execute, int compile, int show, int graph)
{

    /*yydebug(1);*/

    /* parse yyin and build and AST */
    yyparse(&root_node);

    if (!root_node) {
        return EXIT_SUCCESS;
    }

    /* Compile the AST */
    cs = compile_ast(root_node);
    gf = Frame_from_CompileState(cs, 0);

    if (compile) {
        /* Serialize program */
        serialize_program(gf);
    }

    if (show) {
        /* Print the bytecode */
        print_instructions(gf);
    }

    if (graph) {
        /* Print a DOT graph representation */
        puts("digraph hierarchy {");
        puts("node [color=Green,fontcolor=Blue]");
        print_ast_graph(root_node, 0);
        puts("}");
    }

    if (execute) {
        /* Execute the bytecode */
        eval(gf);
    }

    CompileState_delete(cs);

    Frame_delete(gf);

    destroy_tree(root_node);

    return 1;
}

/*  Safely allocate memory, and quit on failure */
void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result) {
	die("alloc failed\n");
    }
    return result;
}

void die(const char* format, ... )
{
    va_list arglist;

    fflush(stdout);

    fprintf(stderr, "Fatal Error: ");

    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    va_end (arglist);

    //cleanup();
    exit(1);
}
