#ifndef ASTEXEC_H
#define ASTEXEC_H

struct ASTNode;
struct ExecEnviron;

/* creates the execution engine */
struct ExecEnviron* create_env();

/* removes the ExecEnviron */
void free_env(struct ExecEnviron *e);

/* executes an AST */
void exec_AST(struct ExecEnviron* e, struct ASTNode *root);

#endif
