#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct luci_obj_t
{
    enum { obj_none_t, obj_int_t, obj_str_t } type;
    union
    {
	int integer;
	char *string;
    } value;
} luci_obj_t;

/* luci_func_t is a type of function that returns a luci_obj_t * */
typedef luci_obj_t * (*luci_func_t) (luci_obj_t *);

typedef struct Symbol
{
    enum { sym_obj_t, sym_func_t } type;
    char *name;
    union
    {
	luci_obj_t * object;
	luci_func_t funcptr;  /* function ptr to a luci_func_t */
    } data;
    struct Symbol *next;
} Symbol;


luci_obj_t *luci_sum(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_diff(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_prod(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_div(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_lt(luci_obj_t *left, luci_obj_t *right);
luci_obj_t *luci_gt(luci_obj_t *left, luci_obj_t *right);

#endif
