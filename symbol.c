#include "symbol.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* global singleton for a NONE object */
luci_obj_t *NONE_OBJ;


luci_obj_t *luci_sum(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer + right->value.integer;
    return ret;
}

luci_obj_t *luci_diff(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer - right->value.integer;
    return ret;
}
luci_obj_t *luci_prod(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer * right->value.integer;
    return ret;
}
luci_obj_t *luci_div(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer / right->value.integer;
    return ret;
}
luci_obj_t *luci_lt(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer < right->value.integer;
    return ret;
}
luci_obj_t *luci_gt(luci_obj_t *left, luci_obj_t *right)
{
    assert(left->type == obj_int_t);
    assert(left->type == obj_int_t);
    luci_obj_t *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.integer = left->value.integer > right->value.integer;
    return ret;
}
