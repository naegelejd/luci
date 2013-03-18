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

    &&do_POP,
    &&do_PUSHNULL,
    &&do_LOADK,
    &&do_LOADS,
    &&do_LOADG,
    &&do_DUP,
    &&do_STORE,
    &&do_BINOP,
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
