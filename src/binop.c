/*
 * See Copyright Notice in luci.h
 */

/**
 * @file binop.c
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "luci.h"
#include "object.h"
#include "binop.h"


/**
 * Evaluates a conditional statement.
 *
 * @param cond a LuciObject that has a boolean equivalent
 * @returns 0 if False, non-zero if True.
*/
static int evaluate_condition(LuciObject *cond)
{
    int huh = 0;

    if (cond == NULL) {
	return 0;
    }

    if (ISTYPE(cond, obj_int_t)) {
        huh = AS_INT(cond)->i;
    } else if (ISTYPE(cond, obj_float_t)) {
        huh = (int)AS_FLOAT(cond)->f;
    } else if (ISTYPE(cond, obj_string_t)) {
        huh = AS_STRING(cond)->len;
    } else if (ISTYPE(cond, obj_list_t)) {
        huh = AS_LIST(cond)->count;
    } else if (ISTYPE(cond, obj_map_t)) {
        huh = AS_MAP(cond)->count;
    } else {
        huh = 1;
    }

    return huh;
}

/**
 * Performs a binary operation on two long integers
 *
 * @param l left-hand long integer
 * @param r right-hand long integer
 * @param op binary operation type
 * @returns long integer result of operation
 */
long int_op(long l, long r, op_type op)
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

/**
 * Performs a binary operation on two double floating-point values
 *
 * @param l left-hand double
 * @param r right-hand double
 * @param op binary operation type
 * @returns double floating-point result of operation
 */
double float_op(double l, double r, op_type op)
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

/**
 * Performs a binary operation on two LuciStringObjs
 *
 * @param l left-hand LuciStringObj
 * @param r right-hand LuciStringObj
 * @param op binary operation type
 * @returns LuciStringObj result of operation
 */
LuciObject *string_op(LuciStringObj *l, LuciStringObj *r, op_type op)
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

/**
 * Performs a binary operation on two LuciObjects
 *
 * Determines the types of the operands and, depending
 * on the types, calls the corresponding binary operation
 * function on both object's values.
 *
 * @param left left-hand LuciObject
 * @param right right-hand LuciObject
 * @param op binary operation type
 * @returns LuciObject result of operation
 */
LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, op_type op)
{
    LuciObject *result = NULL;

    if (!left || !right) {
        /* uh oh */
        /* return NULL; */
        DIE("%s\n", "NULL object in binary expression dispatcher");
    }

    if (!TYPES_MATCH(left, right)) {
        if (ISTYPE(left, obj_float_t) && ISTYPE(right, obj_int_t)) {
            double f = float_op(AS_FLOAT(left)->f,
                    (double)(AS_INT(right)->i), op);
            result = LuciFloat_new(f);
            return result;
        }
        else if (ISTYPE(right, obj_float_t) && ISTYPE(left, obj_int_t)) {
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
        if (ISTYPE(left, obj_int_t)) {
            long i = int_op(AS_INT(left)->i, AS_INT(right)->i, op);
            result = LuciInt_new(i);
            return result;
        } else if (ISTYPE(left, obj_float_t)) {
            double f = float_op(AS_FLOAT(left)->f, AS_FLOAT(right)->f, op);
            result = LuciFloat_new(f);
            return result;
        } else if (ISTYPE(left, obj_string_t)) {
            result = string_op(AS_STRING(left), AS_STRING(right), op);
            return result;
        } else {
            DIE("%s\n", "Invalid type to binary operand");
        }
    }
}

