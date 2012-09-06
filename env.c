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

static struct LuciObject *dispatch_statement(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_int_expression(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_double_expression(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_string_expression(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_id_expression(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_bin_expression(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_parameters(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_assignment(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_while(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_if(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_call(ExecEnviron *e, struct ASTNode *a);
static struct LuciObject *exec_statement(ExecEnviron *e, struct ASTNode *a);

/* Lookup Array for AST nodes which yield values */
static LuciObject * (*exec_lookup[])(ExecEnviron *e, ASTNode *a) =
{
    exec_int_expression,
    exec_double_expression,
    exec_string_expression,
    exec_id_expression,
    exec_bin_expression,
    exec_parameters,
    exec_assignment,
    exec_while,
    exec_if,
    exec_call,
    exec_statement,
};

static LuciObject *dispatch_statement(ExecEnviron *e, ASTNode *a)
{
    if (!a || a == NULL)
    {
	return NULL;
    }
    if (!(exec_lookup[a->type]))
    {
	die("IDK what to do");
    }
    else
    {
	return exec_lookup[a->type](e, a);
    }
}

static LuciObject *exec_int_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_int_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_int_t, with value %d\n",
		a->data.i_val);
    }
    LuciObject *ret = create_object(obj_int_t);
    ret->value.i_val = a->data.i_val;

    return ret;
}

static LuciObject *exec_double_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_double_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_double_t, with value %f\n",
		a->data.d_val);
    }

    LuciObject *ret = create_object(obj_double_t);
    ret->value.d_val = a->data.d_val;

    return ret;
}

static LuciObject *exec_string_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_str_t);

    if (VERBOSE) {
	printf("Allocating a new object of type obj_str_t, with value %s\n",
		a->data.s_val);
    }
    LuciObject *ret = create_object(obj_str_t);
    /* copy the 'string' from the ASTNode to the LuciObject */
    ret->value.s_val = (char *) alloc(strlen(a->data.s_val) + 1);
    strcpy (ret->value.s_val, a->data.s_val);

    return ret;
}

static LuciObject *exec_id_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_id_t);
    assert(e);
    Symbol *s;
    if (!(s = get_symbol(e, a->data.name))) {
	die("Can't find symbol %s");
    }
    if (s->type == sym_obj_t)
    {
	int t = s->data.object->type;
	if (VERBOSE) {
	    printf("Found symbol %s with type %d. Returning its object, with type:%d\n",
		    a->data.name, s->type, t);
	}
	LuciObject *copy = create_object(t);
	/* copy the symbol's data and return it */
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
	if (VERBOSE) {
	    printf("Found symbol %s, but it's not an object\n", a->data.name);
	}
	return NULL;	 /* look up a->name in symbol table */
    }
}

static LuciObject *exec_bin_expression(ExecEnviron *e, ASTNode *a)
{
    assert(a->type == ast_expression_t);
    LuciObject *left = dispatch_statement(e, a->data.expression.left);
    LuciObject *right = dispatch_statement(e, a->data.expression.right);
    LuciObject *result = solve_bin_expr(left, right, a->data.expression.op);
    destroy_object(left);
    destroy_object(right);
    return result;
}

static LuciObject *exec_parameters(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_parameters_t);

    LuciObject *next = NULL;
    int i;
    for (i = a->data.parameters.count - 1; i >= 0; i--)
    {
	/* create the Object */
	LuciObject *item = dispatch_statement(e, a->data.parameters.parameters[i]);
	/* create the list item container */
	LuciObject *tail = create_object(obj_list_t);
	/* link this container to 'next' container */
	tail->value.list.next = next;
	/* store ptr to actual object in container */
	tail->value.list.item = item;
	/* point 'next' to this container */
	next = tail;
    }
    return next;
}

static LuciObject *exec_assignment(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_assignment_t);
    assert(e);

    ASTNode *right = a->data.assignment.right;
    LuciObject *r = dispatch_statement(e, right);

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

    /* return an empty LuciObject * */
    return NULL;
    //s->data.object.type = obj_int_t;
    //s->data.object.value.i_val = r->value.i_val;
}

int evaluate_cond(struct ExecEnviron *e, struct ASTNode *cond)
{
    int huh = 0;
    LuciObject *val = dispatch_statement(e, cond);
    if (val == NULL) {
	return 0;
    }
    switch (val->type)
    {
	case obj_int_t:
	    huh = val->value.i_val;
	    break;
	case obj_double_t:
	    huh = (int)val->value.d_val;
	    break;
	case obj_str_t:
	    huh = 1;
	    break;
	default:
	    huh = 0;
    }
    destroy_object(val);
    return huh;
}

static LuciObject *exec_while(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_while_t);

    if (VERBOSE) {
	printf("Begin while loop\n");
    }

    int huh = evaluate_cond(e, a->data.while_loop.cond);
    while (huh)
    {
	dispatch_statement(e, a->data.while_loop.statements);
	huh = evaluate_cond(e, a->data.while_loop.cond);
    }
    return NULL;
}

static LuciObject *exec_if(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_if_t);

    if (VERBOSE) {
	printf("Begin if block\n");
    }

    int huh = evaluate_cond(e, a->data.if_else.cond);
    printf("%d\n", huh);
    if (huh)
    {
	dispatch_statement(e, a->data.if_else.ifstatements);
    }
    else
    {
	dispatch_statement(e, a->data.if_else.elstatements);
    }
    return NULL;
}

static LuciObject *exec_call(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_call_t);

    if (VERBOSE) {
	printf("Calling %s\n", a->data.call.name);
    }
    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	die("Invalid function");
    }
    if (s->type != sym_func_t)
    {
	char *msg = " is not a function";
	char comb[strlen(s->name) + strlen(msg) + 1];
	strncpy(comb, s->name, strlen(s->name) + 1);
	strncat(comb, msg, strlen(msg) );
	die(comb);
    }
    else
    {
	LuciObject *param_list = dispatch_statement(e, a->data.call.parameters);
	LuciObject *ret = /*(LuciObject *)*/((*(s->data.funcptr))(param_list));
	destroy_object(param_list);
	return ret;
    }
}

static LuciObject *exec_statement(struct ExecEnviron *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_statements_t);
    int i;
    for (i=0; i < a->data.statements.count; i++)
    {
	LuciObject *ret = dispatch_statement(e, a->data.statements.statements[i]);
	destroy_object(ret);
    }
    return NULL;
}

void exec_AST(struct ExecEnviron *e, struct ASTNode *a)
{
    dispatch_statement(e, a);
}

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

    extern struct func_init builtins[];
    int i;
    for (i = 0; builtins[i].name != 0; i++)
    {
	Symbol *sym = add_symbol(e, builtins[i].name, sym_func_t);
	sym->data.funcptr = builtins[i].func;
    }

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
	    destroy_object(ptr->data.object);
	}
	/* free the Symbol struct itself */
	free(ptr);
	ptr = next;
    }
    /* Free the environment struct */
    free(e);
}
