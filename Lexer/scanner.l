%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Parser/parser.tab.h"
%}

/*
 * Etapa semantica (conforme operadores.md):
 * - Reconhece operadores e simbolos.
 * - Preenche yylval para NUMBER/IDENT.
 */
%%

"=="        { return OP_Igualdade; }
"!="        { return OP_Diferente; }
"+="        { return OP_atribuicao_soma; }
"&&"        { return OP_AND; }
"+"         { return '+'; }
"*"         { return '*'; }
"/"         { return '/'; }  
";"         { return ';'; }
"-"	        { return '-'; }
">"         { return '>'; }
"<"         { return '<'; }
"||"        { return OP_OR; }
"**"        { return OP_EXP; }
"++"        { return OP_INCREMENT; }
"--"        { return OP_DECREMENT; }


[0-9]+      {
    yylval.ival = atoi(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.sval = strdup(yytext);
    return IDENT;
}

[ \t\n]+    { /* ignora espacos */ }

.           { printf("Caractere desconhecido: %s\n", yytext); }

%%

/* Fim do arquivo de entrada. */
int yywrap(void) {
    return 1;
}
