%{

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "ast.h"
#include "env.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

int LINENUM = 1;

void yyerror(const char *msg);

%}

%error-verbose

%union {
    long i_val;
    double f_val;
    char *s_val;
    char *id;
    struct AstNode *node;
}

%start program

%type <node> expr cond call assignment statement statements
%type <node> while_loop for_loop if_else
%type <node> func_def params
%type <node> empty_list list_items list_index list
%type <node> program

%token <i_val> INT
%token <f_val> FLOAT
%token <s_val> STRING
%token <id> ID
%token NEWLINE COMMA
%token WHILE FOR IN DO DONE
%token IF THEN ELSE END
%token DEF RETURN

%left LGOR LGAND
%left BWOR BWXOR BWAND
%left EQUAL NOTEQ
%left LTHAN LTHEQ GTHAN GTHEQ
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left POW
%left LPAREN RPAREN
%left LSQUARE RSQUARE

%right ASSIGN
%right LGNOT BWNOT
%right UPLUS UMINUS ULGNOT UBWNOT

%%

program:
        /* nothing */       { $$ = 0; }
    |   statements          { (*(struct AstNode **)root) = $1; }
    ;

statements:
        statement NEWLINE
                { $$ = make_statements(LINENUM, NULL, $1); }
    |   statements statement NEWLINE
                { $$ = make_statements(LINENUM, $1, $2); }
    ;

statement:
        func_def            { $$ = $1; }
    |   assignment          { $$ = $1; }
    |   call                { $$ = $1; }
    |   while_loop          { $$ = $1; }
    |   for_loop            { $$ = $1; }
    |   if_else             { $$ = $1; }
    ;

assignment:
        ID ASSIGN expr
                { $$ = make_assignment(LINENUM, $1, $3); }
    |   ID LSQUARE expr RSQUARE ASSIGN expr
                { $$ = make_list_assignment(LINENUM, $1, $3, $6); }
    ;

func_def:
        DEF ID LPAREN empty_list RPAREN NEWLINE statements END
            { $$ = make_func_def(LINENUM, $2, $4, $7, NULL); }
    |   DEF ID LPAREN empty_list RPAREN NEWLINE RETURN expr NEWLINE END
            { $$ = make_func_def(LINENUM,
                    $2, $4, make_statements(LINENUM, NULL, NULL), $8);
            }
    |   DEF ID LPAREN empty_list RPAREN NEWLINE statements RETURN expr NEWLINE END
            { $$ = make_func_def(LINENUM, $2, $4, $7, $9); }
    |   DEF ID LPAREN params RPAREN NEWLINE statements END
            { $$ = make_func_def(LINENUM, $2, $4, $7, NULL); }
    |   DEF ID LPAREN params RPAREN NEWLINE RETURN expr NEWLINE END
            { $$ = make_func_def(LINENUM,
                    $2, $4, make_statements(LINENUM, NULL, NULL), $8);
            }
    |   DEF ID LPAREN params RPAREN NEWLINE statements RETURN expr NEWLINE END
            { $$ = make_func_def(LINENUM, $2, $4, $7, $9); }
    ;

params:
        ID      { $$ = make_list_def(LINENUM, NULL,
                        make_string_expr(LINENUM, $1));
                }
    |   params COMMA ID
                { $$ = make_list_def(LINENUM,  $1,
                        make_string_expr(LINENUM, $3));
                }
    ;

call:
        ID LPAREN empty_list RPAREN
                { $$ = make_func_call(LINENUM, $1, $3); }
    |   ID LPAREN list_items RPAREN
                { $$ = make_func_call(LINENUM, $1, $3); }
    ;

list:
        LSQUARE empty_list RSQUARE      { $$ = $2; }
    |   LSQUARE list_items RSQUARE      { $$ = $2; }
    ;

empty_list:
    /* nothing */   { $$ = make_list_def(LINENUM, NULL, NULL); }
    ;

list_items:
        expr        { $$ = make_list_def(LINENUM, NULL, $1); }
    |   list_items COMMA expr   { $$ = make_list_def(LINENUM, $1, $3); }
    ;

list_index:
        expr LSQUARE expr RSQUARE
                { $$ = make_list_index(LINENUM, $1, $3); }
    ;

while_loop:
        WHILE cond DO NEWLINE statements DONE
                { $$ = make_while_loop(LINENUM, $2, $5); }
    ;

for_loop:
        FOR ID IN expr DO NEWLINE statements DONE
                { $$ = make_for_loop(LINENUM, $2, $4, $7); }
    ;

if_else:
        IF cond THEN NEWLINE statements END
                { $$ = make_if_else(LINENUM, $2, $5, NULL); }
    |   IF cond THEN NEWLINE statements ELSE NEWLINE statements END
                { $$ = make_if_else(LINENUM, $2, $5, $8); }
    ;

cond:
        expr                { $$ = $1; }
    |   expr NEWLINE        { $$ = $1; }
    ;

expr:
        INT                     { $$ = make_int_expr(LINENUM, $1); }
    |   FLOAT                   { $$ = make_float_expr(LINENUM, $1); }
    |   STRING                  { $$ = make_string_expr(LINENUM, $1); }
    |   ID                      { $$ = make_id_expr(LINENUM, $1); }
    |   expr PLUS expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_add_t); }
    |   expr MINUS expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_sub_t); }
    |   expr TIMES expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_mul_t); }
    |   expr DIVIDE expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_div_t); }
    |   expr POW expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_pow_t); }
    |   expr MOD expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_mod_t); }
    |   expr LTHAN expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_lt_t); }
    |   expr GTHAN expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_gt_t); }
    |   expr NOTEQ expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_neq_t); }
    |   expr EQUAL expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_eq_t); }
    |   expr LTHEQ expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_lte_t); }
    |   expr GTHEQ expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_gte_t); }
    |   expr LGOR expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_lor_t); }
    |   expr LGAND expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_land_t); }
    |   expr BWXOR expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_bxor_t); }
    |   expr BWOR expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_bor_t); }
    |   expr BWAND expr
                { $$ = make_binary_expr(LINENUM, $1, $3, op_band_t); }
    |   BWNOT expr %prec UBWNOT
                { $$ = make_binary_expr(LINENUM,
                        make_int_expr(LINENUM, 0), $2, op_bnot_t);
                }
    |   LGNOT expr %prec ULGNOT
                { $$ = make_binary_expr(LINENUM,
                        make_int_expr(LINENUM, 0), $2, op_lnot_t);
                }
    |   MINUS expr %prec UMINUS
                { $$ = make_binary_expr(LINENUM,
                        make_int_expr(LINENUM, 0), $2, op_sub_t);
                }
    |   LPAREN expr RPAREN      { $$ = $2; }
    |   list_index              { $$ = $1; }
    |   list                    { $$ = $1; }
    |   call                    { $$ = $1; }
    ;

%%

void yyerror(const char *msg)
{
    die("Syntax Error: %s @ line #%d\n", msg, LINENUM);
}

