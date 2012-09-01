#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "functions.h"
#include "types.h"
#include "ast.h"

extern int VERBOSE;

static luci_obj_t *add(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *sub(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *mul(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *divide(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *mod(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *power(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *eq(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *lt(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *gt(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *lte(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *gte(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *lgnot(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *lgor(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *lgand(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *bwnot(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *bwxor(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *bwor(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *bwand(luci_obj_t *left, luci_obj_t *right);

static int types_match(luci_obj_t *left, luci_obj_t *right)
{
    return (left->type == right->type);
}

luci_obj_t * (*solvers[])(luci_obj_t *left, luci_obj_t *right) = {
    add,
    sub,
    mul,
    divide,
    mod,
    power,
    eq,
    lt,
    gt,
    lte,
    gte,
    lgnot,
    lgor,
    lgand,
    bwxor,
    bwor,
    bwand,
    bwnot
};

luci_obj_t *solve_bin_expr(luci_obj_t *left, luci_obj_t *right, int op)
{
    if (!types_match(left, right))
    {
	if (VERBOSE)
	    printf("Types don't match\n");
	return NULL;
    }
    luci_obj_t *result = NULL;
    result = solvers[op](left, right);
    if (!result) {
	return NULL;
    }
    else {
	return result;
    }
}

static luci_obj_t *add(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val + right->value.i_val;
    return ret;
}

static luci_obj_t *sub(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val - right->value.i_val;
    return ret;
}

static luci_obj_t *mul(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val * right->value.i_val;
    return ret;
}

static luci_obj_t *divide(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val / right->value.i_val;
    return ret;
}

static luci_obj_t *mod(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val % right->value.i_val;
    return ret;
}

static luci_obj_t *power(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = pow(left->value.i_val, right->value.i_val);
    return ret;
}

static luci_obj_t *eq(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = (left->value.i_val == right->value.i_val);
    return ret;
}

static luci_obj_t *lt(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val < right->value.i_val;
    return ret;
}

static luci_obj_t *gt(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val > right->value.i_val;
    return ret;
}

static luci_obj_t *lte(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val <= right->value.i_val;
    return ret;
}

static luci_obj_t *gte(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val >= right->value.i_val;
    return ret;
}

static luci_obj_t *lgnot(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = !(right->value.i_val);
    return ret;
}

static luci_obj_t *lgor(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val || right->value.i_val;
    return ret;
}

static luci_obj_t *lgand(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val && right->value.i_val;
    return ret;
}

static luci_obj_t *bwnot(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = ~(right->value.i_val);
    return ret;
}

static luci_obj_t *bwxor(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val ^ right->value.i_val;
    return ret;
}

static luci_obj_t *bwor(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val | right->value.i_val;
    return ret;
}

static luci_obj_t *bwand(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val & right->value.i_val;
    return ret;
}
