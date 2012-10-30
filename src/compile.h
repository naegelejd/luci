#ifndef COMPILE_H
#define COMPILE_H

#include "ast.h"        /* for AstNode */
#include "symbol.h"     /* for Symbol Table */
#include "constant.h"   /* for Constant Table */
#include "common.h"     /* for LuciObject */

typedef enum {
    NOP,
    LOADK,
    LOADS,
    STORE,
    BINOP,
    CALL,
    JUMPL,
    JUMPN,
    EXIT,
} Opcode;

typedef struct {
    Opcode opcode;
    int a;
    int b;
} Instruction;

#define BASE_INSTR_COUNT 256

typedef struct {
    int count;
    int size;
    Instruction **instructions;
    SymbolTable *symtable;
    ConstantTable *cotable;
} Program;


Program * compile_ast(AstNode *);
void destroy_program(Program *);
void print_instructions(Program *prog);

#endif
