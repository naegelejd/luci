%option noyywrap

%{
#include <string.h>
#include "parser.h"

extern int LINE_NUM;

%}

%x ERROR

INTEGER	    [0-9]+
DOUBLE	    [0-9]+\.[0-9]*|\.[0-9]+
WORD	    [a-zA-Z_][a-zA-Z_0-9]*
STRING	    \"[^\"]+\"
WS	    [ \t\f\r]+

%%

[wW][hH][iI][lL][eE]	{ return WHILE; }
[dD][oO]		{ return DO; }
[dD][oO][nN][eE]	{ return DONE; }

[iI][fF]		{ return IF; }
[tT][hH][eE][nN]	{ return THEN; }
[eE][lL][sS][eE]	{ return ELSE; }
[eE][nN][dD]		{ return END; }

{STRING}	{ yylval.s_val = strndup(yytext + 1, strlen(yytext) - 2); return STRING; }
{DOUBLE}	{ yylval.d_val = atof(yytext); return DOUBLE; }
{INTEGER}	{ yylval.i_val = atoi(yytext); return INT; }
{WORD}		{ yylval.id = strdup(yytext); return ID; }
{WS}	; /* ignore */

,	{ return COMMA; }
#.*\n	{ LINE_NUM++; } /* ignore one-line comments */

^\n	{ LINE_NUM++; /* ignore empty lines */ }
\n	{ LINE_NUM++; return NEWLINE; }


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

%%
