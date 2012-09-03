#ifndef TYPES_H
#define TYPES_H

typedef enum {	op_add_t,
		op_sub_t,
		op_mul_t,
		op_div_t,
		op_mod_t,
		op_pow_t,
		op_eq_t,
		op_neq_t,
		op_lt_t,
		op_gt_t,
		op_lte_t,
		op_gte_t,
		op_lnot_t,
		op_lor_t,
		op_land_t,
		op_bxor_t,
		op_bor_t,
		op_band_t,
		op_bnot_t
} op_type;

typedef struct luci_obj_t
{
    enum { obj_none_t, obj_int_t, obj_double_t, obj_str_t } type;
    union
    {
	int i_val;
	double d_val;
	char *s_val;
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

#endif
