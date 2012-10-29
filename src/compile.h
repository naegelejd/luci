#ifndef COMPILE_H
#define COMPILE_H

#include "ast.h"        /* for AstNode */
#include "symbol.h"     /* for Symbol Table */
#include "common.h"     /* for LuciObject */

typedef enum {
    CREATE,
    INCREF,
    DECREF,
    BINOP,
    LOAD,
    STORE,
    CALL,
    JUMPL,
    JUMPN,
    TEST,
    BUILD_LIST,
    EXIT,
} Opcode;


typedef enum {
    imm_long_t,
    imm_double_t,
    imm_string_t
} ImmediateType;

typedef struct {
    ImmediateType t;
    union {
        long l;
        double d;
        char *s;
    } v;
} Immediate;

typedef struct {
    Opcode opcode;
    Immediate *a;
    Immediate *b;
    Immediate *c;
} Instruction;

#define BASE_INSTR_COUNT 256

typedef struct {
    int count;
    int size;
    Instruction **instructions;
    SymbolTable *symtable;
    /* ConstantTable *constants; */
} Program;


Program * compile_ast(AstNode *);
void eval(Program *);
void destroy_program(Program *);
void print_instructions(Program *prog);

#endif
