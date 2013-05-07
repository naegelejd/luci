/*
 * See Copyright Notice in luci.h
 */

/**
 * @file listtype.h
 */

#ifndef LUCI_LISTTYPE_H
#define LUCI_LISTTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_list_t;

#define INIT_LIST_SIZE 32   /**< initial allocated size of a list */

/** List object type */
typedef struct LuciList_ {
    LuciObject base;    /**< base implementation */
    LuciObject **items; /**< pointer to items array */
    unsigned int count;	/**< current number of items in list */
    unsigned int size;	/**< current count of allocated items */
} LuciListObj;

/** casts LuciObject o to a LuciListObj */
#define AS_LIST(o)      ((LuciListObj *)(o))

LuciObject *LuciList_new();
LuciObject* LuciList_copy(LuciObject *);
LuciObject* LuciList_len(LuciObject *);
LuciObject* LuciList_asbool(LuciObject *);
LuciObject* LuciList_add(LuciObject *, LuciObject *);
LuciObject* LuciList_eq(LuciObject *, LuciObject *);
LuciObject* LuciList_append(LuciObject *, LuciObject *);
LuciObject* LuciList_contains(LuciObject *m, LuciObject *o);
LuciObject* LuciList_cget(LuciObject *, LuciObject *);
LuciObject* LuciList_cput(LuciObject *, LuciObject *, LuciObject *);
LuciObject* LuciList_next(LuciObject *, LuciObject *);
void LuciList_print(LuciObject *);


#endif
