/*
 * See Copyright Notice in luci.h
 */

%option noyywrap
/* %option debug */

%x str

%{

#include "luci.h"
#include "parser.tab.h"

extern void yyerror(const char *msg);

static int line = 1;
static int col = 1;
static int interactive = 0;

void luci_start_interactive(void);
void newline(void);
void comment(void);
int get_line_num(void);
int get_last_col_num(void);

char string_buffer[MAX_STR_CONST];
char *str_buf_ptr;

%}

D           [0-9]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+

INTEGER     {D}+|0[xX]{H}+
FLOAT       {D}+{E}|{D}*\.{D}+({E})?|{D}+\.{D}*({E})?
WORD        [a-zA-Z_][a-zA-Z_0-9]*
WS          [ \t\f\r]+

%%

\"        {
                col++;
                str_buf_ptr = string_buffer;
                BEGIN(str);
            }
<str>{
\"        {
                col++;
                BEGIN(INITIAL);
                *str_buf_ptr = '\0';
                yylval.string_v = strndup(string_buffer,
                        strlen(string_buffer));
                return STRING;
            }
\n        { yyerror("Unterminated string literal"); }
\\n       { col++; *str_buf_ptr++ = '\n'; }
\\t       { col++; *str_buf_ptr++ = '\t'; }
\\r       { col++; *str_buf_ptr++ = '\r'; }
\\b       { col++; *str_buf_ptr++ = '\b'; }
\\f       { col++; *str_buf_ptr++ = '\f'; }
\\(.|\n)      { col++; *str_buf_ptr++ = yytext[1]; }
[^\\\n\"]+    {
                    char *sptr = yytext;
                    while (*sptr) {
                        *str_buf_ptr++ = *sptr++;
                        col++;
                    }
                }
} /* <str> */

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
{INTEGER}       {
                    yylval.int_v = strtol(yytext, NULL, 0);
                    col += yyleng;
                    return INT;
                }
{FLOAT}         {
                    yylval.float_v = strtod(yytext, NULL);
                    col += yyleng;
                    return FLOAT;
                }
{WORD}          {
                    yylval.string_v = strdup(yytext);
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

"$ "    { ECHO; /* interactive mode */ }

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

