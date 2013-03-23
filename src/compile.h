/*
 * See Copyright Notice in luci.h
 */

/**
 * @file compile.h
 */

#ifndef COMPILE_H
#define COMPILE_H

#include <stdint.h>

#include "ast.h"        /* for AstNode */
#include "symbol.h"     /* for Symbol Table */
#include "constant.h"   /* for Constant Table */

/**
 * Enumerated opcode types
 */
typedef enum {
    NOP,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    EQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,
    LGOR,
    LGAND,
    BWXOR,
    BWOR,
    BWAND,

    NEG,
    LGNOT,
    BWNOT,

    POP,
    PUSHNIL,
    LOADK,
    LOADS,
    LOADG,
    DUP,
    STORE,
    CALL,
    RETURN,
    MKMAP,
    MKLIST,
    CGET,
    CPUT,
    MKITER,
    HALT,
    JUMP,
    POPJUMP,
    JUMPZ,
    ITERJUMP
} Opcode;

/** an instruction is a 32-bit unsigned int */
typedef uint32_t Instruction;

/** how much to right-shift an instruction to obtain its opcode */
#define OPCODE_SHIFT    26
/** when masked with an instruction, gives the instruction's argument */
#define OPARG_MASK      0x1FFFFFF
/** maximum possible (signed) value an opcode argument */
#define OPARG_MAXIMUM   0x3FFFFFF
/** when masked with an instruction, 0 if positive, else negative */
#define OPARG_NEG_BIT   0x2000000
/** extracts the opcode from the given instruction */
#define OPCODE(i)   ((i) >> OPCODE_SHIFT)
/** extracts the argument from the given instruction */
#define OPARG(i)    ((i) & OPARG_NEG_BIT ? -((i) & OPARG_MASK) : ((i) & OPARG_MASK))

/** initial size of instructions array */
#define BASE_INSTR_COUNT    256
/** initial symtable scale (0=smallest) */
#define BASE_SYMTABLE_SCALE 0
/** initial constant table size */
#define BASE_COTABLE_SIZE   0xFF


/**
 * A linked list node used to track nested loops
 */
struct loop_jump {
    uint32_t addr;          /**< the instruction address to jump to */
    struct loop_jump *next; /**< next loop-jump in linked list */
};

/**
 * Tracks all breaks, continues in a loop.
 * Also tracks parent loops.
 */
struct loop_list {
    enum { LOOP_TYPE_WHILE, LOOP_TYPE_FOR } loop_type; /**< loop type */
    struct loop_jump *breaks;       /**< linked list of breaks */
    struct loop_jump *continues;    /**< linked list of continues */
    struct loop_list *parent;       /**< parent loop_list (if nested) */
};

/**
 * Derived from a CompileState.
 *
 * Used in both compilation and mainly interpreting
 */
typedef struct _frame {
    uint16_t nparams;       /**< number of parameters */
    uint16_t nlocals;       /**< number of local symbols */
    uint16_t nconstants;    /**< number of constants */
    uint32_t ninstrs;       /**< total number of instructions */
    Instruction *instructions;  /**< array of instructions */
    Instruction *ip;            /**< current instruction pointer */
    LuciObject **locals;        /**< array of local LuciObjects */
    LuciObject **globals;       /**< array of global LuciObjects */
    LuciObject **constants;     /**< array of constant LuciObjects */
} Frame;

/**
 * Essential state storage entity within compilation.
 *
 * Each CompileState represents either the global program's scope
 * or individual function definition scopes.
 */
typedef struct _compile_state {
    uint32_t instr_count;       /**< instruction count */
    uint32_t instr_alloc;       /**< size of instructions array */
    Instruction *instructions;  /**< array of instructions */
    SymbolTable *ltable;        /**< symbol table for locals */
    SymbolTable *gtable;        /**< symbol table for globals */
    ConstantTable *ctable;      /**< constant table */
    struct loop_list *current_loop; /**< used while compiling loops */
} CompileState;


CompileState * compile_ast(CompileState *, AstNode *);
CompileState * CompileState_new(void);
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
