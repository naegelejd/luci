#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "ast.h"
#include "common.h"


const char *TYPE_NAMES[] = {
    "int",
    "float",
    "string",
    "id",
    "expr",
    "list index",
    "list assignment",
    "list definition",
    "assignment",
    "while loop",
    "for loop",
    "if else",
    "function call",
    "function definition",
    "statements"
};

static AstNode *create_node(int type);
static AstNode *make_int_expr(long);
static AstNode *make_float_expr(double);
static AstNode *make_string_expr(char *);
static AstNode *make_id_expr(char *);
static AstNode *make_binary_expr(AstNode *, AstNode *, int op);
static AstNode *make_list_index(AstNode *, AstNode *);
static AstNode *make_list_assignment(char *, AstNode *, AstNode *);
static AstNode *make_list_def(AstNode *, AstNode *);
static AstNode *make_assignment(char *, AstNode *);
static AstNode *make_while_loop(AstNode *, AstNode *);
static AstNode *make_for_loop(char *, AstNode *, AstNode *);
static AstNode *make_if_else(AstNode *, AstNode *, AstNode *);
static AstNode *make_func_call(char *, AstNode *);
static AstNode *make_func_def(char *, AstNode *, AstNode *, AstNode *);
static AstNode *make_statements(AstNode *, AstNode *);


AstNode * construct_node(AstType atype, ... )
{
    AstNode *x = NULL, *y = NULL, *z = NULL;
    AstNode *new = NULL;
    int i;
    long l;
    double d;
    char *s;

    va_list args;
    va_start(args, atype);

    switch(atype) {
        case ast_int_t:
            l = va_arg(args, long);
            new = make_int_expr(l);
            break;
        case ast_float_t:
            d = va_arg(args, double);
            new = make_float_expr(d);
            break;
        case ast_string_t:
            s = va_arg(args, char *);
            new = make_string_expr(s);
            break;
        case ast_id_t:
            s = va_arg(args, char *);
            new = make_id_expr(s);
            break;
        case ast_expr_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            i = va_arg(args, int);
            new = make_binary_expr(x, y, i);
            break;
        case ast_listindex_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_list_index(x, y);
            break;
        case ast_listassign_t:
            s = va_arg(args, char *);
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_list_assignment(s, x, y);
            break;
        case ast_list_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_list_def(x, y);
            break;
        case ast_assign_t:
            s = va_arg(args, char *);
            x = va_arg(args, AstNode *);
            new = make_assignment(s, x);
            break;
        case ast_while_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_while_loop(x, y);
            break;
        case ast_for_t:
            s = va_arg(args, char *);
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_for_loop(s, x, y);
            break;
        case ast_if_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            z = va_arg(args, AstNode *);
            new = make_if_else(x, y, z);
            break;
        case ast_call_t:
            s = va_arg(args, char *);
            x = va_arg(args, AstNode *);
            new = make_func_call(s, x);
            break;
        case ast_func_t:
            s = va_arg(args, char *);
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            z = va_arg(args, AstNode *);
            new = make_func_def(s, x, y, z);
            break;
        case ast_stmnts_t:
            x = va_arg(args, AstNode *);
            y = va_arg(args, AstNode *);
            new = make_statements(x, y);
            break;
        default:
            /* va_end(args); */
            die("Unknown AST Node type: %d", atype);
    }

    va_end(args);

    return new;
}

void destroy_tree(AstNode *root)
{
    /* don't free a NULL statement */
    if (!root)
        return;

    int i;
    switch (root->type)
    {
        case ast_stmnts_t:
            for (i = 0; i < root->data.statements.count; i++)
            {
                destroy_tree(root->data.statements.statements[i]);
            }
            free(root->data.statements.statements);
            break;
        case ast_func_t:
            free(root->data.func_def.name);
            destroy_tree(root->data.func_def.param_list);
            destroy_tree(root->data.func_def.statements);
            destroy_tree(root->data.func_def.ret_expr);
            break;
        case ast_list_t:
            for (i = 0; i < root->data.list.count; i++)
            {
                destroy_tree(root->data.list.items[i]);
            }
            free(root->data.list.items);
            break;
        case ast_while_t:
            destroy_tree(root->data.while_loop.cond);
            destroy_tree(root->data.while_loop.statements);
            break;
        case ast_for_t:
            destroy_tree(root->data.for_loop.list);
            destroy_tree(root->data.for_loop.statements);
            free(root->data.for_loop.name);
            break;
        case ast_if_t:
            destroy_tree(root->data.if_else.cond);
            destroy_tree(root->data.if_else.ifstatements);
            destroy_tree(root->data.if_else.elstatements);
            break;
        case ast_assign_t:
            destroy_tree(root->data.assignment.right);
            free(root->data.assignment.name);
            break;
        case ast_call_t:
            destroy_tree(root->data.call.arglist);
            free(root->data.call.name);
            break;
        case ast_listindex_t:
            destroy_tree(root->data.listindex.list);
            destroy_tree(root->data.listindex.index);
            break;
        case ast_listassign_t:
            destroy_tree(root->data.listassign.index);
            destroy_tree(root->data.listassign.right);
            free(root->data.listassign.name);
            break;
        case ast_expr_t:
            destroy_tree(root->data.expression.left);
            destroy_tree(root->data.expression.right);
            break;
        case ast_id_t:
            free(root->data.name);
            break;
        case ast_string_t:
            free(root->data.s_val);
            break;
        default:
            break;
    }
    /* for all nodes */
    yak("Deleting %s\n", TYPE_NAMES[root->type]);
    free(root);
    return;
}


static AstNode *create_node(int type) {
    AstNode *new = alloc(sizeof(*new));
    new->type = type;
    return new;
}

static AstNode *make_int_expr(long val)
{
    AstNode *result = create_node(ast_int_t);
    result->data.i_val = val;
    yak("Made expression node from val %ld\n", val);
    return result;
}

static AstNode *make_float_expr(double val)
{
    AstNode *result = create_node(ast_float_t);
    result->data.f_val = val;
    yak("Made expression node from val %f\n", val);
    return result;
}

static AstNode *make_string_expr(char *val)
{
    AstNode *result = create_node(ast_string_t);
    result->data.s_val = val;
    yak("Made expression node from string %s\n", val);
    return result;
}

static AstNode *make_id_expr(char *name)
{
    AstNode *result = create_node(ast_id_t);
    result->data.name = name;
    yak("Made expression node from id %s\n", name);
    return result;
}

static AstNode *make_binary_expr(AstNode *left, AstNode *right, int op)
{
    AstNode *result = create_node(ast_expr_t);
    result->data.expression.left = left;
    result->data.expression.right = right;
    result->data.expression.op = op;
    yak("Made binary expression node with op %d\n", op);
    return result;
}

static AstNode *make_list_index(AstNode *list, AstNode *index)
{
    AstNode *result = create_node(ast_listindex_t);
    result->data.listindex.list = list;
    result->data.listindex.index = index;
    yak("Made list index reference\n");
    return result;
}

static AstNode *make_list_assignment(char *id, AstNode *index, AstNode *right)
{
    AstNode *result = create_node(ast_listassign_t);
    result->data.listassign.name = id;
    result->data.listassign.index = index;
    result->data.listassign.right = right;
    yak("Made list assignment node to id: %s\n", id);
    return result;
}

static AstNode *make_list_def(AstNode *result, AstNode *to_append)
{
    if (!result)
    {
        result = create_node(ast_list_t);
        result->data.list.size = AST_LIST_SIZE;
        result->data.list.items = alloc(result->data.list.size *
                sizeof(*result->data.list.items));
        result->data.list.count = 0;
    }
    /* checking for a NULL 'to_append' allows for empty list creation */
    if (to_append) {
        assert(result->type == ast_list_t);
        if (++(result->data.list.count) > result->data.list.size) {
            result->data.list.size = result->data.list.size << 1;
            result->data.list.items = realloc(result->data.list.items,
                result->data.list.size * sizeof(*result->data.list.items));
        }
        result->data.list.items[result->data.list.count - 1] = to_append;
    }
    return result;
}

static AstNode *make_assignment(char *id, AstNode *right)
{
    AstNode *result = create_node(ast_assign_t);
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    yak("Made assignment node to id: %s\n", id);
    return result;
}

static AstNode *make_while_loop(AstNode *cond, AstNode *statements)
{
    AstNode *result = create_node(ast_while_t);
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    yak("Made while node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

static AstNode *make_for_loop(char *id, AstNode *list, AstNode *statements)
{
    AstNode *result = create_node(ast_for_t);
    result->data.for_loop.name = id;
    result->data.for_loop.list = list;
    result->data.for_loop.statements = statements;
    yak("Made for node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

static AstNode *make_if_else(AstNode *cond, AstNode *block1, AstNode *block2)
{
    AstNode *result = create_node(ast_if_t);
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    yak("Made if-else node\n");
    return result;
}

static AstNode *make_func_call(char *id, AstNode *arglist)
{
    AstNode *result = create_node(ast_call_t);
    result->data.call.name = id;
    result->data.call.arglist = arglist;
    yak("Made call node with name: %s\n", id);
    return result;
}

static AstNode *make_func_def(char *name, AstNode *param_list,
        AstNode *statements, AstNode *return_expr)
{
    AstNode *result = create_node(ast_func_t);
    result->data.func_def.name = name;
    result->data.func_def.param_list = param_list;
    result->data.func_def.statements = statements;
    result->data.func_def.ret_expr = return_expr;
    yak("Made function definition node with name: %s\n", name);
    return result;
}

static AstNode *make_statements(AstNode *result, AstNode *to_append)
{
    if (!result)
    {
        result = create_node(ast_stmnts_t);
        result->data.statements.count = 0;
        result->data.statements.size = AST_STMNTS_SIZE;
        result->data.statements.statements = alloc(
                result->data.statements.size *
                sizeof(*result->data.statements.statements));
    }

    /* checking if to_append is not NULL allows us to make empty statements */
    if (to_append) {
        /* realloc the array of statement pointers if necessary */
        if(++(result->data.statements.count) > result->data.statements.size) {
            result->data.statements.size = result->data.statements.size << 1;
            result->data.statements.statements = realloc(
                    result->data.statements.statements,
                    result->data.statements.size *
                    sizeof(*result->data.statements.statements));
        }
        result->data.statements.statements[result->data.statements.count - 1] =
                to_append;
        yak("Added a new statement\n");
    }
    return result;
}
