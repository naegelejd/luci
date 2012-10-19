#ifndef COMMON_H
#define COMMON_H

/* for FILE* */
#include <stdio.h>

/* public functions */
struct ExecContext *get_root_env();

/* common functions */
void *alloc(size_t size);
void yak(const char *, ... );
void die(const char *, ... );

/* initial allocated size of a new List */
#define INIT_LIST_SIZE 32

/* enumeration of supported unary/binary operators */
typedef enum {
    op_add_t,
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
    obj_float_t,
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
	double f_val;
	char *s_val;
	struct
	{
	    FILE *ptr;
	    long size;	/* in bytes */
	    LuciFileMode mode;
	} file;
	struct
	{
	    struct LuciObject **items;
	    int count;	/* current number of items in list */
	    int size;	/* current count of allocated item pointers */
	} list;
    } value;
} LuciObject;

/* LuciFunction is a type of function that returns a LuciObject * */
typedef LuciObject * (*LuciFunction) (LuciObject *);

/* A name and either a LuciObject or a function */
typedef struct Symbol
{
    enum { sym_bobj_t, sym_bfunc_t, sym_uobj_t, sym_ufunc_t } type;
    char *name;
    union
    {
	LuciObject * object;
	LuciFunction funcptr;  /* function ptr to a LuciFunction */
	void *user_defined;
    } data;
    struct Symbol *next;
} Symbol;

#endif
