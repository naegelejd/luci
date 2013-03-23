/*
 * See Copyright Notice in luci.h
 */

/**
 * @file ast.h
 */

#ifndef AST_H
#define AST_H

/** the initial size of the array which represents a list of expressions */
#define AST_CONTAINER_SIZE 32

/** the initial size of the array which holds pointers to statement nodes */
#define AST_STMNTS_SIZE 32

/** enumeration of supported unary/binary operators */
typedef enum {
    /* binary operations */
    op_add_t,
    op_sub_t,
    op_mul_t,
    op_div_t,
    op_mod_t,
    op_pow_t,
    op_eq_t,
    op_neq_t,
    op_lt_t,
    op_gt_t,
    op_lte_t,
    op_gte_t,
    op_lgor_t,
    op_lgand_t,
    op_bwxor_t,
    op_bwor_t,
    op_bwand_t,
    /* unary operations */
    op_neg_t,
    op_lgnot_t,
    op_bwnot_t,
} op_type;

/** type enumeration of all AST nodes */
typedef enum {
    ast_integer_t,
    ast_float_t,
    ast_string_t,
    ast_id_t,
    ast_unexpr_t,
    ast_binexpr_t,
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

/** AST Node representing a variable name. */
typedef struct
{
    char *val;  /**< string value of variable name */
} AstID;

/** AST Node representing a unary expression. */
typedef struct
{
    struct AstNode *right;  /**< right-hand side of expression */
    op_type op;             /**< unary operator type */
} AstUnaryExpression;

/** AST Node representing a binary expression. */
typedef struct
{
    struct AstNode *left;   /**< left-hand side of expression */
    struct AstNode *right;  /**< right-hand side of expression */
    op_type op;             /**< binary operator type */
} AstBinaryExpression;

/** AST Node representing a variable assignment */
typedef struct
{
    struct AstNode *right;  /**< value of the assignment */
    char *name;             /**< name being assigned to */
} AstAssignment;

/** AST Node representing a LuciMapObj definition */
typedef struct
{
    int count;      /**< current # of map key-val pairs */
    int size;       /**< allocated # of map key-val pairs */
    struct AstNode **pairs;     /**< array of map key-val pairs */
} AstMapDef;

/** AST Node representing a LuciMapObj's key-value pair */
typedef struct
{
    struct AstNode *key;    /**< key */
    struct AstNode *val;    /**< value */
} AstMapKeyVal;

/** AST Node representing a LuciListObj definition */
typedef struct
{
    int count;      /**< current # of list items */
    int size;       /**< allocated # of list items */
    struct AstNode **items;     /**< array of list items */
} AstListDef;

/** AST Node representing a container access
 *
 * e.g.
 * l[0] *or* m["hello"]
 */
typedef struct
{
    struct AstNode *container;  /**< container */
    struct AstNode *index;      /**< index/key into container */
} AstContainerAccess;

/** AST Node representing a container assignment */
typedef struct
{
    struct AstNode *container;  /**< container */
    struct AstNode *index;      /**< index/key into container */
    struct AstNode *right;      /**< right-hand side of assignment */
} AstContainerAssign;

/** AST Node representing a while loop */
typedef struct
{
    struct AstNode *cond;       /**< while-loop test condition */
    struct AstNode *statements; /**< while-loop body statements */
} AstWhileLoop;

/** AST Node representing a for loop */
typedef struct
{
    struct AstNode *container;       /**< container to iterate over */
    struct AstNode *statements; /**< for-loop body statements */
    char *iter;                 /**< name of the step variable */
} AstForLoop;

/** AST Node representing an if-else block */
typedef struct
{
    struct AstNode *cond;       /**< if-else test condition */
    struct AstNode *ifstatements;   /**< if body statements */
    struct AstNode *elstatements;   /**< else body statements */
} AstIfElse;

/** AST Node representing a function call */
typedef struct
{
    struct AstNode *arglist;    /**< list of arguments */
    struct AstNode *funcname;   /**< function name */
} AstFuncCall;

/** AST Node representing a function definition */
typedef struct
{
    struct AstNode *param_list; /**< list of parameters */
    struct AstNode *statements; /**< function body statements */
    char *funcname;             /**< function name */
} AstFuncDef;

/** AST Node representing a block of statements */
typedef struct
{
    int count;          /**< current # of statements */
    int size;           /**< allocated # of statements */
    struct AstNode ** statements;   /**< array of statements */
} AstStatements;

/** AST Node representing the `return` keyword */
typedef struct
{
    struct AstNode *expr;   /**< what to return */
} AstReturn;

/**
 * A node in an Abstract Syntax Tree. Upon parsing a
 * Luci program, all syntactical entities in the code
 * are construct as AST nodes.
 */
typedef struct AstNode
{
    AstType type;   /**< the type of the node */
    int lineno;     /**< the line # this node was constructed from */
    int column;     /**< the column # this node was constructed from */

    union {
        long i;     /**< integer constant */
        double f;   /**< floating-point constant */
        char *s;    /**< string constant */
        AstID id;   /**< ID node */
        AstUnaryExpression unexpr;      /**< unary expression node */
        AstBinaryExpression binexpr;    /**< binary expression node */
        AstContainerAccess contaccess;  /**< container access node */
        AstContainerAssign contassign;  /**< container assignment node */
        AstMapDef mapdef;               /**< map definition node */
        AstMapKeyVal mapkeyval;         /**< map key-value pair node */
        AstListDef listdef;             /**< list definition node */
        AstAssignment assignment;       /**< assignment node */
        AstWhileLoop while_loop;        /**< while-loop node */
        AstForLoop for_loop;            /**< for-loop node */
        AstIfElse if_else;              /**< if-else block node */
        AstFuncCall call;               /**< function call node */
        AstFuncDef funcdef;             /**< function definition node */
        AstStatements statements;       /**< statements block node */
        AstReturn return_stmt;          /**< return statement node */
    } data;     /**< this node's payload */
} AstNode;

void destroy_tree(AstNode *);

AstNode *make_int_constant(long);
AstNode *make_float_constant(double);
AstNode *make_string_constant(char *);
AstNode *make_id_expr(char *);
AstNode *make_unary_expr(AstNode *, op_type op);
AstNode *make_binary_expr(AstNode *, AstNode *, op_type op);
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
