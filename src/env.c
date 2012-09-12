#include "types.h"
#include "env.h"
#include "ast.h"
#include "functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>


static struct LuciObject *dispatch_statement(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_int_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_double_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_string_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_id_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_bin_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list_index(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list_assignment(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_assignment(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_while(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_if(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_call(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_statement(ExecContext *e, struct ASTNode *a);

/* Lookup Array for AST nodes which yield values */
static LuciObject * (*exec_lookup[])(ExecContext *e, ASTNode *a) =
{
    exec_int_expression,
    exec_double_expression,
    exec_string_expression,
    exec_id_expression,
    exec_bin_expression,
    exec_list_index,
    exec_list_assignment,
    exec_list,
    exec_assignment,
    exec_while,
    exec_if,
    exec_call,
    exec_statement,
};

static LuciObject *dispatch_statement(ExecContext *e, ASTNode *a)
{
    if (!a || a == NULL)
    {
	return NULL;
    }
    if (!(exec_lookup[a->type]))
    {
	die("IDK what to do\n");
    }
    else
    {
	return exec_lookup[a->type](e, a);
    }
}

static LuciObject *exec_int_expression(ExecContext *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_int_t);

    yak("Allocating a new object of type obj_int_t, with value %d\n",
	    a->data.i_val);

    LuciObject *ret = create_object(obj_int_t);
    ret->value.i_val = a->data.i_val;

    return ret;
}

static LuciObject *exec_double_expression(ExecContext *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_double_t);

    yak("Allocating a new object of type obj_double_t, with value %f\n",
	    a->data.d_val);

    LuciObject *ret = create_object(obj_double_t);
    ret->value.d_val = a->data.d_val;

    return ret;
}

static LuciObject *exec_string_expression(ExecContext *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_str_t);

    yak("Allocating a new object of type obj_str_t, with value %s\n",
	    a->data.s_val);

    LuciObject *ret = create_object(obj_str_t);
    /* copy the 'string' from the ASTNode to the LuciObject */
    ret->value.s_val = (char *) alloc(strlen(a->data.s_val) + 1);
    strcpy (ret->value.s_val, a->data.s_val);

    return ret;
}

static LuciObject *exec_id_expression(ExecContext *e, ASTNode *a)
{
    assert(a);
    assert(a->type == ast_id_t);
    assert(e);
    Symbol *s;
    if (!(s = get_symbol(e, a->data.name))) {
	die("Can't find symbol %s\n", a->data.name);
    }
    if (s->type == sym_obj_t)
    {
	LuciObject *orig = s->data.object;

	/* check if symbol's value is NULL */
	if (!orig) {
	    yak("Found symbol with NULL value.\n");
	    return NULL;
	}

	/* else, return a copy of symbol's object */
	int t = orig->type;
	yak("Found symbol %s with type %d. Returning its object, with type:%d\n",
		a->data.name, s->type, t);
	/* copy the symbol's data and return it
	   This allows the user to do whatever they want with
	   the VALUE of the symbol(variable), but the symbol itself
	   will retain this original value until a NEW ASSIGNMENT
	   overwrites it.
	*/
	return copy_object(orig);
    }
    else
    {
	yak("Found symbol %s, but it's not an object\n", a->data.name);
	return NULL;	 /* look up a->name in symbol table */
    }
}

static LuciObject *exec_bin_expression(ExecContext *e, ASTNode *a)
{
    assert(a->type == ast_expression_t);
    LuciObject *left = dispatch_statement(e, a->data.expression.left);
    LuciObject *right = dispatch_statement(e, a->data.expression.right);
    LuciObject *result = solve_bin_expr(left, right, a->data.expression.op);
    destroy_object(left);
    destroy_object(right);
    return result;
}

static LuciObject *exec_list_index(struct ExecContext *e, struct ASTNode *a)
{
    assert(a->type = ast_listindex_t);
    LuciObject *list = dispatch_statement(e, a->data.listindex.list);
    LuciObject *index = dispatch_statement(e, a->data.listindex.index);

    if (!list) {
	die("Can't index a NULL object\n");
    }
    if (list->type != obj_list_t) {
	die("Can't index a non-list object\n");
    }
    if (!index) {
	die("Can't index list with NULL index\n");
    }
    /* Only allow integer indexes */
    if (index->type != obj_int_t) {
	die("List index must be integer type\n");
    }
    int idx = index->value.i_val;
    destroy_object(index);

    int i = 0, found = 0;
    LuciObject *cur = list;
    LuciObject *ret = NULL;
    /*for (cur = list, i = 0; cur != NULL, i == idx; cur = cur->next, i++); */
    while (cur) {
	if (i == idx) {
	    ret = copy_object(cur->value.list.item);
	    found = 1;
	    break;
	}
	cur = cur->value.list.next;
	i++;
    }

    destroy_object(list);
    if (!found) {
	die("List index exceeds length of list\n");
    }

    return ret;
}

/*
   The execution of a list index assignment must traverse
   the list, and find the correct index first.
   Ideally, the exec_list_index function could return this object,
   but for now, since I'm creating copies of all referenced symbols,
   this wouldn't work because exec_list_index returns a COPY of the value
   at the list's index.
*/
static LuciObject *exec_list_assignment(struct ExecContext *e, struct ASTNode *a)
{
    assert(a->type = ast_listassign_t);

    LuciObject *index = dispatch_statement(e, a->data.listassign.index);
    if (!index || (index->type != obj_int_t)) {
	die("Index is not an integer\n");
    }
    int idx = index->value.i_val;
    /* we got the list index so destroy the object */
    destroy_object(index);

    LuciObject *right = dispatch_statement(e, a->data.listassign.right);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.listassign.name)))
    {
	destroy_object(right);
	die("List %s does not exist\n", a->data.listassign.name);
    }
    else
    {
	LuciObject *list = s->data.object;
	if (!list || (list->type != obj_list_t)) {
	    destroy_object(right);
	    die("Can't index a non-list object in assignment\n");
	}

	int i = 0, found = 0;
	LuciObject *cur = list;
	while (cur) {
	    if (i == idx) {
		/* destroy the existing list index's contents */
		destroy_object(cur->value.list.item);
		/* set the list's contents at the index */
		cur->value.list.item = right;
		found = 1;
		break;
	    }
	    cur = cur->value.list.next;
	    i++;
	}
	if (!found) {
	    destroy_object(right);
	    die("List index exceeds length of list\n");
	}
    }

    /* return an empty LuciObject * */
    return NULL;
}

/*
   Executes a list node in the abstract syntax tree.
*/
static LuciObject *exec_list(struct ExecContext *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_list_t);

    LuciObject *next = NULL;
    int i;
    for (i = a->data.list.count - 1; i >= 0; i--)
    {
	/* create the Object */
	LuciObject *item = dispatch_statement(e, a->data.list.items[i]);
	/* create the list item container */
	LuciObject *tail = create_object(obj_list_t);
	/* link this container to 'next' container */
	tail->value.list.next = next;
	/* store ptr to actual object in container */
	tail->value.list.item = item;
	/* point 'next' to this container */
	next = tail;

	yak("Adding new list item to list\n");
    }
    return next;
}

/*
   Executes a variable assignment node in the abstract
   syntax tree.
*/
static LuciObject *exec_assignment(struct ExecContext *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_assignment_t);
    assert(e);

    LuciObject *right = dispatch_statement(e, a->data.assignment.right);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.assignment.name)))
    {
	s = add_symbol(e, a->data.assignment.name, sym_obj_t);
    }
    else
    {
	/* if the symbol already exists, free its existing Object */
	/* TODO: this should close any open file pointers...
	   if a symbol points to a FILE object and the user
	   wants to point the symbol to something else, it would
	   be nice for the file pointer to be closed
	*/
	destroy_object(s->data.object);
    }
    /* set the symbol's new payload */
    s->data.object = right;

    /* return an empty LuciObject * */
    return NULL;
}

/*
   Evaluates a conditional statement, returning an integer
   value of 0 if False, and non-zero if True.
*/
int evaluate_cond(struct ExecContext *e, struct ASTNode *cond)
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

/*
   Executes a while loop in the abstract syntax tree.
*/
static LuciObject *exec_while(struct ExecContext *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_while_t);

    yak("Begin while loop\n");

    int huh = evaluate_cond(e, a->data.while_loop.cond);
    while (huh)
    {
	dispatch_statement(e, a->data.while_loop.statements);
	huh = evaluate_cond(e, a->data.while_loop.cond);
    }
    return NULL;
}

/*
   Executes an if/else statement in the abstract syntax tree.
*/
static LuciObject *exec_if(struct ExecContext *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_if_t);

    yak("Begin if block\n");

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

/*
   Executes a function call statement in the abstract
   syntax tree.
   First it searches for the function corresponding to the user
   specified ID in the Execution Context's symbol table.
   If found, the function's parameters node is executed and the results
   are passed to the function's definition.
*/
static LuciObject *exec_call(struct ExecContext *e, struct ASTNode *a)
{
    assert(a);
    assert(a->type == ast_call_t);

    yak("Calling %s\n", a->data.call.name);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	die("Invalid function name: %s\n", a->data.call.name);
    }
    if (s->type != sym_func_t)
    {
	die("%s is not a function\n", s->name);
    }
    else
    {
	LuciObject *param_list = dispatch_statement(e, a->data.call.param_list);
	if (param_list) {
	    if (param_list->type != obj_list_t) {
		die("Malformed function parameters\n");
	    }
	}
	LuciObject *ret = /*(LuciObject *)*/((*(s->data.funcptr))(param_list));
	destroy_object(param_list);
	return ret;
    }
}

/*
   Executes a statement node in the abstract syntax tree.
*/
static LuciObject *exec_statement(struct ExecContext *e, struct ASTNode *a)
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

/*
   Entry point for executing the abstract syntax tree.
*/
void exec_AST(struct ExecContext *e, struct ASTNode *a)
{
    dispatch_statement(e, a);
}

/*
   Adds a new symbol to the symbol table of the specified
   Execution Context, returning a pointer to the new symbol.

   Only the symbol's name and type must be given. The caller
   is responsible for setting the symbol's data.
*/
Symbol *add_symbol (struct ExecContext *e, char const *name, int type)
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

/*
   Searches for a symbol with a matching name in the
   symbol table of the specified Execution Context
*/
Symbol *get_symbol (struct ExecContext *e, const char *name)
{
    Symbol *ptr;
    for (ptr = e->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next)
	if (strcmp (ptr->name, name) == 0)
	    return ptr;
    return 0;
}

/*
   Creates an Execution Context, which consists
   of various members, most importantly its symbol table.
*/
ExecContext *create_env(void)
{
    /* Check that we have dispatchers for all types of statements */
    assert(ast_last_t == (sizeof(exec_lookup) / sizeof(*exec_lookup)));

    ExecContext *e = calloc(1, sizeof(struct ExecContext));

    /* give the global context a name */
    char *name = "global";
    e->name = alloc(strlen(name) + 1);
    strcpy(e->name, name);

    extern struct func_def builtins[];
    int i;
    for (i = 0; builtins[i].name != 0; i++)
    {
	Symbol *sym = add_symbol(e, builtins[i].name, sym_func_t);
	sym->data.funcptr = builtins[i].func;
    }

    return e;
}

/*
   Destroys the Execution Context by freeing its members.
*/
void destroy_env(ExecContext *e)
{
    /* destroy the Context's name */
    free(e->name);

    /* destroy the Context's symbol table */
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
