/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "luci.h"
#include "builtin.h"
#include "object.h"

static int close_file(FILE *fp)
{
    int ret = 1;
    if (fp) {
	ret = fclose(fp);
	fp = NULL;
    }
    return ret;
}


struct var_def globals[] =
{
    {"stdout", 0},
    {"stderr", 0},
    {"stdin", 0},
    {"e", 0},
    {"pi", 0},
    {0, 0}
};

void init_variables(void)
{
    /* stdout */
    LuciObject *luci_stdout = LuciFile_new(stdout, 0, f_append_m);
    globals[0].object = luci_stdout;

    /* stderr */
    LuciObject *luci_stderr = LuciFile_new(stderr, 0, f_append_m);
    globals[1].object = luci_stderr;

    /* stdin */
    LuciObject *luci_stdin = LuciFile_new(stdin, 0, f_read_m);
    globals[2].object = luci_stdin;

    /* e */
    LuciObject *luci_e = LuciFloat_new(M_E);
    globals[3].object = luci_e;

    /* pi */
    LuciObject *luci_pi = LuciFloat_new(M_PI);
    globals[4].object = luci_pi;
}


const struct func_def builtins[] = {
    {"help", luci_help},
    {"dir", luci_dir},
    {"exit", luci_exit},
    {"print",  luci_print},
    {"input", luci_readline},
    {"readline", luci_readline},
    {"type",  luci_typeof},
    {"assert", luci_assert},
    {"str", luci_cast_str},
    {"int", luci_cast_int},
    {"float", luci_cast_float},
    {"open", luci_fopen},
    {"close", luci_fclose},
    {"read", luci_fread},
    {"write", luci_fwrite},
    {"readlines", luci_flines},
    {"range", luci_range},
    {"sum", luci_sum},
    {"len", luci_len},
    {"max", luci_max},
    {"min", luci_min},
    {0, 0}
};

LuciObject *luci_help(LuciObject **args, unsigned int c)
{
    int width = 32;

    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    printf("              HELP               \n");
    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    printf("        BUILTIN FUNCTIONS        \n");
    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

    int i, len, f, l, j;
    for (i = 0; builtins[i].name != 0; i++)
    {
	len = strlen(builtins[i].name);
	f = (width - len) / 2;
	l = width - f;
	for (j = 0; j < f; j++)
	    printf(" ");
	printf("%s", builtins[i].name);
	for (j = 0; j < l; j++)
	    printf(" ");
	printf("\n");
    }
    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

    return NULL;
}

/* print all names in scope... */
LuciObject *luci_dir(LuciObject **args, unsigned int c)
{
    return NULL;
}

LuciObject *luci_exit(LuciObject **args, unsigned int c)
{
    exit(EXIT_SUCCESS);
    return NULL;
}

LuciObject *luci_print(LuciObject **args, unsigned int c)
{
    int i;
    if (c > 0) {
        print_object(args[0]);
    }
    for (i = 1; i < c; i++) {
	printf(" ");
	print_object(args[i]);
    }
    printf("\n");

    return NULL;
}

LuciObject *luci_readline(LuciObject **args, unsigned int c)
{
    size_t lenmax = 64, len = 0;
    int ch;
    FILE *read_from = NULL;
    char *input;

    if (c < 1) {
	LUCI_DEBUG("%s\n", "readline from stdin");
	read_from = stdin;
    }
    else {
	LuciObject *item = args[0];
	if (item && (item->type == obj_file_t)) {
	    LUCI_DEBUG("%s\n", "readline from file");
	    read_from = AS_FILE(item)->ptr;
	}
	else {
	    DIE("%s", "Can't readline from non-file object\n");
	}
    }

    input = alloc(lenmax * sizeof(char));
    if (input == NULL) {
	DIE("%s", "Failed to allocate buffer for reading stdin\n");
    }
    do {
	ch = fgetc(read_from);

	if (len >= lenmax) {
	    lenmax = lenmax << 1;
	    if ((input = realloc(input, lenmax * sizeof(char))) == NULL) {
		free (input);
		DIE("%s", "Failed to allocate buffer for reading\n");
	    }
	}
	input[len++] = (char)ch;
    } while (ch != EOF && ch != '\n');

    if (ch == EOF) {
	free(input);
	LUCI_DEBUG("%s\n", "readline at EOF, returning NULL");
	return NULL;
    }

    /* overwrite the newline or EOF char with a NUL terminator */
    input[--len] = '\0';
    LuciObject *ret = LuciString_new(input);

    /* destroy the input buffer */
    free(input);

    LUCI_DEBUG("Read line\n", AS_STRING(ret)->s);

    return ret;
}

LuciObject *luci_typeof(LuciObject **args, unsigned int c)
{
    char *which;
    LuciObject *ret = NULL;

    if (c < 1) {
	DIE("%s", "Missing parameter to type()\n");
    }

    /* grab the first parameter from the param list */
    LuciObject *item = args[0];
    if (!item) {
	which = "None";
    }
    else {
	switch(item->type)
	{
	    case obj_int_t:
		which = "int";
		break;
	    case obj_float_t:
		which = "float";
		break;
	    case obj_str_t:
		which = "string";
		break;
	    case obj_file_t:
		which = "file";
		break;
	    case obj_list_t:
		which = "list";
		break;
            case obj_map_t:
                which = "map";
                break;
            case obj_func_t:
                which = "function";
                break;
            case obj_libfunc_t:
                which = "libfunction";
                break;
	    default:
		which = "None";
	}
    }

    ret = LuciString_new(strdup(which));
    return ret;
}

LuciObject *luci_assert(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing condition parameter to assert()\n");
    }

    LuciObject *item = args[0];

    switch(item->type)
    {
	case obj_int_t:
	    assert(AS_INT(item)->i);
	    break;
	case obj_float_t:
	    assert((int)AS_FLOAT(item)->f);
	    break;
	case obj_str_t:
	    assert(strcmp("", AS_STRING(item)->s) != 0);
	    break;
	case obj_list_t:
	    assert(AS_LIST(item)->count); /* assert that it isn't empty? */
	    break;
	default:
	    ;
    }
    return NULL;
}

LuciObject *luci_cast_int(LuciObject **args, unsigned int c)
{
    LuciObject *ret = NULL;
    if (c < 1) {
	DIE("%s", "Missing parameter to int()\n");
    }
    LuciObject *item = args[0];

    if (!item) {
	DIE("%s", "Can't cast NULL to int\n");
    }

    switch (item->type) {
	case obj_int_t:
            ret = LuciInt_new(AS_INT(item)->i);
	    break;
	case obj_float_t:
            ret = LuciInt_new((long)AS_FLOAT(item)->f);
	    break;
	case obj_str_t:
        {
            long i;
	    int scanned = sscanf(AS_STRING(item)->s, "%ld", &i);
	    if (scanned <= 0 || scanned == EOF) {
		DIE("%s", "Could not cast to int\n");
	    }
            ret = LuciInt_new(i);
	    break;
        }
	default:
	    DIE("%s", "Could not cast to int\n");
    }
    return ret;
}

LuciObject *luci_cast_float(LuciObject **args, unsigned int c)
{
    LuciObject *ret = NULL;
    if (c < 1) {
	DIE("%s", "Missing parameter to int()\n");
    }
    LuciObject *item = args[0];

    if (!item) {
	DIE("%s", "Can't cast NULL to int\n");
    }

    switch (item->type) {
	case obj_int_t:
            ret = LuciFloat_new((double)AS_INT(item)->i);
	    break;
	case obj_float_t:
            ret = LuciFloat_new(AS_FLOAT(item)->f);
	    break;
	case obj_str_t:
        {
            double f;
	    int scanned = sscanf(AS_STRING(item)->s, "%f", (float *)&f);
	    if (scanned <= 0 || scanned == EOF) {
		DIE("%s", "Could not cast to float\n");
	    }
            ret = LuciFloat_new(f);
	    break;
        }
	default:
	    DIE("%s", "Could not cast to float\n");
    }

    return ret;
}

LuciObject *luci_cast_str(LuciObject **args, unsigned int c)
{
    LuciObject *ret = NULL;
    char *s = NULL;

    if (c < 1) {
	DIE("%s", "Missing parameter to str()\n");
    }
    /* grab the first parameter from the param list */
    LuciObject *item = args[0];

    switch (item->type)
    {
	case obj_int_t:
	    s = alloc(32);
	    sprintf(s, "%ld", AS_INT(item)->i);
	    /* ret->s[16] = '\0'; */
            ret = LuciString_new(s);
	    break;
	case obj_float_t:
	    s = alloc(32);
	    sprintf(s, "%f", (float)AS_FLOAT(item)->f);
	    /* AS_STRING(ret)->s[16] = '\0'; */
            ret = LuciString_new(s);
	    break;
	case obj_str_t:
	    s = alloc(AS_STRING(item)->len + 1);
	    strcpy(s, AS_STRING(item)->s);
            ret = LuciString_new(s);
	    break;
	default:
            DIE("%s\n", "Cannot cast to string");
	    break;
    }
    LUCI_DEBUG("str() returning %s\n", AS_STRING(ret)->s);

    return ret;
}

static int get_file_mode(const char *req_mode)
{
    if (strcmp(req_mode, "r") == 0) {
	return f_read_m;
    }
    else if (strcmp(req_mode, "w") == 0) {
	return f_write_m;
    }
    else if (strcmp(req_mode, "a") == 0) {
	return f_append_m;
    }
    else {
	return -1;
    }
}

LuciObject *luci_fopen(LuciObject **args, unsigned int c)
{
    char *filename;
    char *req_mode;
    int mode;
    FILE *file = NULL;

    if (c < 2) {
	DIE("%s", "Missing parameter to open()\n");
    }

    LuciObject *fname_obj = args[0];
    if (fname_obj->type != obj_str_t) {
	DIE("%s", "Parameter 1 to open must be a string\n");
    }
    LuciObject *mode_obj = args[1];
    if (mode_obj->type != obj_str_t) {
	DIE("%s", "Parameter 2 to open must be a string\n");
    }

    filename = AS_STRING(fname_obj)->s;
    req_mode = AS_STRING(mode_obj)->s;

    mode = get_file_mode(req_mode);
    if (mode < 0) {
	DIE("%s\n", "Invalid mode to open()");
    }

    /*
       Open in read-binary mode and fseek to SEEK_END to
       calculate the file's size in bytes.
       Then close it and reopen it the way the user requests
    */
    long file_length;
    /* store the FILE's byte length */
    if (!(file = fopen(filename, "rb"))) {
	file_length = 0;
    }
    else {
	fseek(file, 0, SEEK_END);
	file_length = ftell(file);
	fseek(file, 0, SEEK_SET);
	close_file(file);
    }

    if (!(file = fopen(filename, req_mode)))
    {
	DIE("Could not open file %s\n", filename);
    }

    LuciObject *ret = LuciFile_new(file, file_length, mode);

    LUCI_DEBUG("Opened file %s of size %ld bytes with mode %s.\n",
	    filename, file_length, req_mode);

    return ret;
}

LuciObject *luci_fclose(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to close()\n");
    }

    LuciObject *fobj = args[0];

    if (!(fobj->type == obj_file_t)) {
	DIE("%s", "Not a file object\n");
    }

    if (AS_FILE(fobj)->ptr) {
	close_file(AS_FILE(fobj)->ptr);
    }
    /* else, probably already closed (it's NULL) */

    LUCI_DEBUG("%s\n", "Closed file object.");

    return NULL;
}

LuciObject *luci_fread(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to read()\n");
    }

    LuciObject *fobj = args[0];

    if (!(fobj->type == obj_file_t)) {
	DIE("%s", "Not a file object\n");
    }

    if (AS_FILE(fobj)->mode != f_read_m) {
	DIE("%s", "Can't open  It is opened for writing.\n");
    }

    /* seek to file start, we're gonna read the whole thing */
    /* fseek(fobj->ptr, 0, SEEK_SET); */

    long len = AS_FILE(fobj)->size;
    char *read = alloc(len + 1);
    fread(read, sizeof(char), len, AS_FILE(fobj)->ptr);
    read[len] = '\0';

    /* fseek(fobj->ptr, 0, SEEK_SET); */

    LuciObject *ret = LuciString_new(read);

    return ret;
}

LuciObject *luci_fwrite(LuciObject **args, unsigned int c)
{
    if (c < 2) {
	DIE("%s", "Missing parameter to write()\n");
    }

    /* grab the FILE parameter */
    LuciObject *fobj = args[0];
    if (!fobj || (fobj->type != obj_file_t)) {
	DIE("%s", "Not a file object\n");
    }

    /* grab string parameter */
    LuciObject *text_obj = args[1];
    if (!text_obj || (text_obj->type != obj_str_t) ) {
	DIE("%s", "Not a string\n");
    }
    char *text = AS_STRING(text_obj)->s;

    if (AS_FILE(fobj)->mode == f_read_m) {
	DIE("%s", "Can't write to  It is opened for reading.\n");
    }

    fwrite(text, sizeof(char), strlen(text), AS_FILE(fobj)->ptr);
    return NULL;
}

LuciObject *luci_flines(LuciObject **args, unsigned int c)
{
    LuciObject *list = LuciList_new();
    LuciObject *line = luci_readline(args, c);
    while (line) {
	list_append_object(list, line);
	line = luci_readline(args, c);
    }

    return list;
}

LuciObject * luci_range(LuciObject **args, unsigned int c)
{
    int start, end, incr;
    LuciObject *first, *second, *third;

    if (c < 1) {
	DIE("%s", "Missing parameter to range()\n");
    }

    first = args[0];
    if (first->type != obj_int_t) {
	DIE("%s", "First parameter to range must be integer\n");
    }

    if (c > 1) {
	second = args[1];
	if (second->type != obj_int_t) {
	    DIE("%s", "Second parameter to range must be integer\n");
	}
	start = AS_INT(first)->i;
	end = AS_INT(second)->i;

	if (c > 2) {
            /* Ternary range(X, Y, Z) call */
	    third = args[2];
	    if (third->type != obj_int_t) {
		DIE("%s", "Third parameter to range must be integer\n");
	    }
	    incr = AS_INT(third)->i;
	}
	else {
	    incr = 1;
	}
    }
    else {
        /* Basic range(X) call, increment by 1 starting from 0 */
	start = 0;
	end = AS_INT(first)->i;
	incr = 1;
    }

    /* Build a list of integers from start to end, incrementing by incr */
    LuciObject *item, *list = LuciList_new();
    int i;
    if (incr < 0) {
        if (start <= end) {
            /* return empty list if idiotically requested */
            return list;
        }

        for (i = start; i > end; i += incr) {
            item = LuciInt_new(i);
            list_append_object(list, item);
        }
    }
    else {
        if (start >= end) {
            /* return empty list if idiotically requested */
            return list;
        }

        for (i = start; i < end; i += incr) {
            item = LuciInt_new(i);
            list_append_object(list, item);
        }
    }

    return list;
}

LuciObject * luci_sum(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to sum()\n");
    }

    LuciObject *list = args[0];

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Must specify a list to calculate sum\n");
    }

    LuciObject *item;
    double sum = 0;
    int i, found_float = 0;
    for (i = 0; i < AS_LIST(list)->count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    DIE("%s", "Can't calulate sum of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		sum += (double)AS_INT(item)->i;
		break;
	    case obj_float_t:
		found_float = 1;
		sum += AS_FLOAT(item)->f;
		break;
	    default:
		DIE("%s", "Can't calculate sum of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = LuciInt_new((long)sum);
    }
    else {
	ret = LuciFloat_new(sum);
    }

    return ret;
}

LuciObject *luci_len(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to len()\n");
    }

    LuciObject *list = args[0];

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Must specify a list to calculate len\n");
    }

    LuciObject *ret = LuciInt_new(AS_LIST(list)->count);
    return ret;
}

LuciObject *luci_max(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to max()\n");
    }

    LuciObject *list = args[0];

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Must specify a list to calculate max\n");
    }

    LuciObject *item;
    double max = 0;
    int i, found_float = 0;
    for (i = 0; i < AS_LIST(list)->count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    DIE("%s", "Can't calulate max of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		if ( (double)AS_INT(item)->i > max) {
		    max = (double)AS_INT(item)->i;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (AS_FLOAT(item)->f > max) {
		    max = AS_FLOAT(item)->f;
		}
		break;
	    default:
		DIE("%s", "Can't calculate max of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = LuciInt_new((long)max);
    }
    else {
	ret = LuciFloat_new(max);
    }
    return ret;
}

LuciObject *luci_min(LuciObject **args, unsigned int c)
{
    if (c < 1) {
	DIE("%s", "Missing parameter to min()\n");
    }

    LuciObject *list = args[0];

    if (!list || (list->type != obj_list_t)) {
	DIE("%s", "Must specify a list to calculate min\n");
    }

    LuciObject *item;
    double min = 0;
    int i, found_float = 0;

    for (i = 0; i < AS_LIST(list)->count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    DIE("%s", "Can't calulate max of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		if (i == 0) {
		    min = (double)AS_INT(item)->i;
		}
		else if ( (double)AS_INT(item)->i < min) {
		    min = (double)AS_INT(item)->i;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (i == 0) {
		    min = AS_FLOAT(item)->f;
		}
		else if (AS_FLOAT(item)->f < min) {
		    min = AS_FLOAT(item)->f;
		}
		break;
	    default:
		DIE("%s", "Can't calculate min of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = LuciInt_new((long)min);
    }
    else {
	ret = LuciFloat_new(min);
    }
    return ret;
}

