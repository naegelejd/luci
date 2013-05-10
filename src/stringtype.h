/*
 * See Copyright Notice in luci.h
 */

/**
 * @file stringtype.h
 */

#ifndef LUCI_STRINGTYPE_H
#define LUCI_STRINGTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_string_t;

/** String object type */
typedef struct LuciString_ {
    LuciObject base;    /**< base implementation */
    char * s;           /**< pointer to C-string */
    long len;           /**< string length */
} LuciStringObj;

/** casts LuciObject o to a LuciStringObj */
#define AS_STRING(o)    ((LuciStringObj *)(o))

LuciObject *LuciString_new(char *s);
LuciObject* LuciString_copy(LuciObject *);
LuciObject* LuciString_repr(LuciObject *);
LuciObject* LuciString_asbool(LuciObject *);
LuciObject* LuciString_len(LuciObject *);
LuciObject* LuciString_add(LuciObject *, LuciObject *);
LuciObject* LuciString_mul(LuciObject *, LuciObject *);
LuciObject* LuciString_eq(LuciObject *, LuciObject *);
LuciObject* LuciString_contains(LuciObject *m, LuciObject *o);
LuciObject* LuciString_cget(LuciObject *, LuciObject *);
LuciObject* LuciString_cput(LuciObject *, LuciObject *, LuciObject *);
LuciObject* LuciString_next(LuciObject *, LuciObject *);
void LuciString_print(LuciObject *);
void LuciString_mark(LuciObject *);
void LuciString_finalize(LuciObject *);

#endif
