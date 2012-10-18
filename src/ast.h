#ifndef AST_H
#define AST_H

/* the initial size of the array which represents a list of expressions */
#define AST_LIST_SIZE 32

/* the initial size of the array which holds pointers to statement nodes */
#define AST_STMNTS_SIZE 32

typedef enum {
    ast_int_t,
    ast_float_t,
    ast_string_t,
    ast_id_t,
    ast_expr_t,
    ast_listindex_t,
    ast_listassign_t,
    ast_list_t,
    ast_assign_t,
    ast_while_t,
    ast_for_t,
    ast_if_t,
    ast_call_t,
    ast_func_t,
    ast_stmnts_t,
    ast_last_t
} AstType;

/* for verbosity */
const char *NTYPES[15];

struct AstNode;

typedef struct
{
    struct AstNode *left, *right;
    int op;
} AstExpression;

typedef struct
{
    struct AstNode *right;
    char *name;
} AstAssignment;

typedef struct
{
    int count;
    int size;
    struct AstNode **items;
} AstListDef;

typedef struct
{
    struct AstNode *list;
    struct AstNode *index;
} AstListIndex;

typedef struct
{
    char *name;
    struct AstNode *index;
    struct AstNode *right;
} AstListAssign;

typedef struct
{
    struct AstNode *cond;
    struct AstNode *statements;
} AstWhileLoop;

typedef struct
{
    struct AstNode *list;
    struct AstNode *statements;
    char *name;
} AstForLoop;

typedef struct
{
    struct AstNode *cond;
    struct AstNode *ifstatements;
    struct AstNode *elstatements;
} AstIfElse;

typedef struct
{
    struct AstNode *arglist;
    char *name;
} AstFuncCall;

typedef struct
{
    struct AstNode *param_list;
    struct AstNode *statements;
    struct AstNode *ret_expr;
    char *name;
} AstFuncDef;

typedef struct
{
    int count;
    int size;
    struct AstNode ** statements;
} AstStatements;

typedef struct AstNode
{
    AstType type;
    int lineno;
    int column;

    union {
        long i_val;
        double f_val;
        char *s_val;
        char *name;

        AstExpression expression;
        AstListIndex listindex;
        AstListAssign listassign;
        AstListDef list;
        AstAssignment assignment;
        AstWhileLoop while_loop;
        AstForLoop for_loop;
        AstIfElse if_else;
        AstFuncCall call;
        AstFuncDef func_def;
        AstStatements statements;
    } data;
} AstNode;

void destroy_tree(AstNode *);

AstNode *make_int_expr(long);
AstNode *make_float_expr(double);
AstNode *make_string_expr(char *);
AstNode *make_id_expr(char *);
AstNode *make_binary_expr(AstNode *, AstNode *, int op);
AstNode *make_list_index(AstNode *, AstNode *);
AstNode *make_list_assignment(char *, AstNode *, AstNode *);
AstNode *make_list_def(AstNode *, AstNode *);
AstNode *make_assignment(char *, AstNode *);
AstNode *make_while_loop(AstNode *, AstNode *);
AstNode *make_for_loop(char *, AstNode *, AstNode *);
AstNode *make_if_else(AstNode *, AstNode *, AstNode *);
AstNode *make_func_call(char *, AstNode *);
AstNode *make_func_def(char *, AstNode *, AstNode *, AstNode *);
AstNode *make_statements(AstNode *, AstNode *);

int print_ast_graph(AstNode *root, int);

#endif
