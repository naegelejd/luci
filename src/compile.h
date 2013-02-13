/*
 * See Copyright Notice in luci.h
 */

#ifndef COMPILE_H
#define COMPILE_H

#include <stdint.h>

#include "ast.h"        /* for AstNode */
#include "symbol.h"     /* for Symbol Table */
#include "constant.h"   /* for Constant Table */

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
    MKMAP,
    MKLIST,
    CGET,
    CPUT,
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


struct loop_jump {
    uint32_t addr;
    struct loop_jump *next;
};

struct loop_list {
    uint8_t loop_type;      /* possible types defined above */
    struct loop_jump *breaks;
    struct loop_jump *continues;
    struct loop_list *parent;
};

typedef struct _program {
    uint32_t ip;    /* (compilation) instr count. (interp) instr pointer */
    uint32_t size;   /* size of array allocated for instructions */
    Instruction *instructions;
    SymbolTable *ltable;
    SymbolTable *gtable;
    ConstantTable *ctable;
    struct loop_list *current_loop;
} Program;

typedef struct _frame {
    uint16_t nparams;
    uint16_t nlocals;
    uint16_t nconstants;
    uint32_t ip;
    uint32_t ninstrs;
    Instruction *instructions;
    LuciObject **locals;
    LuciObject **globals;
    LuciObject **constants;
} Frame;

typedef struct _compile_state {
    uint32_t instr_count;
    uint32_t instr_alloc;
    Instruction *instructions;
    SymbolTable *ltable;
    SymbolTable *gtable;
    ConstantTable *ctable;
    struct loop_list *current_loop;
} CompileState;


CompileState * compile_ast(CompileState *, AstNode *);
CompileState * CompileState_new(void);
CompileState * CompileState_init(CompileState *);
CompileState * CompileState_refresh(CompileState *);
void CompileState_delete(CompileState *);

Frame * Frame_from_CompileState(CompileState *, uint16_t);
void Frame_delete(Frame *);
Frame * Frame_copy(Frame *);
void Frame_delete_copy(Frame *);
void Frame_delete_interactive(Frame *f);

void print_instructions(Frame *);

char * serialize_program(Frame *);

#endif
