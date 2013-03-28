/*
 * See Copyright Notice in luci.h
 */

/**
 * @file ast.c
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "luci.h"
#include "ast.h"

/** defined in lexer.l */
extern int get_line_num();
/** defined in lexer.l */
extern int get_last_col_num();

/**
 * String representations of each abstract
 * syntax tree node.
 */
const char *TYPE_NAMES[] = {
    "int",
    "float",
    "string",
    "id",
    "expr",
    "container access",
    "container assignment",
    "map definition",
    "map key-value pair",
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

/**
 * Allocates and AST Node and sets it's type
 * as well as the line,column numbers of the source
 * file from which the node came from.
 *
 * @param type the type of AST Node
 * @returns new AST Node
 */
static AstNode *create_node(AstType type);
static AstNode *create_node(AstType type) {
    AstNode *new = alloc(sizeof(*new));
    new->type = type;
    new->lineno = get_line_num();
    new->column = get_last_col_num();
    return new;
}

/**
 * Creates a new AST Node representing a @code nil @endcode expression.
 *
 * @returns new AST Node
 */
AstNode *make_nil_expression()
{
    return create_node(ast_nil_t);
}

/**
 * Creates a new AST Node representing an integer constant.
 *
 * @param val a long integer value
 * @returns new AST Node
 */
AstNode *make_int_constant(long val)
{
    AstNode *result = create_node(ast_integer_t);
    result->data.i = val;
    LUCI_DEBUG("Made expression node from val %ld\n", val);
    return result;
}

/**
 * Creates a new AST Node representing a floating-point constant.
 *
 * @param val a double floating-point value
 * @returns new AST Node
 */
AstNode *make_float_constant(double val)
{
    AstNode *result = create_node(ast_float_t);
    result->data.f = val;
    LUCI_DEBUG("Made expression node from val %f\n", val);
    return result;
}

/**
 * Creates a new AST Node representing a string of characters.
 *
 * @param val an allocated C-string
 * @returns new AST Node
 */
AstNode *make_string_constant(char *val)
{
    AstNode *result = create_node(ast_string_t);
    result->data.s = val;
    LUCI_DEBUG("Made expression node from string %s\n", val);
    return result;
}

/**
 * Creates a new AST Node representing a symbol.
 *
 * @param name C-string name of the symbol
 * @returns new AST Node
 */
AstNode *make_id_expr(char *name)
{
    AstNode *result = create_node(ast_id_t);
    result->data.id.val = name;
    LUCI_DEBUG("Made expression node from id %s\n", name);
    return result;
}

/**
 * Creates a new AST Node representing a unary expression
 *
 * @param right the right-hand operand
 * @param op operation type
 * @returns new AST Node
 */
AstNode *make_unary_expr(AstNode *right, op_type op)
{
    AstNode *result = create_node(ast_unexpr_t);
    result->data.unexpr.right = right;
    result->data.unexpr.op = op;
    LUCI_DEBUG("Made unary expression node with op %d\n", op);
    return result;
}

/**
 * Creates a new AST Node representing a binary expression
 *
 * @param left the left-hand operand
 * @param right the right-hand operand
 * @param op operation type
 * @returns new AST Node
 */
AstNode *make_binary_expr(AstNode *left,
        AstNode *right, op_type op)
{
    AstNode *result = create_node(ast_binexpr_t);
    result->data.binexpr.left = left;
    result->data.binexpr.right = right;
    result->data.binexpr.op = op;
    LUCI_DEBUG("Made binary expression node with op %d\n", op);
    return result;
}

/**
 * Creates a new AST Node representing a container access.
 *
 * @param container container to index into.
 * @param index where in the container to pull a value from
 * @returns new AST Node
 */
AstNode *make_container_access(AstNode *container, AstNode *index)
{
    AstNode *result = create_node(ast_contaccess_t);
    result->data.contaccess.container = container;
    result->data.contaccess.index = index;
    LUCI_DEBUG("%s\n", "Made continer access reference");
    return result;
}

/**
 * Creates a new AST Node representing a container assignment.
 *
 * @param name container (symbol) name
 * @param index where in the container to store the value
 * @param right expression to evaluate and store
 * @returns new AST Node
 */
AstNode *make_container_assignment(AstNode *name,
        AstNode *index, AstNode *right)
{
    AstNode *result = create_node(ast_contassign_t);
    result->data.contassign.container = name;
    result->data.contassign.index = index;
    result->data.contassign.right = right;
    LUCI_DEBUG("%s\n", "Made container assignment node");
    return result;
}

/**
 * Creates a new AST Node representing a map definition.
 *
 * @param result existing or empty map definition node.
 * @param to_append key-value pair node to append to the map definition
 * @returns new AST Node
 */
AstNode *make_map_def(AstNode *result, AstNode *to_append)
{
    if (!result) {
        result = create_node(ast_mapdef_t);
        result->data.mapdef.size = AST_CONTAINER_SIZE;
        result->data.mapdef.pairs = alloc(result->data.mapdef.size *
                sizeof(*result->data.mapdef.pairs));
        result->data.mapdef.count = 0;
    }
    /* checking for a NULL 'to_append' allows for empty list creation */
    if (to_append) {
        assert(result->type == ast_mapdef_t);
        if (++(result->data.mapdef.count) > result->data.mapdef.size) {
            result->data.mapdef.size = result->data.mapdef.size << 1;
            result->data.mapdef.pairs = realloc(result->data.mapdef.pairs,
                result->data.mapdef.size * sizeof(*result->data.mapdef.pairs));
        }
        result->data.mapdef.pairs[result->data.mapdef.count - 1] = to_append;
    }
    return result;
}

/**
 * Creates a new AST Node representing a key-value pair in a map.
 *
 * @param key an expression to use as the key
 * @param val an expression to use as the value
 * @returns new AST Node
 */
AstNode *make_map_keyval(AstNode *key, AstNode *val)
{
    AstNode *result = create_node(ast_mapkeyval_t);
    result->data.mapkeyval.key = key;
    result->data.mapkeyval.val = val;
    return result;
}

/**
 * Creates a new AST Node representing a list definition.
 *
 * @param result existing or empty list definition node
 * @param to_append new expression to append to the list definition
 * @returns new AST Node
 */
AstNode *make_list_def(AstNode *result, AstNode *to_append)
{
    if (!result) {
        result = create_node(ast_listdef_t);
        result->data.listdef.size = AST_CONTAINER_SIZE;
        result->data.listdef.items = alloc(result->data.listdef.size *
                sizeof(*result->data.listdef.items));
        result->data.listdef.count = 0;
    }
    /* checking for a NULL 'to_append' allows for empty list creation */
    if (to_append) {
        assert(result->type == ast_listdef_t);
        if (++(result->data.listdef.count) > result->data.listdef.size) {
            result->data.listdef.size = result->data.listdef.size << 1;
            result->data.listdef.items = realloc(result->data.listdef.items,
                result->data.listdef.size * sizeof(*result->data.listdef.items));
        }
        result->data.listdef.items[result->data.listdef.count - 1] = to_append;
    }
    return result;
}

/**
 * Creates a new AST Node representing an assignment
 *
 * @param id symbol name in which to store the assignment
 * @param right expression to evaluate and store
 * @returns new AST Node
 */
AstNode *make_assignment(char *id, AstNode *right)
{
    AstNode *result = create_node(ast_assign_t);
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    LUCI_DEBUG("Made assignment node to %s\n", id);
    return result;
}

/**
 * Creates a new AST Node representing a while-loop
 *
 * @param cond condition to evaluate
 * @param statements body of while-loop
 * @returns new AST Node
 */
AstNode *make_while_loop(AstNode *cond, AstNode *statements)
{
    AstNode *result = create_node(ast_while_t);
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    LUCI_DEBUG("Made while node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

/**
 * Creates a new AST Node representing a for-loop.
 *
 * @param iter symbol name in which to store the value of each iteration
 * @param container container to iterate over
 * @param statements body of the for-loop
 * @returns new AST Node
 */
AstNode *make_for_loop(char *iter,
        AstNode *container, AstNode *statements)
{
    AstNode *result = create_node(ast_for_t);
    result->data.for_loop.iter = iter;
    result->data.for_loop.container = container;
    result->data.for_loop.statements = statements;
    LUCI_DEBUG("Made for node containing %d stmts\n",
            statements->data.statements.count);
    return result;
}

/**
 * Creates a new AST Node representing an if-else block.
 *
 * @param cond condition to evaluate
 * @param block1 statements to evaluate if cond is true
 * @param block2 statements to evaluate if cond is false
 * @returns new AST Node
 */
AstNode *make_if_else(AstNode *cond, AstNode *block1, AstNode *block2)
{
    AstNode *result = create_node(ast_if_t);
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    LUCI_DEBUG("%s\n", "Made if-else node");
    return result;
}

/**
 * Creates a new AST Node representing a function call.
 *
 * @param name name of the funcion being called
 * @param arglist list of arguments passed to the function
 * @returns new AST Node
 */
AstNode *make_func_call(AstNode *name, AstNode *arglist)
{
    AstNode *result = create_node(ast_call_t);
    result->data.call.funcname = name;
    result->data.call.arglist = arglist;
    LUCI_DEBUG("%s\n", "Made call node with name");
    return result;
}

/**
 * Creates a new AST Node representing a function definition.
 *
 * @param name name of the function being defined
 * @param param_list list of parameters of the function
 * @param statements statements comprising the function's body
 * @returns new AST Node
 */
AstNode *make_func_def(char *name, AstNode *param_list,
        AstNode *statements)
{
    AstNode *result = create_node(ast_func_t);
    result->data.funcdef.funcname = name;
    result->data.funcdef.param_list = param_list;
    result->data.funcdef.statements = statements;
    LUCI_DEBUG("Made function definition node with name %s\n", name);
    return result;
}

/**
 * Creates a new AST Node representing a block of statements.
 *
 * @param list empty or existing list of statements to append to.
 * @param new new statement to add to the list of statements.
 * @returns new AST Node
 */
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
        LUCI_DEBUG("%s\n", "Added a new statement");
    }
    return list;
}

/**
 * Creates a new AST Node representing a @code break @endcode keyword.
 *
 * @returns new AST Node
 */
AstNode *make_break()
{
    return create_node(ast_break_t);
}

/**
 * Creates a new AST Node representing a @code continue @endcode keyword.
 *
 * @returns new AST Node
 */
AstNode *make_continue()
{
    return create_node(ast_continue_t);
}

/**
 * Creates a new AST Node representing a @code return @endcode keyword.
 *
 * @param expr expression to evaluate and return (may be NULL).
 * @returns new AST Node
 */
AstNode *make_return(AstNode *expr)
{
    AstNode *ret = create_node(ast_return_t);
    ret->data.return_stmt.expr = expr;
    return ret;
}

/**
 * Creates a new AST Node representing a @code pass @endcode keyword.
 *
 * @returns new AST Node
 */
AstNode *make_pass()
{
    return create_node(ast_pass_t);
}

/**
 * Prints a Graphviz "dot" graph representing the abstract syntax
 * tree to @code stdout @endcode.
 *
 * @param root root-level node (usually an AstStatements node)
 * @param id used recursively to track relationships between nodes.
 * @returns an id used recursively to track relationships between nodes.
 */
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
            printf("%d [label=\"func def: %s\"]\n", rID,
                    root->data.funcdef.funcname);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.funcdef.param_list, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.funcdef.statements, id);
            break;
        case ast_listdef_t:
            printf("%d [label=\"list\"]\n", rID);
            for (i = 0; i < root->data.listdef.count; i++)
            {
                printf("%d -> %d\n", rID, ++id);
                id = print_ast_graph(root->data.listdef.items[i], id);
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
            id = print_ast_graph(root->data.for_loop.container, id);
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
            assert(args->type == ast_listdef_t);
            for (i = 0; i < args->data.listdef.count; i++)
            {
                printf("%d -> %d\n", rID, ++id);
                id = print_ast_graph(args->data.listdef.items[i], id);
            }
            break;
        case ast_contaccess_t:
            printf("%d [label=\"contaccess\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.contaccess.container, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.contaccess.index, id);
            break;
        case ast_contassign_t:
            printf("%d [label=\"contassign\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.contassign.container, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.contassign.index, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.contassign.right, id);
            break;
        case ast_unexpr_t:
            printf("%d [label=\"unexpr\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.unexpr.right, id);
            break;
        case ast_binexpr_t:
            printf("%d [label=\"binexpr\"]\n", rID);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.binexpr.left, id);
            printf("%d -> %d\n", rID, ++id);
            id = print_ast_graph(root->data.binexpr.right, id);
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

/**
 * Deallocates all nodes in a tree starting at the root.
 *
 * @param root root-level node (usually an AstStatements node)
 */
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
            free(root->data.funcdef.funcname);
            destroy_tree(root->data.funcdef.param_list);
            destroy_tree(root->data.funcdef.statements);
            break;

        case ast_listdef_t:
            for (i = 0; i < root->data.listdef.count; i++)
            {
                destroy_tree(root->data.listdef.items[i]);
            }
            free(root->data.listdef.items);
            break;

        case ast_mapdef_t:
            for (i = 0; i < root->data.mapdef.count; i++)
            {
                destroy_tree(root->data.mapdef.pairs[i]);
            }
            free(root->data.mapdef.pairs);
            break;

        case ast_mapkeyval_t:
            destroy_tree(root->data.mapkeyval.key);
            destroy_tree(root->data.mapkeyval.val);
            break;

        case ast_while_t:
            destroy_tree(root->data.while_loop.cond);
            destroy_tree(root->data.while_loop.statements);
            break;

        case ast_for_t:
            destroy_tree(root->data.for_loop.container);
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

        case ast_contaccess_t:
            destroy_tree(root->data.contaccess.container);
            destroy_tree(root->data.contaccess.index);
            break;

        case ast_contassign_t:
            destroy_tree(root->data.contassign.index);
            destroy_tree(root->data.contassign.right);
            destroy_tree(root->data.contassign.container);
            break;

        case ast_unexpr_t:
            destroy_tree(root->data.unexpr.right);
            break;

        case ast_binexpr_t:
            destroy_tree(root->data.binexpr.left);
            destroy_tree(root->data.binexpr.right);
            break;

        case ast_id_t:
            free(root->data.id.val);
            break;

        case ast_string_t:
            free(root->data.s);
            break;

        case ast_return_t:
            destroy_tree(root->data.return_stmt.expr);
            break;

        default:
            break;
    }
    /* for all nodes */
    LUCI_DEBUG("Deleting %s\n", TYPE_NAMES[root->type]);
    free(root);
    return;
}

