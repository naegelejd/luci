/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "luci.h"
#include "object.h"
#include "binop.h"


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

LuciObject *string_op(LuciStringObj *l, LuciStringObj *r, int op)
{
    switch (op) {
        case op_add_t:
        {
	    char *s = alloc(l->len + r->len + 1);
	    strcpy(s, l->s);
	    strcat(s, r->s);
            return LuciString_new(s);
        } break;

        case op_eq_t:
        {
            if ((l->len == r->len) &&
                    (strcmp(l->s, r->s) == 0)) {
                return LuciInt_new(1);
            } else {
                return LuciInt_new(0);
            }
        }

        default:
            DIE("%s\n", "Invalid operand to type string");
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
                result = string_op(AS_STRING(left), AS_STRING(right), op);
                return result;
            } break;
            default:
                DIE("%s\n", "Invalid type to binary operand");
                break;
        }
    }
}

