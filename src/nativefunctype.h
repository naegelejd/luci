/*
 * See Copyright Notice in luci.h
 */

/**
 * @file nativefunctype.h
 */

#ifndef LUCI_NATIVEFUNCTYPE_H
#define LUCI_NATIVEFUNCTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_libfunc_t;

/** Function pointer signature for Luci's builtin library functions */
typedef LuciObject* (*LuciCFunc)(LuciObject **, unsigned int);

/** Library function type */
typedef struct LuciLibFunc_ {
    LuciObject base;        /**< base implementation */
    LuciCFunc func;         /**< pointer to a Luci C function */
    char *help;             /**< help string for Luci C function */
    int min_args;           /**< minimum number of arguments to function */
} LuciLibFuncObj;

/** casts LuciObject o to a LuciLibFuncObj */
#define AS_LIBFUNC(o)  ((LuciLibFuncObj *)(o))

LuciObject *LuciLibFunc_new(LuciCFunc fptr, char *help, int min_args);
LuciObject* LuciLibFunc_copy(LuciObject *);
LuciObject* LuciLibFunc_asbool(LuciObject *);
void LuciLibFunc_print(LuciObject *in);
void LuciLibFunc_mark(LuciObject *in);

#endif
