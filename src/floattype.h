/*
 * See Copyright Notice in luci.h
 */

/**
 * @file floattype.h
 */

#ifndef LUCI_FLOATTYPE_H
#define LUCI_FLOATTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_float_t;

/** Floating-point object type */
typedef struct LuciFloatObj_
{
    LuciObject base;    /**< base implementation */
    double f;           /**< double-precision floating point value */
} LuciFloatObj;

/** casts LuciObject o to a LuciFloatObj */
#define AS_FLOAT(o)     ((LuciFloatObj *)(o))

LuciObject *LuciFloat_new(double d);
LuciObject* LuciFloat_copy(LuciObject *);
LuciObject* LuciFloat_repr(LuciObject *);
LuciObject* LuciFloat_asbool(LuciObject *);
LuciObject* LuciFloat_add(LuciObject *, LuciObject *);
LuciObject* LuciFloat_sub(LuciObject *, LuciObject *);
LuciObject* LuciFloat_mul(LuciObject *, LuciObject *);
LuciObject* LuciFloat_div(LuciObject *, LuciObject *);
LuciObject* LuciFloat_mod(LuciObject *, LuciObject *);
LuciObject* LuciFloat_pow(LuciObject *, LuciObject *);
LuciObject* LuciFloat_eq(LuciObject *, LuciObject *);
LuciObject* LuciFloat_neq(LuciObject *, LuciObject *);
LuciObject* LuciFloat_lt(LuciObject *, LuciObject *);
LuciObject* LuciFloat_gt(LuciObject *, LuciObject *);
LuciObject* LuciFloat_lte(LuciObject *, LuciObject *);
LuciObject* LuciFloat_gte(LuciObject *, LuciObject *);
LuciObject* LuciFloat_bwxor(LuciObject *, LuciObject *);
LuciObject* LuciFloat_bwor(LuciObject *, LuciObject *);
LuciObject* LuciFloat_bwand(LuciObject *, LuciObject *);

LuciObject* LuciFloat_neg(LuciObject *);
LuciObject* LuciFloat_bwnot(LuciObject *);

void LuciFloat_print(LuciObject *);
void LuciFloat_mark(LuciObject *in);

#endif
