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

/** defined in scanner */
extern void yy_luci_reset();
/** defined in generated parser */
extern FILE *yyin;
/** defined in generated parser */
extern int yyparse();
/** defined in lexer.l */
extern void yy_luci_init(bool);

#ifdef DEBUG
/** defined in generated parser */
extern int yydebug;
#endif

jmp_buf LUCI_EXCEPTION_BUF;

/** Current version of Luci (for printing to stdout) */
static const char * const version_string = "Luci v0.2";

enum {
    MODE_EXE,
    MODE_PRINT,
    MODE_GRAPH,
    MODE_SERIAL,
    MODE_SYNTAX
} main_modes;   /**< Luci's run-time modes of operation */

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
    puts("    -n\t\tCheck syntax and don't execute");
    puts("    -g\t\tPrint a Graphviz dot spec for the parsed AST");
    puts("    -p\t\tShow the compiled bytecode source");
    puts("    -c\t\tCompile the source to a .lxc file (i.e. do nothing)");
    printf("\n%s\n", version_string);

    /* puts("\nSize (type) static address:"); */
    /* printf("%ld (%s) %p\n", sizeof(LuciIntObj), "int", &obj_int_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciFloatObj), "float", &obj_float_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciStringObj), "string", &obj_string_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciFileObj), "file", &obj_file_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciListObj), "list", &obj_list_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciMapObj), "map", &obj_map_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciIteratorObj), "iterator", &obj_iterator_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciFunctionObj), "func", &obj_func_t); */
    /* printf("%ld (%s) %p\n", sizeof(LuciLibFuncObj), "libfunc", &obj_libfunc_t); */
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
            printf("%s\n", "This option is not yet supported");
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-n") == 0) {
            mode = MODE_SYNTAX;
        } else if (i == (argc - 1)) {
            infilename = arg;
        } else {
            LUCI_DIE("Invalid option: %s\n", arg);
        }
    }

    if (infilename == NULL) {
        /* interactive mode */
        yyin = stdin;
        return luci_interactive();
    } else if (!(yyin = fopen(infilename, "r"))) {
        LUCI_DIE("Can't read from file %s\n", infilename);
    }
    LUCI_DEBUG("Reading from %s\n", infilename? infilename : "stdin");

    /* initialize the scanner in non-interactive mode */
    yy_luci_init(false);

    /* parse yyin and build and AST */
    yyparse();
    extern AstNode* g_root_node;    /* parser.y */
    if (!g_root_node) {
        /* empty program */
        return EXIT_SUCCESS;
    } else if (mode == MODE_SYNTAX) {
        printf("%s\n", "Syntax valid");
        return EXIT_SUCCESS;
    } else if (mode == MODE_GRAPH) {
        print_ast_graph(g_root_node);
        return EXIT_SUCCESS;
    }

    /* initialize systems */
    gc_init();
    compiler_init();

    /* Compile the AST */
    CompileState *cs = compile_ast(g_root_node);

    ast_destroy(g_root_node);

    LuciObject *gf = LuciFunction_new();
    convert_to_function(cs, gf, 0);

    compile_state_delete(cs);

    if (setjmp(LUCI_EXCEPTION_BUF) == 0) {
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
                LUCI_DIE("%s\n", "Invalid mode?!");
        }
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
 *
 * returns EXIT_SUCCESS or EXIT_FAILURE
 */
int luci_interactive(void)
{
    yy_luci_init(true);

    printf("\nWelcome to Interactive %s\n\n", version_string);

    /* initialize systems */
    gc_init();
    compiler_init();

    CompileState *cs = NULL;
    LuciObject *gf = NULL;

    while (true) {
        /* set up interactive prompt in the lexer */
        putc('$', stdout);
        putc(' ', stdout);

        /* parse yyin and build and AST */
        yyparse();
        extern AstNode *g_root_node;    /* parser.y */
        if (!g_root_node) {
            /* didn't parse anything... is it any empty line or EOF? */
            if (feof(yyin)) {
                break;
            } else {
                continue;
            }
        }

        /* Compile the AST */
        cs = compile_ast_incremental(cs, gf, g_root_node);

        /* clean up AST memory */
        ast_destroy(g_root_node);
        g_root_node = NULL;

        gf = LuciFunction_new();
        convert_to_function(cs, gf, 0);

        if (setjmp(LUCI_EXCEPTION_BUF) == 0) {

            /* Execute the bytecode */
            eval(gf);

        }

        /* print one line of spacing */
        fprintf(stdout, "%s", "\n");

        /* remove the EOF flag */
        clearerr(yyin);
        /* reset scanner */
        yy_luci_reset();
    }

    printf("Goodbye\n");

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

