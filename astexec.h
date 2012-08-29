#ifndef ASTEXEC_H
#define ASTEXEC_H

struct ASTNode;
struct Symbol;
struct ExecEnviron;

/* executes an AST */
void exec_AST(struct ExecEnviron* e, struct ASTNode *root);

struct Symbol *get_symbol(struct ExecEnviron *e, const char *name);
struct Symbol *put_symbol(struct ExecEnviron *e, const char *name, int type);

/* creates the execution engine */
struct ExecEnviron* create_env();

/* removes the ExecEnviron */
void destroy_env(struct ExecEnviron *e);

#endif
