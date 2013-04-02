/*
 * See Copyright Notice in luci.h
 */

/**
 * @file dispatch.h
 */

/**
 * This static dispatch table is used only when the GCC "computed goto"
 * (Labels as Values) extension is utilized in Luci's bytecode interpreter.
 *
 * See interpret.c.
 *
 * This table must contain an address to a label for EACH bytecode
 * instruction defined in compile.h.
 */
static void* dispatch_table[] = {
    &&do_NOP,

    &&do_ADD,
    &&do_SUB,
    &&do_MUL,
    &&do_DIV,
    &&do_MOD,
    &&do_POW,
    &&do_EQ,
    &&do_NEQ,
    &&do_LT,
    &&do_GT,
    &&do_LTE,
    &&do_GTE,
    &&do_LGOR,
    &&do_LGAND,
    &&do_BWXOR,
    &&do_BWOR,
    &&do_BWAND,

    &&do_NEG,
    &&do_LGNOT,
    &&do_BWNOT,

    &&do_POP,
    &&do_PUSHNIL,
    &&do_LOADK,
    &&do_LOADS,
    &&do_LOADG,
    &&do_LOADB,
    &&do_DUP,
    &&do_STORE,
    &&do_CALL,
    &&do_RETURN,
    &&do_MKMAP,
    &&do_MKLIST,
    &&do_CGET,
    &&do_CPUT,
    &&do_MKITER,
    &&do_HALT,
    &&do_JUMP,
    &&do_POPJUMP,
    &&do_JUMPZ,
    &&do_ITERJUMP
};
