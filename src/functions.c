#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "functions.h"
#include "types.h"
#include "ast.h"

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
    switch(type)
    {
	case obj_str_t:
	    ret->value.s_val = NULL;
	    break;
	case obj_file_t:
	    ret->value.file.f_ptr = NULL;
	    break;
	case obj_list_t:
	    ret->value.list.next = NULL;
	    break;
	default:
	    break;
    }

    return ret;
}

LuciObject *copy_object(LuciObject *orig)
{
    if (!orig) {
	return NULL;
    }

    LuciObject *copy = create_object(orig->type);
    switch(orig->type)
    {
	case obj_int_t:
	    copy->value.i_val = orig->value.i_val;
	    break;
	case obj_double_t:
	    copy->value.d_val = orig->value.d_val;
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
	default:
	    break;
    }
    return copy;
}

void destroy_object(LuciObject *trash)
{
    if (trash)
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

const struct func_def builtins[] =
{
    "help", luci_help,
    "print",  luci_print,
    "input", luci_input,
    "type",  luci_typeof,
    "assert", luci_assert,
    "str", luci_str,
    "open", luci_fopen,
    "close", luci_fclose,
    "read", luci_fread,
    "write", luci_fwrite,
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

LuciObject *luci_print(LuciObject *in)
{
    assert(in->type == obj_list_t);

    LuciObject *ptr = in;
    LuciObject *item = NULL;
    while (ptr)
    {
	item = ptr->value.list.item;
	if (!item)
	{
	    printf("None");
	}
	else {
	    switch (item->type)
	    {
		case obj_int_t:
		    printf("%d", item->value.i_val);
		    break;
		case obj_double_t:
		    printf("%f", item->value.d_val);
		    break;
		case obj_str_t:
		    printf("%s", item->value.s_val);
		    break;
		default:
		    printf("None");
	    }
	}
	printf(" ");
	ptr = ptr->value.list.next;
    }
    printf("\n");

    return NULL;
}

LuciObject *luci_input(LuciObject *in)
{
    size_t lenmax = 64, len = 0;
    char *input = alloc(lenmax * sizeof(char));
    int c;

    if (input == NULL) {
	die("Failed to allocate buffer for reading stdin\n");
    }
    do {
	c = fgetc(stdin);

	if (++len >= lenmax) {
	    lenmax = lenmax << 1;
	    if ((input = realloc(input, lenmax * sizeof(char))) == NULL) {
		free (input);
		die("Failed to allocate buffer for reading stdin\n");
	    }
	    input[len] = c;
	}
    } while (c != EOF && c != '\n');
    input[++len] = '\0';

    /* this might be bad practice? */
    /* I'm malloc'ing and copying the input char[] in case
       the size of the input buffer is far larger than it needs to be?
    */
    LuciObject *ret = create_object(obj_str_t);
    ret->value.s_val = alloc(len * sizeof(char));
    strcpy(ret->value.s_val, input);

    free(input);

    yak("Read line from stdin\n", ret->value.s_val);

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
	assert(in->type == obj_list_t);
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
		case obj_double_t:
		    which = "double";
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
    assert(in->type == obj_list_t);
    LuciObject *param = in->value.list.item;

    switch(param->type)
    {
	case obj_int_t:
	    assert(in->value.i_val);
	    break;
	case obj_double_t:
	    assert((int)in->value.d_val);
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

LuciObject *luci_str(LuciObject *in)
{
    LuciObject *ret = NULL;
    if (!in)
    {
	return ret;
    }
    else
    {
	/* grab the first parameter from the param list */
	assert(in->type == obj_list_t);
	LuciObject *param = in->value.list.item;

	/* allocate our return string object */
	ret = create_object(obj_str_t);
	switch (param->type)
	{
	    case obj_int_t:
		ret->value.s_val = alloc(16);
		sprintf(ret->value.s_val, "%d", param->value.i_val);
		//ret->value.s_val[16] = '\0';
		break;
	    case obj_double_t:
		ret->value.s_val = alloc(16);
		sprintf(ret->value.s_val, "%f", (float)param->value.d_val);
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
    assert(in->type == obj_list_t);
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

    assert(in->type == obj_list_t);
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

    assert(in->type == obj_list_t);
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
	return NULL;
    }

    assert(in->type == obj_list_t);
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
	case obj_double_t:
	    ret->value.d_val = left->value.d_val + right->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = left->value.d_val - right->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = left->value.d_val * right->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = left->value.d_val / right->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = (double)((int)left->value.d_val % (int)right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = pow(left->value.d_val, right->value.d_val);
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
	case obj_double_t:
	    r = (left->value.d_val == right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = (left->value.d_val < right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = (left->value.d_val > right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = !left->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = (left->value.d_val || right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = (left->value.d_val && right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = ~(int)right->value.d_val;
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = ((int)left->value.d_val | (int)right->value.d_val);
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
	case obj_double_t:
	    ret = create_object(obj_double_t);
	    ret->value.d_val = ((int)left->value.d_val & (int)right->value.d_val);
	    break;
	default:
	    ret = NULL;
    }
    return ret;
}

