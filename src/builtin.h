#ifndef BUILTIN_H
#define BUILTIN_H

#include "object.h"
#include "stack.h"

struct func_def
{
    const char *name;
    struct LuciObject * (*func) (Stack *, int);
};

struct var_def
{
    const char *name;
    LuciObject *object;
};

/* populates the array of var_def structs with valid LuciObjects */
void init_variables(void);

LuciObject *luci_help(Stack *, int);
LuciObject *luci_dir(Stack *, int);
LuciObject *luci_print(Stack *, int);
LuciObject *luci_readline(Stack *, int);
LuciObject *luci_typeof(Stack *, int);
LuciObject *luci_assert(Stack *, int);

LuciObject *luci_cast_int(Stack *, int);
LuciObject *luci_cast_float(Stack *, int);
LuciObject *luci_cast_str(Stack *, int);

LuciObject *luci_fopen(Stack *, int);
LuciObject *luci_fclose(Stack *, int);
LuciObject *luci_fread(Stack *, int);
LuciObject *luci_fwrite(Stack *, int);
LuciObject *luci_flines(Stack *, int);

LuciObject *luci_range(Stack *, int);
LuciObject *luci_sum(Stack *, int);
LuciObject *luci_len(Stack *, int);
LuciObject *luci_max(Stack *, int);
LuciObject *luci_min(Stack *, int);

LuciObject *get_list_node(LuciObject *list, int index);

#endif
