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


struct func_def
{
    const char *name;
    struct LuciObject * (*func) (struct LuciObject *);
};

struct LuciObject *luci_help(struct LuciObject *);
struct LuciObject *luci_print(struct LuciObject *);
struct LuciObject *luci_input(struct LuciObject *in);
struct LuciObject *luci_typeof(struct LuciObject *);
struct LuciObject *luci_assert(struct LuciObject *);
struct LuciObject *luci_str(struct LuciObject *);

struct LuciObject *luci_fopen(struct LuciObject *);
struct LuciObject *luci_fclose(struct LuciObject *);
struct LuciObject *luci_fread(struct LuciObject *);
struct LuciObject *luci_fwrite(struct LuciObject *);

struct LuciObject *luci_range(struct LuciObject *);
struct LuciObject *luci_sum(struct LuciObject *);

struct LuciObject *solve_bin_expr(struct LuciObject *left,
	struct LuciObject *right, int op);

int types_match(struct LuciObject *left, struct LuciObject *right);

#endif
