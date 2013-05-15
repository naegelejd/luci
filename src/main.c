/*
 * See Copyright Notice in luci.h
 */

/**
 * @file main.c
 */

#include "luci.h"
#include "lucitypes.h"
#include "gc.h"
#include "ast.h"
#include "compile.h"
#include "interpret.h"

/** defined in generated scanner */
extern void yyrestart();
/** defined in generated parser */
extern FILE *yyin;
/** defined in generated parser */
extern int yyparse();
/** defined in lexer.l */
extern void luci_start_interactive(void);


#ifdef DEBUG
/** defined in generated parser */
extern int yydebug;
#endif

/** Current version of Luci (for printing to stdout) */
static const char * const version_string = "Luci v0.2";

/**< Luci's run-time modes of operation */
enum {
    MODE_EXE,
    MODE_PRINT,
    MODE_GRAPH,
    MODE_SERIAL,
    MODE_SYNTAX
} main_modes;

int luci_main(int argc, char *argv[]);
int luci_interactive(void);


/**
 * Prints basic command-line help to stdout
 */
static void help()
{
    puts("Usage: luci [options] filename\n");
    puts("Options:");
    puts("    -h\t\tShow help and exit");
    puts("    -v\t\tVerbose mode");
    puts("    -n\t\tCheck syntax and don't execute");
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

    if (argc < 2) {
        /* interactive mode */
	yyin = stdin;
        return luci_interactive();
    }

    unsigned int mode = MODE_EXE;

    char *arg;
    char *infilename = NULL;
    unsigned int i;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (strcmp(arg, "-h") == 0) {
            help();
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-V") == 0) {
            fprintf(stdout, "%s\n", version_string);
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-g") == 0) {
            mode = MODE_GRAPH;
        } else if (strcmp(arg, "-p") == 0) {
            mode = MODE_PRINT;
        } else if (strcmp(arg, "-c") == 0) {
            mode = MODE_SERIAL;
        } else if (strcmp(arg, "-n") == 0) {
            mode = MODE_SYNTAX;
        } else if (i == (argc - 1)) {
            infilename = arg;
        } else {
            DIE("Invalid option: %s\n", arg);
        }
    }

    if (infilename == NULL) {
        /* interactive mode */
        yyin = stdin;
        return luci_interactive();
    } else if (!(yyin = fopen(infilename, "r"))) {
        DIE("Can't read from file %s\n", infilename);
    }
    LUCI_DEBUG("Reading from %s\n", infilename? infilename : "stdin");

    /* parse yyin and build and AST */
    AstNode *root_node = NULL;
    yyparse(&root_node);
    if (!root_node) {
        /* empty program */
        return EXIT_SUCCESS;
    }

    if (mode == MODE_GRAPH) {
        print_ast_graph(root_node);
        return EXIT_SUCCESS;
    }

    /* initialize systems */
    gc_init();
    compiler_init();

    /* Compile the AST */
    CompileState *cs = compile_ast(root_node);

    ast_destroy(root_node);

    LuciObject *gf = LuciFunction_new();
    convert_to_function(cs, gf, 0);

    compile_state_delete(cs);

    switch (mode) {
        case MODE_EXE:
            /* Execute the bytecode */
            eval(gf);
            break;
        case MODE_PRINT:
            /* Print the bytecode */
            print_instructions(gf);
            break;
        case MODE_SERIAL:
            /* Serialize program */
            serialize_program(gf);
            break;
        default:
            DIE("%s\n", "Invalid mode?!");
    }

    /* cleanup systems */
    compiler_finalize();
    gc_finalize();

    return EXIT_SUCCESS;
}

/**
 * Initiates Luci's interactive mode.
 *
 * Input is read from stdin, parsed, compiled, then executed.
 * The interpreter's state is maintained between successive
 * user inputs to stdin.
 */
int luci_interactive(void)
{
    printf("\nWelcome to Interactive %s\n\n", version_string);

    /* initialize systems */
    gc_init();
    compiler_init();

    CompileState *cs = NULL;
    LuciObject *gf = NULL;

    while (true) {
        /* set up interactive prompt in the lexer */
        luci_start_interactive();

        AstNode *root_node = NULL;
        /* parse yyin and build and AST */
        yyparse(&root_node);

        if (!root_node) {
            printf("Goodbye\n");
            break;
        }

        /* Compile the AST */
        cs = compile_ast_incremental(cs, gf, root_node);

        /* clean up AST memory */
        ast_destroy(root_node);
        root_node = NULL;

        gf = LuciFunction_new();
        convert_to_function(cs, gf, 0);

        /* print a spacing between input/output */
        fprintf(stdout, "%s", "  \n\n");

        /* Execute the bytecode */
        eval(gf);

        /* print one more line of spacing */
        fprintf(stdout, "%s", "\n");

        /* remove the EOF flag */
        clearerr(yyin);
        /* restart the token scanner */
        yyrestart(yyin);
    }

    if (cs != NULL) {
        compile_state_delete(cs);
    }

    /* cleanup systems */
    compiler_finalize();
    gc_finalize();

    return EXIT_SUCCESS;
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

