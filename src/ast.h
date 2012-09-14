#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef struct ASTNode
{
    enum {  ast_int_t,
	    ast_double_t,
	    ast_str_t,
	    ast_id_t,
	    ast_expression_t,
	    ast_listindex_t,
	    ast_listassign_t,
	    ast_list_t,
	    ast_assignment_t,
	    ast_while_t,
	    ast_for_t,
	    ast_if_t,
	    ast_call_t,
	    ast_statements_t,
	    ast_last_t
    } type;
    union
    {
	int i_val;
	double d_val;
	char *s_val;
	char *name;
	struct
	{
	    struct ASTNode *left, *right;
	    int op;
	} expression;
	struct
	{
	    struct ASTNode *list;
	    struct ASTNode *index;
	} listindex;
	struct
	{
	    char *name;
	    struct ASTNode *index;
	    struct ASTNode *right;
	} listassign;
	struct
	{
	    int count;
	    struct ASTNode **items;
	} list;
	struct
	{
	    struct ASTNode *right;
	    char *name;
	} assignment;
	struct
	{
	    struct ASTNode *cond;
	    struct ASTNode *statements;
	} while_loop;
	struct
	{
	    struct ASTNode *list;
	    struct ASTNode *statements;
	    char *name;
	} for_loop;
	struct
	{
	    struct ASTNode *cond;
	    struct ASTNode *ifstatements;
	    struct ASTNode *elstatements;
	} if_else;
	struct
	{
	    struct ASTNode *param_list;
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
ASTNode *make_expr_from_int(int);
ASTNode *make_expr_from_double(double);
ASTNode *make_expr_from_string(char *);
ASTNode *make_expr_from_id(char *);
ASTNode *make_binary_expr(ASTNode *, ASTNode *, int op);
ASTNode *make_list_index(ASTNode *, ASTNode *);
ASTNode *make_list_assignment(char *, ASTNode *, ASTNode *);
ASTNode *make_list(ASTNode *, ASTNode *);
ASTNode *make_call(char *, ASTNode *);
ASTNode *make_assignment(char *, ASTNode *);
ASTNode *make_while_loop(ASTNode *, ASTNode *);
ASTNode *make_for_loop(char *, ASTNode *, ASTNode *);
ASTNode *make_if_else(ASTNode *, ASTNode *, ASTNode *);
ASTNode *make_statement(ASTNode *, ASTNode *);

#endif
