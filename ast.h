#ifndef AST_H
#define AST_H

typedef struct ASTNode
{
    enum {t_num, t_id, t_expression, t_assignment, t_call, t_statements, t_last} type;
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

void destroy_AST(ASTNode *);
ASTNode *make_expr_from_num(int);
ASTNode *make_expr_from_id(char *);
ASTNode *make_expression(ASTNode *, ASTNode *, char);
ASTNode *make_call(char *, ASTNode *);
ASTNode *make_assignment(char *, ASTNode *);
ASTNode *make_statement(ASTNode *, ASTNode *);

#endif
