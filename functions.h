#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct luci_obj_t;

struct func_init
{
    const char *name;
    struct luci_obj_t * (*func) (struct luci_obj_t *);
};

struct luci_obj_t *luci_help(struct luci_obj_t *);
struct luci_obj_t *luci_print(struct luci_obj_t *);
struct luci_obj_t *luci_typeof(struct luci_obj_t *);
struct luci_obj_t *luci_assert(struct luci_obj_t *);
struct luci_obj_t *luci_str(struct luci_obj_t *);

struct luci_obj_t *solve_bin_expr(struct luci_obj_t *left,
	struct luci_obj_t *right, int op);

int types_match(struct luci_obj_t *left, struct luci_obj_t *right);

#endif
