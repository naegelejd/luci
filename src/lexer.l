/*
 * See Copyright Notice in luci.h
 */

%option noyywrap
/* %option debug */

%{

#include <string.h>
#include "parser.tab.h"


static int line = 1;
static int col = 1;
static int interactive = 0;

void luci_start_interactive(void);
void newline(void);
void comment(void);
int get_line_num(void);
int get_last_col_num(void);

%}

INTEGER     [0-9]+
FLOAT       [0-9]+\.[0-9]*|\.[0-9]+
WORD        [a-zA-Z_][a-zA-Z_0-9]*
STRING      \"[^\"]+\"|\"\"
WS          [ \t\f\r]+

%%

"/*"            { comment(); }

"while"         { col += yyleng; return WHILE; }
"for"           { col += yyleng; return FOR; }
"in"            { col += yyleng; return IN; }
"if"            { col += yyleng; return IF; }
"else"          { col += yyleng; return ELSE; }
"def"           { col += yyleng; return DEF; }
"break"         { col += yyleng; return BREAK; }
"continue"      { col += yyleng; return CONTINUE; }
"return"        { col += yyleng; return RETURN; }
"pass"          { col += yyleng; return PASS; }
"nil"           { col += yyleng; return NIL; }

"true"          {
                    yylval.int_v = 1;
                    col += yyleng;
                    return INT;
                }
"false"         {
                    yylval.int_v = 0;
                    col += yyleng;
                    return INT;
                }

{STRING}        {
                    yylval.string_v = strndup(yytext + 1, strlen(yytext) - 2);
                    col += yyleng;
                    return STRING;
                }
{FLOAT}         {
                    yylval.float_v = atof(yytext);
                    col += yyleng;
                    return FLOAT;
                }
{INTEGER}       {
                    yylval.int_v = atoi(yytext);
                    col += yyleng;
                    return INT;
                }
{WORD}          { yylval.string_v = strdup(yytext);
                    col += yyleng;
                    return ID;
                }

^\n|{WS}+\n     { newline(); /* ignore empty lines */ }
\n              { newline(); /* ignore empty lines */ }
#.*\n           { newline(); /* ignore one-line comments */ }
{WS}+           { col += yyleng; } /* ignore whitespace/semicolon */

"{"     { col++; return LBRACK; }
"}"     { col++; return RBRACK; }
";"     { col++; return SEMICOLON; }
":"     { col++; return COLON; }
","     { col++; return COMMA; }
"["     { col++; return LSQUARE; }
"]"     { col++; return RSQUARE; }

"-"     { col++; return MINUS; }
"+"     { col++; return PLUS; }
"**"    { col += 2; return POW; }
"*"     { col++; return TIMES; }
"/"     { col++; return DIVIDE; }
"%"     { col++; return MOD; }

"<="    { col += 2; return LTHEQ; }
">="    { col += 2; return GTHEQ; }
"=="    { col += 2; return EQUAL; }
"!="    { col += 2; return NOTEQ; }
"<"     { col++; return LTHAN; }
">"     { col++; return GTHAN; }

"!"     { col++; return LGNOT; }
"||"    { col += 2; return LGOR; }
"&&"    { col += 2; return LGAND; }

"("     { col++; return LPAREN; }
")"     { col++; return RPAREN; }
"="     { col++; return ASSIGN; }

"^"     { col++; return BWXOR; }
"|"     { col++; return BWOR; }
"&"     { col++; return BWAND; }
"~"     { col++; return BWNOT; }

"$ "    { ECHO; }

.       { col += yyleng; /* ignore invalid chars */ }

<<EOF>> {
            yyterminate();
        }


%%


void luci_start_interactive(void)
{
    interactive = 1;
    yy_set_interactive(1);
    putc('$', stdout);
    putc(' ', stdout);
    line = 1;
    col = 1;
}

void newline(void)
{
    line++;
    col = 1;
    if (interactive) {
        unput(' ');
        unput('$');
    }
}

int get_line_num(void)
{
    return line;
}

int get_last_col_num(void)
{
    return col - yyleng;
}

void comment(void)
{
    char c, c1;

loop:
    while ((c = input()) != '*' && c != 0) {
        ;
    }

    if ((c1 = input()) != '/' && c != 0) {
        unput(c1);
        goto loop;
    }

}

