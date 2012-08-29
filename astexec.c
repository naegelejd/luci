#include "astexec.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

typedef double (*unary_func)(double);

typedef struct ExecEnviron
{
    int variable;  /* This is where the symbol table will live */
    unary_func func;
} ExecEnviron;

static int exec_num_expression(ExecEnviron *e, ASTNode *a);
static int exec_id_expression(ExecEnviron *e, ASTNode *a);
static int exec_bin_expression(ExecEnviron *e, ASTNode *a);
static void exec_assignment(ExecEnviron *e, ASTNode *a);
static void exec_call(ExecEnviron *e, ASTNode *a);
static void exec_statement(ExecEnviron *e, ASTNode *a);

/* Lookup Array for AST nodes which yield values */
static int (*val_execs[])(ExecEnviron *e, ASTNode *a) =
{
    exec_num_expression,
    exec_id_expression,
    exec_bin_expression,
    NULL,
    NULL,
    NULL
};

/* Lookup Array for AST nodes which do not yield values */
static void (*run_execs[])(ExecEnviron *e, ASTNode *a) =
{
    NULL,	/* Numbers are canonical, not executed */
    NULL,	/* IDs are canonical, not executed */
    NULL,	/* binary expressions not executed */
    exec_assignment,
    exec_call,
    exec_statement,
};

/* Dispatches any value expression */
static int dispatch_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(val_execs[a->type]);
    return val_execs[a->type](e, a);
}

static void dispatch_statement(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(run_execs[a->type]);
    return run_execs[a->type](e, a);
}

static int exec_num_expression(ExecEnviron *e, ASTNode *a)
{
    /* This entire function could/should be split into separate
	functions for executing a NUMBER expression or an ID expression (symtable lookup)
    */
    assert(a);
    assert(a->type == t_num);
    return a->data.val;
}

static int exec_id_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == t_id);
    assert(e);
    return e->variable;	 /* look up a->name in symbol table */
}

static int exec_bin_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a->type == t_expression);
    const int left = dispatch_expression(e, a->data.expression.left);
    const int right = dispatch_expression(e, a->data.expression.right);
    switch (a->data.expression.op)
    {
	case '+':
	    return left + right;
	case '-':
	    return left - right;
	case '*':
	    return left * right;
	case '/':
	    return left / right;
	case '<':
	    return left < right;
	case '>':
	    return left > right;
	default:
	    fprintf(stderr, "OOPS: Unknown operator: %c\n", a->data.expression.op);
	    exit(1);
    }
}

static void exec_assignment(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == t_assignment);
    assert(e);
    struct ASTNode *r = a->data.assignment.right;
    e->variable = dispatch_expression(e, r);
}

static void exec_call(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == t_call);
    //printf("Calling %s\n", a->data.call.name);
    printf("%d\n", dispatch_expression(e, a->data.call.param));
    //e->func(dispatch_expression(e, a->data.call.param));
}

static void exec_statement(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == t_statements);
    int i;
    for (i=0; i < a->data.statements.count; i++)
    {
	dispatch_statement(e, a->data.statements.statements[i]);
    }
}

void exec_AST(struct ExecEnviron *e, struct ASTNode *a)
{
    dispatch_statement(e, a);
}

struct ExecEnviron *create_env()
{
    assert(t_last == (sizeof(val_execs) / sizeof(*val_execs)));
    assert(t_last == (sizeof(run_execs) / sizeof(*run_execs)));
    return calloc(1, sizeof(struct ExecEnviron));
}

void free_env(struct ExecEnviron *e)
{
    free(e);
}
