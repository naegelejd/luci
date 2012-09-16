#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "functions.h"
#include "types.h"
#include "driver.h"

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
    int ret;
    ret = fclose(fp);
    fp = NULL;
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
	    ret->value.file.f_ptr = NULL;
	    break;
	case obj_list_t:
	    ret->value.list.count = 0;
	    ret->value.list.size = INIT_LIST_SIZE;
	    ret->value.list.items = alloc(ret->value.list.size *
		    sizeof(*ret->value.list.items));
	    ret->value.list.items = NULL;
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

    /* thes next object pointers are for copying list objects */
    LuciObject * orig_ptr, * copy_ptr;

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
	    copy->value.file.f_ptr = orig->value.file.f_ptr;
	    copy->value.file.size = orig->value.file.size;
	    copy->value.file.mode = orig->value.file.mode;
	    break;
	case obj_list_t:
	    orig_ptr = orig;
	    copy_ptr = copy;
	    copy_ptr->value.list.item = copy_object(orig_ptr->value.list.item);
	    orig_ptr = orig_ptr->value.list.next;
	    while (orig_ptr) {
		copy_ptr->value.list.next = create_object(obj_list_t);
		copy_ptr->value.list.next->value.list.item = copy_object(orig_ptr->value.list.item);
		orig_ptr = orig_ptr->value.list.next;
		copy_ptr = copy_ptr->value.list.next;
	    }
	default:
	    break;
    }
    return copy;
}

void destroy_object(LuciObject *trash)
{
    if (trash && (--(trash->refcount) <= 0))
    {
	/* if this object is part of a linked list, destroy the next node */
	if (trash->type == obj_list_t)
	{
	    destroy_object(trash->value.list.item);
	    destroy_object(trash->value.list.next);
	}

	/* close any open files */
	if (trash->type == obj_file_t)
	{
	    if (trash->value.file.f_ptr) {
		/*
		yak("Closing file object\n");
		close_file(trash->value.file.f_ptr);
		*/
	    }
	}

	/* if this object contains a string, it WAS malloc'd */
	if (trash->type == obj_str_t) {
	    yak("Freeing str object with val %s\n", trash->value.s_val);
	    free(trash->value.s_val);
	    trash->value.s_val = NULL;
	}
	yak("Freeing obj with type %d\n", trash->type);

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
	list->value.list.size << 1;
	/* realloc the list array */
	list->value.list.items = realloc(list->value.list.items,
		list->value.list.size * sizeof(*list->value.list.items));
    }
    list->value.liste.items[list->value.list.count - 1] = item;
    return 1;
}

const struct func_def builtins[] =
{
    "help", luci_help,
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
    0, 0
};

LuciObject *luci_help(LuciObject *in)
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

void print_object(LuciObject *in)
{
    int i;
    LuciObject *node;
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
	    i = 0;
	    while (1) {
		node = get_list_node(in, i++);
		if (!node) {
		    break;
		}
		print_object(node->value.list.item);
		printf(", ");
	    }
	    printf("]");
	    break;
	default:
	    printf("None");
    }
}

LuciObject *luci_print(LuciObject *in)
{
    int i = 0;
    LuciObject *ptr = in;
    LuciObject *item = NULL;
    while (ptr)
    {
	item = ptr->value.list.item;
	print_object(item);
	printf(" ");
	ptr = ptr->value.list.next;
    }
    printf("\n");

    return NULL;
}

LuciObject *luci_readline(LuciObject *param)
{
    size_t lenmax = 64, len = 0;
    int c;
    FILE *read_from = NULL;

    if (!param) {
	yak("readline from stdin\n");
	read_from = stdin;
    }
    else {
	LuciObject *item = param->value.list.item;
	if (item && (item->type == obj_file_t)) {
	    yak("readline from file\n");
	    read_from = item->value.file.f_ptr;
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

LuciObject *luci_typeof(LuciObject *in)
{
    LuciObject *ret = create_object(obj_str_t);
    char *which;

    if (!in)
    {
	which = "None";
    }
    else
    {
	/* grab the first parameter from the param list */
	LuciObject *param = in->value.list.item;
	if (!param) {
	    which = "None";
	}
	else {
	    switch(param->type)
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
    }
    ret->value.s_val = alloc(strlen(which) + 1);
    strcpy(ret->value.s_val, which);

    return ret;
}

LuciObject *luci_assert(LuciObject *in)
{
    assert(in);
    LuciObject *param = in->value.list.item;

    switch(param->type)
    {
	case obj_int_t:
	    assert(in->value.i_val);
	    break;
	case obj_float_t:
	    assert((int)in->value.f_val);
	    break;
	case obj_str_t:
	    assert(strcmp("", in->value.s_val) != 0);
	    break;
	case obj_list_t:
	    assert(param->value.list.item); /* assert that its HEAD item exists?? */
	    break;
	default:
	    ;
    }
    return NULL;
}

LuciObject *luci_cast_int(LuciObject *param)
{
    LuciObject *ret = NULL;
    if (param) {
	LuciObject *item = param->value.list.item;

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
    }
    return ret;
}

LuciObject *luci_cast_float(LuciObject *param)
{
    LuciObject *ret = NULL;
    if (param) {
	LuciObject *item = param->value.list.item;

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
    }
    return ret;
}


LuciObject *luci_cast_str(LuciObject *in)
{
    LuciObject *ret = NULL;
    if (!in)
    {
	return ret;
    }
    else
    {
	/* grab the first parameter from the param list */
	LuciObject *param = in->value.list.item;

	/* allocate our return string object */
	ret = create_object(obj_str_t);
	switch (param->type)
	{
	    case obj_int_t:
		ret->value.s_val = alloc(32);
		sprintf(ret->value.s_val, "%d", param->value.i_val);
		//ret->value.s_val[16] = '\0';
		break;
	    case obj_float_t:
		ret->value.s_val = alloc(32);
		sprintf(ret->value.s_val, "%f", (float)param->value.f_val);
		//ret->value.s_val[16] = '\0';
		break;
	    case obj_str_t:
		ret->value.s_val = alloc(strlen(param->value.s_val) + 1);
		strcpy(ret->value.s_val, param->value.s_val);
		break;
	    default:
		break;
	}
	yak("str() returning %s\n", ret->value.s_val);
    }

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

LuciObject *luci_fopen(LuciObject *in)
{
    char *filename;
    char *req_mode;
    int mode;
    FILE *file = NULL;

    if (!in)
    {
	/* TODO: throw error here ?? */
	return NULL;
    }

    /* TODO: proper error checking */
    /* check that there are two proper parameters to fopen */
    assert(in->value.list.next);
    assert(in->value.list.next->type == obj_list_t);

    LuciObject *fname_obj = in->value.list.item;
    LuciObject *mode_obj = in->value.list.next->value.list.item;

    /* both parameters should be strings */
    assert(fname_obj->type = obj_str_t);
    assert(mode_obj->type = obj_str_t);

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
    ret->value.file.f_ptr = file;
    ret->value.file.mode = mode;
    ret->value.file.size = file_length;

    yak("Opened file %s of size %ld bytes with mode %s.\n",
	    filename, file_length, req_mode);

    return ret;
}
LuciObject *luci_fclose(LuciObject *in)
{
    if (!in)
    {
	return NULL;
    }

    LuciObject *fobj = in->value.list.item;

    if (!(fobj->type == obj_file_t)) {
	die("Not a file object\n");
    }

    if (fobj->value.file.f_ptr) {
	close_file(fobj->value.file.f_ptr);
    }
    /* else, probably already closed (it's NULL) */

    yak("Closed file object.\n");

    return NULL;
}

LuciObject *luci_fread(LuciObject *in)
{
    if (!in) {
	die("Missing file object\n");
    }

    LuciObject *fobj = in->value.list.item;

    if (!(fobj->type == obj_file_t)) {
	die("Not a file object\n");
    }

    if (fobj->value.file.mode != f_read_m) {
	die("Can't open file. It is opened for writing.\n");
    }

    /* seek to file start, we're gonna read the whole thing */
    /* fseek(fobj->value.file.f_ptr, 0, SEEK_SET); */

    long len = fobj->value.file.size;
    char *read = alloc(len + 1);
    fread(read, sizeof(char), len, fobj->value.file.f_ptr);
    read[len] = '\0';

    /* fseek(fobj->value.file.f_ptr, 0, SEEK_SET); */

    LuciObject *ret = create_object(obj_str_t);
    ret->value.s_val = read;

    return ret;
}

LuciObject *luci_fwrite(LuciObject *in)
{
    if (!in) {
	die("Need file to write to\n");
    }

    assert(in->value.list.next);
    assert(in->value.list.next->type == obj_list_t);

    /* grab the FILE parameter */
    LuciObject *fobj = in->value.list.item;
    if (!(fobj->type == obj_file_t)) {
	die("Not a file object\n");
    }

    /* grab string parameter */
    LuciObject *param2 = in->value.list.next;
    if (!param2) {
	die("Missing string parameter\n");
    }
    LuciObject *text_obj = param2->value.list.item;
    if (!text_obj || (text_obj->type != obj_str_t) ) {
	die("Not a string\n");
    }
    char *text = text_obj->value.s_val;

    if (fobj->value.file.mode == f_read_m) {
	die("Can't write to file. It is opened for reading.\n");
    }

    fwrite(text, sizeof(char), strlen(text), fobj->value.file.f_ptr);
    return NULL;
}

LuciObject *luci_flines(LuciObject *param)
{
    LuciObject *list = NULL;
    LuciObject *prev = list;
    LuciObject *cur = list;
    LuciObject *line = luci_readline(param);
    while (line) {
	cur = create_object(obj_list_t);
	if (!list) {
	    list = cur;
	}
	cur->value.list.item = line;
	if (prev) {
	    prev->value.list.next = cur;
	}
	prev = cur;
	line = luci_readline(param);
    }

    return list;
}

LuciObject * luci_range(LuciObject *param_list)
{
    LuciObject *first = param_list;
    LuciObject *second = NULL;
    LuciObject *third = NULL;

    int start, end, incr;

    if (!first) {
	die("Specify at least an integer endpoint for range\n");
    }
    if (first->value.list.item->type != obj_int_t) {
	die("First parameter to range must be integer\n");
    }

    second = first->value.list.next;
    if (second) {
	if (second->value.list.item->type != obj_int_t) {
	    die("Second parameter to range must be integer\n");
	}
	start = first->value.list.item->value.i_val;
	end = second->value.list.item->value.i_val;
	third = second->value.list.next;
	if (third) {
	    if (third->value.list.item->type != obj_int_t) {
		die("Third parameter to range must be integer\n");
	    }
	    incr = third->value.list.item->value.i_val;
	}
	else {
	    incr = 1;
	}
    }
    else {
	start = 0;
	end = first->value.list.item->value.i_val;
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

    LuciObject *tail, *item, *next = NULL;
    int i;
    for (i = end - incr; i >= start; i -= incr)
    {
	/* create the Object */
	item = create_object(obj_int_t);
	if (decreasing) {
	    item->value.i_val = -i;
	}
	else {
	    item->value.i_val = i;
	}
	/* create the list item container */
	tail = create_object(obj_list_t);
	/* link this container to 'next' container */
	tail->value.list.next = next;
	/* store ptr to actual object in container */
	tail->value.list.item = item;
	/* point 'next' to this container */
	next = tail;

	/*yak("Adding new list item to list\n");*/
    }

    return next;
}

LuciObject * luci_sum(LuciObject *param_list)
{
    if (!param_list) {
	die("Need list to calculate sum\n");
    }

    LuciObject *list = param_list->value.list.item;

    if (!list || (list->type != obj_list_t)) {
	die("Must specify a list to calculate sum\n");
    }

    LuciObject *ptr = list;
    double sum = 0;
    int found_float = 0;
    while (ptr) {
	if (!ptr->value.list.item) {
	    die("Can't calulate sum of list containing NULL value\n");
	}
	switch (ptr->value.list.item->type)
	{
	    case obj_int_t:
		sum += (double)ptr->value.list.item->value.i_val;
		break;
	    case obj_float_t:
		found_float = 1;
		sum += (ptr->value.list.item->value.f_val);
		break;
	    default:
		die("Can't calculate sum of list containing non-numeric value\n");
	}
	ptr = ptr->value.list.next;
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
	    huh = 1;
	    break;
	case obj_file_t:
	    huh = 1;
	    break;
	default:
	    huh = 0;
    }
    return huh;
}

/* returns the list NODE at the given index
    or NULL if not found (index exceeds list bounds)
*/
LuciObject *get_list_node(LuciObject *list, int index)
{
    if (!list || (list->type != obj_list_t)) {
	return NULL;
    }
    int i = 0, found = 0;
    LuciObject *cur = list;
    LuciObject *ret = NULL;
    while (cur) {
	if (i == index) {
	    ret = cur;
	    break;
	}
	cur = cur->value.list.next;
	i++;
    }
    return ret;
}


LuciObject *solve_bin_expr(LuciObject *left, LuciObject *right, int op)
{
    if (!types_match(left, right))
    {
	yak("Types don't match\n");
	return NULL;
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
	    r = (left->value.file.f_ptr == right->value.file.f_ptr);
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

