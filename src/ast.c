#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "types.h"
#include "ast.h"

extern int VERBOSE;

/* for verbosity */
const char *NTYPES[] = {"int", "float", "string", "id", "expr",
	"list_index", "list_assign", "list", "assignment",
	"while", "for", "if_else", "call", "func_def", "statements"};

static ASTNode *create_node(int type) {
    ASTNode *new = alloc(sizeof(*new));
    new->type = type;
    return new;
}

ASTNode *make_expr_from_int(int val)
{
    ASTNode *result = create_node(ast_int_t);
    result->data.i_val = val;
    yak("Made expression node from val %d\n", val);
    return result;
}

ASTNode *make_expr_from_float(double val)
{
    ASTNode *result = create_node(ast_float_t);
    result->data.f_val = val;
    yak("Made expression node from val %f\n", val);
    return result;
}

ASTNode *make_expr_from_string(char *val)
{
    ASTNode *result = create_node(ast_str_t);
    result->data.s_val = val;
    yak("Made expression node from string %s\n", val);
    return result;
}

ASTNode *make_expr_from_id(char *name)
{
    ASTNode *result = create_node(ast_id_t);
    result->data.name = name;
    yak("Made expression node from id %s\n", name);
    return result;
}

ASTNode *make_binary_expr(ASTNode *left, ASTNode *right, int op)
{
    ASTNode *result = create_node(ast_expression_t);
    result->data.expression.left = left;
    result->data.expression.right = right;
    result->data.expression.op = op;
    yak("Made binary expression node with op %d\n", op);
    return result;
}

ASTNode *make_list_index(ASTNode *list, ASTNode *index)
{
    ASTNode *result = create_node(ast_listindex_t);
    result->data.listindex.list = list;
    result->data.listindex.index = index;
    yak("Made list index reference\n");
    return result;
}

ASTNode *make_list_assignment(char *id, ASTNode *index, ASTNode *right)
{
    ASTNode *result = create_node(ast_listassign_t);
    result->data.listassign.name = id;
    result->data.listassign.index = index;
    result->data.listassign.right = right;
    yak("Made list assignment node to id: %s\n", id);
    return result;
}

ASTNode *make_list(ASTNode *result, ASTNode *to_append)
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

ASTNode *make_call(char *id, ASTNode *param_list)
{
    ASTNode *result = create_node(ast_call_t);
    result->data.call.name = id;
    result->data.call.param_list = param_list;
    yak("Made call node with name: %s\n", id);
    return result;
}

ASTNode *make_func_def(ASTNode *call_sig, ASTNode *statements)
{
    ASTNode *result = create_node(ast_funcdef_t);
    result->data.func_def.call_sig = call_sig;
    result->data.func_def.statements = statements;
    yak("Made function definition node with name: %s\n", call_sig->data.call.name);
    return result;
}

ASTNode *make_assignment(char *id, ASTNode *right)
{
    ASTNode *result = create_node(ast_assignment_t);
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    yak("Made assignment node to id: %s\n", id);
    return result;
}

ASTNode *make_while_loop(ASTNode *cond, ASTNode *statements)
{
    ASTNode *result = create_node(ast_while_t);
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    yak("Made while node containing %d stmts\n",
	    statements->data.statements.count);
    return result;
}

ASTNode *make_for_loop(char *id, ASTNode *list, ASTNode *statements)
{
    ASTNode *result = create_node(ast_for_t);
    result->data.for_loop.name = id;
    result->data.for_loop.list = list;
    result->data.for_loop.statements = statements;
    yak("Made for node containing %d stmts\n",
	    statements->data.statements.count);
    return result;
}

ASTNode *make_if_else(ASTNode *cond, ASTNode *block1, ASTNode *block2)
{
    ASTNode *result = create_node(ast_if_t);
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    yak("Made if-else node\n");
    return result;
}

ASTNode *make_statement(ASTNode *result, ASTNode *to_append)
{
    if (!result)
    {
	result = create_node(ast_statements_t);
	result->data.statements.count = 0;
	result->data.statements.size = AST_STMNTS_SIZE;
	result->data.statements.statements = alloc(result->data.statements.size *
		sizeof(*result->data.statements.statements));
    }

    /* realloc the array of statement pointers if necessary */
    if(++(result->data.statements.count) > result->data.statements.size) {
	result->data.statements.size = result->data.statements.size << 1;
	result->data.statements.statements = realloc(result->data.statements.statements,
	    result->data.statements.size * sizeof(*result->data.statements.statements));
    }
    result->data.statements.statements[result->data.statements.count - 1] = to_append;
    yak("Added a new statement\n");
    return result;
}

void destroy_AST(ASTNode *root)
{
    /* don't free a NULL statement */
    if (!root)
	return;

    int i;
    switch (root->type)
    {
	case ast_statements_t:
	    for (i = 0; i < root->data.statements.count; i++)
	    {
		destroy_AST(root->data.statements.statements[i]);
	    }
	    free(root->data.statements.statements);
	    break;
	case ast_list_t:
	    for (i = 0; i < root->data.list.count; i++)
	    {
		destroy_AST(root->data.list.items[i]);
	    }
	    free(root->data.list.items);
	    break;
	case ast_while_t:
	    destroy_AST(root->data.while_loop.cond);
	    destroy_AST(root->data.while_loop.statements);
	    break;
	case ast_for_t:
	    destroy_AST(root->data.for_loop.list);
	    destroy_AST(root->data.for_loop.statements);
	    free(root->data.for_loop.name);
	    break;
	case ast_if_t:
	    destroy_AST(root->data.if_else.cond);
	    destroy_AST(root->data.if_else.ifstatements);
	    destroy_AST(root->data.if_else.elstatements);
	    break;
	case ast_assignment_t:
	    destroy_AST(root->data.assignment.right);
	    free(root->data.assignment.name);
	    break;
	case ast_call_t:
	    destroy_AST(root->data.call.param_list);
	    free(root->data.call.name);
	    break;
	case ast_listindex_t:
	    destroy_AST(root->data.listindex.list);
	    destroy_AST(root->data.listindex.index);
	    break;
	case ast_listassign_t:
	    destroy_AST(root->data.listassign.index);
	    destroy_AST(root->data.listassign.right);
	    free(root->data.listassign.name);
	    break;
	case ast_expression_t:
	    destroy_AST(root->data.expression.left);
	    destroy_AST(root->data.expression.right);
	    break;
	case ast_id_t:
	    free(root->data.name);
	    break;
	case ast_str_t:
	    free(root->data.s_val);
	    break;
	default:
	    break;
    }
    /* for all nodes */
    yak("Deleting %s\n", NTYPES[root->type]);
    free(root);
    return;
}
