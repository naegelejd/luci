#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct luci_obj_t;

struct luci_obj_t *solve_bin_expr(struct luci_obj_t *left,
	struct luci_obj_t *right, int op);

#endif
