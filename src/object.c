#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "luci.h"
#include "object.h"
#include "gc.h"

/* temporary */
#include "compile.h" /* for destroying function object FOR NOW */

#define REFCOUNT(o) (((LuciObject *)o)->refcount)
#define TYPEOF(o)   (((LuciObject *)o)->type)


/* LuciIntObj */
LuciObject *LuciInt_new(long l)
{
    LuciIntObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_int_t;
    o->i = l;
    return (LuciObject *)o;
}

/* LuciFloatObj */
LuciObject *LuciFloat_new(double d)
{
    LuciFloatObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_float_t;
    o->f = d;
    return (LuciObject *)o;
}

/* LuciStringObj */
LuciObject *LuciString_new(char *s)
{
    LuciStringObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_str_t;
    o->s = strdup(s);
    o->len = strlen(o->s);
    return (LuciObject *)o;
}

/* LuciFileObj */
LuciObject *LuciFile_new(FILE *fp, long size, int mode)
{
    LuciFileObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_file_t;
    o->ptr = fp;
    o->mode = mode;
    o->size = size;
    return (LuciObject *)o;
}

/* LuciListObj */
LuciObject *LuciList_new()
{
    LuciListObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_list_t;
    o->count = 0;
    o->size = INIT_LIST_SIZE;
    o->items = alloc(o->size * sizeof(*o->items));
    return (LuciObject *)o;
}

/* LuciIteratorObj */
LuciObject *LuciIterator_new(LuciObject *list, unsigned int step)
{
    LuciIteratorObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_iterator_t;
    o->idx = 0;
    o->step = step;
    o->list = list;
    return (LuciObject *)o;
}

/* LuciFunctionObj */
LuciObject *LuciFunction_new(void *frame)
{
    LuciFunctionObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_func_t;
    o->frame = frame;
    return (LuciObject *)o;
}

/* LuciLibFunc */
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int))
{
    LuciLibFuncObj *o = gc_malloc(sizeof(*o));
    REFCOUNT(o) = 0;
    TYPEOF(o) = obj_libfunc_t;
    o->func = func;
    return (LuciObject *)o;
}

/*
LuciObject *create_object(int type)
{
    LuciObject *ret = gc_malloc(sizeof(*ret));
    ret->type = type;
    ret->refcount = 0;
    switch(type)
    {
	case obj_str_t:
	    ret->value.string.s = NULL;
            ret->value.string.len = 0;
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
            ret->value.iterator.step = 1;
            break;

	default:
	    break;
    }
    LUCI_DEBUG("Creating obj @ %lu with type %d\n",
            (unsigned long) ret, ret->type);
    return ret;
}
*/

LuciObject *incref(LuciObject *orig)
{
    if (!orig)
        return NULL;
        /*DIE("%s", "Attempt to incref NULL\n"); */

    orig->refcount ++;

    return orig;
}

LuciObject *decref(LuciObject *orig)
{
    if (!orig)
        return NULL;
        /*
        DIE("%s", "Attempt to decref NULL\n");
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

    if (!orig) {
	return NULL;
    }

    switch(orig->type)
    {
	case obj_int_t:
            return LuciInt_new(((LuciIntObj *)orig)->i);
	    break;
	case obj_float_t:
            return LuciFloat_new(((LuciFloatObj *)orig)->f);
	    break;
	case obj_str_t:
            return LuciString_new(((LuciStringObj *)orig)->s);
	    break;
        case obj_file_t:
        {
            LuciFileObj *fileobj = (LuciFileObj *)orig;
            return LuciFile_new(fileobj->ptr, fileobj->size, fileobj->mode);
	    break;
        }
	case obj_list_t:
        {
            // TODO: FIX THIS WHOLE COPY
            LuciListObj *listobj = (LuciListObj *)orig;
            LuciObject *copy = LuciList_new();

	    for (i = 0; i < listobj->count; i++) {
		list_append_object(copy, list_get_object(orig, i));
	    }
            return copy;
	    break;
        }
        case obj_iterator_t:
        {
            LuciIteratorObj *iterobj = (LuciIteratorObj *)orig;
            return LuciIterator_new(iterobj->list, iterobj->step);
            break;
        }
        case obj_func_t:
            return LuciFunction_new(((LuciFunctionObj *)orig)->frame);
            break;
        case obj_libfunc_t:
            return LuciLibFunc_new(((LuciLibFuncObj *)orig)->func);
            break;
	default:
            return NULL;
	    break;
    }
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
            if (((LuciFileObj *)trash)->ptr) {
                /* TODO: method of closing only open files */
                /* LUCI_DEBUG("%s\n", "Closing file object"); */
                /* close_file(trash->value.file.ptr); */
            }
            break;

        case obj_str_t:
            LUCI_DEBUG("Freeing string %s\n", ((LuciStringObj *)trash)->s);
            free(((LuciStringObj *)trash)->s);
            ((LuciStringObj *)trash)->s = NULL;
            break;

        case obj_list_t:
        {
            LuciListObj *listobj = (LuciListObj *)trash;
            for (i = 0; i < listobj->count; i++) {
                destroy(listobj->items[i]);
            }
            free(listobj->items);
            break;
        }

        case obj_iterator_t:
            /* destroy the list if possible */
            destroy(((LuciIteratorObj *)trash)->list);
            break;

        case obj_func_t:
            Frame_delete(((LuciFunctionObj *)trash)->frame);
            break;

        default:
            break;
    }
    LUCI_DEBUG("Destroying obj @ %lu with type %d\n",
            (unsigned long) trash, trash->type);

    /* destroy the LuciObject itself */
    gc_free(trash);
    trash = NULL;
}

int list_append_object(LuciObject *list, LuciObject *item)
{
    LuciListObj *listobj = NULL;

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Can't append item to non-list object\n");
    }

    listobj = (LuciListObj *)list;

    if (listobj->count > listobj->size) {
	listobj->size = listobj->size << 1;
	/* realloc the list array */
	listobj->items = realloc(listobj->items,
		listobj->size * sizeof(*listobj->items));
	LUCI_DEBUG("%s\n", "Reallocated space for list");
    }
    /* increment count after appending object */
    listobj->items[listobj->count ++] = item;
    return 1;
}

/*
 * Returns a COPY of the object in the list at the index
 */
LuciObject *list_get_object(LuciObject *list, int index)
{
    LuciListObj *listobj = NULL;

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Can't iterate over non-list object\n");
    }

    listobj = (LuciListObj *)list;

    /* convert negative indices to a index starting from list end */
    while (index < 0) {
	index = listobj->count - abs(index);
    }

    if (index >= listobj->count) {
	DIE("%s", "List index out of bounds\n");
	/* return NULL; */
    }
    return copy_object(listobj->items[index]);
}

LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index)
{
    LuciListObj *listobj = NULL;

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Can't iterate over non-list object\n");
    }
    if (!item) {
	DIE("%s", "Can't set list item to NULL\n");
    }

    listobj = (LuciListObj *)list;

    while (index < 0) {
	index = listobj->count = abs(index);
    }
    /* list_get_object will take care of any more error handling */
    LuciObject *old = list_get_object(list, index);
    listobj->items[index] = item;
    return old;
}

LuciObject *iterator_next_object(LuciObject *iterator)
{
    LuciIteratorObj *iter;
    LuciListObj *list;
    uint32_t idx;

    if (!iterator || (iterator->type != obj_iterator_t)) {
        DIE("%s", "Can't get next from non-iterator object\n");
    }

    iter = (LuciIteratorObj *)iterator;

    idx = iter->idx;
    list = (LuciListObj *)iter->list;

    if (iter->idx >= list->count) {
        return NULL;
    } else {
        iter->idx += iter->step;
        return copy_object(list->items[idx]);
    }
}

