#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "object.h"
#include "builtin.h"
#include "stack.h"

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
    "stdout", 0,
    "stderr", 0,
    "stdin", 0,
    "e", 0,
    "pi", 0,
    0, 0
};

void init_variables(void)
{
    /* stdout */
    LuciObject *luci_stdout = create_object(obj_file_t);
    luci_stdout->value.file.ptr = stdout;
    luci_stdout->value.file.size = 0;
    luci_stdout->value.file.mode = f_append_m;
    globals[0].object = luci_stdout;

    /* stderr */
    LuciObject *luci_stderr = create_object(obj_file_t);
    luci_stdout->value.file.ptr = stderr;
    luci_stdout->value.file.size = 0;
    luci_stdout->value.file.mode = f_append_m;
    globals[1].object = luci_stderr;

    /* stdin */
    LuciObject *luci_stdin = create_object(obj_file_t);
    luci_stdout->value.file.ptr = stdin;
    luci_stdout->value.file.size = 0;
    luci_stdout->value.file.mode = f_read_m;
    globals[2].object = luci_stdin;

    /* e */
    LuciObject *luci_e = create_object(obj_float_t);
    luci_e->value.f = M_E;
    globals[3].object = luci_e;

    /* pi */
    LuciObject *luci_pi = create_object(obj_float_t);
    luci_pi->value.f = M_PI;
    globals[4].object = luci_pi;
}


const struct func_def builtins[] = {
    "help", luci_help,
    "dir", luci_dir,
    "print",  luci_print,
    "input", luci_readline,
    "readline", luci_readline,
    "type",  luci_typeof,
    "assert", luci_assert,
    "str", luci_cast_str,
    "int", luci_cast_int,
    "float", luci_cast_float,
    "open", luci_fopen,
    "close", luci_fclose,
    "read", luci_fread,
    "write", luci_fwrite,
    "readlines", luci_flines,
    "range", luci_range,
    "sum", luci_sum,
    "len", luci_len,
    "max", luci_max,
    "min", luci_min,
    0, 0
};

LuciObject *luci_help(Stack *lstack, int c)
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

/* HACK: iterates over root ExecContext's symbol table */
LuciObject *luci_dir(Stack *lstack, int c)
{
    /* this is a terrible way of printing all vars in the global
       namespace but it works for now.
    */
    /*
    ExecContext *root_env = get_root_env();

    Symbol *ptr;
    for (ptr = root_env->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next) {
	if (ptr->type == sym_bobj_t) {
	    puts(ptr->name);
	}
    }
    */
    return NULL;
}

void print_object(LuciObject *in)
{
    int i;
    LuciObject *item;
    if (!in)
    {
	printf("None");
	return;
    }
    switch (in->type)
    {
	case obj_int_t:
	    printf("%d", in->value.i);
	    break;
	case obj_float_t:
	    printf("%f", in->value.f);
	    break;
	case obj_str_t:
	    printf("%s", in->value.s);
	    break;
	case obj_list_t:
	    printf("[");
	    for (i = 0; i < in->value.list.count; i++) {
		item = list_get_object(in, i);
		print_object(item);
		printf(", ");
	    }
	    printf("]");
	    break;
	default:
	    printf("None");
    }
}

LuciObject *luci_print(Stack *lstack, int c)
{
    int i;
    LuciObject *item = NULL;
    for (i = 0; i < c; i++) {
	item = st_pop(lstack);
	print_object(item);
	printf(" ");
    }
    printf("\n");

    return NULL;
}

LuciObject *luci_readline(Stack *lstack, int c)
{
    size_t lenmax = 64, len = 0;
    int ch;
    FILE *read_from = NULL;

    if (c < 1) {
	yak("readline from stdin\n");
	read_from = stdin;
    }
    else {
	LuciObject *item = st_pop(lstack);
	if (item && (item->type == obj_file_t)) {
	    yak("readline from file\n");
	    read_from = item->value.file.ptr;
	}
	else {
	    die("Can't readline from non-file object\n");
	}
    }

    char *input = alloc(lenmax * sizeof(char));
    if (input == NULL) {
	die("Failed to allocate buffer for reading stdin\n");
    }
    do {
	ch = fgetc(read_from);

	if (len >= lenmax) {
	    lenmax = lenmax << 1;
	    if ((input = realloc(input, lenmax * sizeof(char))) == NULL) {
		free (input);
		die("Failed to allocate buffer for reading\n");
	    }
	}
	input[len++] = (char)ch;
    } while (ch != EOF && ch != '\n');

    if (ch == EOF) {
	free(input);
	yak("readline at EOF, returning NULL\n");
	return NULL;
    }

    /* overwrite the newline or EOF char with a NUL terminator */
    input[--len] = '\0';

    /* this might be bad practice? */
    /* I'm malloc'ing and copying the input char[] in case
       the size of the input buffer is far larger than it needs to be?
    */
    LuciObject *ret = create_object(obj_str_t);
    ret->value.s = alloc((len + 1)* sizeof(char));
    strncpy(ret->value.s, input, len);
    ret->value.s[len] = '\0';

    /* destroy the input buffer */
    free(input);

    yak("Read line\n", ret->value.s);

    return ret;
}

LuciObject *luci_typeof(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to type()\n");
    }

    char *which;
    LuciObject *ret = create_object(obj_str_t);

    /* grab the first parameter from the param list */
    LuciObject *item = st_pop(lstack);
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
	    default:
		which = "None";
	}
    }
    ret->value.s = alloc(strlen(which) + 1);
    strcpy(ret->value.s, which);

    return ret;
}

LuciObject *luci_assert(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing condition parameter to assert()\n");
    }

    LuciObject *item = st_pop(lstack);

    switch(item->type)
    {
	case obj_int_t:
	    assert(item->value.i);
	    break;
	case obj_float_t:
	    assert((int)item->value.f);
	    break;
	case obj_str_t:
	    assert(strcmp("", item->value.s) != 0);
	    break;
	case obj_list_t:
	    assert(item->value.list.count); /* assert that it isn't empty? */
	    break;
	default:
	    ;
    }
    return NULL;
}

LuciObject *luci_cast_int(Stack *lstack, int c)
{
    LuciObject *ret = NULL;
    if (c < 1) {
	die("Missing parameter to int()\n");
    }
    LuciObject *item = st_pop(lstack);

    if (!item) {
	die("Can't cast NULL to int\n");
    }
    ret = create_object(obj_int_t);
    int scanned = 0;
    switch (item->type) {
	case obj_int_t:
	    ret->value.i = item->value.i;
	    break;
	case obj_float_t:
	    ret->value.i = (int)item->value.f;
	    break;
	case obj_str_t:
	    scanned = sscanf(item->value.s, "%d", &(ret->value.i));
	    if (scanned <= 0 || scanned == EOF) {
		die("Could not cast to int\n");
	    }
	    break;
	default:
	    die("Could not cast to int\n");
    }
    return ret;
}

LuciObject *luci_cast_float(Stack *lstack, int c)
{
    LuciObject *ret = NULL;
    if (c < 1) {
	die("Missing parameter to int()\n");
    }
    LuciObject *item = st_pop(lstack);

    if (!item) {
	die("Can't cast NULL to int\n");
    }
    ret = create_object(obj_float_t);
    int scanned = 0;
    switch (item->type) {
	case obj_int_t:
	    ret->value.f = (double)item->value.i;
	    break;
	case obj_float_t:
	    ret->value.f = item->value.f;
	    break;
	case obj_str_t:
	    scanned = sscanf(item->value.s, "%f", (float *)&(ret->value.f));
	    if (scanned <= 0 || scanned == EOF) {
		die("Could not cast to float\n");
	    }
	    break;
	default:
	    die("Could not cast to float\n");
    }

    return ret;
}

LuciObject *luci_cast_str(Stack *lstack, int c)
{
    LuciObject *ret = NULL;
    if (c < 1) {
	die("Missing parameter to str()\n");
    }
    /* grab the first parameter from the param list */
    LuciObject *item = st_pop(lstack);

    /* allocate our return string object */
    ret = create_object(obj_str_t);
    switch (item->type)
    {
	case obj_int_t:
	    ret->value.s = alloc(32);
	    sprintf(ret->value.s, "%d", item->value.i);
	    /* ret->value.s[16] = '\0'; */
	    break;
	case obj_float_t:
	    ret->value.s = alloc(32);
	    sprintf(ret->value.s, "%f", (float)item->value.f);
	    /* ret->value.s[16] = '\0'; */
	    break;
	case obj_str_t:
	    ret->value.s = alloc(strlen(item->value.s) + 1);
	    strcpy(ret->value.s, item->value.s);
	    break;
	default:
	    break;
    }
    yak("str() returning %s\n", ret->value.s);

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

LuciObject *luci_fopen(Stack *lstack, int c)
{
    char *filename;
    char *req_mode;
    int mode;
    FILE *file = NULL;

    if (c < 2) {
	die("Missing parameter to open()\n");
    }

    LuciObject *fname_obj = st_pop(lstack);
    if (fname_obj->type != obj_str_t) {
	die("Parameter 1 to open must be a string\n");
    }
    LuciObject *mode_obj = st_pop(lstack);
    if (mode_obj->type != obj_str_t) {
	die("Parameter 2 to open must be a string\n");
    }

    filename = fname_obj->value.s;
    req_mode = mode_obj->value.s;

    mode = get_file_mode(req_mode);
    if (mode < 0) {
	die("Invalid file open mode: %d\n", mode);
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
	die("Could not open file %s\n", filename);
    }

    LuciObject *ret = create_object(obj_file_t);
    ret->value.file.ptr = file;
    ret->value.file.mode = mode;
    ret->value.file.size = file_length;

    yak("Opened file %s of size %ld bytes with mode %s.\n",
	    filename, file_length, req_mode);

    return ret;
}

LuciObject *luci_fclose(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to close()\n");
    }

    LuciObject *fobj = st_pop(lstack);

    if (!(fobj->type == obj_file_t)) {
	die("Not a file object\n");
    }

    if (fobj->value.file.ptr) {
	close_file(fobj->value.file.ptr);
    }
    /* else, probably already closed (it's NULL) */

    yak("Closed file object.\n");

    return NULL;
}

LuciObject *luci_fread(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to read()\n");
    }

    LuciObject *fobj = st_pop(lstack);

    if (!(fobj->type == obj_file_t)) {
	die("Not a file object\n");
    }

    if (fobj->value.file.mode != f_read_m) {
	die("Can't open file. It is opened for writing.\n");
    }

    /* seek to file start, we're gonna read the whole thing */
    /* fseek(fobj->value.file.ptr, 0, SEEK_SET); */

    long len = fobj->value.file.size;
    char *read = alloc(len + 1);
    fread(read, sizeof(char), len, fobj->value.file.ptr);
    read[len] = '\0';

    /* fseek(fobj->value.file.ptr, 0, SEEK_SET); */

    LuciObject *ret = create_object(obj_str_t);
    ret->value.s = read;

    return ret;
}

LuciObject *luci_fwrite(Stack *lstack, int c)
{
    if (c < 2) {
	die("Missing parameter to write()\n");
    }

    /* grab the FILE parameter */
    LuciObject *fobj = st_pop(lstack);
    if (!fobj || (fobj->type != obj_file_t)) {
	die("Not a file object\n");
    }

    /* grab string parameter */
    LuciObject *text_obj = st_pop(lstack);
    if (!text_obj || (text_obj->type != obj_str_t) ) {
	die("Not a string\n");
    }
    char *text = text_obj->value.s;

    if (fobj->value.file.mode == f_read_m) {
	die("Can't write to file. It is opened for reading.\n");
    }

    fwrite(text, sizeof(char), strlen(text), fobj->value.file.ptr);
    return NULL;
}

LuciObject *luci_flines(Stack *lstack, int c)
{
    LuciObject *list = create_object(obj_list_t);
    LuciObject *line = luci_readline(lstack, c);
    while (line) {
	list_append_object(list, line);
	line = luci_readline(lstack, c);
    }

    return list;
}

LuciObject * luci_range(Stack *lstack, int c)
{
    int start, end, incr;
    LuciObject *first, *second, *third;

    if (c < 1) {
	die("Missing parameter to range()\n");
    }

    first = st_pop(lstack);
    if (first->type != obj_int_t) {
	die("First parameter to range must be integer\n");
    }

    if (c > 1) {
	second = st_pop(lstack);
	if (second->type != obj_int_t) {
	    die("Second parameter to range must be integer\n");
	}
	start = first->value.i;
	end = second->value.i;
	if (c > 2) {
	    third = st_pop(lstack);
	    if (third->type != obj_int_t) {
		die("Third parameter to range must be integer\n");
	    }
	    incr = third->value.i;
	}
	else {
	    incr = 1;
	}
    }
    else {
	start = 0;
	end = first->value.i;
	incr = 1;
    }

    if (((end < start) && (incr > 0)) || ((end > start) && (incr < 0))){
	die("Invalid incrementor for requested range\n");
    }

    int decreasing = 0;
    /* reverse our range and incr if we're generating a range including negatives */
    if (end < start) {
	end = -end;
	start = -start;

	incr = -incr;
    }

    /* Build a list of integers from start to end, incrementing by incr */
    LuciObject *item, *list = create_object(obj_list_t);
    int i;
    for (i = start; i <= end - incr; i += incr)
    {
	/* create the Object */
	item = create_object(obj_int_t);
	if (decreasing) {
	    item->value.i = -i;
	}
	else {
	    item->value.i = i;
	}
	list_append_object(list, item);
	/*yak("Adding new list item to list\n");*/
    }

    return list;
}

LuciObject * luci_sum(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to sum()\n");
    }

    LuciObject *list = st_pop(lstack);

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate sum\n");
    }

    LuciObject *item;
    double sum = 0;
    int i, found_float = 0;
    for (i = 0; i < list->value.list.count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    die("Can't calulate sum of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		sum += (double)item->value.i;
		break;
	    case obj_float_t:
		found_float = 1;
		sum += (item->value.f);
		break;
	    default:
		die("Can't calculate sum of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i = (long)sum;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f = sum;
    }

    return ret;
}

LuciObject *luci_len(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to len()\n");
    }

    LuciObject *list = st_pop(lstack);

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate len\n");
    }

    LuciObject *ret = create_object(obj_int_t);
    ret->value.i = list->value.list.count;
    return ret;
}

LuciObject *luci_max(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to max()\n");
    }

    LuciObject *list = st_pop(lstack);

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate max\n");
    }

    LuciObject *item;
    double max = 0;
    int i, found_float = 0;
    for (i = 0; i < list->value.list.count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    die("Can't calulate max of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		if ( (double)item->value.i > max) {
		    max = (double)item->value.i;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (item->value.f > max) {
		    max = item->value.f;
		}
		break;
	    default:
		die("Can't calculate max of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i = (long)max;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f = max;
    }
    return ret;
}

LuciObject *luci_min(Stack *lstack, int c)
{
    if (c < 1) {
	die("Missing parameter to min()\n");
    }

    LuciObject *list = st_pop(lstack);

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate min\n");
    }

    LuciObject *item;
    double min = 0;
    int i, found_float = 0;

    for (i = 0; i < list->value.list.count; i ++) {
	item = list_get_object(list, i);
	if (!item) {
	    die("Can't calulate max of list containing NULL value\n");
	}
	switch (item->type) {
	    case obj_int_t:
		if (i == 0) {
		    min = (double)item->value.i;
		}
		else if ( (double)item->value.i < min) {
		    min = (double)item->value.i;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (i == 0) {
		    min = item->value.f;
		}
		else if (item->value.f < min) {
		    min = item->value.f;
		}
		break;
	    default:
		die("Can't calculate min of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i = (long)min;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f = min;
    }
    return ret;
}

