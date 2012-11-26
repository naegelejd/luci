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
    PUSHNULL,
    LOADK,
    LOADS,
    LOADG,
    DUP,
    STORE,
    BINOP,
    CALL,
    RETURN,
    MKLIST,
    LISTGET,
    LISTPUT,
    MKITER,
    HALT,
    /* here begins extended length instructions */
    JUMP,
    POPJUMP,
    JUMPZ,
    ITERJUMP
} Opcode;

typedef uint16_t Instruction;

#define BASE_INSTR_COUNT 256
#define BASE_SYMTABLE_SCALE 0
#define BASE_COTABLE_SIZE 0xFF

#define LOOP_TYPE_WHILE 0
#define LOOP_TYPE_FOR   1


struct _loop_jump {
    uint32_t addr;
    struct _loop_jump *next;
};

struct _loop_list {
    uint8_t loop_type;      /* possible types defined above */
    struct _loop_jump *breaks;
    struct _loop_jump *continues;
    struct _loop_list *parent;
};

typedef struct _program {
    uint32_t ip;    /* (compilation) instr count. (interp) instr pointer */
    uint32_t size;   /* size of array allocated for instructions */
    Instruction *instructions;
    SymbolTable *locals;
    SymbolTable *globals;
    ConstantTable *cotable;
    struct _loop_list *current_loop;
} Program;


Program * compile_ast(AstNode *);
Program * program_new(void);
Program * program_init(Program *);
void program_delete(Program *);
void print_instructions(Program *prog);

#endif
