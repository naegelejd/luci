/*
 * See Copyright Notice in luci.h
 */

/**
 * @file functiontype.h
 */

#ifndef LUCI_FUNCTIONTYPE_H
#define LUCI_FUNCTIONTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_func_t;

/** User-defined function type */
typedef struct LuciFunction_ {
    LuciObject base;            /**< base implementation */
    Instruction *instructions;  /**< array of instructions */
    Instruction *ip;            /**< current instruction pointer */
    LuciObject **locals;        /**< array of local LuciObjects */
    LuciObject **globals;       /**< array of global LuciObjects */
    LuciObject **constants;     /**< array of constant LuciObjects */
    uint32_t ninstrs;           /**< total number of instructions */
    uint16_t nparams;           /**< number of parameters */
    uint16_t nlocals;           /**< number of local symbols */
    uint16_t nconstants;        /**< number of constants */
} LuciFunctionObj;

/** casts LuciObject o to a LuciFunctionObj */
#define AS_FUNCTION(o)  ((LuciFunctionObj *)(o))

LuciObject *LuciFunction_new();
LuciObject* LuciFunction_copy(LuciObject *);
LuciObject* LuciFunction_asbool(LuciObject *);
void LuciFunction_print(LuciObject *in);


#endif
