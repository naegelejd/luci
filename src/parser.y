/*
 * See Copyright Notice in luci.h
 */

%{

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "ast.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

void yyerror(const char *msg);

%}

%error-verbose

%union {
    long int_v;
    double float_v;
    char *string_v;
    struct AstNode *node;
}

%start program

%type <node> statement statements
%type <node> expr id call assignment
%type <node> while_loop for_loop if_else
%type <node> func_def params return
%type <node> empty_list list_items list_index list_access list_assign list
%type <node> program

%token <int_v> INT
%token <float_v> FLOAT
%token <string_v> STRING ID

%token NEWLINE SEMICOLON COMMA
%token WHILE FOR IN DO DONE
%token BREAK CONTINUE
%token IF THEN ELSE END
%token DEF RETURN
%token PASS

%left LGOR LGAND
%left BWOR BWXOR BWAND
%left EQUAL NOTEQ
%left LTHAN LTHEQ GTHAN GTHEQ
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left POW
%left LPAREN RPAREN
%left LSQUARE RSQUARE
%left LBRACK RBRACK

%right ASSIGN
%right LGNOT BWNOT
%right UPLUS UMINUS ULGNOT UBWNOT

%%

program:
        /* nothing */       { $$ = 0; }
    |   statements          { (*(struct AstNode **)root) = $1; }
    ;

statements:
        statement
                { $$ = make_statements(NULL, $1); }
    |   statements statement
                { $$ = make_statements($1, $2); }
    ;

statement:
        while_loop          { $$ = $1; }
    |   for_loop            { $$ = $1; }
    |   if_else             { $$ = $1; }
    |   func_def            { $$ = $1; }
    |   expr SEMICOLON              { $$ = $1; }
    |   list_assign SEMICOLON       { $$ = $1; }
    |   assignment SEMICOLON        { $$ = $1; }
    |   return SEMICOLON            { $$ = $1; }
    |   BREAK SEMICOLON             { $$ = make_break(); }
    |   CONTINUE SEMICOLON          { $$ = make_continue(); }
    |   PASS SEMICOLON              { $$ = make_pass(); }
    ;

assignment:
        ID ASSIGN expr
                { $$ = make_assignment($1, $3); }
    |   ID ASSIGN assignment
                { $$ = make_assignment($1, $3); }
    ;

list_assign:
        id list_index ASSIGN expr
                { $$ = make_list_assignment($1, $2, $4); }
    ;

func_def:
        DEF ID LPAREN empty_list RPAREN LBRACK statements RBRACK
            { $$ = make_func_def($2, $4, $7); }
    |   DEF ID LPAREN params RPAREN LBRACK statements RBRACK
            { $$ = make_func_def($2, $4, $7); }
    ;

return:     RETURN expr     { $$ = make_return($2); };

params:
        ID      { $$ = make_list_def(NULL, make_string_constant($1)); }
    |   params COMMA ID
                { $$ = make_list_def($1, make_string_constant($3)); }
    ;

call:
        id LPAREN empty_list RPAREN
                { $$ = make_func_call($1, $3); }
    |   id LPAREN list_items RPAREN
                { $$ = make_func_call($1, $3); }
    ;

list:
        LSQUARE empty_list RSQUARE      { $$ = $2; }
    |   LSQUARE list_items RSQUARE      { $$ = $2; }
    ;

empty_list:
    /* nothing */   { $$ = make_list_def(NULL, NULL); }
    ;

list_items:
        expr        { $$ = make_list_def(NULL, $1); }
    |   list_items COMMA expr   { $$ = make_list_def($1, $3); }
    ;

list_access:
        expr list_index { $$ = make_list_access($1, $2); };

list_index:
        LSQUARE expr RSQUARE
                { $$ = $2; /* make_list_index($1, $3); */ };

while_loop:
        WHILE expr LBRACK statements RBRACK
                { $$ = make_while_loop($2, $4); }
    ;

for_loop:
        FOR ID IN expr LBRACK statements RBRACK
                { $$ = make_for_loop($2, $4, $6); }
    ;

if_else:
        IF expr LBRACK statements RBRACK
                { $$ = make_if_else($2, $4, NULL); }
    |   IF expr LBRACK statements RBRACK ELSE LBRACK statements RBRACK
                { $$ = make_if_else($2, $4, $8); }
    ;

id:     ID                      { $$ = make_id_expr($1); }
    ;

expr:
        INT                     { $$ = make_int_constant($1); }
    |   FLOAT                   { $$ = make_float_constant($1); }
    |   STRING                  { $$ = make_string_constant($1); }
    |   id                      { $$ = $1; }
    |   expr PLUS expr
                { $$ = make_binary_expr($1, $3, op_add_t); }
    |   expr MINUS expr
                { $$ = make_binary_expr($1, $3, op_sub_t); }
    |   expr TIMES expr
                { $$ = make_binary_expr($1, $3, op_mul_t); }
    |   expr DIVIDE expr
                { $$ = make_binary_expr($1, $3, op_div_t); }
    |   expr POW expr
                { $$ = make_binary_expr($1, $3, op_pow_t); }
    |   expr MOD expr
                { $$ = make_binary_expr($1, $3, op_mod_t); }
    |   expr LTHAN expr
                { $$ = make_binary_expr($1, $3, op_lt_t); }
    |   expr GTHAN expr
                { $$ = make_binary_expr($1, $3, op_gt_t); }
    |   expr NOTEQ expr
                { $$ = make_binary_expr($1, $3, op_neq_t); }
    |   expr EQUAL expr
                { $$ = make_binary_expr($1, $3, op_eq_t); }
    |   expr LTHEQ expr
                { $$ = make_binary_expr($1, $3, op_lte_t); }
    |   expr GTHEQ expr
                { $$ = make_binary_expr($1, $3, op_gte_t); }
    |   expr LGOR expr
                { $$ = make_binary_expr($1, $3, op_lor_t); }
    |   expr LGAND expr
                { $$ = make_binary_expr($1, $3, op_land_t); }
    |   expr BWXOR expr
                { $$ = make_binary_expr($1, $3, op_bxor_t); }
    |   expr BWOR expr
                { $$ = make_binary_expr($1, $3, op_bor_t); }
    |   expr BWAND expr
                { $$ = make_binary_expr($1, $3, op_band_t); }
    |   BWNOT expr %prec UBWNOT
                { $$ = make_binary_expr(
                        make_int_constant(0), $2, op_bnot_t);
                }
    |   LGNOT expr %prec ULGNOT
                { $$ = make_binary_expr(
                        make_int_constant(0), $2, op_lnot_t);
                }
    |   MINUS expr %prec UMINUS
                { $$ = make_binary_expr(
                        make_int_constant(0), $2, op_sub_t);
                }
    |   LPAREN expr RPAREN      { $$ = $2; }
    |   list_access             { $$ = $1; }
    |   list                    { $$ = $1; }
    |   call                    { $$ = $1; }
    ;

%%

void yyerror(const char *msg)
{
    die("(Syntax) %s @ line #%d, col #%d\n",
            msg, get_line_num(), get_last_col_num());
}

