#ifndef OBJECT_H
#define OBJECT_H

/* initial allocated size of a new List */
#define INIT_LIST_SIZE 32

/* Types of LuciObjects */
typedef enum {
    obj_int_t,
    obj_float_t,
    obj_str_t,
    obj_file_t,
    obj_list_t,
    obj_func_t
} LuciOType;

typedef enum {
    f_read_m,
    f_write_m,
    f_append_m
} LuciFileMode;

typedef union {
    int i_val;
    double f_val;
    char *s_val;
    struct {
        FILE *ptr;
        long size;	/* in bytes */
        LuciFileMode mode;
    } file;
    struct {
        struct LuciObject **items;
        int count;	/* current number of items in list */
        int size;	/* current count of allocated item pointers */
    } list;
    struct LuciObject * (*func)(struct LuciObject *);
} LuciOVal;

/* LuciFunction is a type of function that returns a LuciObject * */
/* typedef struct LuciObject * (*LuciFunction) (struct LuciObject *); */

/* Generic Object which allows for dynamic typing */
typedef struct LuciObject
{
    LuciOType type;
    int refcount;
    LuciOVal value;
} LuciObject;

/* creates and initializes a new LuciObject */
struct LuciObject *create_object(int type);

/* incrementes the object's refcount and returns it */
struct LuciObject *reference_object(struct LuciObject* orig);

/* duplicates a LuciObject, creating a new one */
struct LuciObject *copy_object(struct LuciObject* orig);

/* decrements object's refcount, and potentially destroys it */
void destroy_object(struct LuciObject *trash);

int list_append_object(struct LuciObject *list, struct LuciObject *item);
struct LuciObject *list_get_object(struct LuciObject *list, int index);

#endif
