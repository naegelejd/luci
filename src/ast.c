/*
 * See Copyright Notice in luci.h
 */

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
    "list access",
    "list assignment",
    "list definition",
    "assignment",
    "while loop",
    "for loop",
    "if else",
    "function call",
    "function definition",
    "statements",
    "break",
    "continue",
    "return",
    "pass"
};

static AstNode *create_node(int type);


static AstNode *create_node(int type) {
    AstNode *new = alloc(sizeof(*new));
    new->type = type;
    new->lineno = get_line_num();
    new->column = get_last_col_num();
    return new;
}


AstNode *make_int_constant(long val)
{
    AstNode *result = create_node(ast_integer_t);
    result->data.i = val;
    yak("Made expression node from val %ld\n", val);
    return result;
}

AstNode *make_float_constant(double val)
{
    AstNode *result = create_node(ast_float_t);
    result->data.f = val;
    yak("Made expression node from val %f\n", val);
    return result;
}

AstNode *make_string_constant(char *val)
{
    AstNode *result = create_node(ast_string_t);
    result->data.s = val;
    yak("Made expression node from string %s\n", val);
    return result;
}

AstNode *make_id_expr(char *name)
{
    AstNode *result = create_node(ast_id_t);
    result->data.id.val = name;
    yak("Made expression node from id %s\n", name);
    return result;
}

AstNode *make_binary_expr(AstNode *left,
        AstNode *right, int op)
{
    AstNode *result = create_node(ast_expr_t);
    result->data.expression.left = left;
    result->data.expression.right = right;
    result->data.expression.op = op;
    yak("Made binary expression node with op %d\n", op);
    return result;
}

AstNode *make_list_access(AstNode *list, AstNode *index)
{
    AstNode *result = create_node(ast_listaccess_t);
    result->data.listaccess.list = list;
    result->data.listaccess.index = index;
    yak("Made list access reference\n");
    return result;
}

AstNode *make_list_assignment(AstNode *name,
        AstNode *index, AstNode *right)
{
    AstNode *result = create_node(ast_listassign_t);
    result->data.listassign.list = name;
    result->data.listassign.index = index;
    result->data.listassign.right = right;
    yak("Made list assignment node\n");
    return result;
}

AstNode *make_list_def(AstNode *result, AstNode *to_append)
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

AstNode *make_assignment(char *id, AstNode *right)
{
    AstNode *result = create_node(ast_assign_t);
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    yak("Made assignment node to\n", id);
    return result;
}

AstNode *make_while_loop(AstNode *cond, AstNode *statements)
{
    AstNode *result = create_node(ast_while_t);
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    yak("Made while node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

AstNode *make_for_loop(char *iter,
        AstNode *list, AstNode *statements)
{
    AstNode *result = create_node(ast_for_t);
    result->data.for_loop.iter = iter;
    result->data.for_loop.list = list;
    result->data.for_loop.statements = statements;
    yak("Made for node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

AstNode *make_if_else(AstNode *cond, AstNode *block1, AstNode *block2)
{
    AstNode *result = create_node(ast_if_t);
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    yak("Made if-else node\n");
    return result;
}

AstNode *make_func_call(AstNode *name, AstNode *arglist)
{
    AstNode *result = create_node(ast_call_t);
    result->data.call.funcname = name;
    result->data.call.arglist = arglist;
    yak("Made call node with name\n");
    return result;
}

AstNode *make_func_def(AstNode *name, AstNode *param_list,
        AstNode *statements)
{
    AstNode *result = create_node(ast_func_t);
    result->data.funcdef.funcname = name;
    result->data.funcdef.param_list = param_list;
    result->data.funcdef.statements = statements;
    yak("Made function definition node with name\n", name);
    return result;
}

AstNode *make_statements(AstNode *list, AstNode *new)
{
    if (!list)
    {
        list = create_node(ast_stmnts_t);
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

AstNode *make_break()
{
    return create_node(ast_break_t);
}

AstNode *make_continue()
{
    return create_node(ast_continue_t);
}

AstNode *make_return(AstNode *expr)
{
    AstNode *ret = create_node(ast_return_t);
    ret->data.return_stmt.expr = expr;
    return ret;
}

AstNode *make_pass()
{
    return create_node(ast_pass_t);
}


int print_ast_graph(AstNode *root, int id)
{
    AstNode *args = NULL;
    int i = 0;
    int rID = id;

    if (!root)  {
        /* could be non-existing 'else' in an 'if' clause, for example */
        return id;
    }

    switch (root->type)
    {
        case ast_stmnts_t:
            printf("%d [label=\"statements\"]\n", rID);
            for (i = 0; i < root->data.statements.count; i++)
            {
                printf("%d -> %d\n", rID, ++id);
                id = print_ast_graph(root->data.statements.statements[i], id);
            }
            break;
        case ast_func_t:
            printf("%d [label=\"func def\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.funcdef.funcname, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.funcdef.param_list, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.funcdef.statements, id);
            break;
        case ast_list_t:
            printf("%d [label=\"list\"]\n", rID);
            for (i = 0; i < root->data.list.count; i++)
            {
                printf("%d -> %d\n", rID, ++id);
                id = print_ast_graph(root->data.list.items[i], id);
            }
            break;
        case ast_while_t:
            printf("%d [label=\"while\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.while_loop.cond, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.while_loop.statements, id);
            break;
        case ast_for_t:
            printf("%d [label=\"for\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.for_loop.list, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.for_loop.statements, id);
            break;
        case ast_if_t:
            printf("%d [label=\"if\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.if_else.cond, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.if_else.ifstatements, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.if_else.elstatements, id);
            break;
        case ast_assign_t:
            printf("%d [label=\"assign\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            printf("%d [label=\"ID: %s\"]\n", id, root->data.assignment.name);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.assignment.right, id);
            break;
        case ast_call_t:
            printf("%d [label=\"func call\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.call.funcname, id);
            args = root->data.call.arglist;
            assert(args->type == ast_list_t);
            for (i = 0; i < args->data.list.count; i++)
            {
                printf("%d -> %d\n", rID, ++id);
                id = print_ast_graph(args->data.list.items[i], id);
            }
            break;
        case ast_listaccess_t:
            printf("%d [label=\"listaccess\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.listaccess.list, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.listaccess.index, id);
            break;
        case ast_listassign_t:
            printf("%d [label=\"listassign\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.listassign.list, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.listassign.index, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.listassign.right, id);
            break;
        case ast_expr_t:
            printf("%d [label=\"expression\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.expression.left, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.expression.right, id);
            break;
        case ast_id_t:
            printf("%d [label=\"ID: %s\"]\n", rID, root->data.id.val);
            break;
        case ast_string_t:
            printf("%d [label=\"string: %s\"]\n", rID,
                    root->data.s);
            break;
        case ast_float_t:
            printf("%d [label=\"float: %g\"]\n", rID,
                    root->data.f);
            break;
        case ast_integer_t:
            printf("%d [label=\"int: %ld\"]\n", rID,
                    root->data.i);
            break;
        default:
            break;
    }

    return id;
}

void destroy_tree(AstNode *root)
{
    /* don't free a NULL statement */
    if (!root) return;

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
            destroy_tree(root->data.funcdef.funcname);
            destroy_tree(root->data.funcdef.param_list);
            destroy_tree(root->data.funcdef.statements);
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
            free(root->data.for_loop.iter);
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
            destroy_tree(root->data.call.funcname);
            break;
        case ast_listaccess_t:
            destroy_tree(root->data.listaccess.list);
            destroy_tree(root->data.listaccess.index);
            break;
        case ast_listassign_t:
            destroy_tree(root->data.listassign.index);
            destroy_tree(root->data.listassign.right);
            destroy_tree(root->data.listassign.list);
            break;
        case ast_expr_t:
            destroy_tree(root->data.expression.left);
            destroy_tree(root->data.expression.right);
            break;
        case ast_id_t:
            free(root->data.id.val);
            break;
        case ast_string_t:
            free(root->data.s);
            break;
        default:
            break;
    }
    /* for all nodes */
    yak("Deleting %s\n", TYPE_NAMES[root->type]);
    free(root);
    return;
}

