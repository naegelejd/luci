#include "types.h"
#include "driver.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

extern int VERBOSE;

const char *NTYPES[] = {"INT", "DOUBLE", "STRING", "ID", "EXPR", "ASSGNMT",
	"WHILE", "CALL", "STMT"};

void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result)
    {
	die("alloc failed");
    }
    return result;
}

ASTNode *make_expr_from_int(int val)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_int_t;
    result->data.i_val = val;
    if (VERBOSE)
	printf("Made expression node from val %d\n", val);
    return result;
}

ASTNode *make_expr_from_double(double val)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_double_t;
    result->data.d_val = val;
    if (VERBOSE)
	printf("Made expression node from val %f\n", val);
    return result;
}

ASTNode *make_expr_from_string(char *val)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_str_t;
    result->data.s_val = val;
    if (VERBOSE)
	printf("Made expression node from string %s\n", val);
    return result;
}

ASTNode *make_expr_from_id(char *name)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_id_t;
    result->data.name = name;
    if (VERBOSE)
	printf("Made expression node from id %s\n", name);
    return result;
}

ASTNode *make_binary_expr(ASTNode *left, ASTNode *right, int op)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_expression_t;
    result->data.expression.left = left;
    result->data.expression.right = right;
    result->data.expression.op = op;
    if (VERBOSE)
	printf("Made binary expression node with op %c\n", op);
    return result;
}

ASTNode *make_params(ASTNode *result, ASTNode *to_append)
{
    if (!result)
    {
	result = alloc(sizeof(*result));
	result->type = ast_parameters_t;
	result->data.parameters.count = 0;
	result->data.parameters.parameters = 0;
    }
    assert(result->type == ast_parameters_t);
    result->data.parameters.count++;
    result->data.parameters.parameters = realloc(result->data.parameters.parameters,
	    result->data.parameters.count * sizeof(*result->data.parameters.parameters));
    result->data.parameters.parameters[result->data.parameters.count - 1] = to_append;
    if (VERBOSE)
	printf("Added a new parameter\n");
    return result;
}

ASTNode *make_call(char *id, ASTNode *parameters)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_call_t;
    result->data.call.name = id;
    result->data.call.parameters = parameters;
    if (VERBOSE)
	printf("Made call node with name: %s\n", id);
    return result;
}

ASTNode *make_assignment(char *id, ASTNode *right)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_assignment_t;
    result->data.assignment.name = id;
    result->data.assignment.right = right;
    if (VERBOSE)
	printf("Made assignment node to id: %s\n", id);
    return result;
}

ASTNode *make_while(ASTNode *cond, ASTNode *statements)
{
    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_while_t;
    result->data.while_loop.cond = cond;
    result->data.while_loop.statements = statements;
    if (VERBOSE) {
	printf("Made while node containing %d stmts\n",
		statements->data.statements.count);
    }
    return result;
}

ASTNode *make_if_else(ASTNode *cond, ASTNode *block1, ASTNode *block2)
{
    assert(cond->type == ast_expression_t);
    assert(block1->type == ast_statements_t);
    /* Bad assertion, block2 could be NULL */
    /* assert(block2->type == ast_statements_t); */

    ASTNode *result = alloc(sizeof(*result));
    result->type = ast_if_t;
    result->data.if_else.cond = cond;
    result->data.if_else.ifstatements = block1;
    result->data.if_else.elstatements = block2;

    if (VERBOSE)
	printf("Made if-else node\n");
    return result;
}

ASTNode *make_statement(ASTNode *result, ASTNode *to_append)
{
    if (!result)
    {
	result = alloc(sizeof(*result));
	result->type = ast_statements_t;
	result->data.statements.count = 0;
	result->data.statements.statements = 0;
    }
    assert(result->type == ast_statements_t);
    result->data.statements.count++;
    result->data.statements.statements = realloc(result->data.statements.statements,
	    result->data.statements.count * sizeof(*result->data.statements.statements));
    result->data.statements.statements[result->data.statements.count - 1] = to_append;
    if (VERBOSE)
	printf("Added a new statement\n");
    return result;
}

void destroy_AST(ASTNode *root)
{
    /* don't free a NULL statement */
    if (!root)
	return;
    if (root->type == ast_statements_t)
    {
	int i;
	for (i = 0; i < root->data.statements.count; i++)
	{
	    destroy_AST(root->data.statements.statements[i]);
	}
	free(root->data.statements.statements);
    }
    else if (root->type == ast_while_t)
    {
	destroy_AST(root->data.while_loop.cond);
	destroy_AST(root->data.while_loop.statements);
    }
    else if (root->type == ast_if_t)
    {
	destroy_AST(root->data.if_else.cond);
	destroy_AST(root->data.if_else.ifstatements);
	destroy_AST(root->data.if_else.elstatements);
    }
    else if (root->type == ast_assignment_t)
    {
	destroy_AST(root->data.assignment.right);
	free(root->data.assignment.name);
    }
    else if (root->type == ast_call_t)
    {
	destroy_AST(root->data.call.parameters);
	free(root->data.call.name);
    }
    else if (root->type == ast_expression_t)
    {
	destroy_AST(root->data.expression.left);
	destroy_AST(root->data.expression.right);
    }
    else if (root->type == ast_id_t)
    {
	free(root->data.name);
    }
    else if (root->type == ast_str_t)
    {
	free(root->data.s_val);
    }
    /* for all nodes */
    if (VERBOSE)
	printf("Deleting %s\n", NTYPES[root->type]);
    free(root);
    return;
}
