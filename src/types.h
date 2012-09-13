#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

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

/* Types of LuciObjects */
typedef enum {
    obj_int_t,
    obj_double_t,
    obj_str_t,
    obj_file_t,
    obj_list_t
} LuciObjectType;

typedef enum {
    f_read_m,
    f_write_m,
    f_append_m
} LuciFileMode;

/* Generic Object which allows for dynamic typing */
typedef struct LuciObject
{
    LuciObjectType type;
    int refcount;
    union
    {
	int i_val;
	double d_val;
	char *s_val;
	struct
	{
	    FILE *f_ptr;
	    long size;	/* in bytes */
	    LuciFileMode mode;
	} file;
	struct
	{
	    struct LuciObject *item;
	    struct LuciObject *next;
	} list;
    } value;
} LuciObject;

/* LuciFunction is a type of function that returns a LuciObject * */
typedef LuciObject * (*LuciFunction) (LuciObject *);

typedef struct Symbol
{
    enum { sym_obj_t, sym_func_t } type;
    char *name;
    union
    {
	LuciObject * object;
	LuciFunction funcptr;  /* function ptr to a LuciFunction */
    } data;
    struct Symbol *next;
} Symbol;

#endif
