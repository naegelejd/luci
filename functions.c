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
static LuciObject *add(LuciObject *left, LuciObject *right);
static LuciObject *sub(LuciObject *left, LuciObject *right);
static LuciObject *mul(LuciObject *left, LuciObject *right);
static LuciObject *divide(LuciObject *left, LuciObject *right);
static LuciObject *mod(LuciObject *left, LuciObject *right);
static LuciObject *power(LuciObject *left, LuciObject *right);
static LuciObject *eq(LuciObject *left, LuciObject *right);
static LuciObject *neq(LuciObject *left, LuciObject *right);
static LuciObject *lt(LuciObject *left, LuciObject *right);
static LuciObject *gt(LuciObject *left, LuciObject *right);
static LuciObject *lte(LuciObject *left, LuciObject *right);
static LuciObject *gte(LuciObject *left, LuciObject *right);
static LuciObject *lgnot(LuciObject *left, LuciObject *right);
static LuciObject *lgor(LuciObject *left, LuciObject *right);
static LuciObject *lgand(LuciObject *left, LuciObject *right);
static LuciObject *bwnot(LuciObject *left, LuciObject *right);
static LuciObject *bwxor(LuciObject *left, LuciObject *right);
static LuciObject *bwor(LuciObject *left, LuciObject *right);
static LuciObject *bwand(LuciObject *left, LuciObject *right);


LuciObject *create_object(int type)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = type;
    if (type == obj_list_t) {
	ret->value.list.next = NULL;
    }
    return ret;
}

void destroy_object(LuciObject *trash)
{
    if (trash)
    {
	/* if this object is part of a linked list, destroy the next node */
	if (trash->type == obj_list_t)
	{
	    destroy_object(trash->value.list.item);
	    destroy_object(trash->value.list.next);
	}

	/* if this object contains a string, it WAS malloc'd */
	if (trash->type == obj_str_t) {
	    if (VERBOSE)
		printf("Freeing str object with val %s\n", trash->value.s_val);
	    free(trash->value.s_val);
	    trash->value.s_val = NULL;
	}
	if (VERBOSE)
	    printf("Freeing obj with type %d\n", trash->type);

	/* destroy the LuciObject itself */
	free(trash);
	trash = NULL;
    }
}

const struct func_init builtins[] =
{
    "help", luci_help,
    "print",  luci_print,
    "type",  luci_typeof,
    "assert", luci_assert,
    "str", luci_str,
    "open", luci_fopen,
    "close", luci_fclose,
    "read", luci_fread,
    "write", luci_fwrite,
    0, 0
};

LuciObject *luci_help(LuciObject *in)
{
    printf("_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_\n");
    printf("              HELP               \n");
    printf("_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_\n");
    return NULL;
}

LuciObject *luci_print(LuciObject *in)
{
    assert(in->type == obj_list_t);

    if (!in )
    {
	printf("None\n");
    }
    else
    {
	LuciObject *ptr = in->value.list.next;
	LuciObject *item = NULL;
	while (ptr)
	{
	    item = ptr->value.list.item;
	    switch(item->type)
	    {
		case obj_int_t:
		    printf("%d", item->value.i_val);
		    break;
		case obj_double_t:
		    printf("%f", item->value.d_val);
		    break;
		case obj_str_t:
		    printf("%s", item->value.s_val);
		    break;
		default:
		    printf("None");
	    }
	    printf(" ");
	    ptr = ptr->value.list.next;
	}
	printf("\n");
    }

    return NULL;
}

LuciObject *luci_typeof(LuciObject *in)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_str_t;
    char *which;
    if (!in)
    {
	which = "None";
    }
    else
    {
	switch(in->type)
	{
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
    }
    ret->value.s_val = alloc(strlen(which) + 1);
    strcpy(ret->value.s_val, which);

    return ret;
}

LuciObject *luci_assert(LuciObject *in)
{
    assert(in);
    switch(in->type)
    {
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

LuciObject *luci_str(LuciObject *in)
{
    LuciObject *ret = NULL;
    if (!in)
    {
	return ret;
    }
    else
    {
	ret = create_object(obj_str_t);
	switch (in->type)
	{
	    case obj_int_t:
		ret->value.s_val = alloc(16);
		sprintf(ret->value.s_val, "%d", in->value.i_val);
		//ret->value.s_val[16] = '\0';
		break;
	    case obj_double_t:
		ret->value.s_val = alloc(16);
		sprintf(ret->value.s_val, "%f", (float)in->value.d_val);
		//ret->value.s_val[16] = '\0';
		break;
	    case obj_str_t:
		ret->value.s_val = alloc(strlen(in->value.s_val) + 1);
		strcpy(ret->value.s_val, in->value.s_val);
		break;
	    default:
		break;
	}
	if (VERBOSE)
	    printf("str() returning %s\n", ret->value.s_val);
    }

    return ret;
}

LuciObject *luci_fopen(LuciObject *in)
{
    return NULL;
}
LuciObject *luci_fclose(LuciObject *in)
{
    return NULL;
}
LuciObject *luci_fread(LuciObject *in)
{
    return NULL;
}
LuciObject *luci_fwrite(LuciObject *in)
{
    return NULL;
}


LuciObject * (*solvers[])(LuciObject *left, LuciObject *right) = {
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

int types_match(LuciObject *left, LuciObject *right)
{
    if (left && right)
	return (left->type == right->type);
    else
	return 0;
}

LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, int op)
{
    if (!types_match(left, right))
    {
	if (VERBOSE)
	    printf("Types don't match\n");
	return NULL;
    }
    LuciObject *result = NULL;
    result = solvers[op](left, right);
    return result;
}

static LuciObject *add(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = create_object(left->type);
    switch (left->type)
    {
	case obj_int_t:
	    ret->value.i_val = left->value.i_val + right->value.i_val;
	    break;
	case obj_double_t:
	    ret->value.d_val = left->value.d_val + right->value.d_val;
	    break;
	case obj_str_t:
	    ret->value.s_val = alloc(strlen(left->value.s_val) +
		    strlen(right->value.s_val) + 1);
	    strcpy(ret->value.s_val, left->value.s_val);
	    strcat(ret->value.s_val, right->value.s_val);
	    break;
	default:
	    break;
    }
    return ret;
}

static LuciObject *sub(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val - right->value.i_val;
    return ret;
}

static LuciObject *mul(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val * right->value.i_val;
    return ret;
}

static LuciObject *divide(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val / right->value.i_val;
    return ret;
}

static LuciObject *mod(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val % right->value.i_val;
    return ret;
}

static LuciObject *power(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = pow(left->value.i_val, right->value.i_val);
    return ret;
}

static LuciObject *eq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = create_object(obj_int_t);
    int r;
    switch (left->type)
    {
	case obj_int_t:
	    r = (left->value.i_val == right->value.i_val);
	    break;
	case obj_double_t:
	    r = (left->value.d_val == right->value.d_val);
	    break;
	case obj_str_t:
	    r = !(strcmp(left->value.s_val, right->value.s_val));
	    break;
	default:
	    r = 0;
    }
    ret->value.i_val = r;
    return ret;
}

static LuciObject *neq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = (left->value.i_val != right->value.i_val);
    return ret;
}

static LuciObject *lt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val < right->value.i_val;
    return ret;
}

static LuciObject *gt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val > right->value.i_val;
    return ret;
}

static LuciObject *lte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val <= right->value.i_val;
    return ret;
}

static LuciObject *gte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val >= right->value.i_val;
    return ret;
}

static LuciObject *lgnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = !(right->value.i_val);
    return ret;
}

static LuciObject *lgor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val || right->value.i_val;
    return ret;
}

static LuciObject *lgand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val && right->value.i_val;
    return ret;
}

static LuciObject *bwnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = ~(right->value.i_val);
    return ret;
}

static LuciObject *bwxor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val ^ right->value.i_val;
    return ret;
}

static LuciObject *bwor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val | right->value.i_val;
    return ret;
}

static LuciObject *bwand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val & right->value.i_val;
    return ret;
}
