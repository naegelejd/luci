#ifndef COMMON_H
#define COMMON_H

/* for FILE* */
#include <stdio.h>

/* common functions */
void *alloc(size_t size);
void yak(const char *, ... );
void die(const char *, ... );

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


#endif
