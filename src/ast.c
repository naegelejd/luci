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

static AstNode *create_node(int lineno, int type);


static AstNode *create_node(int lineno, int type) {
    AstNode *new = alloc(sizeof(*new));
    new->lineno = lineno;
    new->type = type;
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


AstNode *make_int_expr(int lineno, long val)
{
    AstNode *result = create_node(lineno, ast_int_t);
    result->data.i_val = val;
    yak("Made expression node from val %ld\n", val);
    return result;
}

AstNode *make_float_expr(int lineno, double val)
{
    AstNode *result = create_node(lineno, ast_float_t);
    result->data.f_val = val;
    yak("Made expression node from val %f\n", val);
    return result;
}

AstNode *make_string_expr(int lineno, char *val)
{
    AstNode *result = create_node(lineno, ast_string_t);
    result->data.s_val = val;
    yak("Made expression node from string %s\n", val);
    return result;
}

AstNode *make_id_expr(int lineno, char *name)
{
    AstNode *result = create_node(lineno, ast_id_t);
    result->data.name = name;
    yak("Made expression node from id %s\n", name);
    return result;
}

AstNode *make_binary_expr(int lineno, AstNode *left,
        AstNode *right, int op)
{
    AstNode *result = create_node(lineno, ast_expr_t);
    result->data.expression.left = left;
    result->data.expression.right = right;
    result->data.expression.op = op;
    yak("Made binary expression node with op %d\n", op);
    return result;
}

AstNode *make_list_index(int lineno, AstNode *list, AstNode *index)
{
    AstNode *result = create_node(lineno, ast_listindex_t);
    result->data.listindex.list = list;
    result->data.listindex.index = index;
    yak("Made list index reference\n");
    return result;
}

AstNode *make_list_assignment(int lineno, char *id,
        AstNode *index, AstNode *right)
{
    AstNode *result = create_node(lineno, ast_listassign_t);
    result->data.listassign.name = id;
    result->data.listassign.index = index;
    result->data.listassign.right = right;
    yak("Made list assignment node to id: %s\n", id);
    return result;
}

AstNode *make_list_def(int lineno, AstNode *result, AstNode *to_append)
{
    if (!result)
    {
        result = create_node(lineno, ast_list_t);
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

AstNode *make_assignment(int lineno, char *id, AstNode *right)
{
    AstNode *result = create_node(lineno, ast_assign_t);
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    yak("Made assignment node to id: %s\n", id);
    return result;
}

AstNode *make_while_loop(int lineno, AstNode *cond, AstNode *statements)
{
    AstNode *result = create_node(lineno, ast_while_t);
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    yak("Made while node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

AstNode *make_for_loop(int lineno, char *id,
        AstNode *list, AstNode *statements)
{
    AstNode *result = create_node(lineno, ast_for_t);
    result->data.for_loop.name = id;
    result->data.for_loop.list = list;
    result->data.for_loop.statements = statements;
    yak("Made for node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

AstNode *make_if_else(int lineno, AstNode *cond, AstNode *block1, AstNode *block2)
{
    AstNode *result = create_node(lineno, ast_if_t);
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    yak("Made if-else node\n");
    return result;
}

AstNode *make_func_call(int lineno, char *id, AstNode *arglist)
{
    AstNode *result = create_node(lineno, ast_call_t);
    result->data.call.name = id;
    result->data.call.arglist = arglist;
    yak("Made call node with name: %s\n", id);
    return result;
}

AstNode *make_func_def(int lineno, char *name, AstNode *param_list,
        AstNode *statements, AstNode *return_expr)
{
    AstNode *result = create_node(lineno, ast_func_t);
    result->data.func_def.name = name;
    result->data.func_def.param_list = param_list;
    result->data.func_def.statements = statements;
    result->data.func_def.ret_expr = return_expr;
    yak("Made function definition node with name: %s\n", name);
    return result;
}

AstNode *make_statements(int lineno, AstNode *list, AstNode *new)
{
    if (!list)
    {
        list = create_node(lineno, ast_stmnts_t);
        list->data.statements.count = 0;
        list->data.statements.size = AST_STMNTS_SIZE;
        list->data.statements.statements = alloc(
                list->data.statements.size *
                sizeof(*list->data.statements.statements));
    }

    /* if 'new' is NULL, then return the empty array node 'list' */
    if (new) {
        /* realloc the array of statement pointers if necessary */
        if(++(list->data.statements.count) > list->data.statements.size) {
            list->data.statements.size <<= 1;
            list->data.statements.statements = realloc(
                    list->data.statements.statements,
                    list->data.statements.size *
                    sizeof(*list->data.statements.statements));
        }
        list->data.statements.statements[list->data.statements.count - 1] =
                new;
        yak("Added a new statement\n");
    }
    return list;
}
