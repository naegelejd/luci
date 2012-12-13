/*
 * See Copyright Notice in luci.h
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "object.h"

struct func_def
{
    const char *name;
    LuciObject * (*func) (LuciObject **, unsigned int);
};

struct var_def
{
    const char *name;
    LuciObject *object;
};

/* populates the array of var_def structs with valid LuciObjects */
void init_variables(void);

/* used by print. useful in debugging */
void print_object(LuciObject *);

LuciObject *luci_help(LuciObject **, unsigned int);
LuciObject *luci_dir(LuciObject **, unsigned int);
LuciObject *luci_print(LuciObject **, unsigned int);
LuciObject *luci_readline(LuciObject **, unsigned int);
LuciObject *luci_typeof(LuciObject **, unsigned int);
LuciObject *luci_assert(LuciObject **, unsigned int);

LuciObject *luci_cast_int(LuciObject **, unsigned int);
LuciObject *luci_cast_float(LuciObject **, unsigned int);
LuciObject *luci_cast_str(LuciObject **, unsigned int);

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

//LuciObject *get_list_node(LuciObject *list, int index);

#endif
