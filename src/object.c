/*
 * See Copyright Notice in luci.h
 */

/**
 * @file object.c
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "luci.h"
#include "gc.h"
#include "object.h"
#include "map.h"

/* temporary */
#include "compile.h" /* for destroying function object FOR NOW */


/**
 * Creates a new LuciIntObj
 *
 * @param l long integer value
 * @returns new LuciIntObj
 */
LuciObject *LuciInt_new(long l)
{
    LuciIntObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_int_t;
    o->i = l;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFloatObj
 *
 * @param d double floating-point value
 * @returns new LuciFloatObj
 */
LuciObject *LuciFloat_new(double d)
{
    LuciFloatObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_float_t;
    o->f = d;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciStringObj
 *
 * @param s C-string value
 * @returns new LuciStringObj
 */
LuciObject *LuciString_new(char *s)
{
    LuciStringObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_str_t;
    o->s = s;   /* not a copy! */
    o->len = strlen(o->s);
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFileObj
 *
 * @param fp C file pointer
 * @param size length of file
 * @param mode enumerated file mode
 * @returns new LuciFileObj
 */
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode)
{
    LuciFileObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_file_t;
    o->ptr = fp;
    o->mode = mode;
    o->size = size;
    return (LuciObject *)o;
}

/**
 * Creates a new, empty LuciListObj
 *
 * @returns new empty LuciListObj
 */
LuciObject *LuciList_new()
{
    LuciListObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_list_t;
    o->count = 0;
    o->size = INIT_LIST_SIZE;
    o->items = alloc(o->size * sizeof(*o->items));
    return (LuciObject *)o;
}

/**
 * Creates a new LuciIteratorObj
 *
 * @param container container to iterate over
 * @param step iteration increment size
 * @returns new LuciIteratorObj
 */
LuciObject *LuciIterator_new(LuciObject *container, unsigned int step)
{
    LuciIteratorObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_iterator_t;
    o->idx = 0;
    o->step = step;
    o->container = container;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFunctionObj
 *
 * @param frame Frame struct defining function
 * @returns new LuciFunctionObj
 */
LuciObject *LuciFunction_new(void *frame)
{
    LuciFunctionObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_func_t;
    o->frame = frame;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciLibFuncObj
 *
 * @param func C function pointer matching LuciLibFunc signature
 * @returns new LuciLibFuncObj
 */
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int))
{
    LuciLibFuncObj *o = gc_malloc(sizeof(*o));
    TYPEOF(o) = obj_libfunc_t;
    o->func = func;
    return (LuciObject *)o;
}

/*
LuciObject *create_object(int type)
{
    LuciObject *ret = gc_malloc(sizeof(*ret));
    ret->type = type;
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

/**
 * Prints a LuciObject to stdout.
 *
 * @param in LuciObject to print
 */
void print_object(LuciObject *in)
{
    int i;
    LuciObject *item;

    if (!in) {
	printf("None");
	return;
    }

    switch (in->type) {
	case obj_int_t:
	    printf("%ld", AS_INT(in)->i);
	    break;

	case obj_float_t:
	    printf("%f", AS_FLOAT(in)->f);
	    break;

	case obj_str_t:
	    printf("%s", AS_STRING(in)->s);
	    break;

	case obj_list_t:
	    printf("[");
	    for (i = 0; i < AS_LIST(in)->count; i++) {
		item = list_get_object(in, i);
		print_object(item);
		printf(", ");
	    }
	    printf("]");
	    break;

        case obj_map_t:
            printf("{");
            for (i = 0; i < AS_MAP(in)->size; i++) {
                if (AS_MAP(in)->keys[i]) {
                    printf("\"");
                    print_object(AS_MAP(in)->keys[i]);
                    printf("\":");
                    print_object(AS_MAP(in)->vals[i]);
                    printf(", ");
                }
            }
            printf("}");
            break;

        case obj_file_t:
            printf("<file>");
            break;

        case obj_func_t:
            printf("<func>");
            break;

        case obj_libfunc_t:
            printf("<libfunc>");
            break;

	default:
            DIE("%s\n", "Can't print invalid type");
    }
}

/**
 * Performs a deep copy of a LuciObject.
 *
 * @param orig LuciObject to copy
 * @returns deep copy of orig
 */
LuciObject *copy_object(LuciObject *orig)
{
    LuciObject *copy = NULL;

    if (!orig) {
	return NULL;
    }

    switch(orig->type)
    {
	case obj_int_t:
            copy = LuciInt_new(((LuciIntObj *)orig)->i);
	    break;

	case obj_float_t:
            copy = LuciFloat_new(((LuciFloatObj *)orig)->f);
	    break;

	case obj_str_t:
            /* duplicate string first */
            copy = LuciString_new(strdup(((LuciStringObj *)orig)->s));
	    break;

        case obj_file_t:
        {
            LuciFileObj *fileobj = (LuciFileObj *)orig;
            copy = LuciFile_new(fileobj->ptr, fileobj->size, fileobj->mode);
	    break;
        }

	case obj_list_t:
        {
            LuciListObj *listobj = (LuciListObj *)orig;
            int i;

            copy = LuciList_new();

	    for (i = 0; i < listobj->count; i++) {
		list_append_object(copy, list_get_object(orig, i));
	    }
	    break;
        }

        case obj_map_t:
        {
            LuciMapObj *mapobj = (LuciMapObj *)orig;
            int i;

            copy = LuciMap_new();

            for (i = 0; i < mapobj->size; i++) {
                if (mapobj->keys[i]) {
                    map_set(copy,
                            copy_object(mapobj->keys[i]),
                            copy_object(mapobj->vals[i]));
                }
            }
            break;
        }

        case obj_iterator_t:
        {
            LuciIteratorObj *iterobj = (LuciIteratorObj *)orig;
            copy = LuciIterator_new(iterobj->container, iterobj->step);
            break;
        }

        case obj_func_t:
            copy = LuciFunction_new(((LuciFunctionObj *)orig)->frame);
            break;

        case obj_libfunc_t:
            copy = LuciLibFunc_new(((LuciLibFuncObj *)orig)->func);
            break;

	default:
            ;
    }

    return copy;
}


/**
 * Deallocates and destroys a LuciObject.
 *
 * @param trash LuciObject to destroy
 */
void destroy(LuciObject *trash)
{
    if (!trash) {
        LUCI_DEBUG("%s\n", "Destroying a NULL object...");
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

        case obj_map_t:
        {
            LuciMapObj *mapobj = (LuciMapObj *)trash;
            for (i = 0; i < mapobj->size; i++) {
                if (mapobj->keys[i]) {
                    destroy(mapobj->keys[i]);
                    destroy(mapobj->vals[i]);
                }
            }
            free(mapobj->keys);
            free(mapobj->vals);
            break;
        }

        case obj_iterator_t:
            /* destroy the list if possible */
            destroy(((LuciIteratorObj *)trash)->container);
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

/**
 * Computes a hash of a LuciStringObj
 *
 * djb2 (hash(i) = hash(i - 1) * 33 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_0(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 5381;
    int c;

    while ((c = *str++))
        h = ((h << 5) + h) + c;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * sdbm (hash(i) = hash(i - 1) * 65599 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_1(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++))
        h = c + (h << 6) + (h << 16) - h;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * One-at-a-time (Bob Jenkins)
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_2(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++)) {
        h += c;
        h += h << 10;
        h ^= h >> 6;
    }
    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    return h;
}

/**
 * Appends a LuciObject to a LuciListObj
 *
 * @param list LuciListObj to append to
 * @param item LuciObject to append to the list
 * @returns 1 on success, 0 otherwise
 */
int list_append_object(LuciObject *list, LuciObject *item)
{
    LuciListObj *listobj = NULL;

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Can't append item to non-list object\n");
    }

    listobj = (LuciListObj *)list;

    if (listobj->count >= listobj->size) {
	listobj->size = listobj->size << 1;
	/* realloc the list array */
	listobj->items = realloc(listobj->items,
		listobj->size * sizeof(*listobj->items));
        if (!listobj->items) {
            DIE("%s", "Failed to dynamically expand list while appending\n");
        }
	LUCI_DEBUG("%s\n", "Reallocated space for list");
    }
    /* increment count after appending object */
    listobj->items[listobj->count++] = item;
    return 1;
}

/**
 * Returns a COPY of the object in the list at the index
 *
 * @param list LuciListObj to grab from
 * @param index index from which to grab object
 * @returns copy of LuciObject at index
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

/**
 * Assigns a LuciObject to a LuciListObj at a given index.
 *
 * @param list LuciListObj to modify
 * @param item LuciObj to insert into the list
 * @param index index at which to store object
 * @returns the object that formerly resided at index
 */
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

/**
 * Returns the next LuciObject in a container.
 *
 * @param iterator from which to compute next object
 * @returns next object in iterator's sequence or NULL if finished iterating
 */
LuciObject *iterator_next_object(LuciObject *iterator)
{
    if (!iterator || (iterator->type != obj_iterator_t)) {
        DIE("%s", "Can't get next from non-iterator object\n");
    }

    LuciIteratorObj *iter = (LuciIteratorObj *)iterator;

    switch (TYPEOF(iter->container)) {
        case obj_list_t:
        {
            LuciListObj *list = (LuciListObj *)iter->container;
            uint32_t idx = iter->idx;   /* save current index */

            if (iter->idx >= list->count) {
                return NULL;
            } else {
                iter->idx += iter->step;
                return copy_object(list->items[idx]);
            }
        } break;

        case obj_map_t:
        {
            LuciMapObj *map = (LuciMapObj *)iter->container;

            if (iter->idx >= map->size) {
                return NULL;
            }

            if (iter->idx == 0) {
                /* be sure we're at a valid 'index' in the map's hash table */
                while ((iter->idx < map->size) &&
                        (map->keys[iter->idx] == NULL)) {
                    iter->idx += 1;
                }
            }

            uint32_t idx = iter->idx;   /* save this valid index */
            do {
                /* increment the index for the next call */
                iter->idx += iter->step;
                /* this will leave the idx at either a valid key
                 * or the last index in the map's key array */
            } while ((iter->idx < map->size) &&
                    (map->keys[iter->idx] == NULL));

            return copy_object(map->keys[idx]);
        } break;

        default:
            DIE("%s\n", "Cannot iterate over a non-container type");
    }
    return NULL;
}

