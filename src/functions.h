#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct LuciObject;

/* creates and initializes a new LuciObject */
struct LuciObject *create_object(int type);

/* incrementes the object's refcount and returns it */
struct LuciObject *reference_object(struct LuciObject* orig);

/* duplicates a LuciObject, creating a new one */
struct LuciObject *copy_object(struct LuciObject* orig);

/* destroys a LuciObject * */
void destroy_object(struct LuciObject *trash);

int list_append_object(struct LuciObject *list, struct LuciObject *item);
struct LuciObject *list_get_object(struct LuciObject *list, int index);

struct func_def
{
    const char *name;
    struct LuciObject * (*func) (struct LuciObject *);
};

struct LuciObject *luci_help(struct LuciObject *);
struct LuciObject *luci_print(struct LuciObject *);
struct LuciObject *luci_readline(struct LuciObject *in);
struct LuciObject *luci_typeof(struct LuciObject *);
struct LuciObject *luci_assert(struct LuciObject *);

struct LuciObject *luci_cast_int(struct LuciObject *);
struct LuciObject *luci_cast_float(struct LuciObject *);
struct LuciObject *luci_cast_str(struct LuciObject *);

struct LuciObject *luci_fopen(struct LuciObject *);
struct LuciObject *luci_fclose(struct LuciObject *);
struct LuciObject *luci_fread(struct LuciObject *);
struct LuciObject *luci_fwrite(struct LuciObject *);
struct LuciObject *luci_flines(struct LuciObject *param);

struct LuciObject *luci_range(struct LuciObject *);
struct LuciObject *luci_sum(struct LuciObject *);

struct LuciObject *solve_bin_expr(struct LuciObject *left,
	struct LuciObject *right, int op);

struct LuciObject *get_list_node(struct LuciObject *list, int index);
int evaluate_condition(struct LuciObject *);
int types_match(struct LuciObject *left, struct LuciObject *right);

#endif
