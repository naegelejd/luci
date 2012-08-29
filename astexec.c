#include "astexec.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* Function type.  */
typedef double (*unary_func)(double);
typedef double (*func_t) (double);

typedef struct Symbol
{
    enum { t_int, t_func } type;
    char *name;
    union
    {
	int i_val;      /* value of a VAR */
	func_t funcptr;  /* value of a FNCT */
    } value;
    struct Symbol *next;
} Symbol;

typedef struct ExecEnviron
{
    //int variable;  /* This is where the symbol table will live */
    //unary_func func;
    Symbol *symtable;
} ExecEnviron;

static int exec_num_expression(ExecEnviron *e, ASTNode *a);
static int exec_id_expression(ExecEnviron *e, ASTNode *a);
static int exec_bin_expression(ExecEnviron *e, ASTNode *a);
static void exec_assignment(ExecEnviron *e, ASTNode *a);
static int exec_call(ExecEnviron *e, ASTNode *a);
static void exec_statement(ExecEnviron *e, ASTNode *a);

/* Lookup Array for AST nodes which yield values */
static int (*val_execs[])(ExecEnviron *e, ASTNode *a) =
{
    exec_num_expression,
    exec_id_expression,
    exec_bin_expression,
    NULL,
    exec_call,
    NULL
};

/* Lookup Array for AST nodes which do not yield values */
static void (*run_execs[])(ExecEnviron *e, ASTNode *a) =
{
    NULL,	/* Numbers are canonical, not executed */
    NULL,	/* IDs are canonical, not executed */
    NULL,	/* binary expressions not executed */
    exec_assignment,
    NULL,
    exec_statement,
};

static int dispatch_statement(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    if (!(run_execs[a->type]))
    {
	if (!(val_execs[a->type]))
	{
	    fprintf(stderr, "IDK what to do\n");
	    exit(1);
	}
	else
	{
	    return val_execs[a->type](e, a);
	}
    }
    else
    {
	run_execs[a->type](e, a);
	return 1;
    }
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
    Symbol *s;
    if (!(s = get_symbol(e, a->data.name))) {
	fprintf(stderr, "Can't find symbol %s\n", a->data.name);
	exit(1);
    }
    if (s->type == t_int)
    {
	return s->value.i_val;
    }
    else
    {
	return 0;	 /* look up a->name in symbol table */
    }
}

static int exec_bin_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a->type == t_expression);
    const int left = dispatch_statement(e, a->data.expression.left);
    const int right = dispatch_statement(e, a->data.expression.right);
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

    ASTNode *right = a->data.assignment.right;
    int r = dispatch_statement(e, right);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.assignment.name)))
    {
	s = put_symbol(e, a->data.assignment.name, t_int);
    }
    /* set the integer symbol's value */
    s->value.i_val = r;
}

static int exec_call(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == t_call);
    //printf("Calling %s\n", a->data.call.name);
    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	fprintf(stderr, "Invalid function\n");
	exit(1);
    }
    if (s->type != t_func)
    {
	fprintf(stderr, "%s is not a function\n", s->name);
	exit(1);
    }
    else
    {
	//printf("%d\n", dispatch_statement(e, a->data.call.param));
	return (int)((*(s->value.funcptr))(dispatch_statement(e, a->data.call.param)));
    }
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


Symbol *put_symbol (ExecEnviron *e, char const *name, int type)
{
    Symbol *ptr = (Symbol *) malloc (sizeof (Symbol));
    ptr->name = (char *) malloc (strlen (name) + 1);
    strcpy (ptr->name, name);
    ptr->type = type;
    /* set values for different types */
    ptr->value.i_val = 0; /* Set value to 0 even if fctn.  */
    ptr->next = (Symbol *)(e->symtable);
    e->symtable = ptr;
    return ptr;
}

Symbol *get_symbol (ExecEnviron *e, const char *name)
{
    Symbol *ptr;
    for (ptr = e->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next)
	if (strcmp (ptr->name, name) == 0)
	    return ptr;
    return 0;
}

struct func_init
{
    char const *name;
    double (*func) (double);
};

struct func_init const arith_funcs[] =
{
    "sin",  sin,
    "cos",  cos,
    "atan", atan,
    "ln",   log,
    "exp",  exp,
    "sqrt", sqrt,
    0, 0
};

ExecEnviron *create_env(void)
{
    /* Check that we have dispatchers for all types of statements */
    assert(t_last == (sizeof(val_execs) / sizeof(*val_execs)));
    assert(t_last == (sizeof(run_execs) / sizeof(*run_execs)));

    ExecEnviron *e = calloc(1, sizeof(struct ExecEnviron));

    int i;
    for (i = 0; arith_funcs[i].name != 0; i++)
    {
	Symbol *sym = put_symbol(e, arith_funcs[i].name, t_func);
	sym->value.funcptr = arith_funcs[i].func;
    }
    /* add print function */
    Symbol *sym = put_symbol(e, "print", t_func);
    sym->value.funcptr = (void *)puts;

    return e;
}

void destroy_env(ExecEnviron *e)
{
    Symbol *ptr = e->symtable;
    Symbol *next = ptr;
    while (ptr != (Symbol *) 0)
    {
	next = (Symbol *)ptr->next;
	/* free the char* name of each Symbol */
	free(ptr->name);
	/* free the Symbol struct itself */
	free(ptr);
	ptr = next;
    }
    /* Free the environment struct */
    free(e);
}
