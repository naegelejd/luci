#ifndef ASTEXEC_H
#define ASTEXEC_H

/* ASTNode from ast.h */
struct ASTNode;

/* Symbol from symbol.h */
struct Symbol;
struct LuciObject;

typedef struct ExecContext
{
    char* name;
    struct Symbol *symtable;
} ExecContext;

/* executes an AST */
void exec_AST(struct ExecContext* e, struct ASTNode *root);

struct Symbol *add_symbol (ExecContext *e, char const *name, int type);
struct Symbol *get_symbol (ExecContext *e, const char *name);

/* creates the execution engine */
struct ExecContext* create_env();

/* removes the ExecContext */
void destroy_env(struct ExecContext *e);

#endif
