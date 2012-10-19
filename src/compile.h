#ifndef COMPILE_H
#define COMPILE_H

#include "ast.h"        /* for AstNode */
#include "common.h"     /* for LuciObject */

typedef enum {
    PUSH,
    POP,
    CREATE,
    INCREF,
    DECREF,
    BINOP,
    LOOKUP,
    STORE,
    LABEL,
    JUMPL,
    JUMPN,
    TEST
} Opcode;

typedef struct {
    Opcode opcode;
    LuciObject *a;
    LuciObject *b;
    LuciObject *c;
} Instruction;

#define BASE_INSTR_COUNT 256

typedef struct {
    int count;
    int size;
    Instruction **instructions;
} Program;


void compile_ast(AstNode *);
void eval(Program *);

#endif
