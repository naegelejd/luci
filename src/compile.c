#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "common.h"
#include "functions.h"
#include "stack.h"
#include "ast.h"

static void _compile(AstNode *, Program *);
static void add_instr(Program *, Opcode,
        Immediate *, Immediate *, Immediate *);

static int destroy_symbol(Symbol *s, int force);
static Symbol *add_symbol (Symbol *head, char const *name, int type);
static Symbol *get_symbol (Symbol *head, const char *name);

static char *instr_names[] = {
    "CREATE",
    "INCREF",
    "DECREF",
    "BINOP",
    "LOAD",
    "STORE",
    "CALL",
    "JUMPL",
    "JUMPN",
    "TEST",
    "BUILD_LIST",
    "EXIT"
};

/**
 * Public entry point for compilation.
 *
 * Allocates a Program struct and passes it to
 * the AST walker/compiler.
 */
Program * compile_ast(AstNode *root)
{
    if (!root)
        die("Nothing to compile\n");

    Program *prog = alloc(sizeof(*prog));
    prog->count = 0;
    prog->size = BASE_INSTR_COUNT;
    prog->instructions = alloc(prog->size * sizeof(*(prog->instructions)));

    /* compile the AST */
    _compile(root, prog);

    /* end the program with an EXIT instr */
    add_instr(prog, EXIT, NULL, NULL, NULL);

    return prog;
}

void destroy_program(Program *prog)
{
    /* destroy each instruction, instruction list, and program */
    int i = 0;
    Instruction **instructions = prog->instructions;
    for (i = 0; i < prog->count; i ++) {
        free(instructions[i]);
    }
    free(prog->instructions);
    free(prog);
}

static void add_instr(Program *prog, Opcode op,
        Immediate *a, Immediate *b, Immediate *c)
{
    if (!prog)
        die("Program not allocated. Can't add instruction\n");
    if (!(prog->instructions))
        die("Instruction list not allocated. Can't add instruction\n");

    /* Allocate and initialize the instruction */
    Instruction *new_instr = alloc(sizeof(*new_instr));
    new_instr->opcode = op;
    new_instr->a = a;
    new_instr->b = c;
    new_instr->b = c;

    /* Reallocate the program's instruction list if necessary */
    if (++(prog->count) > prog->size) {
        prog->size <<= 1;
        prog->instructions = realloc(prog->instructions,
                prog->size * sizeof(*(prog->instructions)));
    }

    /* Append the new instruction to the instruction list */
    prog->instructions[prog->count - 1] = new_instr;
}

static Immediate * new_long_immediate(long l) {
    Immediate *new = alloc(sizeof(*new));
    new->t = imm_long_t;
    new->v.l = l;
    return new;
}

static Immediate * new_double_immediate(double d) {
    Immediate *new = alloc(sizeof(*new));
    new->t = imm_double_t;
    new->v.d = d;
    return new;
}

static Immediate * new_string_immediate(char *s) {
    Immediate *new = alloc(sizeof(*new));
    new->t = imm_string_t;
    new->v.s = s;
    return new;
}

static void _compile(AstNode *node, Program *prog)
{
    int i = 0;
    Immediate *x = NULL, *y = NULL, *z = NULL;
    AstNode *tmp = NULL;
    long addr = 0;

    if (!node) {
        /* don't compile a NULL statement */
        yak("NOP");
        return;
    }

    switch (node->type)
    {
        case ast_stmnts_t:
            for (i = 0; i < node->data.statements.count; i++)
            {
                _compile(node->data.statements.statements[i], prog);
            }
            break;
        case ast_func_t:
            yak("PUSH params");
            _compile(node->data.funcdef.param_list, prog);
            _compile(node->data.funcdef.statements, prog);
            _compile(node->data.funcdef.ret_expr, prog);
            yak("STORE func->funcname in symtable");
            _compile(node->data.funcdef.funcname, prog);
            break;
        case ast_list_t:
            /* create a LuciObject list from the AstNode */
            yak("CREATE new list");
            /* push the new list object onto the stack */
            for (i = 0; i < node->data.list.count; i++)
            {
                _compile(node->data.list.items[i], prog);
            }
            x = new_long_immediate(node->data.list.count);
            add_instr(prog, BUILD_LIST, x, NULL, NULL);
            break;
        case ast_while_t:
            yak("LABEL while# begin");
            yak("TEST cond? JUMP 2 : JUMP while# end");
            _compile(node->data.while_loop.cond, prog);
            _compile(node->data.while_loop.statements, prog);
            yak("JUMP to while loop#");
            yak("LABEL while# end");
            break;
        case ast_for_t:
            _compile(node->data.for_loop.list, prog);
            _compile(node->data.for_loop.statements, prog);
            _compile(node->data.for_loop.iter, prog);
            break;
        case ast_if_t:
            yak("TEST cond? JUMP 2 : JUMP else #");
            _compile(node->data.if_else.cond, prog);
            _compile(node->data.if_else.ifstatements, prog);
            yak("LABEL else#");
            _compile(node->data.if_else.elstatements, prog);
            break;
        case ast_assign_t:
            _compile(node->data.assignment.right, prog);
            _compile(node->data.assignment.name, prog);
            add_instr(prog, DECREF, NULL, NULL, NULL);
            tmp = node->data.assignment.name;
            x = new_string_immediate(tmp->data.id_val);
            add_instr(prog, STORE, x, NULL, NULL);
            yak("DECREF (sp) if not NULL");
            yak("STORE (--sp) (sp++)");
            break;
        case ast_call_t:
            /* compile arglist, which pushes list onto stack */
            _compile(node->data.call.arglist, prog);
            /* compile funcname, which pushes symbol value onto stack */
            _compile(node->data.call.funcname, prog);
            add_instr(prog, CALL, NULL, NULL, NULL);
            yak("CALL (--sp)(--sp)");
            break;
        case ast_listindex_t:
            _compile(node->data.listindex.list, prog);
            _compile(node->data.listindex.index, prog);
            yak("POP -> index");
            yak("POP -> list");
            yak("GET list index");
            break;
        case ast_listassign_t:
            _compile(node->data.listassign.list, prog);
            _compile(node->data.listassign.index, prog);
            _compile(node->data.listassign.right, prog);
            yak("LASSIGN list index x");
            break;
        case ast_expr_t:
            _compile(node->data.expression.left, prog);
            _compile(node->data.expression.right, prog);
            /* make long immediate from int op */
            x = new_long_immediate(node->data.expression.op);
            add_instr(prog, BINOP, x, NULL, NULL);
            yak("OP (--sp) (--sp)"); /* push result ! */
            break;
        case ast_id_t:
            x = new_string_immediate(node->data.id_val);
            add_instr(prog, LOAD, x, NULL, NULL);
            yak("LOAD %s\n", node->data.id_val);
            yak("PUSH &%s\n", node->data.id_val);
            break;
        case ast_string_t:
            x = new_string_immediate(node->data.s_val);
            add_instr(prog, CREATE, x, NULL, NULL);
            yak("CREATE new \"%s\"\n", node->data.s_val);
            yak("PUSH &new\n");
            break;
        case ast_float_t:
            x = new_double_immediate(node->data.f_val);
            add_instr(prog, CREATE, x, NULL, NULL);
            yak("CREATE new %g\n", node->data.f_val);
            yak("PUSH &new\n");
            break;
        case ast_int_t:
            x = new_long_immediate(node->data.i_val);
            add_instr(prog, CREATE, x, NULL, NULL);
            yak("CREATE new %ld\n", node->data.i_val);
            yak("PUSH &new\n");
            break;
        default:
            break;
    }

    return;
}

/*
   Searches for a symbol with a matching name in the
   symbol table of the specified Execution Context
*/
static Symbol *get_symbol (Symbol *head, const char *name)
{
    Symbol *ptr;
    for (ptr = head; ptr != (Symbol *) 0; ptr = (Symbol *)ptr->next)
        if (strcmp (ptr->name, name) == 0)
            return ptr;

    return 0;
}

/*
   Adds a new symbol to the symbol table of the specified
   Execution Context, returning a pointer to the new symbol.

   Only the symbol's name and type must be given. The caller
   is responsible for setting the symbol's data.
*/
static Symbol *add_symbol (Symbol *head, char const *name, int type)
{
    Symbol *new=NULL, *ptr=NULL, *prev=NULL;

    /* Create New Symbol */
    yak("creating new symbol %s\n", name);
    new = (Symbol *) alloc (sizeof (Symbol));
    new->name = (char *) alloc (strlen (name) + 1);
    strcpy (new->name, name);
    new->type = type;

    yak("searching for symbol %s\n", name);
    /* First, do a search for the symbol */
    for (ptr = head, prev = NULL;
            ptr != (Symbol *) NULL;
            prev = ptr, ptr = ptr->next) {

	/* if we find the symbol */
	if (strcmp (ptr->name, name) == 0) {
	    yak("found existing symbol %s\n", name);
	    /* maintain the linked list by pointing the previous symbol
             * to the next
             */
	    if (prev)
		prev->next = ptr->next;
	    else
		head = ptr->next;
	    /* free the symbol's memory */
	    if (!(destroy_symbol(ptr, 0))) {
		return 0;
	    }
	}
    }

    if (prev) {
        /* append new to the end of the symbol table */
        prev->next = new;
        new->next = NULL;
    }

    /* caller must explicitly set the data (payload) */
    return new;
}

/* destroys the memory used by Symbol *s
   This will will only delete builtin symbols
   if `force` is non-zero
*/
static int destroy_symbol(Symbol *s, int force)
{
    if (!s) {
	return 1;
    }

    /* TODO: this should close any open file pointers...
       if a symbol points to a FILE object and the user
       wants to point the symbol to something else, it would
       be nice for the file pointer to be closed
    */

    /* destroy the symbol's name */
    yak("destroying symbol %s (type %d)\n", s->name, s->type);
    free(s->name);
    /* destroy the symbol's payload */
    switch (s->type)
    {
	case sym_bobj_t:
	    if (!force) {
		return 0;
	    }
	    /* else, destroy object */
	    destroy_object(s->data.object);
	    break;
	case sym_bfunc_t:
	    if (!force) {
		return 0;
	    }
	    break;
	case sym_uobj_t:
	    destroy_object(s->data.object);
	    break;
	case sym_ufunc_t:
	    break;
	default:
	    ;
    }
    /* destroy the symbol (struct) */
    free(s);
    s = NULL;
    /* return success */
    return 1;
}

void print_instructions(Program *prog)
{
    int i = 0;
    for (i = 0; i < prog->count; i ++)
        printf("%03x: %s\n", i, instr_names[prog->instructions[i]->opcode]);
}

#define EVER ;;

void eval(Program *prog)
{
    LuciObject *x = NULL, *y = NULL, *z = NULL;
    Immediate *a = NULL, *b = NULL, *c = NULL;
    Instruction **all_instr = prog->instructions;
    Instruction *instr = NULL;

    Symbol *s = NULL;
    Symbol *symtable = add_symbol(NULL, "print", sym_bfunc_t);
    symtable->data.funcptr = luci_print;

    Stack lstack;
    st_init(&lstack);

    int ip = 0;
    int i = 0;

    for(EVER) {
        instr = all_instr[ip++];
        a = instr->a;
        b = instr->b;
        c = instr->c;
        switch (instr->opcode) {
            case CREATE:
                switch (a->t) {
                    case imm_long_t:
                        printf("Creating int %ld\n", a->v.l);
                        x = create_object(obj_int_t);
                        x->value.i_val = a->v.l;
                        break;
                    case imm_double_t:
                        printf("Creating float %g\n", a->v.d);
                        x = create_object(obj_float_t);
                        x->value.f_val = a->v.d;
                        break;
                    case imm_string_t:
                        printf("Creating string %s\n", a->v.s);
                        x = create_object(obj_str_t);
                        x->value.s_val = alloc(strlen(a->v.s) + 1);
                        strcpy(x->value.s_val, a->v.s);
                        break;
                    default:
                        die("Can't create unknown type\n");
                }
                printf("Pushing new object @ %ld\n", x);
                st_push(&lstack, x);
                break;
            case BINOP:
                x = st_pop(&lstack);
                y = st_pop(&lstack);
                printf("Performing binary op %d on top 2 stack values\n", a->v.l);
                /* a->v.l is the (long) expression op enum */
                z = solve_bin_expr(x, y, a->v.l);
                break;
            case LOAD:
                /* lookup symbol (arg to LOAD) */
                printf("LOAD symbol %s\n", a->v.s);
                s = get_symbol(symtable, a->v.s);
                /* if symbol DNE, push NULL onto stack */
                if (!s) {
                    printf("Symbol not found\n");
                    st_push(&lstack, NULL);
                    break;
                }

                switch (s->type) {
                    case sym_uobj_t:
                    case sym_bobj_t:
                        x = s->data.object;
                        break;
                    case sym_ufunc_t:
                    case sym_bfunc_t:
                        x = s->data.funcptr;
                        break;
                    default:
                        die("uh oh, what is symbol '%s' pointing to?", s->name);
                }
                printf("Found symbol %s, pushing object\n", a->v.s);
                /* push object onto stack */
                st_push(&lstack, x);
                break;
            case STORE:
                printf("STORE object in symbol %s\n", a->v.s);
                /* add new symbol with name arg to STORE */
                s = add_symbol(symtable, a->v.s, sym_uobj_t);
                /* set symbol payload to object at top of stack */
                x = st_pop(&lstack);
                printf("Storing obj @ %ld\n", x);
                s->data.object = x;
                break;
            case DECREF:
                puts("DECREF object on top of stack");
                x = st_pop(&lstack);
                destroy_object(x);
                break;
            case INCREF:
                x = st_pop(&lstack);
                reference_object(x);
                break;
            case CALL:
                printf("Calling function at top of stack\n");
                x = st_pop(&lstack);    /* funcptr obj */
                y = st_pop(&lstack);    /* arglist obj */
                luci_print(y);
                break;
            case BUILD_LIST:
                printf("Building list\n");
                y = create_object(obj_list_t);
                for (i = 0; i < a->v.l; i ++) {
                    x = st_pop(&lstack);
                    list_append_object(y, x);
                }
                st_push(&lstack, y);
                break;
            case EXIT:
                puts("EXIT");
                goto done_eval;
                break;
            default:
                die("Invalid opcode: %d\n", instr->opcode);
                break;
        }
    }

done_eval:;

    /* destroy the Context's symbol table */
    Symbol *ptr = symtable;
    Symbol *next = ptr;
    while (ptr != (Symbol *) 0)
    {
	next = (Symbol *)ptr->next;
	/* destroy symbol, force destruction of builtins */
	destroy_symbol(ptr, 1);
	ptr = next;
    }

    return;
}
