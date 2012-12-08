#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "luci.h"
#include "common.h"
#include "object.h"

/* temporary */
#include "compile.h" /* for destroying function object FOR NOW */


LuciObject *create_object(int type)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = type;
    ret->refcount = 0;
    switch(type)
    {
	case obj_str_t:
	    ret->value.s = NULL;
	    break;

	case obj_file_t:
	    ret->value.file.ptr = NULL;
	    break;

	case obj_list_t:
	    ret->value.list.count = 0;
	    ret->value.list.size = INIT_LIST_SIZE;
	    ret->value.list.items = alloc(ret->value.list.size *
		    sizeof(*ret->value.list.items));
	    break;

        case obj_iterator_t:
            ret->value.iterator.list = NULL;
            ret->value.iterator.idx = 0;
            ret->value.iterator.incr = 1; /* important */
            break;

	default:
	    break;
    }
    LUCI_DEBUG("Creating obj @ %lu with type %d\n",
            (unsigned long) ret, ret->type);
    return ret;
}

LuciObject *incref(LuciObject *orig)
{
    if (!orig)
        return NULL;
        /*die("Attempt to incref NULL\n"); */

    orig->refcount ++;

    return orig;
}

LuciObject *decref(LuciObject *orig)
{
    if (!orig)
        return NULL;
        /*
        die("Attempt to decref NULL\n");
        */

    orig->refcount --;

    if (orig->refcount < 1) {
        destroy(orig);
        return NULL;
    }

    return orig;
}

LuciObject *copy_object(LuciObject *orig)
{
    int i;
    LuciObject *copy = NULL;

    if (!orig) {
	return NULL;
    }

    /* create the initial copy with only its type specified */
    /* copy will have a refcount of 0 */
    copy = create_object(orig->type);

    switch(orig->type)
    {
	case obj_int_t:
	    copy->value.i = orig->value.i;
	    break;
	case obj_float_t:
	    copy->value.f = orig->value.f;
	    break;
	case obj_str_t:
	    copy->value.s = alloc(strlen(orig->value.s) + 1);
	    strcpy(copy->value.s, orig->value.s);
	    break;
	case obj_file_t:
	    copy->value.file.ptr = orig->value.file.ptr;
	    copy->value.file.size = orig->value.file.size;
	    copy->value.file.mode = orig->value.file.mode;
	    break;
	case obj_list_t:
	    for (i = 0; i < orig->value.list.count; i++) {
		list_append_object(copy, list_get_object(orig, i));
	    }
	    break;
        case obj_iterator_t:
            copy->value.iterator.list = orig->value.iterator.list;
            copy->value.iterator.idx = orig->value.iterator.idx;
            copy->value.iterator.incr = orig->value.iterator.incr;
            break;
        case obj_func_t:
            copy->value.func.frame = orig->value.func.frame;
            copy->value.func.deleter = orig->value.func.deleter;
            break;
        case obj_libfunc_t:
            copy->value.libfunc = orig->value.libfunc;
            break;
	default:
	    break;
    }
    return copy;
}

void destroy(LuciObject *trash)
{
    if (!trash) {
        LUCI_DEBUG("%s\n", "Destroying a NULL object...");
        return;
    }

    if (trash->refcount > 0) {
        /* Attempts to destroy a referenced obj are ignored */
        LUCI_DEBUG("Rejecting destruction of object (refcount %d)\n", trash->refcount);
        return;
    }

    int i;
    switch(trash->type) {

        case obj_file_t:
            if (trash->value.file.ptr) {
                /* TODO: method of closing only open files */
                /* LUCI_DEBUG("%s\n", "Closing file object"); */
                /* close_file(trash->value.file.ptr); */
            }
            break;

        case obj_str_t:
            LUCI_DEBUG("Freeing string %s\n", trash->value.s);
            free(trash->value.s);
            trash->value.s = NULL;
            break;

        case obj_list_t:
            for (i = 0; i < trash->value.list.count; i++) {
                destroy(trash->value.list.items[i]);
            }
            free(trash->value.list.items);
            break;

        case obj_iterator_t:
            /* destroy the list if possible */
            destroy(trash->value.iterator.list);
            break;

        case obj_func_t:
            Frame_delete(trash->value.func.frame);
            break;

        default:
            break;
    }
    LUCI_DEBUG("Destroying obj @ %lu with type %d\n",
            (unsigned long) trash, trash->type);

    /* destroy the LuciObject itself */
    free(trash);
    trash = NULL;
}

int list_append_object(LuciObject *list, LuciObject *item)
{
    if (!list || (list->type != obj_list_t)) {
	die("Can't append item to non-list object\n");
    }
    assert(list->type == obj_list_t);

    if (list->value.list.count > list->value.list.size) {
	list->value.list.size = list->value.list.size << 1;
	/* realloc the list array */
	list->value.list.items = realloc(list->value.list.items,
		list->value.list.size * sizeof(*list->value.list.items));
	LUCI_DEBUG("%s\n", "Reallocated space for list");
    }
    /* increment count after appending object */
    list->value.list.items[list->value.list.count ++] = item;
    return 1;
}

/*
 * Returns a COPY of the object in the list at the index
 */
LuciObject *list_get_object(LuciObject *list, int index)
{
    if (!list || (list->type != obj_list_t)) {
	die("Can't iterate over non-list object\n");
    }
    assert(list->type == obj_list_t);

    /* convert negative indices to a index starting from list end */
    while (index < 0) {
	index = list->value.list.count - abs(index);
    }

    if (index >= list->value.list.count) {
	die("List index out of bounds\n");
	/* return NULL; */
    }
    return copy_object(list->value.list.items[index]);
}

LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index)
{
    if (!item) {
	die("Can't set list item to NULL\n");
    }
    assert(list->type == obj_list_t);

    while (index < 0) {
	index = list->value.list.count = abs(index);
    }
    /* list_get_object will take care of any more error handling */
    LuciObject *old = list_get_object(list, index);
    list->value.list.items[index] = item;
    return old;
}

LuciObject *iterator_next_object(LuciObject *iter)
{
    LuciObject *list;
    uint32_t idx;

    if (!iter) {
        die("Can't get next from NULL iter\n");
    }
    assert(iter->type == obj_iterator_t);

    idx = iter->value.iterator.idx;
    list = iter->value.iterator.list;

    if (iter->value.iterator.idx >= list->value.list.count) {
        return NULL;
    } else {
        iter->value.iterator.idx += iter->value.iterator.incr;
        return copy_object(list->value.list.items[idx]);
    }
}

