#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "object.h"

LuciObject *create_object(int type)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = type;
    ret->refcount = 0;
    switch(type)
    {
	case obj_str_t:
	    ret->value.s_val = NULL;
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
	default:
	    break;
    }
    yak("Creating obj @ %lu with type %d\n",
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
    if (!orig)
	return NULL;

    /* create the initial copy with only its type specified */
    LuciObject *copy = create_object(orig->type);
    int i;
    switch(orig->type)
    {
	case obj_int_t:
	    copy->value.i_val = orig->value.i_val;
	    break;
	case obj_float_t:
	    copy->value.f_val = orig->value.f_val;
	    break;
	case obj_str_t:
	    copy->value.s_val = alloc(strlen(orig->value.s_val) + 1);
	    strcpy(copy->value.s_val, orig->value.s_val);
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
        case obj_func_t:
            copy->value.func = orig->value.func;
            break;
	default:
	    break;
    }
    return copy;
}

void destroy(LuciObject *trash)
{
    if (!trash) {
        yak("Destroying a NULL object...\n");
        return;
    }

    if (trash->refcount > 0) {
        /* Attempts to destroy a referenced obj are ignored */
        return;
    }

    int i;
    switch(trash->type) {
        case obj_list_t:
            for (i = 0; i < trash->value.list.count; i++) {
                destroy(trash->value.list.items[i]);
            }
            free(trash->value.list.items);
            break;
        case obj_file_t:
            if (trash->value.file.ptr) {
                /* TODO: method of closing only open files */
                /* yak("Closing file object\n"); */
                /* close_file(trash->value.file.ptr); */
            }
            break;
        case obj_str_t:
            yak("Freeing string %s\n", trash->value.s_val);
            free(trash->value.s_val);
            trash->value.s_val = NULL;
            break;
        default:
            break;
    }
    yak("Destroying obj @ %lu with type %d\n",
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
    if (list->value.list.count > list->value.list.size) {
	list->value.list.size = list->value.list.size << 1;
	/* realloc the list array */
	list->value.list.items = realloc(list->value.list.items,
		list->value.list.size * sizeof(*list->value.list.items));
	yak("Reallocated space for list\n");
    }
    /* increment count after appending object */
    list->value.list.items[list->value.list.count ++] = item;
    return 1;
}

LuciObject *list_get_object(LuciObject *list, int index)
{
    if (!list || (list->type != obj_list_t)) {
	die("Can't iterate over non-list object\n");
    }
    while (index < 0) {
	index = list->value.list.count - abs(index);
    }
    if (index >= list->value.list.count) {
	die("List index out of bounds\n");
	/* return NULL; */
    }
    return list->value.list.items[index];
}

LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index)
{
    if (!item) {
	die("Can't set list item to NULL\n");
    }
    while (index < 0) {
	index = list->value.list.count = abs(index);
    }
    /* list_get_object will take care of any more error handling */
    LuciObject *old = list_get_object(list, index);
    list->value.list.items[index] = item;
    return old;
}


