#include <string.h>
#include <math.h>
#include "object.h"
#include "common.h"
#include "binop.h"

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

static int types_match(LuciObject *left, LuciObject *right);
static int evaluate_condition(LuciObject *);

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

static int types_match(LuciObject *left, LuciObject *right)
{
    if (left && right)
	return (left->type == right->type);
    else
	return 0;
}

/*
   Evaluates a conditional statement, returning an integer
   value of 0 if False, and non-zero if True.
*/
static int evaluate_condition(LuciObject *cond)
{
    int huh = 0;
    if (cond == NULL) {
	return 0;
    }
    switch (cond->type)
    {
	case obj_int_t:
	    huh = cond->value.i;
	    break;
	case obj_float_t:
	    huh = (int)cond->value.f;
	    break;
	case obj_str_t:
	    huh = strlen(cond->value.s);
	    break;
	case obj_list_t:
	    huh = cond->value.list.count;
	    break;
	case obj_file_t:
	    huh = 1;
	    break;
	default:
	    huh = 0;
    }
    return huh;
}

LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, int op)
{
    if (left->type == obj_float_t && right->type == obj_int_t) {
	/* maybe bzero it ? */
	int i = right->value.i;
	right->value.i = 0;
	right->type = obj_float_t;
	right->value.f = (float)i;
    }
    else if (right->type == obj_float_t && left->type == obj_int_t) {
	int i = left->value.i;
	left->value.i = 0;
	left->type = obj_float_t;
	left->value.f = (float)i;
    }
    else if (!types_match(left, right))
    {
	die("Type mismatch in expression\n");
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
	    ret->value.i = left->value.i + right->value.i;
	    break;
	case obj_float_t:
	    ret->value.f = left->value.f + right->value.f;
	    break;
	case obj_str_t:
	    ret->value.s = alloc(strlen(left->value.s) +
		    strlen(right->value.s) + 1);
	    strcpy(ret->value.s, left->value.s);
	    strcat(ret->value.s, right->value.s);
	    break;
	default:
	    break;
    }
    return ret;
}

static LuciObject *sub(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = left->value.i - right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = left->value.f - right->value.f;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *mul(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = left->value.i * right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = left->value.f * right->value.f;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *divide(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    /* this could probably be replace by a generic right->value.f
       since both the float and int obj->value structs are aligned
    */
    if ((right->type == obj_int_t && right->value.i == 0) ||
	(right->type == obj_float_t && right->value.f == 0.0)) {
	    /* memory leak */
	    /* exit(1); */
	    die("Divide by zero error\n");
    }

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = left->value.i / right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = left->value.f / right->value.f;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *mod(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = left->value.i % right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = (double)((int)left->value.f % (int)right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *power(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = pow(left->value.i, right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = pow(left->value.f, right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *eq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = create_object(obj_int_t);
    int r;
    switch (left->type)
    {
	case obj_int_t:
	    r = (left->value.i == right->value.i);
	    break;
	case obj_float_t:
	    r = (left->value.f == right->value.f);
	    break;
	case obj_str_t:
	    r = !(strcmp(left->value.s, right->value.s));
	    break;
	case obj_file_t:
	    r = (left->value.file.ptr == right->value.file.ptr);
	default:
	    r = 0;
    }
    ret->value.i = r;
    return ret;
}

static LuciObject *neq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = eq(left, right);
    if (ret)
	ret->value.i = !ret->value.i;
    return ret;
}

static LuciObject *lt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i < right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = (left->value.f < right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *gt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i > right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = (left->value.f > right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = gt(left, right);
    if (ret)
	ret->value.i = !ret->value.i;
    return ret;
}

static LuciObject *gte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = lt(left, right);
    if (ret)
	ret->value.i = !ret->value.i;
    return ret;
}

static LuciObject *lgnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = !right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = !left->value.f;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lgor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i || right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = (left->value.f || right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lgand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i && right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = (left->value.f && right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = ~right->value.i;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = ~(int)right->value.f;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwxor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i = left->value.i ^ right->value.i;
    return ret;
}

static LuciObject *bwor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i | right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = ((int)left->value.f | (int)right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i = (left->value.i & right->value.i);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f = ((int)left->value.f & (int)right->value.f);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}
