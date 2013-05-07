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
    FLAG_DEEP_COPY,
    sizeof(LuciFunctionObj),

    LuciFunction_copy,
    unary_nil,
    LuciFunction_asbool,
    unary_nil,
    unary_nil,
    unary_nil,
    unary_nil,

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
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciFunction_print
};

/**
 * Creates a new LuciFunctionObj
 *
 * @param frame Frame struct defining function
 * @returns new LuciFunctionObj
 */
LuciObject *LuciFunction_new()
{
    LuciFunctionObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_func_t);
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
        DIE("%s", "Can't copy NULL function\n");
    }

    LuciObject *copy = LuciFunction_new();
    LuciFunctionObj *oldf = AS_FUNCTION(orig);
    LuciFunctionObj *newf = AS_FUNCTION(copy);

    newf->nparams = oldf->nparams;
    newf->nlocals = oldf->nlocals;
    newf->nconstants = oldf->nconstants;
    newf->ip = oldf->ip;
    newf->ninstrs = oldf->ninstrs;
    newf->instructions = oldf->instructions;
    newf->globals = oldf->globals;
    newf->constants = oldf->constants;

    LUCI_DEBUG("Copying frame:\nnparams: %d\nnlocals: %d\nnconstants: %d\n",
            newf->nparams, newf->nlocals, newf->nconstants);

    /* Copy the frame's local variable array */
    LuciObject **locals = alloc(newf->nlocals * sizeof(*locals));
    int i;
    for (i = 0; i < newf->nlocals; i++) {
        if (oldf->locals[i]) {
            /* copy the object */
            LuciObject *lcl = oldf->locals[i];
            locals[i] = lcl->type->copy(lcl);
        }
    }
    newf->locals = locals;

    return copy;
}

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
