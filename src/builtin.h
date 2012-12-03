#ifndef BUILTIN_H
#define BUILTIN_H

#include "object.h"

struct func_def
{
    const char *name;
    struct LuciObject * (*func) (LuciObject **, int);
};

struct var_def
{
    const char *name;
    LuciObject *object;
};

/* populates the array of var_def structs with valid LuciObjects */
void init_variables(void);

LuciObject *luci_help(LuciObject **, int);
LuciObject *luci_dir(LuciObject **, int);
LuciObject *luci_print(LuciObject **, int);
LuciObject *luci_readline(LuciObject **, int);
LuciObject *luci_typeof(LuciObject **, int);
LuciObject *luci_assert(LuciObject **, int);

LuciObject *luci_cast_int(LuciObject **, int);
LuciObject *luci_cast_float(LuciObject **, int);
LuciObject *luci_cast_str(LuciObject **, int);

LuciObject *luci_fopen(LuciObject **, int);
LuciObject *luci_fclose(LuciObject **, int);
LuciObject *luci_fread(LuciObject **, int);
LuciObject *luci_fwrite(LuciObject **, int);
LuciObject *luci_flines(LuciObject **, int);

LuciObject *luci_range(LuciObject **, int);
LuciObject *luci_sum(LuciObject **, int);
LuciObject *luci_len(LuciObject **, int);
LuciObject *luci_max(LuciObject **, int);
LuciObject *luci_min(LuciObject **, int);

//LuciObject *get_list_node(LuciObject *list, int index);

#endif
