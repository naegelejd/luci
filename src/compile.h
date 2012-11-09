/*
 * See Copyright Notice in luci.h
 */

#ifndef COMPILE_H
#define COMPILE_H

#include <stdint.h>

#include "ast.h"        /* for AstNode */
#include "symbol.h"     /* for Symbol Table */
#include "constant.h"   /* for Constant Table */
#include "common.h"     /* for LuciObject */

typedef enum {
    NOP,
    POP,
    LOADK,
    LOADS,
    DUP,
    STORE,
    BINOP,
    CALL,
    MKLIST,
    LISTGET,
    LISTPUT,
    HALT,
    /* here begins extended length instructions */
    JUMP,
    JUMPZ,
} Opcode;

typedef uint16_t Instruction;

#define BASE_INSTR_COUNT 256
#define BASE_SYMTABLE_SCALE 0
#define BASE_COTABLE_SIZE 0xFF

typedef struct {
    int count;
    int size;
    Instruction *instructions;
    SymbolTable *symtable;
    ConstantTable *cotable;
} Program;


Program * compile_ast(AstNode *);
Program * program_new(void);
void program_delete(Program *);
void print_instructions(Program *prog);

#endif
