%option noyywrap

%{

#include <string.h>
#include "parser.tab.h"

extern int LINE_NUM;

%}

%x ERROR

INTEGER	    [0-9]+
FLOAT	    [0-9]+\.[0-9]*|\.[0-9]+
WORD	    [a-zA-Z_][a-zA-Z_0-9]*
STRING	    \"[^\"]+\"
WS	    [ \t\f\r]+

%%

[wW][hH][iI][lL][eE]	{ return WHILE; }
[fF][oO][rR]		{ return FOR; }
[iI][nN]		{ return IN; }
[dD][oO]		{ return DO; }
[dD][oO][nN][eE]	{ return DONE; }
[iI][fF]		{ return IF; }
[tT][hH][eE][nN]	{ return THEN; }
[eE][lL][sS][eE]	{ return ELSE; }
[eE][nN][dD]		{ return END; }
[dD][eE][fF]            { return DEF; }
[rR][eE][tT][uU][rR][nN]    { return RETURN; }

^\n|{WS}\n	{ LINE_NUM++; /* ignore empty lines */ }
\n	{ LINE_NUM++; return NEWLINE; }
#.*\n	{ LINE_NUM++; } /* ignore one-line comments */
{WS}	; /* ignore */

{STRING}	{
                    yylval.s_val = strndup(yytext + 1, strlen(yytext) - 2);
                    return STRING;
                }
{FLOAT}		{ yylval.f_val = atof(yytext); return FLOAT; }
{INTEGER}	{ yylval.i_val = atoi(yytext); return INT; }
{WORD}		{ yylval.id = strdup(yytext); return ID; }

","	{ return COMMA; }
"["	{ return LSQUARE; }
"]"	{ return RSQUARE; }

"-"	{ return MINUS; }
"+"	{ return PLUS; }
"**"	{ return POW; }
"*"	{ return TIMES; }
"/"	{ return DIVIDE; }
"%"	{ return MOD; }

"<="	{ return LTHEQ; }
">="	{ return GTHEQ; }
"=="	{ return EQUAL; }
"!="	{ return NOTEQ; }
"<"	{ return LTHAN; }
">"	{ return GTHAN; }

"!"	{ return LGNOT; }
"||"	{ return LGOR; }
"&&"	{ return LGAND; }

"("	{ return LPAREN; }
")"	{ return RPAREN; }
"="	{ return ASSIGN; }

"^"	{ return BWXOR; }
"|"	{ return BWOR; }
"&"	{ return BWAND; }
"~"	{ return BWNOT; }

<<EOF>>     {
                yy_delete_buffer(YY_CURRENT_BUFFER);
                yyterminate();
            }

%%
