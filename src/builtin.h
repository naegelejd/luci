/*
 * See Copyright Notice in luci.h
 */

/**
 * @file builtin.h
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "object.h"

/** LuciObject and corresponding name available to users
 * as a builtin symbol definition */
typedef struct object_record
{
    const char *name;       /**< builtin symbol name */
    LuciObject *object;     /**< object pointer */
} LuciObjectRecord;

/** global registry of builtin constant types */
extern const LuciObjectRecord builtins_registry[];


void init_luci_builtins(void);

LuciObject *luci_help(LuciObject **, unsigned int);
LuciObject *luci_dir(LuciObject **, unsigned int);
LuciObject *luci_exit(LuciObject **, unsigned int);
LuciObject *luci_print(LuciObject **, unsigned int);
LuciObject *luci_readline(LuciObject **, unsigned int);
LuciObject *luci_typeof(LuciObject **, unsigned int);
LuciObject *luci_assert(LuciObject **, unsigned int);
LuciObject *luci_copy(LuciObject **, unsigned int);

LuciObject *luci_cast_int(LuciObject **, unsigned int);
LuciObject *luci_cast_float(LuciObject **, unsigned int);
LuciObject *luci_cast_str(LuciObject **, unsigned int);

LuciObject *luci_hex(LuciObject **, unsigned int);

LuciObject *luci_fopen(LuciObject **, unsigned int);
LuciObject *luci_fclose(LuciObject **, unsigned int);
LuciObject *luci_fread(LuciObject **, unsigned int);
LuciObject *luci_fwrite(LuciObject **, unsigned int);
LuciObject *luci_flines(LuciObject **, unsigned int);

LuciObject *luci_range(LuciObject **, unsigned int);
LuciObject *luci_sum(LuciObject **, unsigned int);
LuciObject *luci_len(LuciObject **, unsigned int);
LuciObject *luci_max(LuciObject **, unsigned int);
LuciObject *luci_min(LuciObject **, unsigned int);

LuciObject *luci_contains(LuciObject **, unsigned int);

#endif
