/*
 * See Copyright Notice in luci.h
 */

/**
 * @file compile.h
 */

#ifndef COMPILE_H
#define COMPILE_H

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

/** global symbol table for builtin functions */
extern SymbolTable *builtin_symbols;
/** global builtins array (from final builtins symbol table) */
extern LuciObject **builtins;


/**
 * A linked list node used to track nested loops
 */
typedef struct loop_jump_ {
    struct loop_jump_ *next;    /**< next loop-jump in linked list */
    uint32_t addr;              /**< the instruction address to jump to */
} Loopjump;

/**
 * Tracks all breaks, continues in a loop.
 * Also tracks parent loops.
 */
typedef struct loop_list_ {
    Loopjump *breaks;       /**< linked list of breaks */
    Loopjump *continues;    /**< linked list of continues */
    struct loop_list_ *parent;      /**< parent loop_list (if nested) */
    enum { LOOP_TYPE_WHILE, LOOP_TYPE_FOR } loop_type; /**< loop type */
} Looplist;

/**
 * Essential state storage entity within compilation.
 *
 * Each CompileState represents either the global program's scope
 * or individual function definition scopes.
 */
typedef struct compile_state_ {
    Instruction *instructions;  /**< array of instructions */
    SymbolTable *ltable;        /**< symbol table for locals */
    SymbolTable *gtable;        /**< symbol table for globals */
    ConstantTable *ctable;      /**< constant table */
    Looplist *current_loop;     /**< used while compiling loops */
    uint32_t instr_count;       /**< instruction count */
    uint32_t instr_alloc;       /**< size of instructions array */
} CompileState;


void compiler_init(void);
void compiler_finalize(void);
CompileState *compile_ast(AstNode *root);
CompileState *compile_ast_incremental(CompileState *, LuciObject *, AstNode *);
CompileState *compile_state_new(void);
void compile_state_delete(CompileState *);

void convert_to_function(CompileState *, LuciObject *, uint16_t);

void print_instructions(LuciObject *);

char * serialize_program(LuciObject *);

#endif
