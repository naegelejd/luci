#include "types.h"
#include "env.h"
#include "ast.h"
#include "functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

extern int VERBOSE;

luci_obj_t *luci_print(luci_obj_t *in)
{
    if (!in || in->type == obj_none_t)
    {
	printf("None\n");
    }
    else
    {
	switch(in->type)
	{
	    case obj_int_t:
		printf("%d\n", in->value.i_val);
		break;
	    case obj_double_t:
		printf("%f\n", in->value.d_val);
		break;
	    case obj_str_t:
		printf("%s\n", in->value.s_val);
		break;
	    default:
		printf("None\n");
	}
    }

    return NULL;
}

luci_obj_t *luci_type_str(luci_obj_t *in)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_str_t;
    char *which;
    switch(in->type)
    {
	case obj_none_t:
	    which = "None";
	    break;
	case obj_int_t:
	    which = "int";
	    break;
	case obj_double_t:
	    which = "double";
	    break;
	case obj_str_t:
	    which = "string";
	    break;
	default:
	    which = "None";
    }
    ret->value.s_val = alloc(strlen(which) + 1);
    strcpy(ret->value.s_val, which);

    return ret;
}

/* Lookup Array for AST nodes which yield values */
static luci_obj_t * (*exec_lookup[])(ExecEnviron *e, ASTNode *a) =
{
    exec_int_expression,
    exec_double_expression,
    exec_string_expression,
    exec_id_expression,
    exec_bin_expression,
    exec_assignment,
    exec_while,
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

static luci_obj_t *exec_int_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_int_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_int_t, with value %d\n",
		a->data.i_val);
    }
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = a->data.i_val;

    return ret;
}

static luci_obj_t *exec_double_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_double_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_double_t, with value %f\n",
		a->data.d_val);
    }

    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_double_t;
    ret->value.d_val = a->data.d_val;

    return ret;
}

static luci_obj_t *exec_string_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_str_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_str_t, with value %s\n",
		a->data.s_val);
    }
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_str_t;
    /* copy the 'string' from the ASTNode to the luci_obj_t */
    ret->value.s_val = (char *) alloc(strlen(a->data.s_val) + 1);
    strcpy (ret->value.s_val, a->data.s_val);

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
	if (VERBOSE) {
	    printf("Found symbol %s with type %d. Returning its object, with type:%d\n",
		    a->data.name, s->type, t);
	}
	if (t != obj_none_t)
	{
	    luci_obj_t *copy = alloc(sizeof(*copy));
	    /* copy the symbol's data and return it */
	    copy->type = t;
	    switch(copy->type)
	    {
		case obj_int_t:
		    copy->value.i_val = s->data.object->value.i_val;
		    break;
		case obj_double_t:
		    copy->value.d_val = s->data.object->value.d_val;
		    break;
		case obj_str_t:
		    copy->value.s_val = alloc(strlen(s->data.object->value.s_val) + 1);
		    strcpy(copy->value.s_val, s->data.object->value.s_val);
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
	if (VERBOSE) {
	    printf("Found symbol %s, but it's not an object\n", a->data.name);
	}
	return NULL;	 /* look up a->name in symbol table */
    }
}

static luci_obj_t *exec_bin_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a->type == ast_expression_t);
    luci_obj_t *left = dispatch_statement(e, a->data.expression.left);
    luci_obj_t *right = dispatch_statement(e, a->data.expression.right);
    luci_obj_t *result = solve_bin_expr(left, right, a->data.expression.op);
    destroy_object(left);
    destroy_object(right);
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
	destroy_object(s->data.object);
    }
    /* set the symbol's new payload */
    s->data.object = r;

    /* return an empty luci_obj_t * */
    return NULL;
    //s->data.object.type = obj_int_t;
    //s->data.object.value.i_val = r->value.i_val;
}

int evaluate_cond(struct ExecEnviron *e, struct ASTNode *a)
{
    int huh = 0;
    luci_obj_t *cond = dispatch_statement(e, a->data.while_block.cond);
    if (cond == NULL) {
	return 0;
    }
    switch (cond->type)
    {
	case obj_int_t:
	    huh = cond->value.i_val;
	    break;
	case obj_double_t:
	    huh = (int)cond->value.d_val;
	    break;
	case obj_str_t:
	    huh = 1;
	    break;
	default:
	    huh = 0;
    }
    destroy_object(cond);
    return huh;
}

static luci_obj_t *exec_while(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_while_t);

    if (VERBOSE) {
	printf("Begin while loop\n");
    }

    int huh = evaluate_cond(e, a->data.while_block.cond);
    while (huh)
    {
	dispatch_statement(e, a->data.while_block.statements);
	huh = evaluate_cond(e, a->data.while_block.cond);
    }
}

static luci_obj_t *exec_call(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_call_t);

    if (VERBOSE) {
	printf("Calling %s\n", a->data.call.name);
    }
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
	luci_obj_t *param = dispatch_statement(e, a->data.call.param);
	luci_obj_t *ret = /*(luci_obj_t *)*/((*(s->data.funcptr))(param));
	destroy_object(param);
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
	destroy_object(ret);
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
    Symbol *ptr = (Symbol *) alloc (sizeof (Symbol));
    ptr->name = (char *) alloc (strlen (name) + 1);
    strcpy (ptr->name, name);
    ptr->type = type;
    /* caller must set the data (payload) */
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
    sym->data.funcptr = &luci_print;

    return e;
}

void destroy_object(luci_obj_t *trash)
{
    if (!(trash == NULL))
    {
	if (trash->type == obj_str_t) {
	    if (VERBOSE)
		printf("Freeing str object with val %s\n", trash->value.s_val);
	    free(trash->value.s_val);
	    trash->value.s_val = NULL;
	}
	if (VERBOSE)
	    printf("Freeing obj with type %d\n", trash->type);
	free(trash);
	trash = NULL;
    }
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
	    destroy_object(ptr->data.object);
	}
	/* free the Symbol struct itself */
	free(ptr);
	ptr = next;
    }
    /* Free the environment struct */
    free(e);
}
