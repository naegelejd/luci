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
    LOADB,
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

/** an instruction is a 32-bit unsigned int */
typedef uint32_t Instruction;

/** a dynamic array of instructions */
typedef struct instruction_array_ {
    unsigned int count;
    unsigned int size;
    Instruction *instructions;
} InstructionArray;

/** global symbol table for builtin functions */
extern SymbolTable *builtin_symbols;
/** global builtins array (from final builtins symbol table) */
extern LuciObject **builtins;


/**
 * A linked list node used to track nested loops
 */
typedef struct loop_jump_ {
    uint32_t addr;              /**< the instruction address to jump to */
    struct loop_jump_ *next;    /**< next loop-jump in linked list */
} Loopjump;

/**
 * Tracks all breaks, continues in a loop.
 * Also tracks parent loops.
 */
typedef struct loop_list_ {
    enum { LOOP_TYPE_WHILE, LOOP_TYPE_FOR } loop_type; /**< loop type */
    Loopjump *breaks;       /**< linked list of breaks */
    Loopjump *continues;    /**< linked list of continues */
    struct loop_list_ *parent;      /**< parent loop_list (if nested) */
} Looplist;

/**
 * Derived from a CompileState.
 *
 * Used in both compilation and mainly interpreting
 */
typedef struct frame_ {
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
typedef struct compile_state_ {
    uint32_t instr_count;       /**< instruction count */
    uint32_t instr_alloc;       /**< size of instructions array */
    Instruction *instructions;  /**< array of instructions */
    SymbolTable *ltable;        /**< symbol table for locals */
    SymbolTable *gtable;        /**< symbol table for globals */
    ConstantTable *ctable;      /**< constant table */
    Looplist *current_loop;     /**< used while compiling loops */
} CompileState;


typedef struct luci_scope_ {
    unsigned int nparams;
    InstructionArray *instr_array;
    Instruction *ip;
    SymbolTable *ltable;
    SymbolTable *gtable;
    ConstantTable *ctable;
    Looplist *current_loop;
} LuciScope;


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
