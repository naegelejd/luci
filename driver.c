#include <stdlib.h>
#include <stdio.h>
#include "driver.h"
#include "ast.h"
#include "env.h"

extern yyparse();
extern yydebug();

int VERBOSE;

struct ASTNode *root_node;
struct ExecEnviron *root_env;

int main(int argc, char *argv[])
{
    --argc, ++argv;

    if (argc > 0)
	VERBOSE = 1;
    else
	VERBOSE = 0;

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

void dbgprint(const char *msg)
{
    if (VERBOSE)
    {
	printf("%s\n", msg);
    }
}

void die(const char *msg)
{
    fprintf(stderr, "Fatal Error: %s\n", msg);
    cleanup();
    exit(1);
}

void cleanup(void)
{
    destroy_env(root_env);
    destroy_AST(root_node);
}
