#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct Object
{
    enum { obj_int_t, obj_string_t } type;
    union
    {
	int integer;
	char *string;
    } value;
} Object;

typedef Object (*luci_func_t) (Object);

typedef struct Symbol
{
    enum { sym_int_t, sym_string_t, sym_func_t } type;
    char *name;
    union
    {
	int integer;
	char *string;
	luci_func_t function;  /* value of a func */
    } data;
    struct Symbol *next;
} Symbol;

#endif
