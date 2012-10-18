#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "env.h"
#include "ast.h"
#include "functions.h"


static struct LuciObject *dispatch_statement(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_int_expression(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_float_expression(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_string_expression(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_id_expression(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_bin_expression(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_list_index(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_list_assignment(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_list(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_assignment(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_while(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_for(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_if(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_call(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_func_def(ExecContext *e, struct AstNode *a);
static struct LuciObject *exec_statement(ExecContext *e, struct AstNode *a);

/**
  Lookup Array for AST nodes which yield values
 */
static LuciObject * (*exec_lookup[])(ExecContext *e, AstNode *a) =
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
   of the given AstNode *
*/
static LuciObject *dispatch_statement(ExecContext *e, AstNode *a)
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

static LuciObject *exec_int_expression(ExecContext *e, AstNode *a)
{
    yak("Allocating a new object of type obj_int_t, with value %d\n",
	    a->data.i_val);

    LuciObject *ret = create_object(obj_int_t);
    ret->value.i_val = a->data.i_val;

    return ret;
}

static LuciObject *exec_float_expression(ExecContext *e, AstNode *a)
{
    yak("Allocating a new object of type obj_float_t, with value %f\n",
	    a->data.f_val);

    LuciObject *ret = create_object(obj_float_t);
    ret->value.f_val = a->data.f_val;

    return ret;
}

static LuciObject *exec_string_expression(ExecContext *e, AstNode *a)
{
    yak("Allocating a new object of type obj_str_t, with value %s\n",
	    a->data.s_val);

    LuciObject *ret = create_object(obj_str_t);
    /* copy the 'string' from the AstNode to the LuciObject */
    size_t len = strlen(a->data.s_val);
    ret->value.s_val = (char *) alloc(len + 1);
    strncpy (ret->value.s_val, a->data.s_val, len);
    ret->value.s_val[len] = '\0';

    return ret;
}

static LuciObject *exec_id_expression(ExecContext *e, AstNode *a)
{
    Symbol *s;
    if (!(s = get_symbol(e, a->data.name))) {
	die("Can't find symbol %s\n", a->data.name);
    }
    if (s->type == sym_bobj_t || s->type == sym_uobj_t)
    {
	LuciObject *orig = s->data.object;

	/* check if symbol's value is NULL */
	if (!orig) {
	    yak("Found symbol with NULL value.\n");
	    return NULL;
	}

	/* else, return a copy of symbol's object */
	yak("Found object symbol %s\n", a->data.name);
	/* return a reference to this symbol's value */
	return reference_object(orig);
    }
    else
    {
	die("Found function symbol, but function references aren't supported yet\n");
    }
}

static LuciObject *exec_bin_expression(ExecContext *e, AstNode *a)
{
    LuciObject *left = dispatch_statement(e, a->data.expression.left);
    LuciObject *right = dispatch_statement(e, a->data.expression.right);
    LuciObject *result = solve_bin_expr(left, right, a->data.expression.op);
    destroy_object(left);
    destroy_object(right);
    return result;
}

static LuciObject *exec_list_index(struct ExecContext *e, struct AstNode *a)
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
static LuciObject *exec_list_assignment(struct ExecContext *e, struct AstNode *a)
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
static LuciObject *exec_list(struct ExecContext *e, struct AstNode *a)
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
static LuciObject *exec_assignment(struct ExecContext *e, struct AstNode *a)
{
    /* right could be an object with refcount=1 (expression result,
       function return)
       or an object with refcount>=2 (the value of a symbol)
    */
    LuciObject *right = dispatch_statement(e, a->data.assignment.right);

    Symbol *s;

    if (!(s = add_symbol(e, a->data.assignment.name, sym_uobj_t))) {
	die("Can't create symbol in assignment%s\n", a->data.assignment.name);
    }
    /* set the symbol's new payload */
    s->data.object = right;

    /* return an empty LuciObject * */
    return NULL;
}

/*
   Executes a while loop in the abstract syntax tree.
*/
static LuciObject *exec_while(struct ExecContext *e, struct AstNode *a)
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
static LuciObject *exec_for(struct ExecContext *e, struct AstNode *a)
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

    /* recycle the symbol if it exists or just make new*/
    if (!(s = add_symbol(e, name, sym_uobj_t))) {
	die("Can't create symbol %s\n", name);
    }

    /* iterate through list, assigning value to symbol, then executing all statements */
    /* NOTE: this directly changes the value of the symbol, rather than deleting it
       from the symbol table and re-creating it
    */
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
static LuciObject *exec_if(struct ExecContext *e, struct AstNode *a)
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
   If found, the function's arglist node is executed and the results
   are passed to the function's definition.
*/
static LuciObject *exec_call(struct ExecContext *e, struct AstNode *a)
{
    yak("Calling %s\n", a->data.call.name);

    Symbol *s;
    if (!(s = get_symbol(e, a->data.call.name))) {
	die("Invalid function name: %s\n", a->data.call.name);
    }
    if (s->type == sym_bfunc_t) {
	LuciObject *arglist = dispatch_statement(e, a->data.call.arglist);
	if (arglist->type != obj_list_t) {
	    die("Malformed function arguments\n");
	}
	LuciObject *ret = (*(s->data.funcptr))(arglist);
	destroy_object(arglist);
	return ret;
    }
    else if (s->type == sym_ufunc_t) {
	LuciObject *arglist = dispatch_statement(e, a->data.call.arglist);
	if (arglist->type != obj_list_t) {
	    die("Malformed function arguments\n");
	}
	struct AstNode *func_node = s->data.user_defined;
	char *func_name = func_node->data.func_def.name;
	struct AstNode *param_list = func_node->data.func_def.param_list;

	if (param_list->data.list.count != arglist->value.list.count) {
	    die("Incorrect number of args to function %s\n", func_name);
	}
	/* create an execution context for the function */
	struct ExecContext *local = create_context(func_name, e);

	Symbol *s;
	int i;
	for (i = 0; i < param_list->data.list.count; i++) {
	    if (param_list->data.list.items[i]->type != ast_string_t) {
		die("Malformed function parameters\n");
	    }
	    char *param_name = param_list->data.list.items[i]->data.s_val;
	    if (!(s = add_symbol(local, param_name, sym_uobj_t))) {
		die("Can't create symbol %s local to function %s\n",
			param_name, func_name);
	    }
	    s->data.object = reference_object(arglist->value.list.items[i]);
	}

	/* execute the function's statements, using it's local ExecContext */
	LuciObject *none = dispatch_statement(local, func_node->data.func_def.statements);
	destroy_object(none);

	/* execute the function's return expression */
	LuciObject *ret;
	if (func_node->data.func_def.ret_expr) {
	    ret = dispatch_statement(local, func_node->data.func_def.ret_expr);
	} else {
	    ret = NULL;
	}

	destroy_context(local);
	destroy_object(arglist);
	return ret;
    }
    else
    {
	die("%s is not a function\n", s->name);
    }
}

/*
   Executes a function definition node
*/
static LuciObject *exec_func_def(struct ExecContext *e, struct AstNode *a)
{
    char *name;
    Symbol *s;

    name = a->data.func_def.name;
    yak("Defining function %s\n", name);

    if (!(s = add_symbol(e, name, sym_ufunc_t))) {
	die("Can't create symbol %s\n", name);
    }
    /* store the AstNode for the user-defined function in the symbol */
    s->data.user_defined = a;

    return NULL;
}

/*
   Executes a statement node in the abstract syntax tree.
*/
static LuciObject *exec_statement(struct ExecContext *e, struct AstNode *a)
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
void exec_AST(struct ExecContext *e, struct AstNode *a)
{
    LuciObject *none = dispatch_statement(e, a);
    destroy_object(none);
}

/* destroys the memory used by Symbol *s
   This will will only delete builtin symbols
   if `force` is non-zero
*/
int destroy_symbol(Symbol *s, int force)
{
    if (!s) {
	return 1;
    }

    /* TODO: this should close any open file pointers...
       if a symbol points to a FILE object and the user
       wants to point the symbol to something else, it would
       be nice for the file pointer to be closed
    */

    /* destroy the symbol's name */
    yak("destroying symbol %s (type %d)\n", s->name, s->type);
    free(s->name);
    /* destroy the symbol's payload */
    switch (s->type)
    {
	case sym_bobj_t:
	    if (!force) {
		return 0;
	    }
	    /* else, destroy object */
	    destroy_object(s->data.object);
	    break;
	case sym_bfunc_t:
	    if (!force) {
		return 0;
	    }
	    break;
	case sym_uobj_t:
	    destroy_object(s->data.object);
	    break;
	case sym_ufunc_t:
	    break;
	default:
	    ;
    }
    /* destroy the symbol (struct) */
    free(s);
    s = NULL;
    /* return success */
    return 1;
}

/*
   Adds a new symbol to the symbol table of the specified
   Execution Context, returning a pointer to the new symbol.

   Only the symbol's name and type must be given. The caller
   is responsible for setting the symbol's data.
*/
Symbol *add_symbol (struct ExecContext *e, char const *name, int type)
{
    Symbol *new=NULL, *ptr=NULL, *prev=NULL;

    yak("searching for symbol %s\n", name);
    /* First, do a search for the symbol */
    for (
	    ptr = e->symtable, prev = NULL;
	    ptr != (Symbol *) 0;
	    prev = ptr, ptr = ptr->next
	)
    {
	/* if we find the symbol */
	if (strcmp (ptr->name, name) == 0) {
	    yak("found existing symbol %s\n", name);
	    /* maintain the linked list by pointing the previous symbol to the next */
	    if (prev) {
		prev->next = ptr->next;
	    }
	    else {
		e->symtable = ptr->next;
	    }
	    /* free the symbol's memory */
	    if (!(destroy_symbol(ptr, 0))) {
		return 0;
	    }
	    break;
	}
    }

    /* Create New Symbol */
    yak("creating new symbol %s\n", name);
    new = (Symbol *) alloc (sizeof (Symbol));
    new->name = (char *) alloc (strlen (name) + 1);
    strcpy (new->name, name);
    new->type = type;
    switch (type) {
	case sym_bobj_t:
	    new->data.object = NULL;
	    break;
	case sym_uobj_t:
	    new->data.object = NULL;
	    break;
	case sym_bfunc_t:
	    new->data.funcptr = NULL;
	    break;
	case sym_ufunc_t:
	    new->data.funcptr = NULL;
	    break;
	default:
	    break;
    }
    /* caller must explicitly set the data (payload) */
    new->next = e->symtable;
    e->symtable = new;
    return new;
}

/*
   Searches for a symbol with a matching name in the
   symbol table of the specified Execution Context
*/
Symbol *get_symbol (struct ExecContext *context, const char *name)
{
    Symbol *ptr;
    ExecContext *e = context;
    while (e) {
	for (ptr = e->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next)
	    if (strcmp (ptr->name, name) == 0)
		return ptr;
	e = e->parent;
    }

    return 0;
}

/*
   Creates an Execution Context, which consists
   of various members, most importantly its symbol table.
*/
ExecContext *create_context(const char* name, ExecContext *parent)
{
    /* Check that we have dispatchers for all types of statements */
    if (ast_last_t != (sizeof(exec_lookup) / sizeof(*exec_lookup))) {
	fprintf(stderr, "Mismatch in # of AstNode types and AST execution functions.\n");
	return NULL;
    }

    yak("creating context %s\n", name);
    ExecContext *e = alloc(sizeof(struct ExecContext));

    /* give the global context a name */
    e->name = alloc(strlen(name) + 1);
    strcpy(e->name, name);

    e->parent = parent;

    return e;
}

void initialize_context(ExecContext *e)
{
    yak("populating context %s with builtins\n", e->name);

    int i;
    extern struct func_def builtins[];
    for (i = 0; builtins[i].name != 0; i++)
    {
	Symbol *sym = add_symbol(e, builtins[i].name, sym_bfunc_t);
	sym->data.funcptr = builtins[i].func;
    }

    init_variables();	/* populates extern array of initial symbols */
    extern struct var_def globals[];
    for (i = 0; globals[i].name != 0; i++) {
	Symbol *sym = add_symbol(e, globals[i].name, sym_bobj_t);
	sym->data.object = globals[i].object;
    }

}

/*
   Destroys the Execution Context by freeing its members.
*/
void destroy_context(ExecContext *e)
{
    if (e == NULL) {
        /* context doesn't exist */
        return;
    }

    yak("destroying context %s\n", e->name);
    /* destroy the Context's name */
    free(e->name);

    /* destroy the Context's symbol table */
    Symbol *ptr = e->symtable;
    Symbol *next = ptr;
    while (ptr != (Symbol *) 0)
    {
	next = (Symbol *)ptr->next;
	/* destroy symbol, force destruction of builtins */
	destroy_symbol(ptr, 1);
	ptr = next;
    }
    /* Free the environment struct */
    free(e);
}

