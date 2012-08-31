#ifndef FUNCTIONS_H
#define FUNCTIONS_H

luci_obj_t *luci_sum(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_diff(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_prod(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_div(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_lt(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_gt(luci_obj_t *left, luci_obj_t *right);

#endif
