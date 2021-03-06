/*
 * See Copyright Notice in luci.h
 */

/**
 * @file nativefunctype.c
 */

#include "nativefunctype.h"


/** Type member table for LuciLibFuncObj */
LuciObjectType obj_libfunc_t = {
    "libfunction",
    sizeof(LuciLibFuncObj),

    LuciLibFunc_copy,
    LuciLibFunc_copy,
    unary_nil,
    LuciLibFunc_asbool,
    unary_nil,
    unary_nil,
    unary_nil,
    LuciObject_lgnot,

    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    LuciObject_lgor,
    LuciObject_lgand,
    binary_nil,
    binary_nil,
    binary_nil,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciLibFunc_print,
    LuciLibFunc_mark,
    NULL,       /* finalize */
    NULL,       /* hash 0 */
    NULL        /* hash 1 */
};

/**
 * Creates a new LuciLibFuncObj
 *
 * @param fptr C function pointer matching LuciLibFunc signature
 * @param help static help string
 * @param min_args minimum # of args function must be called with
 * @returns new LuciLibFuncObj
 */
LuciObject *LuciLibFunc_new(LuciCFunc fptr, char *help, int min_args)
{
    LuciLibFuncObj *o = (LuciLibFuncObj*)gc_malloc(&obj_libfunc_t);
    o->func = fptr;
    o->help = help;
    o->min_args = min_args;
    return (LuciObject *)o;
}

/**
 * Copies a LuciLibFuncObj
 *
 * @param orig LucLibFuncObj to copy
 * @returns new copy of orig
 */
LuciObject *LuciLibFunc_copy(LuciObject *orig)
{
    LuciLibFuncObj *tmp = (LuciLibFuncObj *)orig;
    return LuciLibFunc_new(tmp->func, tmp->help, tmp->min_args);
}

/**
 * Returns true
 *
 * @param o
 * @returns LuciIntObj (true)
 */
LuciObject* LuciLibFunc_asbool(LuciObject *o)
{
    return LuciInt_new(true);
}

/**
 * Prints a representation of a LuciLibFunc to stdout
 *
 * @param in LuciLibFunc to print
 */
void LuciLibFunc_print(LuciObject *in)
{
    printf("<libfunction>");
}

/**
 * Marks a LuciLibFuncObj as reachable
 *
 * @param in LuciLibFuncObj
 */
void LuciLibFunc_mark(LuciObject *in)
{
    GC_MARK(in);
}
