#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "functions.h"
#include "types.h"
#include "ast.h"

static luci_obj_t *luci_sum(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *luci_diff(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *luci_prod(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *luci_div(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *luci_lt(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *luci_gt(luci_obj_t *left, luci_obj_t *right);

luci_obj_t *solve_bin_expr(luci_obj_t *left, luci_obj_t *right, int op)
{
    luci_obj_t *result = NULL;
    switch (op)
    {
	case op_add_t:
	    result = luci_sum(left, right);
	    break;
	case op_sub_t:
	    result = luci_diff(left, right);
	    break;
	case op_mul_t:
	    result = luci_prod(left, right);
	    break;
	case op_div_t:
	    result = luci_div(left, right);
	    break;
	case op_gt_t:
	    result = luci_lt(left, right);
	    break;
	case op_lt_t:
	    result = luci_gt(left, right);
	    break;
	case op_eq_t:
	    result = luci_eq(
	default:
	    fprintf(stderr, "OOPS: Unknown operator type: %d\n", op);
	    exit(1);
    }
    if (!result) {
	return NULL;
    }
    else {
	return result;
    }
}

static luci_obj_t *luci_sum(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val + right->value.i_val;
    return ret;
}

static luci_obj_t *luci_diff(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val - right->value.i_val;
    return ret;
}

static luci_obj_t *luci_prod(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val * right->value.i_val;
    return ret;
}

static luci_obj_t *luci_div(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val / right->value.i_val;
    return ret;
}

static luci_obj_t *luci_lt(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val < right->value.i_val;
    return ret;
}

static luci_obj_t *luci_gt(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val > right->value.i_val;
    return ret;
}
