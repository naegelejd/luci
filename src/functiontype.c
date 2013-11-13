/*
 * See Copyright Notice in luci.h
 */

/**
 * @file functiontype.c
 */

#include "functiontype.h"


/** Type member table for LuciFunctionObj */
LuciObjectType obj_func_t = {
    "function",
    sizeof(LuciFunctionObj),

    LuciFunction_copy,
    LuciFunction_copy,
    unary_nil,
    LuciFunction_asbool,
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

    LuciFunction_print,
    LuciFunction_mark,
    LuciFunction_finalize,
    NULL,   /* hash0 */
    NULL    /* hash1 */
};

/**
 * Creates a new, empty LuciFunctionObj
 *
 * @returns new LuciFunctionObj
 */
LuciObject *LuciFunction_new()
{
    LuciFunctionObj *o = (LuciFunctionObj*)gc_malloc(&obj_func_t);
    return (LuciObject *)o;
}

/**
 * Copies a LuciFunctionObj
 *
 * @param orig LucFunctionObj to copy
 * @returns new copy of orig
 */
LuciObject *LuciFunction_copy(LuciObject *orig)
{
    if (orig == NULL) {
        LUCI_DIE("%s", "Can't copy NULL function\n");
    }

    LuciObject *copy = LuciFunction_new();
    LuciFunctionObj *oldf = AS_FUNCTION(orig);
    LuciFunctionObj *newf = AS_FUNCTION(copy);

    newf->nparams = oldf->nparams;
    newf->globals = oldf->globals;
    newf->ip = oldf->ip;

    newf->ninstrs = oldf->ninstrs;
    size_t instrs_size = newf->ninstrs * sizeof(*newf->instructions);
    newf->instructions = alloc(instrs_size);
    memcpy(newf->instructions, oldf->instructions, instrs_size);

    newf->nconstants = oldf->nconstants;
    size_t consts_size = newf->nconstants * sizeof(*newf->constants);
    newf->constants = alloc(consts_size);
    memcpy(newf->constants, oldf->constants, consts_size);

    /* Copy the frame's local variable array */
    newf->nlocals = oldf->nlocals;
    LuciObject **locals = alloc(newf->nlocals * sizeof(*locals));
    int i;
    for (i = 0; i < newf->nlocals; i++) {
        /* the locals array will only ever contain objects created at
         * compile time (e.g. nested functions, ...) since most objects
         * are created at runtime, pushed onto the stack, then stored
         * in a locals array */
        if (oldf->locals[i]) {
            /* copy the object */
            LuciObject *lcl = oldf->locals[i];
            locals[i] = COPY(lcl);
        }
    }
    newf->locals = locals;

    return copy;
}

/**
 * Returns true
 *
 * @param o LuciLibFuncObj
 * @returns LuciIntObj (true)
 */
LuciObject* LuciFunction_asbool(LuciObject *o)
{
    return LuciInt_new(true);
}

/**
 * Prints a representation of a LuciFunction to stdout
 *
 * @param in LuciFunctionObj to print
 */
void LuciFunction_print(LuciObject *in)
{
    printf("<function>");
}

/**
 * Marks a LuciLibFuncObj as reachable
 *
 * marks all locals and constants
 *
 * @param in LuciLibFuncObj
 */
void LuciFunction_mark(LuciObject *in)
{
    int i;
    for (i = 0; i < AS_FUNCTION(in)->nlocals; i++) {
        LuciObject *obj = AS_FUNCTION(in)->locals[i];
        /* functions can contain NULL locals if they haven't yet
         * been populated by an expression from the stack */
        if (obj) {
            MARK(obj);
        }
    }

    for (i = 0; i < AS_FUNCTION(in)->nconstants; i++) {
        LuciObject *obj = AS_FUNCTION(in)->constants[i];
        MARK(obj);
    }

    GC_MARK(in);
}

/**
 * Finalizes a LuciLibFuncObj
 *
 * frees instructions, locals and constants
 *
 * @param in LuciLibFuncObj
 */
void LuciFunction_finalize(LuciObject *in)
{
    free(AS_FUNCTION(in)->instructions);
    free(AS_FUNCTION(in)->locals);
    free(AS_FUNCTION(in)->constants);
}
