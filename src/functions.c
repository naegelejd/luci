#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "functions.h"
#include "common.h"
/* HACK: used for dir() function to iterate over symbol table */
#include "env.h"

/* Forward declarations */
static LuciObject *add(LuciObject *left, LuciObject *right);
static LuciObject *sub(LuciObject *left, LuciObject *right);
static LuciObject *mul(LuciObject *left, LuciObject *right);
static LuciObject *divide(LuciObject *left, LuciObject *right);
static LuciObject *mod(LuciObject *left, LuciObject *right);
static LuciObject *power(LuciObject *left, LuciObject *right);
static LuciObject *eq(LuciObject *left, LuciObject *right);
static LuciObject *neq(LuciObject *left, LuciObject *right);
static LuciObject *lt(LuciObject *left, LuciObject *right);
static LuciObject *gt(LuciObject *left, LuciObject *right);
static LuciObject *lte(LuciObject *left, LuciObject *right);
static LuciObject *gte(LuciObject *left, LuciObject *right);
static LuciObject *lgnot(LuciObject *left, LuciObject *right);
static LuciObject *lgor(LuciObject *left, LuciObject *right);
static LuciObject *lgand(LuciObject *left, LuciObject *right);
static LuciObject *bwnot(LuciObject *left, LuciObject *right);
static LuciObject *bwxor(LuciObject *left, LuciObject *right);
static LuciObject *bwor(LuciObject *left, LuciObject *right);
static LuciObject *bwand(LuciObject *left, LuciObject *right);


static int close_file(FILE *fp)
{
    int ret = 1;
    if (fp) {
	ret = fclose(fp);
	fp = NULL;
    }
    return ret;
}

LuciObject *create_object(int type)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = type;
    ret->refcount = 1;
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

    return ret;
}

LuciObject *reference_object(LuciObject *orig)
{
    if (!orig) {
	return NULL;
    }

    orig->refcount ++;

    return orig;
}

LuciObject *copy_object(LuciObject *orig)
{
    if (!orig) {
	return NULL;
    }

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
	default:
	    break;
    }
    return copy;
}

void destroy_object(LuciObject *trash)
{
    if (trash && (--(trash->refcount) <= 0))
    {
	int i;
	switch(trash->type) {
	    case obj_list_t:
		for (i = 0; i < trash->value.list.count; i++) {
		    destroy_object(trash->value.list.items[i]);
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
		yak("Freeing str object with val %s\n", trash->value.s_val);
		free(trash->value.s_val);
		trash->value.s_val = NULL;
		break;
	    default:
		break;
	}
	yak("Destroying obj with type %d\n", trash->type);

	/* destroy the LuciObject itself */
	free(trash);
	trash = NULL;
    }
}

int list_append_object(LuciObject *list, LuciObject *item)
{
    if (!list || (list->type != obj_list_t)) {
	die("Can't append item to non-list object\n");
    }
    if (++(list->value.list.count) > list->value.list.size) {
	list->value.list.size = list->value.list.size << 1;
	/* realloc the list array */
	list->value.list.items = realloc(list->value.list.items,
		list->value.list.size * sizeof(*list->value.list.items));
	yak("Reallocated space for list\n");
    }
    list->value.list.items[list->value.list.count - 1] = item;
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
    destroy_object(list_get_object(list, index));
    list->value.list.items[index] = item;
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
    luci_e->value.f_val = M_E;
    globals[3].object = luci_e;

    /* pi */
    LuciObject *luci_pi = create_object(obj_float_t);
    luci_pi->value.f_val = M_PI;
    globals[4].object = luci_pi;
}


const struct func_def builtins[] =
{
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

LuciObject *luci_help(LuciObject *paramlist)
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
LuciObject *luci_dir(LuciObject *paramlist)
{
    /* this is a terrible way of printing all vars in the global
       namespace but it works for now.
    */
    extern ExecContext *root_env;

    Symbol *ptr;
    for (ptr = root_env->symtable; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next) {
	if (ptr->type == sym_bobj_t) {
	    puts(ptr->name);
	}
    }
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
	    printf("%d", in->value.i_val);
	    break;
	case obj_float_t:
	    printf("%f", in->value.f_val);
	    break;
	case obj_str_t:
	    printf("%s", in->value.s_val);
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

LuciObject *luci_print(LuciObject *paramlist)
{
    int i;
    LuciObject *item = NULL;
    for (i = 0; i < paramlist->value.list.count; i++) {
	item = list_get_object(paramlist, i);
	print_object(item);
	printf(" ");
    }
    printf("\n");

    return NULL;
}

LuciObject *luci_readline(LuciObject *paramlist)
{
    size_t lenmax = 64, len = 0;
    int c;
    FILE *read_from = NULL;

    if (! paramlist->value.list.count) {
	yak("readline from stdin\n");
	read_from = stdin;
    }
    else {
	LuciObject *item = list_get_object(paramlist, 0);
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
	c = fgetc(read_from);

	if (len >= lenmax) {
	    lenmax = lenmax << 1;
	    if ((input = realloc(input, lenmax * sizeof(char))) == NULL) {
		free (input);
		die("Failed to allocate buffer for reading\n");
	    }
	}
	input[len++] = (char)c;
    } while (c != EOF && c != '\n');

    if (c == EOF) {
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
    ret->value.s_val = alloc((len + 1)* sizeof(char));
    strncpy(ret->value.s_val, input, len);
    ret->value.s_val[len] = '\0';

    /* destroy the input buffer */
    free(input);

    yak("Read line\n", ret->value.s_val);

    return ret;
}

LuciObject *luci_typeof(LuciObject *paramlist)
{
    if (! paramlist->value.list.count)
    {
	die("Missing parameter to type()\n");
    }

    char *which;
    LuciObject *ret = create_object(obj_str_t);

    /* grab the first parameter from the param list */
    LuciObject *item = list_get_object(paramlist, 0);
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
    ret->value.s_val = alloc(strlen(which) + 1);
    strcpy(ret->value.s_val, which);

    return ret;
}

LuciObject *luci_assert(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing condition parameter to assert()\n");
    }

    LuciObject *item = list_get_object(paramlist, 0);

    switch(item->type)
    {
	case obj_int_t:
	    assert(item->value.i_val);
	    break;
	case obj_float_t:
	    assert((int)item->value.f_val);
	    break;
	case obj_str_t:
	    assert(strcmp("", item->value.s_val) != 0);
	    break;
	case obj_list_t:
	    assert(item->value.list.count); /* assert that it isn't empty? */
	    break;
	default:
	    ;
    }
    return NULL;
}

LuciObject *luci_cast_int(LuciObject *paramlist)
{
    LuciObject *ret = NULL;
    if (! paramlist->value.list.count) {
	die("Missing parameter to int()\n");
    }
    LuciObject *item = list_get_object(paramlist, 0);

    if (!item) {
	die("Can't cast NULL to int\n");
    }
    ret = create_object(obj_int_t);
    int scanned = 0;
    switch (item->type) {
	case obj_int_t:
	    ret->value.i_val = item->value.i_val;
	    break;
	case obj_float_t:
	    ret->value.i_val = (int)item->value.f_val;
	    break;
	case obj_str_t:
	    scanned = sscanf(item->value.s_val, "%d", &(ret->value.i_val));
	    if (scanned <= 0 || scanned == EOF) {
		die("Could not cast to int\n");
	    }
	    break;
	default:
	    die("Could not cast to int\n");
    }
    return ret;
}

LuciObject *luci_cast_float(LuciObject *paramlist)
{
    LuciObject *ret = NULL;
    if (! paramlist->value.list.count) {
	die("Missing parameter to int()\n");
    }
    LuciObject *item = list_get_object(paramlist, 0);

    if (!item) {
	die("Can't cast NULL to int\n");
    }
    ret = create_object(obj_float_t);
    int scanned = 0;
    switch (item->type) {
	case obj_int_t:
	    ret->value.f_val = (double)item->value.i_val;
	    break;
	case obj_float_t:
	    ret->value.f_val = item->value.f_val;
	    break;
	case obj_str_t:
	    scanned = sscanf(item->value.s_val, "%f", (float *)&(ret->value.f_val));
	    if (scanned <= 0 || scanned == EOF) {
		die("Could not cast to float\n");
	    }
	    break;
	default:
	    die("Could not cast to float\n");
    }

    return ret;
}

LuciObject *luci_cast_str(LuciObject *paramlist)
{
    LuciObject *ret = NULL;
    if (! paramlist->value.list.count)
    {
	die("Missing parameter to str()\n");
    }
    /* grab the first parameter from the param list */
    LuciObject *item = list_get_object(paramlist, 0);

    /* allocate our return string object */
    ret = create_object(obj_str_t);
    switch (item->type)
    {
	case obj_int_t:
	    ret->value.s_val = alloc(32);
	    sprintf(ret->value.s_val, "%d", item->value.i_val);
	    /* ret->value.s_val[16] = '\0'; */
	    break;
	case obj_float_t:
	    ret->value.s_val = alloc(32);
	    sprintf(ret->value.s_val, "%f", (float)item->value.f_val);
	    /* ret->value.s_val[16] = '\0'; */
	    break;
	case obj_str_t:
	    ret->value.s_val = alloc(strlen(item->value.s_val) + 1);
	    strcpy(ret->value.s_val, item->value.s_val);
	    break;
	default:
	    break;
    }
    yak("str() returning %s\n", ret->value.s_val);

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

LuciObject *luci_fopen(LuciObject *paramlist)
{
    char *filename;
    char *req_mode;
    int mode;
    FILE *file = NULL;

    if (paramlist->value.list.count < 2)
    {
	die("Missing parameter to open()\n");
    }

    LuciObject *fname_obj = list_get_object(paramlist, 0);
    if (fname_obj->type != obj_str_t) {
	die("Parameter 1 to open must be a string\n");
    }
    LuciObject *mode_obj = list_get_object(paramlist, 1);
    if (mode_obj->type != obj_str_t) {
	die("Parameter 2 to open must be a string\n");
    }

    filename = fname_obj->value.s_val;
    req_mode = mode_obj->value.s_val;

    mode = get_file_mode(req_mode);
    if (mode < 0)
    {
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

LuciObject *luci_fclose(LuciObject *paramlist)
{
    if (! paramlist->value.list.count)
    {
	die("Missing parameter to close()\n");
    }

    LuciObject *fobj = list_get_object(paramlist, 0);

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

LuciObject *luci_fread(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing parameter to read()\n");
    }

    LuciObject *fobj = list_get_object(paramlist, 0);

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
    ret->value.s_val = read;

    return ret;
}

LuciObject *luci_fwrite(LuciObject *paramlist)
{
    if (paramlist->value.list.count < 2) {
	die("Missing parameter to write()\n");
    }

    /* grab the FILE parameter */
    LuciObject *fobj = list_get_object(paramlist, 0);
    if (!fobj || (fobj->type != obj_file_t)) {
	die("Not a file object\n");
    }

    /* grab string parameter */
    LuciObject *text_obj = list_get_object(paramlist, 1);
    if (!text_obj || (text_obj->type != obj_str_t) ) {
	die("Not a string\n");
    }
    char *text = text_obj->value.s_val;

    if (fobj->value.file.mode == f_read_m) {
	die("Can't write to file. It is opened for reading.\n");
    }

    fwrite(text, sizeof(char), strlen(text), fobj->value.file.ptr);
    return NULL;
}

LuciObject *luci_flines(LuciObject *paramlist)
{
    LuciObject *list = create_object(obj_list_t);
    LuciObject *line = luci_readline(paramlist);
    while (line) {
	list_append_object(list, line);
	line = luci_readline(paramlist);
    }

    return list;
}

LuciObject * luci_range(LuciObject *paramlist)
{
    int start, end, incr;
    LuciObject *first, *second, *third;

    if (! paramlist->value.list.count) {
	die("Missing parameter to range()\n");
    }

    first = list_get_object(paramlist, 0);
    if (first->type != obj_int_t) {
	die("First parameter to range must be integer\n");
    }

    if (paramlist->value.list.count > 1) {
	second = list_get_object(paramlist, 1);
	if (second->type != obj_int_t) {
	    die("Second parameter to range must be integer\n");
	}
	start = first->value.i_val;
	end = second->value.i_val;
	if (paramlist->value.list.count > 2) {
	    third = list_get_object(paramlist, 2);
	    if (third->type != obj_int_t) {
		die("Third parameter to range must be integer\n");
	    }
	    incr = third->value.i_val;
	}
	else {
	    incr = 1;
	}
    }
    else {
	start = 0;
	end = first->value.i_val;
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
	    item->value.i_val = -i;
	}
	else {
	    item->value.i_val = i;
	}
	list_append_object(list, item);
	/*yak("Adding new list item to list\n");*/
    }

    return list;
}

LuciObject * luci_sum(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing parameter to sum()\n");
    }

    LuciObject *list = list_get_object(paramlist, 0);

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
		sum += (double)item->value.i_val;
		break;
	    case obj_float_t:
		found_float = 1;
		sum += (item->value.f_val);
		break;
	    default:
		die("Can't calculate sum of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i_val = (long)sum;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f_val = sum;
    }

    return ret;
}

LuciObject *luci_len(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing parameter to len()\n");
    }

    LuciObject *list = list_get_object(paramlist, 0);

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate len\n");
    }

    LuciObject *ret = create_object(obj_int_t);
    ret->value.i_val = list->value.list.count;
    return ret;
}

LuciObject *luci_max(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing parameter to max()\n");
    }

    LuciObject *list = list_get_object(paramlist, 0);

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
		if ( (double)item->value.i_val > max) {
		    max = (double)item->value.i_val;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (item->value.f_val > max) {
		    max = item->value.f_val;
		}
		break;
	    default:
		die("Can't calculate max of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i_val = (long)max;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f_val = max;
    }
    return ret;
}

LuciObject *luci_min(LuciObject *paramlist)
{
    if (! paramlist->value.list.count) {
	die("Missing parameter to min()\n");
    }

    LuciObject *list = list_get_object(paramlist, 0);

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
		    min = (double)item->value.i_val;
		}
		else if ( (double)item->value.i_val < min) {
		    min = (double)item->value.i_val;
		}
		break;
	    case obj_float_t:
		found_float = 1;
		if (i == 0) {
		    min = item->value.f_val;
		}
		else if (item->value.f_val < min) {
		    min = item->value.f_val;
		}
		break;
	    default:
		die("Can't calculate min of list containing non-numeric value\n");
	}
    }

    LuciObject *ret;
    if (!found_float) {
	ret = create_object(obj_int_t);
	ret->value.i_val = (long)min;
    }
    else {
	ret = create_object(obj_float_t);
	ret->value.f_val = min;
    }
    return ret;
}


LuciObject * (*solvers[])(LuciObject *left, LuciObject *right) = {
    add,
    sub,
    mul,
    divide,
    mod,
    power,
    eq,
    neq,
    lt,
    gt,
    lte,
    gte,
    lgnot,
    lgor,
    lgand,
    bwxor,
    bwor,
    bwand,
    bwnot
};

int types_match(LuciObject *left, LuciObject *right)
{
    if (left && right)
	return (left->type == right->type);
    else
	return 0;
}

/*
   Evaluates a conditional statement, returning an integer
   value of 0 if False, and non-zero if True.
*/
int evaluate_condition(LuciObject *cond)
{
    int huh = 0;
    if (cond == NULL) {
	return 0;
    }
    switch (cond->type)
    {
	case obj_int_t:
	    huh = cond->value.i_val;
	    break;
	case obj_float_t:
	    huh = (int)cond->value.f_val;
	    break;
	case obj_str_t:
	    huh = strlen(cond->value.s_val);
	    break;
	case obj_list_t:
	    huh = cond->value.list.count;
	    break;
	case obj_file_t:
	    huh = 1;
	    break;
	default:
	    huh = 0;
    }
    return huh;
}

LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, int op)
{
    if (left->type == obj_float_t && right->type == obj_int_t) {
	/* maybe bzero it ? */
	int i = right->value.i_val;
	right->value.i_val = 0;
	right->type = obj_float_t;
	right->value.f_val = (float)i;
    }
    else if (right->type == obj_float_t && left->type == obj_int_t) {
	int i = left->value.i_val;
	left->value.i_val = 0;
	left->type = obj_float_t;
	left->value.f_val = (float)i;
    }
    else if (!types_match(left, right))
    {
	die("Type mismatch in expression\n");
    }
    LuciObject *result = NULL;
    result = solvers[op](left, right);
    return result;
}

static LuciObject *add(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = create_object(left->type);
    switch (left->type)
    {
	case obj_int_t:
	    ret->value.i_val = left->value.i_val + right->value.i_val;
	    break;
	case obj_float_t:
	    ret->value.f_val = left->value.f_val + right->value.f_val;
	    break;
	case obj_str_t:
	    ret->value.s_val = alloc(strlen(left->value.s_val) +
		    strlen(right->value.s_val) + 1);
	    strcpy(ret->value.s_val, left->value.s_val);
	    strcat(ret->value.s_val, right->value.s_val);
	    break;
	default:
	    break;
    }
    return ret;
}

static LuciObject *sub(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = left->value.i_val - right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = left->value.f_val - right->value.f_val;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *mul(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = left->value.i_val * right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = left->value.f_val * right->value.f_val;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *divide(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    /* this could probably be replace by a generic right->value.f_val
       since both the float and int obj->value structs are aligned
    */
    if ((right->type == obj_int_t && right->value.i_val == 0) ||
	(right->type == obj_float_t && right->value.f_val == 0.0)) {
	    /* memory leak */
	    /* printf("peace\n"); */
	    /* exit(1); */
	    die("Divide by zero error\n");
    }

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = left->value.i_val / right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = left->value.f_val / right->value.f_val;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *mod(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = left->value.i_val % right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = (double)((int)left->value.f_val % (int)right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *power(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = pow(left->value.i_val, right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = pow(left->value.f_val, right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *eq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = create_object(obj_int_t);
    int r;
    switch (left->type)
    {
	case obj_int_t:
	    r = (left->value.i_val == right->value.i_val);
	    break;
	case obj_float_t:
	    r = (left->value.f_val == right->value.f_val);
	    break;
	case obj_str_t:
	    r = !(strcmp(left->value.s_val, right->value.s_val));
	    break;
	case obj_file_t:
	    r = (left->value.file.ptr == right->value.file.ptr);
	default:
	    r = 0;
    }
    ret->value.i_val = r;
    return ret;
}

static LuciObject *neq(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = eq(left, right);
    if (ret)
	ret->value.i_val = !ret->value.i_val;
    return ret;
}

static LuciObject *lt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val < right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = (left->value.f_val < right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *gt(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val > right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = (left->value.f_val > right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = gt(left, right);
    if (ret)
	ret->value.i_val = !ret->value.i_val;
    return ret;
}

static LuciObject *gte(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = lt(left, right);
    if (ret)
	ret->value.i_val = !ret->value.i_val;
    return ret;
}

static LuciObject *lgnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = !right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = !left->value.f_val;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lgor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val || right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = (left->value.f_val || right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *lgand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val && right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = (left->value.f_val && right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwnot(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = ~right->value.i_val;
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = ~(int)right->value.f_val;
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwxor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret = alloc(sizeof(*ret));
    ret->type = obj_int_t;
    ret->value.i_val = left->value.i_val ^ right->value.i_val;
    return ret;
}

static LuciObject *bwor(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val | right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = ((int)left->value.f_val | (int)right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

static LuciObject *bwand(LuciObject *left, LuciObject *right)
{
    LuciObject *ret;

    switch (left->type)
    {
	case obj_int_t:
	    ret = create_object(obj_int_t);
	    ret->value.i_val = (left->value.i_val & right->value.i_val);
	    break;
	case obj_float_t:
	    ret = create_object(obj_float_t);
	    ret->value.f_val = ((int)left->value.f_val & (int)right->value.f_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

