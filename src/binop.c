/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "luci.h"
#include "object.h"
#include "binop.h"


/* Forward declarations */
/*
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
*/


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
	    huh = AS_INT(cond)->i;
	    break;
	case obj_float_t:
	    huh = (int)AS_FLOAT(cond)->f;
	    break;
	case obj_str_t:
	    huh = AS_STRING(cond)->len;
	    break;
	case obj_list_t:
	    huh = AS_LIST(cond)->count;
	    break;
	default:
	    huh = 1;
    }
    return huh;
}

long int_op(long l, long r, int op)
{
    switch (op) {
        case op_add_t:
            return l + r;
            break;

        case op_sub_t:
            return l - r;
            break;

        case op_mul_t:
            return l * r;
            break;

        case op_div_t:
            if (r == 0L) {
                DIE("%s\n", "Divide by zero");
            }
            return l / r;

        case op_mod_t:
            if (r == 0L) {
                DIE("%s\n", "Divide by zero");
            }
            return l % r;

        case op_pow_t:
            return pow(l, r);
            break;

        case op_eq_t:
            return l == r;
            break;

        case op_neq_t:
            return l != r;
            break;

        case op_lt_t:
            return l < r;
            break;

        case op_gt_t:
            return l > r;
            break;

        case op_lte_t:
            return l <= r;
            break;

        case op_gte_t:
            return l >= r;
            break;

        case op_lor_t:
            return l || r;
            break;

        case op_land_t:
            return l && r;
            break;

        case op_lnot_t:
            return !r;
            break;

        case op_bxor_t:
            return l ^ r;
            break;

        case op_bor_t:
            return l | r;
            break;

        case op_band_t:
            return l & r;
            break;

        case op_bnot_t:
            return ~r;
            break;

        default:
            DIE("%s\n", "Invalid operand to type int");
    }
}

double float_op(double l, double r, int op)
{
    switch (op) {
        case op_add_t:
            return l + r;
            break;

        case op_sub_t:
            return l - r;
            break;

        case op_mul_t:
            return l * r;
            break;

        case op_div_t:
            if (r == 0.0L) {
                DIE("%s\n", "Divide by zero");
            }
            return l / r;

        case op_pow_t:
            return pow(l, r);
            break;

        case op_eq_t:
            return l == r;
            break;

        case op_neq_t:
            return l != r;
            break;

        case op_lt_t:
            return l < r;
            break;

        case op_gt_t:
            return l > r;
            break;

        case op_lte_t:
            return l <= r;
            break;

        case op_gte_t:
            return l >= r;
            break;

        default:
            DIE("%s\n", "Invalid operand to type float");
    }
}

char *string_op(LuciStringObj *l, LuciStringObj *r, int op)
{
    switch (op) {
        case op_add_t:
        {
	    char *s = alloc(l->len + r->len + 1);
	    strcpy(s, l->s);
	    strcat(s, r->s);
            return s;
        } break;

        default:
            DIE("%s\n", "Invalid operand to type float");
    }
}

LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, int op)
{
    LuciObject *result = NULL;

    if (!left || !right) {
        /* uh oh */
        /* return NULL; */
        DIE("%s\n", "NULL object in binary expression dispatcher");
    }

    if (!TYPES_MATCH(left, right)) {
        if (TYPEOF(left) == obj_float_t && TYPEOF(right) == obj_int_t) {
            double f = float_op(AS_FLOAT(left)->f,
                    (double)(AS_INT(right)->i), op);
            result = LuciFloat_new(f);
            return result;
        }
        else if (TYPEOF(right) == obj_float_t && TYPEOF(left) == obj_int_t) {
            double f = float_op(AS_FLOAT(right)->f,
                    (double)(AS_INT(left)->i), op);
            result = LuciFloat_new(f);
            return result;
        } else {
            DIE("%s", "Type mismatch in expression\n");
        }
    }
    else {
        /* left and right are of same types */
        switch (TYPEOF(left)) {
            case obj_int_t:
            {
                long i = int_op(AS_INT(left)->i, AS_INT(right)->i, op);
                result = LuciInt_new(i);
                return result;
            } break;

            case obj_float_t:
            {
                double f = float_op(AS_FLOAT(left)->f, AS_FLOAT(right)->f, op);
                result = LuciFloat_new(f);
                return result;
            } break;

            case obj_str_t:
            {
                char *s = string_op(AS_STRING(left), AS_STRING(right), op);
                result = LuciString_new(s);
                return result;
            } break;
            default:
                DIE("%s\n", "Invalid type to binary operand");
                break;
        }
    }
}

/*
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
	    ret->value.string.s = alloc(left->value.string.len +
		    right->value.string.len + 1);
	    strcpy(ret->value.string.s, left->value.string.s);
	    strcat(ret->value.string.s, right->value.string.s);
            ret->value.string.len = strlen(ret->value.string.s);
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

    if ((right->type == obj_int_t && right->value.i == 0) ||
	(right->type == obj_float_t && right->value.f == 0.0)) {
	    DIE("%s", "Divide by zero error\n");
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
	    r = !(strcmp(left->value.string.s, right->value.string.s));
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
	    ret->value.f = !right->value.f;
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
*/
