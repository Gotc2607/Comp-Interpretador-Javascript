%option nounput
%option noinput
%option yylineno
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../Parser/parser.tab.h"
#include "../Parser/ast.h"

/* Rastreamento de coluna */
int yycolumn = 1;
int yycolumn_last_token = 1;

/* Buffer com o texto da linha atual (para mensagens de erro mais ricas,
 * ex: "esperado ';' depois de 'let x = 20'"). É reiniciado a cada '\n'
 * e recebe o texto de cada token reconhecido. LINE_BUFFER_SIZE é
 * definido em ast.h para ser compartilhado com o parser. */
char current_line_buffer[LINE_BUFFER_SIZE] = "";
static size_t line_buffer_len = 0;

static void line_buffer_append(const char *text, int len) {
    if (len < 0) return;
    if (line_buffer_len + (size_t)len >= LINE_BUFFER_SIZE - 1)
        len = (int)(LINE_BUFFER_SIZE - 1 - line_buffer_len);
    if (len <= 0) return;
    memcpy(current_line_buffer + line_buffer_len, text, (size_t)len);
    line_buffer_len += (size_t)len;
    current_line_buffer[line_buffer_len] = '\0';
}

static void line_buffer_reset(void) {
    current_line_buffer[0] = '\0';
    line_buffer_len = 0;
}

#define YY_USER_ACTION \
    yycolumn_last_token = yycolumn; \
    ast_current_line = yylineno; \
    ast_current_col  = yycolumn; \
    yycolumn += (int)yyleng; \
    line_buffer_append(yytext, (int)yyleng);
%}

%%

"==="       { return OP_IgualdadeEstrita; }
"=="        { return OP_Igualdade; }
"!=="       { return OP_DiferenteEstrita; }
"!="        { return OP_Diferente; }
"+="        { return OP_atribuicao_soma; }
"-="        { return OP_atribuicao_subtracao; }
"**="       { return OP_atribuicao_potencia; }
"*="        { return OP_atribuicao_multiplicacao; }
"/="        { return OP_atribuicao_divisao; }
"%="        { return OP_atribuicao_resto; }
"&&"        { return OP_AND; }
"//"[^\n]* { /* ignora comentario de linha */ }
"+"         { return '+'; }
"*"         { return '*'; }
"/"         { return '/'; }
";"         { return ';'; }
"{"         { return '{'; }
"}"         { return '}'; }
"-"         { return '-'; }

"\"use strict\"" { return KW_USE_STRICT; }

\"[^\"\n]*\" {
    yylval.sval = (char *)malloc((size_t)yyleng - 1);
    if (!yylval.sval) {
        fprintf(stderr, "Erro: memoria insuficiente ao ler string.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(yylval.sval, yytext + 1, (size_t)yyleng - 2);
    yylval.sval[yyleng - 2] = '\0';
    return STRING;
}

">"         { return '>'; }
"<"         { return '<'; }
"||"        { return OP_OR; }
"!"         { return '!'; }
"("         { return '('; }
")"         { return ')'; }
"%"         { return '%'; }
"++"        { return OP_Incremento; }
"--"        { return OP_Decremento; }
"**"        { return OP_Potencia; }
"<="        { return OP_MenorIgual; }
">="        { return OP_MaiorIgual; }
"="         { return '='; }
"while"     { return WHILE; }
"for"       { return FOR; }
"do"        { return DO; }
"if"        { return IF; }
"else"      { return ELSE; }
"let"       { return LET; }
"const"     { return CONST; }
"var"       { return VAR; }
"??="       { return OP_atribuicao_nullish; }
"["         { return '['; }
"]"         { return ']'; }
"switch"    { return SWITCH; }
"break"     { return BREAK; }
"continue"  { return CONTINUE; }
"case"      { return CASE; }
"default"   { return DEFAULT; }
":"         { return ':'; }
"console.log" { return CONSOLE_LOG; }
"function"  { return FUNCTION; }
"return"    { return RETURN; }
","         { return ','; }

[0-9]+      {
    yylval.ival = atoi(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.ident.name = strdup(yytext);
    yylval.ident.line = yylineno;
    yylval.ident.col  = yycolumn_last_token;
    return IDENT;
}

\n          { yycolumn = 1; yycolumn_last_token = 1; line_buffer_reset(); }
[ \t\r]+    { /* ignora espacos */ }

.           {
    fprintf(stderr,
        "Erro lexico [linha %d, col %d]: caractere desconhecido '%s'.\n",
        yylineno, yycolumn_last_token, yytext);
}

%%

int yywrap(void) {
    return 1;
}
