/*
 * See Copyright Notice in luci.h
 */

/**
 * @file main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "luci.h"
#include "object.h"
#include "gc.h"
#include "ast.h"
#include "compile.h"
#include "interpret.h"

enum {
    INTERACTIVE=1,
    EXECUTE=2,
    SHOW_INSTR=4,
    GRAPH_AST=8,
    SERIALIZE=16
} main_modes; /**< Luci's run-time modes of operation */

/** defined in generated scanner */
extern void yyrestart();
/** defined in generated parser */
extern FILE *yyin;
/** defined in generated parser */
extern int yyparse();
/** defined in generated parser */
extern int yydebug;


/** Current version of Luci (for printing to stdout) */
static const char * const version_string = "Luci v0.2";

int luci_main(int argc, char *argv[]);
void luci_interactive(void);


/**
 * Prints basic command-line help to stdout
 */
static void help()
{
    puts("Usage: luci [options] filename\n");
    puts("Options:");
    puts("    -h\t\tShow help and exit");
    puts("    -v\t\tVerbose mode");
    puts("    -n\t\tDo not execute source");
    puts("    -g\t\tPrint a Graphviz dot spec for the parsed AST");
    puts("    -p\t\tShow the compiled bytecode source");
    puts("    -c\t\tCompile the source to a .lxc file");
    printf("\n%s\n", version_string);

    puts("\nSizes:");
    printf("%ld (%s)\n", sizeof(LuciIntObj), "int");
    printf("%ld (%s)\n", sizeof(LuciFloatObj), "float");
    printf("%ld (%s)\n", sizeof(LuciStringObj), "string");
    printf("%ld (%s)\n", sizeof(LuciFileObj), "file");
    printf("%ld (%s)\n", sizeof(LuciListObj), "list");
    printf("%ld (%s)\n", sizeof(LuciMapObj), "map");
    printf("%ld (%s)\n", sizeof(LuciIteratorObj), "iterator");
    printf("%ld (%s)\n", sizeof(LuciFunctionObj), "func");
    printf("%ld (%s)\n", sizeof(LuciLibFuncObj), "libfunc");
}

/**
 * Main entry point to the Luci compiler/interpreter
 *
 * @param argc number of command-line arguments
 * @param argv C-string array of command-line arguments
 * @returns 1 on error, 0 on success
 */
int luci_main(int argc, char *argv[])
{
#ifdef DEBUG
    yydebug = 1;
#endif
    /* initialize options */
    unsigned short options = 0;
    char *infilename = NULL;

    AstNode *root_node = NULL;
    CompileState *cs = NULL;
    LuciObject *gf = NULL;

    if (argc < 2) {
        /* interactive mode */
	yyin = stdin;
        options = INTERACTIVE;
    }
    else {
        options = EXECUTE;
	int i;
	char *arg;
	for (i = 1; i < argc; i++) {
	    arg = argv[i];
            if (strcmp(arg, "-h") == 0) {
                help();
                goto finish;
            }
            else if (strcmp(arg, "-V") == 0) {
		fprintf(stdout, "%s\n", version_string);
                goto finish;
	    }
            else if (strcmp(arg, "-g") == 0) {
                options |= GRAPH_AST;
            }
            else if (strcmp(arg, "-p") == 0) {
                options |= SHOW_INSTR;
            }
            else if (strcmp(arg, "-c") == 0) {
                options |= SERIALIZE;
            }
            else if (strcmp(arg, "-n") == 0) {
                options &= ~EXECUTE;
            }
            else if (i == (argc - 1)) {
                infilename = arg;
            }
            else {
                DIE("Invalid option: %s\n", arg);
            }
	}
    }

    if (infilename == NULL) {
        /* interactive mode */
        yyin = stdin;
        options = INTERACTIVE;
    }
    else if (!(yyin = fopen(infilename, "r"))) {
        DIE("Can't read from file %s\n", infilename);
    }
    LUCI_DEBUG("Reading from %s\n", infilename? infilename : "stdin");

    if (options == INTERACTIVE) {
        luci_interactive();
        return EXIT_SUCCESS;
    }

    /* initialize LuciObject garbage collector */
    gc_init();

    /* parse yyin and build and AST */
    yyparse(&root_node);

    if (!root_node) {
        /* empty program */
        return EXIT_SUCCESS;
    }

    if (options & GRAPH_AST) {
        /* Print a DOT graph representation */
        puts("digraph hierarchy {");
        puts("node [color=Green,fontcolor=Blue]");
        print_ast_graph(root_node, 0);
        puts("}");

        /* break early if finished */
        if (options == GRAPH_AST) {
            goto cleanup_tree;
        }
    }

    /* Compile the AST */
    cs = compile_ast(NULL, root_node);
    gf = LuciFunction_from_CompileState(cs, 0);

    if (options & SERIALIZE) {
        /* Serialize program */
        serialize_program(gf);
    }

    if (options & SHOW_INSTR) {
        /* Print the bytecode */
        print_instructions(gf);
    }

    if (options & EXECUTE) {
        /* Execute the bytecode */
        eval(gf);
    }

cleanup:
    CompileState_delete(cs);
    LuciFunction_delete(gf);
cleanup_tree:
    destroy_tree(root_node);
    gc_finalize();

finish:
    return EXIT_SUCCESS;
}

/** defined in lexer.l */
extern void luci_start_interactive(void);

/**
 * Initiates Luci's interactive mode.
 *
 * Input is read from stdin, parsed, compiled, then executed.
 * The interpreter's state is maintained between successive
 * user inputs to stdin.
 */
void luci_interactive(void)
{
    AstNode *root_node = NULL;
    CompileState *cs = NULL;
    LuciObject *gf = NULL;

    printf("\nWelcome to Interactive %s\n\n", version_string);

    /* initialize LuciObject garbage collector */
    gc_init();

    while (1) {
        /* set up interactive prompt in the lexer */
        luci_start_interactive();

        /* parse yyin and build and AST */
        yyparse(&root_node);

        if (!root_node) {
            printf("Goodbye\n");
            goto end_interactive;
        }

        /* Compile the AST */
        cs = compile_ast(cs, root_node);
        gf = LuciFunction_from_CompileState(cs, 0);

        /* print a spacing between input/output */
        fprintf(stdout, "%s", "  \n\n");

        /* Execute the bytecode */
        eval(gf);

        /* print one more line of spacing */
        fprintf(stdout, "%s", "\n");

        /* clean up frame's memory */
        LuciFunction_delete_interactive(gf);
        /* clean up AST memory */
        destroy_tree(root_node);
        root_node = NULL;

        /* remove the EOF flag */
        clearerr(yyin);
        /* restart the token scanner */
        yyrestart(yyin);
    }

end_interactive:
    if (cs != NULL) {
        CompileState_delete(cs);
    }
    gc_finalize();
}

/**
 * C main.
 *
 * @param argc number of command-line arguments
 * @param argv C-style string array of command-line arguments
 * @returns 1 on error, 0 on success
 */
int main(int argc, char *argv[])
{
    return luci_main(argc, argv);
}

