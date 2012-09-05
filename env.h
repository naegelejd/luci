#ifndef ASTEXEC_H
#define ASTEXEC_H

/* ASTNode from ast.h */
struct ASTNode;

/* Symbol from symbol.h */
struct Symbol;
struct luci_obj_t;

typedef struct ExecEnviron
{
    //int variable;  /* This is where the symbol table will live */
    //unary_func func;
    struct Symbol *symtable;
} ExecEnviron;

/* executes an AST */
void exec_AST(struct ExecEnviron* e, struct ASTNode *root);

struct Symbol *add_symbol (ExecEnviron *e, char const *name, int type);
struct Symbol *get_symbol (ExecEnviron *e, const char *name);

/* destroys a luci_obj_t * */
void destroy_object(struct luci_obj_t *trash);

/* creates the execution engine */
struct ExecEnviron* create_env();

/* removes the ExecEnviron */
void destroy_env(struct ExecEnviron *e);

#endif
