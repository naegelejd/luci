#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "types.h"
#include "env.h"
#include "ast.h"
#include "functions.h"


static struct LuciObject *dispatch_statement(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_int_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_float_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_string_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_id_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_bin_expression(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list_index(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list_assignment(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_list(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_assignment(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_while(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_for(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_if(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_call(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_func_def(ExecContext *e, struct ASTNode *a);
static struct LuciObject *exec_statement(ExecContext *e, struct ASTNode *a);

/* Lookup Array for AST nodes which yield values */
static LuciObject * (*exec_lookup[])(ExecContext *e, ASTNode *a) =
{
    exec_int_expression,
    exec_float_expression,
    exec_string_expression,
    exec_id_expression,
    exec_bin_expression,
    exec_list_index,
    exec_list_assignment,
    exec_list,
    exec_assignment,
    exec_while,
    exec_for,
    exec_if,
    exec_call,
    exec_func_def,
    exec_statement,
};

/* Calls the corresponding handler function for the type
   of the given ASTNode *
*/
static LuciObject *dispatch_statement(ExecContext *e, ASTNode *a)
{
    if (!e) {
	die("NULL Execution Context\n");
    }

    if (!a) {
	yak("NULL node in the abstract syntax tree\n");
	return NULL;
    }

    if (!(exec_lookup[a->type])) {
	die("IDK what to do\n");
    }
    else {
	return exec_lookup[a->type](e, a);
    }
}

static LuciObject *exec_int_expression(ExecContext *e, ASTNode *a)
{
    yak("Allocating a new object of type obj_int_t, with value %d\n",
	    a->data.i_val);

    LuciObject *ret = create_object(obj_int_t);
    ret->value.i_val = a->data.i_val;

    return ret;
}

static LuciObject *exec_float_expression(ExecContext *e, ASTNode *a)
{
    yak("Allocating a new object of type obj_float_t, with value %f\n",
	    a->data.f_val);

    LuciObject *ret = create_object(obj_float_t);
    ret->value.f_val = a->data.f_val;

    return ret;
}

static LuciObject *exec_string_expression(ExecContext *e, ASTNode *a)
{
    yak("Allocating a new object of type obj_str_t, with value %s\n",
	    a->data.s_val);

    LuciObject *ret = create_object(obj_str_t);
    /* copy the 'string' from the ASTNode to the LuciObject */
    size_t len = strlen(a->data.s_val);
    ret->value.s_val = (char *) alloc(len + 1);
    strncpy (ret->value.s_val, a->data.s_val, len);
    ret->value.s_val[len] = '\0';

    return ret;
}

static LuciObject *exec_id_expression(ExecContext *e, ASTNode *a)
{
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
	yak("Found object symbol %s. Returning its data, with type:%d\n",
		a->data.name, s->type, t);
	/* return a reference to this symbol's value */
	return reference_object(orig);
    }
    else
    {
	yak("Found function symbol %s\n", a->data.name);
	return NULL;	 /* look up a->name in symbol table */
    }
}

static LuciObject *exec_bin_expression(ExecContext *e, ASTNode *a)
{
    LuciObject *left = dispatch_statement(e, a->data.expression.left);
    LuciObject *right = dispatch_statement(e, a->data.expression.right);
    LuciObject *result = solve_bin_expr(left, right, a->data.expression.op);
    destroy_object(left);
    destroy_object(right);
    return result;
}

static LuciObject *exec_list_index(struct ExecContext *e, struct ASTNode *a)
{
    LuciObject *list = dispatch_statement(e, a->data.listindex.list);
    if (!list) {
	die("Can't index a NULL object\n");
    }
    if (list->type != obj_list_t) {
	destroy_object(list);
	die("Can't index a non-list object\n");
    }

    LuciObject *index = dispatch_statement(e, a->data.listindex.index);
    if (!index) {
	die("Can't index list with NULL index\n");
    }
    /* Only allow integer indexes */
    if (index->type != obj_int_t) {
	destroy_object(index);
	die("List index must be integer type\n");
    }
    int idx = index->value.i_val;
    destroy_object(index);

    LuciObject *item = list_get_object(list, idx);
    /* dereference the list */
    destroy_object(list);
    if (!item) {
	die("Index %d out of bounds\n", idx);
    }
    /* COPY the list item.
       Think about it. When operating on an item
       in a list, you request a copy of the contents
       of this list at a given index.
    */
    return copy_object(item);
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
	/* put the new item in the list at the index */
	/* this will also de-reference the item currently at the index */
	list_set_object(list, right, idx);
    }

    /* return an empty LuciObject * */
    return NULL;
}

/*
   Executes a list node in the abstract syntax tree.
*/
static LuciObject *exec_list(struct ExecContext *e, struct ASTNode *a)
{
    LuciObject *list = create_object(obj_list_t);
    int i;
    for (i = 0; i < a->data.list.count; i++)
    {
	/* create the Object */
	LuciObject *item = dispatch_statement(e, a->data.list.items[i]);
	list_append_object(list, item);
	yak("Adding new list item to list\n");
    }
    return list;
}

/*
   Executes a variable assignment node in the abstract
   syntax tree.
*/
static LuciObject *exec_assignment(struct ExecContext *e, struct ASTNode *a)
{
    /* right could be an object with refcount=1 (expression result,
       function return)
       or an object with refcount>=2 (the value of a symbol)
    */
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
   Executes a while loop in the abstract syntax tree.
*/
static LuciObject *exec_while(struct ExecContext *e, struct ASTNode *a)
{
    yak("Begin while loop\n");

    LuciObject *none, *condition = dispatch_statement(e, a->data.while_loop.cond);
    int huh = evaluate_condition(condition);
    destroy_object(condition);
    while (huh)
    {
	/* all statements will return NULL, but I still catch them
	   and 'destroy' them, like in the other node executions
	*/
	none = dispatch_statement(e, a->data.while_loop.statements);
	destroy_object(none);
	condition = dispatch_statement(e, a->data.while_loop.cond);
	huh = evaluate_condition(condition);
	destroy_object(condition);
    }
    return NULL;
}

/*
   Executes a for loop in the abstract syntax tree
*/
static LuciObject *exec_for(struct ExecContext *e, struct ASTNode *a)
{
    yak("Begin for loop\n");

    LuciObject *list = dispatch_statement(e, a->data.for_loop.list);
    if (list->type != obj_list_t) {
	destroy_object(list);
	die("For loop can only iterate over list types\n");
    }

    /* create the symbol ahead of time if it doesn't exist */
    const char *name = a->data.for_loop.name;
    Symbol *s;
    if (!(s = get_symbol(e, name))) {
	s = add_symbol(e, name, sym_obj_t);
    }
    else {
	if (s->type = sym_obj_t) {
	    destroy_object(s->data.object);
	}
    }

    /* iterate through list, assigning value to symbol, then executing all statements */
    LuciObject *item, *none;
    int i;
    for (i = 0; i < list->value.list.count; i++) {
	/* copy the item in the list */
	item = copy_object(list_get_object(list, i));
	/* destroy the existing value of the symbol */
	destroy_object(s->data.object);
	/* assign item copy to symbol */
	s->data.object = item;
	/* execute all statements inside the for loop */
	none = dispatch_statement(e, a->data.for_loop.statements);
	destroy_object(none);
    }

    destroy_object(list);

    return NULL;
}

/*
   Executes an if/else statement in the abstract syntax tree.
*/
static LuciObject *exec_if(struct ExecContext *e, struct ASTNode *a)
{
    yak("Begin if block\n");

    LuciObject *none, *condition = dispatch_statement(e, a->data.if_else.cond);
    int huh = evaluate_condition(condition);
    destroy_object(condition);
    if (huh)
    {
	none = dispatch_statement(e, a->data.if_else.ifstatements);
	destroy_object(none);
    }
    else
    {
	none = dispatch_statement(e, a->data.if_else.elstatements);
	destroy_object(none);
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
	if (param_list->type != obj_list_t) {
	    die("Malformed function parameters\n");
	}
	LuciObject *ret = (*(s->data.funcptr))(param_list);
	destroy_object(param_list);
	return ret;
    }
}

/*
   Executes a function definition node
*/
static LuciObject *exec_func_def(struct ExecContext *e, struct ASTNode *a)
{
    struct ASTNode *call_sig = a->data.func_def.call_sig;
    yak("Defining function %s\n", call_sig->data.call.name);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	/* The function hasn't yet been defined
	   Make a new symbol */
    }
    else {
	/* The function is already defined in the symbol table
	   Gotta clean it up, free it and make a new symbol */
    }

    return NULL;
}

/*
   Executes a statement node in the abstract syntax tree.
*/
static LuciObject *exec_statement(struct ExecContext *e, struct ASTNode *a)
{
    LuciObject *none;
    int i;
    for (i=0; i < a->data.statements.count; i++)
    {
	none = dispatch_statement(e, a->data.statements.statements[i]);
	destroy_object(none);
    }
    return NULL;
}

/*
   Entry point for executing the abstract syntax tree.
*/
void exec_AST(struct ExecContext *e, struct ASTNode *a)
{
    LuciObject *none = dispatch_statement(e, a);
    destroy_object(none);
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
    switch (type) {
	case sym_obj_t:
	    ptr->data.object = NULL;
	    break;
	case sym_func_t:
	    ptr->data.funcptr = NULL;
	    break;
	default:
	    break;
    }
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
    if (ast_last_t != (sizeof(exec_lookup) / sizeof(*exec_lookup))) {
	fprintf(stderr, "Mismatch in # of ASTNode types and AST execution functions.\n");
	return NULL;
    }

    ExecContext *e = alloc(sizeof(struct ExecContext));

    /* give the global context a name */
    char *name = "global";
    e->name = alloc(strlen(name) + 1);
    strcpy(e->name, name);

    int i;
    extern struct func_def builtins[];
    for (i = 0; builtins[i].name != 0; i++)
    {
	Symbol *sym = add_symbol(e, builtins[i].name, sym_func_t);
	sym->data.funcptr = builtins[i].func;
    }

    init_variables();	/* populates extern array of initial symbols */
    extern struct var_def globals[];
    for (i = 0; globals[i].name != 0; i++) {
	Symbol *sym = add_symbol(e, globals[i].name, sym_obj_t);
	sym->data.object = globals[i].object;
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
