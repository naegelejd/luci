/*
 * See Copyright Notice in luci.h
 */

/**
 * @file builtin.c
 */

#include "luci.h"
#include "builtin.h"


static LuciLibFuncObj builtin_print = {
    {&obj_libfunc_t, GC_STATIC},
    luci_print,
    "prints string representations of objects to stdout",
    0
};

static LuciLibFuncObj builtin_help = {
    {&obj_libfunc_t, GC_STATIC},
    luci_help,
    "prints a help string for a given object",
    0
};

static LuciLibFuncObj builtin_exit = {
    {&obj_libfunc_t, GC_STATIC},
    luci_exit,
    "abruptly exits Luci",
    0
};

static LuciLibFuncObj builtin_input = {
    {&obj_libfunc_t, GC_STATIC},
    luci_readline,
    "reads a line from stdin",
    0
};

static LuciLibFuncObj builtin_readline = {
    {&obj_libfunc_t, GC_STATIC},
    luci_readline,
    "reads a line from a file",
    1
};

static LuciLibFuncObj builtin_typeof = {
    {&obj_libfunc_t, GC_STATIC},
    luci_typeof,
    "returns the type of a given object",
    1
};

static LuciLibFuncObj builtin_assert = {
    {&obj_libfunc_t, GC_STATIC},
    luci_assert,
    "asserts that an expression is true",
    1
};

static LuciLibFuncObj builtin_copy = {
    {&obj_libfunc_t, GC_STATIC},
    luci_copy,
    "returns a deep copy of a given object",
    1
};

static LuciLibFuncObj builtin_cast_str = {
    {&obj_libfunc_t, GC_STATIC},
    luci_cast_str,
    "casts an object to a string",
    1
};

static LuciLibFuncObj builtin_cast_int = {
    {&obj_libfunc_t, GC_STATIC},
    luci_cast_int,
    "casts an object to an int",
    1
};

static LuciLibFuncObj builtin_cast_float = {
    {&obj_libfunc_t, GC_STATIC},
    luci_cast_float,
    "casts an object to a float",
    1
};

static LuciLibFuncObj builtin_hex = {
    {&obj_libfunc_t, GC_STATIC},
    luci_hex,
    "returns a string of the hex representation of an int",
    1
};

static LuciLibFuncObj builtin_fopen = {
    {&obj_libfunc_t, GC_STATIC},
    luci_fopen,
    "opens a file",
    1
};

static LuciLibFuncObj builtin_fclose = {
    {&obj_libfunc_t, GC_STATIC},
    luci_fclose,
    "closes a file",
    1
};

static LuciLibFuncObj builtin_fread = {
    {&obj_libfunc_t, GC_STATIC},
    luci_fread,
    "reads a file",
    1
};

static LuciLibFuncObj builtin_fwrite = {
    {&obj_libfunc_t, GC_STATIC},
    luci_fwrite,
    "writes a string to a file",
    2
};

static LuciLibFuncObj builtin_flines = {
    {&obj_libfunc_t, GC_STATIC},
    luci_flines,
    "reads the lines in a file as a list",
    1
};

static LuciLibFuncObj builtin_range = {
    {&obj_libfunc_t, GC_STATIC},
    luci_range,
    "generates a range of integers",
    1
};

static LuciLibFuncObj builtin_sum = {
    {&obj_libfunc_t, GC_STATIC},
    luci_sum,
    "computes the sum of a list of numbers",
    1
};

static LuciLibFuncObj builtin_len = {
    {&obj_libfunc_t, GC_STATIC},
    luci_len,
    "computes the length of a list",
    1
};

static LuciLibFuncObj builtin_max = {
    {&obj_libfunc_t, GC_STATIC},
    luci_max,
    "computes the sum of a list of numbers",
    1
};

static LuciLibFuncObj builtin_min = {
    {&obj_libfunc_t, GC_STATIC},
    luci_min,
    "computes the sum of a list of numbers",
    1
};

static LuciLibFuncObj builtin_contains = {
    {&obj_libfunc_t, GC_STATIC},
    luci_contains,
    "determines if the given container contains a given object",
    2
};

static LuciFileObj builtin_stdout = {
    {&obj_file_t, GC_STATIC},
    NULL,
    0,
    f_append_m
};

static LuciFileObj builtin_stderr = {
    {&obj_file_t, GC_STATIC},
    NULL,
    0,
    f_append_m
};

static LuciFileObj builtin_stdin = {
    {&obj_file_t, GC_STATIC},
    NULL,
    0,
    f_read_m
};

static LuciFloatObj builtin_e = {
    {&obj_float_t, GC_STATIC},
    M_E
};

static LuciFloatObj builtin_pi = {
    {&obj_float_t, GC_STATIC},
    M_PI
};

/** List of all builtin library symbols and their corresponding objects */
const LuciObjectRecord builtins_registry[] = {
    {"print",       (LuciObject*)&builtin_print},
    {"help",        (LuciObject*)&builtin_help},
//  {"dir",         (LuciObject*)&builtin_dir},
    {"exit",        (LuciObject*)&builtin_exit},
    {"input",       (LuciObject*)&builtin_input},
    {"readline",    (LuciObject*)&builtin_readline},
    {"type",        (LuciObject*)&builtin_typeof},
    {"assert",      (LuciObject*)&builtin_assert},
    {"copy",        (LuciObject*)&builtin_copy},
    {"str",         (LuciObject*)&builtin_cast_str},
    {"int",         (LuciObject*)&builtin_cast_int},
    {"float",       (LuciObject*)&builtin_cast_float},
    {"hex",         (LuciObject*)&builtin_hex},
    {"open",        (LuciObject*)&builtin_fopen},
    {"close",       (LuciObject*)&builtin_fclose},
    {"read",        (LuciObject*)&builtin_fread},
    {"write",       (LuciObject*)&builtin_fwrite},
    {"readlines",   (LuciObject*)&builtin_flines},
    {"range",       (LuciObject*)&builtin_range},
    {"sum",         (LuciObject*)&builtin_sum},
    {"len",         (LuciObject*)&builtin_len},
    {"max",         (LuciObject*)&builtin_max},
    {"min",         (LuciObject*)&builtin_min},
    {"contains",    (LuciObject*)&builtin_contains},
    {"stdout",      (LuciObject*)&builtin_stdout},
    {"stderr",      (LuciObject*)&builtin_stderr},
    {"stdin",       (LuciObject*)&builtin_stdin},
    {"e",           (LuciObject*)&builtin_e},
    {"pi",          (LuciObject*)&builtin_pi},
    {NULL, NULL}
};

/** Initializes all builtin symbols with LuciObjects */
void init_luci_builtins(void)
{
    builtin_stdout.ptr = stdout;
    builtin_stderr.ptr = stderr;
    builtin_stdin.ptr = stdin;
}

/** Prints a help message, which is essentially just a list of
 * builtin library functions for now
 *
 * @param args unused
 * @param c unused
 * @returns NULL
 */
LuciObject *luci_help(LuciObject **args, unsigned int c)
{
    int width = 32;

    if (c == 0) {
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        printf("              HELP               \n");
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        printf("        BUILTIN FUNCTIONS        \n");
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

        int i, len, f, l, j;
        for (i = 0; builtins_registry[i].name != 0; i++)
        {
            len = strlen(builtins_registry[i].name);
            f = (width - len) / 2;
            l = width - f;
            for (j = 0; j < f; j++)
                printf(" ");
            printf("%s", builtins_registry[i].name);
            for (j = 0; j < l; j++)
                printf(" ");
            printf("\n");
        }
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    } else {
        unsigned int i;
        for (i = 0; i < c; i++) {
            LuciObject *o = args[i];
            if (!ISTYPE(o, obj_libfunc_t)) {
                printf("No help available for object of type %s\n",
                        o->type->type_name);
            } else {
                printf("%s\n", AS_LIBFUNC(o)->help);
            }
        }
    }
    return LuciNilObj;
}

/** NOT IMPLEMENTED...
 * Prints all names in scope
 *
 * @param args unused
 * @param c unused
 * @returns LuciNilObj
 */
LuciObject *luci_dir(LuciObject **args, unsigned int c)
{
    return LuciNilObj;
}

/**
 * Immediately exits Luci
 *
 * @param args unused
 * @param c unused
 * @returns LuciNilObj
 */
LuciObject *luci_exit(LuciObject **args, unsigned int c)
{
    exit(EXIT_SUCCESS);
    return LuciNilObj;
}

/**
 * Prints objects to stdout
 *
 * @param args list of LuciObjects to print
 * @param c number of LuciObjects to print
 * @returns LuciNilObj
 */
LuciObject *luci_print(LuciObject **args, unsigned int c)
{
    if (c > 0) {
        args[0]->type->print(args[0]);
    }

    unsigned int i;
    for (i = 1; i < c; i++) {
        printf(" ");
        args[i]->type->print(args[i]);
    }
    printf("\n");

    return LuciNilObj;
}

/**
 * Reads a line of input from stdin
 *
 * @param args first arg is either NULL or a LuciFileObj
 * @param c if 0, read from stdin. if > 0, read from file.
 * @returns LuciStringObj containing what was read
 */
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
        if (item && (ISTYPE(item, obj_file_t))) {
            LUCI_DEBUG("%s\n", "readline from file");
            read_from = AS_FILE(item)->ptr;
        }
        else {
            LUCI_DIE("args[0]: %p, type: %s\n", args[0], item->type->type_name);
            LUCI_DIE("%s", "Can't readline from non-file object\n");
        }
    }

    input = alloc(lenmax * sizeof(char));
    if (input == NULL) {
        LUCI_DIE("%s", "Failed to allocate buffer for reading stdin\n");
    }
    do {
        ch = fgetc(read_from);

        if (len >= lenmax) {
            lenmax *= 2;
            if ((input = realloc(input, lenmax * sizeof(char))) == NULL) {
                LUCI_DIE("%s", "Failed to allocate buffer for reading\n");
            }
        }
        input[len++] = (char)ch;
    } while (ch != EOF && ch != '\n');

    if (ch == EOF) {
        LUCI_DEBUG("%s\n", "readline at EOF, returning nil");
        return LuciNilObj;
    }

    /* overwrite the newline or EOF char with a NUL terminator */
    input[--len] = '\0';
    LuciObject *ret = LuciString_new(input);

    LUCI_DEBUG("Read line %s\n", AS_STRING(ret)->s);

    return ret;
}

/**
 * Determines and returns the type of a LuciObject
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciStringObj of the type of the first arg
 */
LuciObject *luci_typeof(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to type()\n");
    }

    /* grab the first parameter from the param list */
    LuciObject *item = args[0];
    if (!item) {
        return LuciString_new(strdup("None"));
    }

    /* Create new LuciString from the object's type name */
    return LuciString_new(strdup(item->type->type_name));
}

/**
 * Asserts that a given LuciObject is equivalent to a boolean True
 *
 * Currently uses C @code assert @endcode , which will exit a program
 * mid-execution if the assertion fails.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciNilObj
 */
LuciObject *luci_assert(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing condition parameter to assert()\n");
    }

    LuciObject *item = args[0];

    if (ISTYPE(item, obj_int_t) && !AS_INT(item)->i) {
        LUCI_DIE("%s\n", "Assertion failed");
    } else if (ISTYPE(item, obj_float_t) && !((long)AS_FLOAT(item)->f)) {
        LUCI_DIE("%s\n", "Float assertion failed");
    } else if (ISTYPE(item, obj_string_t)) {
        if (strcmp("", AS_STRING(item)->s) == 0) {
            LUCI_DIE("%s\n", "String assertion failed");
        }
    } else if (ISTYPE(item, obj_list_t) && (AS_LIST(item)->count == 0)) {
        LUCI_DIE("%s\n", "List assertion failed");
    } else if (ISTYPE(item, obj_map_t) && (AS_MAP(item)->count == 0)) {
        LUCI_DIE("%s\n", "Map assertion failed");
    } else if (ISTYPE(item, obj_file_t) && (AS_FILE(item)->ptr)) {
        LUCI_DIE("%s\n", "File assertion failed");
    }
    return LuciNilObj;
}

/**
 * Deep copies an object
 *
 * @param args list of args
 * @param c number of args
 * @returns deep copy of argument
 */
LuciObject *luci_copy(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s\n", "Missing argument to copy()");
    }
    LuciObject *o = args[0];
    return o->type->copy(o);
}

/**
 * Casts the LuciObject to a LuciIntObj if possible then returns
 * the new object.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciIntObj cast of the first arg
 */
LuciObject *luci_cast_int(LuciObject **args, unsigned int c)
{
    LuciObject *ret = LuciNilObj;
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to int()\n");
    }
    LuciObject *item = args[0];

    if (!item) {
        LUCI_DIE("%s", "Can't cast NULL to int\n");
    }

    if (ISTYPE(item, obj_int_t)) {
        ret = LuciInt_new(AS_INT(item)->i);
    } else if (ISTYPE(item, obj_float_t)) {
        ret = LuciInt_new((long)AS_FLOAT(item)->f);
    } else if (ISTYPE(item, obj_string_t)) {
        long i;
        int scanned = sscanf(AS_STRING(item)->s, "%ld", &i);
        if (scanned <= 0 || scanned == EOF) {
            LUCI_DIE("%s", "Could not cast to int\n");
        }
        ret = LuciInt_new(i);
    } else {
        LUCI_DIE("Cannot cast type %s to type int\n", item->type->type_name);
    }
    return ret;
}

/**
 * Casts a LuciObject to a LuciFloatObj if possible, then returns
 * the new object.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciFloatObj cast of the first arg
 */
LuciObject *luci_cast_float(LuciObject **args, unsigned int c)
{
    LuciObject *ret = LuciNilObj;
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to int()\n");
    }
    LuciObject *item = args[0];

    if (!item) {
        LUCI_DIE("%s", "Can't cast NULL to int\n");
    }

    if (ISTYPE(item, obj_int_t)) {
        ret = LuciFloat_new((double)AS_INT(item)->i);
    } else if (ISTYPE(item, obj_float_t)) {
        ret = LuciFloat_new(AS_FLOAT(item)->f);
    } else if (ISTYPE(item, obj_string_t)) {
        double f;
        int scanned = sscanf(AS_STRING(item)->s, "%f", (float *)&f);
        if (scanned <= 0 || scanned == EOF) {
            LUCI_DIE("%s", "Could not cast to float\n");
        }
        ret = LuciFloat_new(f);
    } else {
        LUCI_DIE("Cannot cast type %s to type float\n", item->type->type_name);
    }

    return ret;
}

/**
 * Returns a hex string representation of an integer
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciStringObj representation of an integer
 */
LuciObject *luci_hex(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s\n", "Missing param to hex()");
    }
    LuciObject *hexint = args[0];

    if (!ISTYPE(hexint, obj_int_t)) {
        LUCI_DIE("Cannot get hex representation of an object of type %s\n",
                hexint->type->type_name);
    }

    char *s = alloc(MAX_INT_DIGITS + 2);
    snprintf(s, MAX_INT_DIGITS, "0x%lX", AS_INT(hexint)->i);
    return LuciString_new(s);
}

/**
 * Casts a LuciObject to a LuciStringObj if possible, then returns
 * the new object.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciStringObj cast of the first arg
 */
LuciObject *luci_cast_str(LuciObject **args, unsigned int c)
{
    LuciObject *ret = LuciNilObj;

    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to str()\n");
    }
    /* grab the first parameter from the param list */
    LuciObject *item = args[0];

    ret = item->type->repr(item);
    if (ISTYPE(ret, obj_nil_t)) {
        LUCI_DIE("Cannot cast object of type %s to type string",
                item->type->type_name);
    }
    LUCI_DEBUG("str() returning %s\n", AS_STRING(ret)->s);

    return ret;
}

/**
 * Determines the file 'open' mode from a given open
 * string.
 *
 * Performs similarly to C fopen
 *
 * @param req_mode C-string representing file-open mode
 * @returns enumerated file mode
 */
static file_mode get_file_mode(const char *req_mode)
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

/** Closes a file pointer opened during the creation
 * of a LuciFileObj
 * @param fp C file pointer to close
 * @returns 1 on success, EOF on failure
 */
static int close_file(FILE *fp)
{
    int ret = 1;
    if (fp) {
        ret = fclose(fp);
        fp = NULL;
    }
    return ret;
}


/**
 * Opens a file in read, write, or append mode.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciFileObj opened from the filename in the first arg
 */
LuciObject *luci_fopen(LuciObject **args, unsigned int c)
{
    char *filename;
    char *req_mode;
    int mode;
    FILE *file = NULL;

    if (c < 2) {
        LUCI_DIE("%s", "Missing parameter to open()\n");
    }

    LuciObject *fname_obj = args[0];
    if (!ISTYPE(fname_obj, obj_string_t)) {
        LUCI_DIE("%s", "Parameter 1 to open must be a string\n");
    }
    LuciObject *mode_obj = args[1];
    if (!ISTYPE(mode_obj, obj_string_t)) {
        LUCI_DIE("%s", "Parameter 2 to open must be a string\n");
    }

    filename = AS_STRING(fname_obj)->s;
    req_mode = AS_STRING(mode_obj)->s;

    mode = get_file_mode(req_mode);
    if (mode < 0) {
        LUCI_DIE("%s\n", "Invalid mode to open()");
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
        LUCI_DIE("Could not open file %s\n", filename);
    }

    LuciObject *ret = LuciFile_new(file, file_length, mode);

    LUCI_DEBUG("Opened file %s of size %ld bytes with mode %s.\n",
            filename, file_length, req_mode);

    return ret;
}

/**
 * Closes an open LuciFileObj.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciNilObj
 */
LuciObject *luci_fclose(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to close()\n");
    }

    LuciObject *fobj = args[0];

    if (!ISTYPE(fobj, obj_file_t)) {
        LUCI_DIE("%s", "Not a file object\n");
    }

    if (AS_FILE(fobj)->ptr) {
        close_file(AS_FILE(fobj)->ptr);
    }
    /* else, probably already closed (it's NULL) */

    LUCI_DEBUG("%s\n", "Closed file object.");

    return LuciNilObj;
}

/**
 * Reads the contents of a file into a LuciStringObj.
 *
 * @param args list of args
 * @param c number of args
 * @returns contents of the file from the first arg.
 */
LuciObject *luci_fread(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to read()\n");
    }

    LuciObject *fobj = args[0];

    if (!ISTYPE(fobj, obj_file_t)) {
        LUCI_DIE("%s", "Not a file object\n");
    }

    if (AS_FILE(fobj)->mode != f_read_m) {
        LUCI_DIE("%s", "Can't open  It is opened for writing.\n");
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

/**
 * Writes a given LuciStringObj to a LuciFileObj.
 *
 * @param args list of args
 * @param c number of args
 * @returns LuciNilObj
 */
LuciObject *luci_fwrite(LuciObject **args, unsigned int c)
{
    if (c < 2) {
        LUCI_DIE("%s", "Missing parameter to write()\n");
    }

    /* grab the FILE parameter */
    LuciObject *fobj = args[0];
    if (!fobj || (!ISTYPE(fobj, obj_file_t))) {
        LUCI_DIE("%s", "Not a file object\n");
    }

    /* grab string parameter */
    LuciObject *text_obj = args[1];
    if (!text_obj || (!ISTYPE(text_obj, obj_string_t)) ) {
        LUCI_DIE("%s", "Not a string\n");
    }
    char *text = AS_STRING(text_obj)->s;

    if (AS_FILE(fobj)->mode == f_read_m) {
        LUCI_DIE("%s", "Can't write to  It is opened for reading.\n");
    }

    fwrite(text, sizeof(char), strlen(text), AS_FILE(fobj)->ptr);

    return LuciNilObj;
}

/**
 * Reads all the lines from the given input to create a
 * list of lines.
 *
 * @param args list of args
 * @param c number of args
 * @returns list of lines from file or stdin
 */
LuciObject *luci_flines(LuciObject **args, unsigned int c)
{
    LuciObject *list = LuciList_new();
    LuciObject *line = luci_readline(args, c);

    /* read lines until either NULL or LuciNilObj returned */
    while (line && !ISTYPE(line, obj_nil_t)) {
        LuciList_append(list, line);
        line = luci_readline(args, c);
    }

    return list;
}

/**
 * Creates a LuciListObj containing a range of numbers.
 *
 * @param args list of args
 * @param c number of args
 * @returns list of numbers
 */
LuciObject * luci_range(LuciObject **args, unsigned int c)
{
    long start, end, incr;
    LuciObject *first, *second, *third;

    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to range()\n");
    }

    first = args[0];
    if (!ISTYPE(first, obj_int_t)) {
        LUCI_DIE("%s", "First parameter to range must be integer\n");
    }

    if (c > 1) {
        second = args[1];
        if (!ISTYPE(second, obj_int_t)) {
            LUCI_DIE("%s", "Second parameter to range must be integer\n");
        }
        start = AS_INT(first)->i;
        end = AS_INT(second)->i;

        if (c > 2) {
            /* Ternary range(X, Y, Z) call */
            third = args[2];
            if (!ISTYPE(third, obj_int_t)) {
                LUCI_DIE("%s", "Third parameter to range must be integer\n");
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
    long i;
    if (incr < 0) {
        if (start <= end) {
            /* return empty list if idiotically requested */
            return list;
        }

        for (i = start; i > end; i += incr) {
            item = LuciInt_new(i);
            LuciList_append(list, item);
        }
    }
    else {
        if (start >= end) {
            /* return empty list if idiotically requested */
            return list;
        }

        for (i = start; i < end; i += incr) {
            item = LuciInt_new(i);
            LuciList_append(list, item);
        }
    }

    return list;
}

/**
 * Computes the sum of a range of numbers.
 *
 * @param args list of args
 * @param c number of args
 * @returns sum of numbers
 */
LuciObject * luci_sum(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to sum()\n");
    }

    LuciObject *list = args[0];

    if (!list || (!ISTYPE(list, obj_list_t))) {
        LUCI_DIE("%s", "Must specify a list to calculate sum\n");
    }

    LuciObject *item;
    double sum = 0;
    unsigned int i, found_float = 0;
    for (i = 0; i < AS_LIST(list)->count; i++) {
        item = AS_LIST(list)->items[i];
        if (!item) {
            LUCI_DIE("%s", "Can't calulate sum of list containing NULL value\n");
        }

        if (ISTYPE(item, obj_int_t)) {
            sum += (double)AS_INT(item)->i;
        } else if (ISTYPE(item, obj_float_t)) {
            found_float = 1;
            sum += AS_FLOAT(item)->f;
        } else {
            LUCI_DIE("%s", "Can't calculate sum of list containing non-numeric value\n");
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

/**
 * Determines and returns the size of a container object.
 *
 * @param args list of args
 * @param c number of args
 * @returns size of container
 */
LuciObject *luci_len(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to len()\n");
    }

    LuciObject *container = args[0];

    return container->type->len(container);
}
/**
 * Finds the maximum value in a LuciListObj.
 *
 * @param args list of args
 * @param c number of args
 * @returns max number in list
 */
LuciObject *luci_max(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to max()\n");
    }

    LuciObject *list = args[0];

    if (!list || (!ISTYPE(list, obj_list_t))) {
        LUCI_DIE("%s", "Must specify a list to calculate max\n");
    }

    LuciObject *item;
    double max = 0;
    unsigned int i, found_float = 0;
    for (i = 0; i < AS_LIST(list)->count; i ++) {
        item = AS_LIST(list)->items[i];
        if (!item) {
            LUCI_DIE("%s", "Can't calulate max of list containing NULL value\n");
        }
        if (ISTYPE(item, obj_int_t)) {
            if ( (double)AS_INT(item)->i > max) {
                max = (double)AS_INT(item)->i;
            }
        } else if (ISTYPE(item, obj_float_t)) {
            found_float = 1;
            if (AS_FLOAT(item)->f > max) {
                max = AS_FLOAT(item)->f;
            }
        } else {
            LUCI_DIE("Can't find max of list containing an object of type %s\n",
                    item->type->type_name);
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

/**
 * Finds the minimum value in a list.
 *
 * @param args list of args
 * @param c number of args
 * @returns min number in list
 */
LuciObject *luci_min(LuciObject **args, unsigned int c)
{
    if (c < 1) {
        LUCI_DIE("%s", "Missing parameter to min()\n");
    }

    LuciObject *list = args[0];

    if (!list || (!ISTYPE(list, obj_list_t))) {
        LUCI_DIE("%s", "Must specify a list to calculate min\n");
    }

    LuciObject *item;
    double min = 0;
    unsigned int i, found_float = 0;

    for (i = 0; i < AS_LIST(list)->count; i ++) {
        item = AS_LIST(list)->items[i];
        if (!item) {
            LUCI_DIE("%s", "Can't calulate max of list containing NULL value\n");
        }
        if (ISTYPE(item, obj_int_t)) {
            if (i == 0) {
                min = (double)AS_INT(item)->i;
            }
            else if ( (double)AS_INT(item)->i < min) {
                min = (double)AS_INT(item)->i;
            }
        } else if (ISTYPE(item, obj_float_t)) {
            found_float = 1;
            if (i == 0) {
                min = AS_FLOAT(item)->f;
            }
            else if (AS_FLOAT(item)->f < min) {
                min = AS_FLOAT(item)->f;
            }
        } else {
            LUCI_DIE("Can't find min of list containing an object of type %s\n",
                    item->type->type_name);
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

/**
 * Determines whether a container contains an object
 *
 * @param args list of args
 * @param c number of args
 * @returns min number in list
 */
LuciObject *luci_contains(LuciObject **args, unsigned int c)
{
    if (c < 2) {
        LUCI_DIE("%s", "Missing parameter to contains()\n");
    }

    LuciObject *cont = args[0];
    LuciObject *item = args[1];

    if (!cont) {
        LUCI_DIE("%s", "NULL container in contains()\n");
    }

    return cont->type->contains(cont, item);
}
