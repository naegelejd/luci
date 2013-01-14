/*
 * See Copyright Notice in luci.h
 */

#ifndef AST_H
#define AST_H

/* the initial size of the array which represents a list of expressions */
#define AST_CONTAINER_SIZE 32

/* the initial size of the array which holds pointers to statement nodes */
#define AST_STMNTS_SIZE 32

typedef enum {
    ast_integer_t,
    ast_float_t,
    ast_string_t,
    ast_id_t,
    ast_expr_t,
    ast_contaccess_t,
    ast_contassign_t,
    ast_mapdef_t,
    ast_mapkeyval_t,
    ast_listdef_t,
    ast_assign_t,
    ast_while_t,
    ast_for_t,
    ast_if_t,
    ast_call_t,
    ast_func_t,
    ast_stmnts_t,
    ast_break_t,
    ast_continue_t,
    ast_return_t,
    ast_pass_t,
    ast_last_t
} AstType;


struct AstNode;

typedef struct
{
    char *val;
} AstID;

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
    struct AstNode **pairs;
} AstMapDef;

typedef struct
{
    struct AstNode *key;
    struct AstNode *val;
} AstMapKeyVal;

typedef struct
{
    int count;
    int size;
    struct AstNode **items;
} AstListDef;

typedef struct
{
    struct AstNode *container;
    struct AstNode *index;
} AstContainerAccess;

typedef struct
{
    struct AstNode *container;
    struct AstNode *index;
    struct AstNode *right;
} AstContainerAssign;

typedef struct
{
    struct AstNode *cond;
    struct AstNode *statements;
} AstWhileLoop;

typedef struct
{
    struct AstNode *list;
    struct AstNode *statements;
    char *iter;
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
    struct AstNode *funcname;
} AstFuncCall;

typedef struct
{
    struct AstNode *param_list;
    struct AstNode *statements;
    char *funcname;
} AstFuncDef;

typedef struct
{
    int count;
    int size;
    struct AstNode ** statements;
} AstStatements;

typedef struct
{
    struct AstNode *expr;
} AstReturn;

typedef struct AstNode
{
    AstType type;
    int lineno;
    int column;

    union {
        long i;
        double f;
        char *s;
        AstID id;
        AstExpression expression;
        AstContainerAccess contaccess;
        AstContainerAssign contassign;
        AstMapDef mapdef;
        AstMapKeyVal mapkeyval;
        AstListDef listdef;
        AstAssignment assignment;
        AstWhileLoop while_loop;
        AstForLoop for_loop;
        AstIfElse if_else;
        AstFuncCall call;
        AstFuncDef funcdef;
        AstStatements statements;
        AstReturn return_stmt;
    } data;
} AstNode;

void destroy_tree(AstNode *);

AstNode *make_int_constant(long);
AstNode *make_float_constant(double);
AstNode *make_string_constant(char *);
AstNode *make_id_expr(char *);
AstNode *make_binary_expr(AstNode *, AstNode *, int op);
AstNode *make_container_access(AstNode *, AstNode *);
AstNode *make_container_assignment(AstNode *, AstNode *, AstNode *);
AstNode *make_list_def(AstNode *, AstNode *);
AstNode *make_map_def(AstNode *, AstNode *);
AstNode *make_map_keyval(AstNode *, AstNode *);
AstNode *make_assignment(char *, AstNode *);
AstNode *make_while_loop(AstNode *, AstNode *);
AstNode *make_for_loop(char *, AstNode *, AstNode *);
AstNode *make_if_else(AstNode *, AstNode *, AstNode *);
AstNode *make_func_call(AstNode *, AstNode *);
AstNode *make_func_def(char *, AstNode *, AstNode *);
AstNode *make_statements(AstNode *, AstNode *);
AstNode *make_break();
AstNode *make_continue();
AstNode *make_return(AstNode *);
AstNode *make_pass();

int print_ast_graph(AstNode *root, int);

#endif
