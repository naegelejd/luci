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
    struct ExecContext *parent;
} ExecContext;

/* executes an AST */
void exec_AST(struct ExecContext* e, struct ASTNode *root);

int destroy_symbol(struct Symbol *s, int force);
struct Symbol *add_symbol (ExecContext *e, char const *name, int type);
struct Symbol *get_symbol (ExecContext *e, const char *name);

/* creates the execution context */
struct ExecContext* create_context(const char* name, struct ExecContext *parent);

/* initializes context with builtins */
void initialize_context(ExecContext *e);

/* removes the ExecContext */
void destroy_context(struct ExecContext *e);

#endif
