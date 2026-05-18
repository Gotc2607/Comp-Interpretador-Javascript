%option nounput
%option noinput
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Parser/parser.tab.h"
%}

%%

"==="       { return OP_IgualdadeEstrita; }
"=="        { return OP_Igualdade; }
"!="        { return OP_Diferente; }
"+="        { return OP_atribuicao_soma; }
"-="        { return OP_atribuicao_subtracao; }
"**="       { return OP_atribuicao_potencia; }
"*="        { return OP_atribuicao_multiplicacao; }
"/="        { return OP_atribuicao_divisao; }
"%="        { return OP_atribuicao_resto; }
"&&"        { return OP_AND; }
"//"[^
]*  { /* ignora comentario de linha */ }
"+"         { return '+'; }
"*"         { return '*'; }
"/"         { return '/'; }  
";"         { return ';'; }
"{"         { return '{'; }
"}"         { return '}'; }
"-"	        { return '-'; }

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
"do"        { return DO; }
"??="       { return OP_atribuicao_nullish; }

[0-9]+      {
    yylval.ival = atoi(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.sval = strdup(yytext);
    return IDENT;
}

[ \t\n]+    {  }

.           { printf("Caractere desconhecido: %s\n", yytext); }

%%

int yywrap(void) {
    return 1;
}