/*
 * See Copyright Notice in luci.h
 */

/**
 * @file inttype.h
 */

#ifndef LUCI_INTTYPE_H
#define LUCI_INTTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_int_t;

/** Integer object Type */
typedef struct LuciIntObj_
{
    LuciObject base;    /**< base implementation */
    long i;             /**< 'long' numerical value */
} LuciIntObj;

/** casts LuciObject o to a LuciIntObj */
#define AS_INT(o)       ((LuciIntObj *)(o))

LuciObject *LuciInt_new(long l);
LuciObject* LuciInt_copy(LuciObject *);
LuciObject* LuciInt_repr(LuciObject *);
LuciObject* LuciInt_asbool(LuciObject *);
LuciObject* LuciInt_add(LuciObject *, LuciObject *);
LuciObject* LuciInt_sub(LuciObject *, LuciObject *);
LuciObject* LuciInt_mul(LuciObject *, LuciObject *);
LuciObject* LuciInt_div(LuciObject *, LuciObject *);
LuciObject* LuciInt_mod(LuciObject *, LuciObject *);
LuciObject* LuciInt_pow(LuciObject *, LuciObject *);
LuciObject* LuciInt_eq(LuciObject *, LuciObject *);
LuciObject* LuciInt_neq(LuciObject *, LuciObject *);
LuciObject* LuciInt_lt(LuciObject *, LuciObject *);
LuciObject* LuciInt_gt(LuciObject *, LuciObject *);
LuciObject* LuciInt_lte(LuciObject *, LuciObject *);
LuciObject* LuciInt_gte(LuciObject *, LuciObject *);
LuciObject* LuciInt_lgor(LuciObject *, LuciObject *);
LuciObject* LuciInt_lgand(LuciObject *, LuciObject *);
LuciObject* LuciInt_bwxor(LuciObject *, LuciObject *);
LuciObject* LuciInt_bwor(LuciObject *, LuciObject *);
LuciObject* LuciInt_bwand(LuciObject *, LuciObject *);

LuciObject* LuciInt_neg(LuciObject *);
LuciObject* LuciInt_lgnot(LuciObject *);
LuciObject* LuciInt_bwnot(LuciObject *);
void LuciInt_print(LuciObject *);
void LuciInt_mark(LuciObject *);

#endif
