/*
 * See Copyright Notice in luci.h
 */

/**
 * @file builtin.h
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "object.h"

/** Function pointer and corresponding name available to
 * users as a builtin library function */
struct func_def
{
    const char *name;       /**< builtin function name */
    /** libfunction pointer */
    LuciObject * (*func) (LuciObject **, unsigned int);
};

/** LuciObject and corresponding name available to users
 * as a builtin symbol definition */
struct var_def
{
    const char *name;       /**< builtin symbol name */
    LuciObject *object;     /**< object pointer */
};

void init_variables(void);

LuciObject *luci_help(LuciObject **, unsigned int);
LuciObject *luci_dir(LuciObject **, unsigned int);
LuciObject *luci_exit(LuciObject **, unsigned int);
LuciObject *luci_print(LuciObject **, unsigned int);
LuciObject *luci_readline(LuciObject **, unsigned int);
LuciObject *luci_typeof(LuciObject **, unsigned int);
LuciObject *luci_assert(LuciObject **, unsigned int);

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
