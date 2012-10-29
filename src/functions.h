#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "object.h"

struct func_def
{
    const char *name;
    struct LuciObject * (*func) (struct LuciObject *);
};

struct var_def
{
    const char *name;
    struct LuciObject *object;
};
/* populates the array of var_def structs with valid LuciObjects */
void init_variables(void);

struct LuciObject *luci_help(struct LuciObject *);
struct LuciObject *luci_dir(struct LuciObject *);
struct LuciObject *luci_print(struct LuciObject *);
struct LuciObject *luci_readline(struct LuciObject *);
struct LuciObject *luci_typeof(struct LuciObject *);
struct LuciObject *luci_assert(struct LuciObject *);

struct LuciObject *luci_cast_int(struct LuciObject *);
struct LuciObject *luci_cast_float(struct LuciObject *);
struct LuciObject *luci_cast_str(struct LuciObject *);

struct LuciObject *luci_fopen(struct LuciObject *);
struct LuciObject *luci_fclose(struct LuciObject *);
struct LuciObject *luci_fread(struct LuciObject *);
struct LuciObject *luci_fwrite(struct LuciObject *);
struct LuciObject *luci_flines(struct LuciObject *);

struct LuciObject *luci_range(struct LuciObject *);
struct LuciObject *luci_sum(struct LuciObject *);
struct LuciObject *luci_len(struct LuciObject *);
struct LuciObject *luci_max(struct LuciObject *);
struct LuciObject *luci_min(struct LuciObject *);

struct LuciObject *solve_bin_expr(struct LuciObject *left,
	struct LuciObject *right, int op);

struct LuciObject *get_list_node(struct LuciObject *list, int index);
int evaluate_condition(struct LuciObject *);
int types_match(struct LuciObject *left, struct LuciObject *right);

#endif
