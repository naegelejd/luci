#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef struct ASTNode
{
    enum {ast_num_t, ast_id_t, ast_expression_t, ast_assignment_t,
	    ast_call_t, ast_statements_t, ast_last_t} type;
    char *repr;
    union
    {
	int val;
	char *name;
	struct
	{
	    struct ASTNode *left, *right;
	    char op;
	} expression;
	struct
	{
	    struct ASTNode *right;
	    char *name;
	} assignment;
	struct
	{
	    struct ASTNode *param;
	    char *name;
	} call;
	struct
	{
	    int count;
	    struct ASTNode ** statements;
	} statements;
    } data;
} ASTNode;

/* linked list for ASTNodes */
typedef struct ASTNode_list
{
    ASTNode *elem;
    struct ASTNode_list* next;
} ASTNode_list;

void *alloc(size_t size);

void destroy_AST(ASTNode *);
ASTNode *make_expr_from_num(int);
ASTNode *make_expr_from_id(char *);
ASTNode *make_expression(ASTNode *, ASTNode *, char);
ASTNode *make_call(char *, ASTNode *);
ASTNode *make_assignment(char *, ASTNode *);
ASTNode *make_statement(ASTNode *, ASTNode *);

#endif
