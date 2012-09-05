#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "functions.h"
#include "types.h"
#include "ast.h"

extern int VERBOSE;

/* Forward declarations */
static luci_obj_t *add(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *sub(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *mul(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *divide(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *mod(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *power(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *eq(luci_obj_t *left, luci_obj_t *right);
static luci_obj_t *neq(luci_obj_t *left, luci_obj_t *right);
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


const struct func_init builtins[] =
{
    "help", luci_help,
    "print",  luci_print,
    "type",  luci_typeof,
    "assert", luci_assert,
    "str", luci_str,
    0, 0
};

luci_obj_t *luci_help(luci_obj_t *in)
{
    printf("_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_\n");
    printf("              HELP               \n");
    printf("_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_\n");
    return NULL;
}

luci_obj_t *luci_print(luci_obj_t *in)
{
    if (!in || in->type == obj_none_t)
    {
	printf("None\n");
    }
    else
    {
	switch(in->type)
	{
	    case obj_int_t:
		printf("%d\n", in->value.i_val);
		break;
	    case obj_double_t:
		printf("%f\n", in->value.d_val);
		break;
	    case obj_str_t:
		printf("%s\n", in->value.s_val);
		break;
	    default:
		printf("None\n");
	}
    }

    return NULL;
}

luci_obj_t *luci_typeof(luci_obj_t *in)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_str_t;
    char *which;
    switch(in->type)
    {
	case obj_none_t:
	    which = "None";
	    break;
	case obj_int_t:
	    which = "int";
	    break;
	case obj_double_t:
	    which = "double";
	    break;
	case obj_str_t:
	    which = "string";
	    break;
	default:
	    which = "None";
    }
    ret->value.s_val = alloc(strlen(which) + 1);
    strcpy(ret->value.s_val, which);

    return ret;
}

luci_obj_t *luci_assert(luci_obj_t *in)
{
    assert(in);
    switch(in->type)
    {
	case obj_none_t:
	    /* raise AssertionError */
	    assert(0);
	    break;
	case obj_int_t:
	    assert(in->value.i_val);
	    break;
	case obj_double_t:
	    assert((int)in->value.d_val);
	    break;
	case obj_str_t:
	    assert(strcmp("", in->value.s_val) != 0);
	    break;
	default:
	    ;
    }
    return NULL;
}

luci_obj_t *luci_str(luci_obj_t *in)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_str_t;

    if (!in)
    {
	ret = NULL;
    }
    else
    {
	switch (in->type)
	{
	    case obj_none_t:
		ret = NULL;
		break;
	    case obj_int_t:
		ret->value.s_val = alloc(17);
		sscanf(ret->value.s_val, "%d", &(in->value.i_val));
		ret->value.s_val[16] = '\0';
		break;
	    case obj_double_t:
		ret->value.s_val = alloc(17);
		sscanf(ret->value.s_val, "%f", (float *)&(in->value.d_val));
		ret->value.s_val[16] = '\0';
		break;
	    case obj_str_t:
		ret->value.s_val = alloc(strlen(in->value.s_val) + 1);
		strcpy(ret->value.s_val, in->value.s_val);
		break;
	    default:
		ret = NULL;
	}
    }

    return ret;
}


luci_obj_t * (*solvers[])(luci_obj_t *left, luci_obj_t *right) = {
    add,
    sub,
    mul,
    divide,
    mod,
    power,
    eq,
    neq,
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

int types_match(luci_obj_t *left, luci_obj_t *right)
{
    return (left->type == right->type);
}

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
    return result;
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

static luci_obj_t *neq(luci_obj_t *left, luci_obj_t *right)
{
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = (left->value.i_val != right->value.i_val);
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
