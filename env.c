#include "env.h"
#include "ast.h"
#include "symbol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

luci_obj_t *print_int(luci_obj_t *in)
{
    assert(in->type == obj_int_t);
    printf("%d\n", in->value.integer);
    return NULL;
}

/* Lookup Array for AST nodes which yield values */
static luci_obj_t * (*exec_lookup[])(ExecEnviron *e, ASTNode *a) =
{
    exec_num_expression,
    exec_id_expression,
    exec_bin_expression,
    exec_assignment,
    exec_call,
    exec_statement,
};

static luci_obj_t *dispatch_statement(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    if (!(exec_lookup[a->type]))
    {
	fprintf(stderr, "IDK what to do\n");
	exit(1);
    }
    else
    {
	return exec_lookup[a->type](e, a);
    }
}

static luci_obj_t *exec_num_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_num_t);

    printf("Allocating a new object of type obj_int_t, with value %d\n", a->data.val);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = a->data.val;

    return ret;
}

static luci_obj_t *exec_id_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_id_t);
    assert(e);
    Symbol *s;
    if (!(s = get_symbol(e, a->data.name))) {
	fprintf(stderr, "Can't find symbol %s\n", a->data.name);
	exit(1);
    }
    if (s->type == sym_obj_t)
    {
	int t = s->data.object->type;
	printf("Found symbol %s with type %d. Returning its object, with type:%d\n",
		a->data.name, s->type, t);
	if (t != obj_none_t)
	{
	    luci_obj_t *copy = alloc(sizeof(*copy));
	    /* copy the symbol's data and return it */
	    copy->type = t;
	    switch(copy->type)
	    {
		case obj_int_t:
		    copy->value.integer = s->data.object->value.integer;
		    break;
		case obj_str_t:
		    copy->value.string = s->data.object->value.string;
		    break;
		default:
		    break;
	    }
	    return copy;
	}
	else
	{
	    return NULL;
	}
    }
    else
    {
	printf("Found symbol %s, but it's not an object\n", a->data.name);
	return NULL;	 /* look up a->name in symbol table */
    }
}

static luci_obj_t *exec_bin_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a->type == ast_expression_t);
    luci_obj_t *left = dispatch_statement(e, a->data.expression.left);
    luci_obj_t *right = dispatch_statement(e, a->data.expression.right);
    luci_obj_t *result;
    switch (a->data.expression.op)
    {
	case '+':
	    result = luci_sum(left, right);
	    break;
	case '-':
	    result = luci_diff(left, right);
	    break;
	case '*':
	    result = luci_prod(left, right);
	    break;
	case '/':
	    result = luci_div(left, right);
	    break;
	case '<':
	    result = luci_lt(left, right);
	    break;
	case '>':
	    result = luci_gt(left, right);
	    break;
	default:
	    fprintf(stderr, "OOPS: Unknown operator: %c\n", a->data.expression.op);
	    exit(1);
    }
    free(left);
    free(right);
    return result;
}

static luci_obj_t *exec_assignment(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_assignment_t);
    assert(e);

    ASTNode *right = a->data.assignment.right;
    luci_obj_t *r = dispatch_statement(e, right);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.assignment.name)))
    {
	s = add_symbol(e, a->data.assignment.name, sym_obj_t);
    }
    else
    {
	/* if the symbol already exists, free its existing Object */
	free(s->data.object);
    }
    /* set the symbol's new payload */
    s->data.object = r;

    /* return an empty luci_obj_t * */
    return NULL;
    //s->data.object.type = obj_int_t;
    //s->data.object.value.integer = r->value.integer;
}

static luci_obj_t *exec_call(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_call_t);
    //printf("Calling %s\n", a->data.call.name);
    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	fprintf(stderr, "Invalid function\n");
	exit(1);
    }
    if (s->type != sym_func_t)
    {
	fprintf(stderr, "%s is not a function\n", s->name);
	exit(1);
    }
    else
    {
	luci_obj_t *p = dispatch_statement(e, a->data.call.param);
	luci_obj_t *ret = /*(luci_obj_t *)*/((*(s->data.funcptr))(p));
	free(p);
	//printf("call ret: %d\n", r);
	return ret;
    }
}

static luci_obj_t *exec_statement(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_statements_t);
    int i;
    for (i=0; i < a->data.statements.count; i++)
    {
	luci_obj_t *ret = dispatch_statement(e, a->data.statements.statements[i]);
	if (!(ret == NULL))
	{
	    free(ret);
	}
    }
    return NULL;
}

void exec_AST(struct ExecEnviron *e, struct ASTNode *a)
{
    dispatch_statement(e, a);
}

/*
struct func_init
{
    const char *name;
    double (*func) (double);
};

const struct func_init arith_funcs[] =
{
    "sin",  sin,
    "cos",  cos,
    "atan", atan,
    "ln",   log,
    "exp",  exp,
    "sqrt", sqrt,
    0, 0
};
*/

Symbol *add_symbol (struct ExecEnviron *e, char const *name, int type)
{
    Symbol *ptr = (Symbol *) malloc (sizeof (Symbol));
    ptr->name = (char *) malloc (strlen (name) + 1);
    strcpy (ptr->name, name);
    ptr->type = type;
    /* set values for different types */
    //ptr->data = NONE_OBJ; /* Set value to NONE even if fctn.  */
    ptr->next = e->symtable;
    e->symtable = ptr;
    return ptr;
}

Symbol *get_symbol (struct ExecEnviron *e, const char *name)
{
    Symbol *ptr;
    for (ptr = e->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next)
	if (strcmp (ptr->name, name) == 0)
	    return ptr;
    return 0;
}

ExecEnviron *create_env(void)
{
    /* Check that we have dispatchers for all types of statements */
    assert(ast_last_t == (sizeof(exec_lookup) / sizeof(*exec_lookup)));

    ExecEnviron *e = calloc(1, sizeof(struct ExecEnviron));

    /*
    int i;
    for (i = 0; arith_funcs[i].name != 0; i++)
    {
	Symbol *sym = add_symbol(e, arith_funcs[i].name, t_func);
	sym->value.funcptr = arith_funcs[i].func;
    }
    */
    /* add print function */
    Symbol *sym = add_symbol(e, "print", sym_func_t);
    sym->data.funcptr = &print_int;

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
	if (ptr->type == sym_obj_t)
	{
	    /* free the Object payload if symbol is an object */
	    free(ptr->data.object);
	}
	/* free the Symbol struct itself */
	free(ptr);
	ptr = next;
    }
    /* Free the environment struct */
    free(e);
}
