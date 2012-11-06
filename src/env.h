/*
 * See Copyright Notice in luci.h
 */

#ifndef ASTEXEC_H
#define ASTEXEC_H

/* AstNode from ast.h */
struct AstNode;

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
void exec_AST(struct ExecContext* e, struct AstNode *root);

/* creates the execution context */
struct ExecContext* create_context(const char* name, struct ExecContext *parent);

/* initializes context with builtins */
void initialize_context(ExecContext *e);

/* removes the ExecContext */
void destroy_context(struct ExecContext *e);

#endif
