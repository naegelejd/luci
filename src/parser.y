/*
 * See Copyright Notice in luci.h
 */

%{

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "luci.h"
#include "ast.h"
#include "binop.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

void yyerror(const char *msg);

/* defined in lexer.l */
extern int get_line_num(void);
extern int get_last_col_num(void);

/* defined in generated lexer */
extern int yylex();

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
%type <node> map_items map_keyval map
%type <node> list_items list
%type <node> container_index container_access container_assign
%type <node> program

%token <int_v> INT
%token <float_v> FLOAT
%token <string_v> STRING ID

%token NEWLINE COLON SEMICOLON COMMA
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
    |   container_assign SEMICOLON  { $$ = $1; }
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

container_assign:
        id container_index ASSIGN expr
                { $$ = make_container_assignment($1, $2, $4); }
    ;

func_def:
        DEF ID LPAREN params RPAREN LBRACK statements RBRACK
            { $$ = make_func_def($2, $4, $7); }
    ;

return:     RETURN expr     { $$ = make_return($2); }
    |       RETURN          { $$ = make_return(NULL); }
    ;

params:
        /* nothing */   { $$ = make_list_def(NULL, NULL); }
    |   ID      { $$ = make_list_def(NULL, make_string_constant($1)); }
    |   params COMMA ID
                { $$ = make_list_def($1, make_string_constant($3)); }
    ;

call:
        id LPAREN list_items RPAREN
                { $$ = make_func_call($1, $3); }
    |   container_access LPAREN list_items RPAREN
                { $$ = make_func_call($1, $3); }
    ;

map:
        LBRACK map_items RBRACK     { $$ = $2; }
    ;

map_items:
        /* nothing */               { $$ = make_map_def(NULL, NULL); }
    |   map_keyval                     { $$ = make_map_def(NULL, $1); }
    |   map_items COMMA map_keyval     { $$ = make_map_def($1, $3); }
    ;

map_keyval:
        expr COLON expr             { $$ = make_map_keyval($1, $3); }
    ;

list:
        LSQUARE list_items RSQUARE      { $$ = $2; }
    ;

list_items:
        /* nothing */       { $$ = make_list_def(NULL, NULL); }
    |   expr                    { $$ = make_list_def(NULL, $1); }
    |   list_items COMMA expr   { $$ = make_list_def($1, $3); }
    ;

container_access:
        expr container_index { $$ = make_container_access($1, $2); };

container_index:
        LSQUARE expr RSQUARE
                { $$ = $2; };

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
    |   container_access        { $$ = $1; }
    |   map                     { $$ = $1; }
    |   list                    { $$ = $1; }
    |   call                    { $$ = $1; }
    ;

%%

void yyerror(const char *msg)
{
    DIE("(Syntax) %s @ line #%d, col #%d\n",
            msg, get_line_num(), get_last_col_num());
}

