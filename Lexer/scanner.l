%{
#include <stdio.h>
#include "../Parser/parser.tab.h"
%}

/* Definições de padrões de token e ações */
%%

"=="        {return OP_Igualdade; } /* adicionando operador de igualdade */
"+"         {return '+'; }
";"         {return ';'; }

[0-9]+      { return NUMBER; } /* adicionando numeros*/
[a-zA-Z_]+  { return IDENT; }  /* adicionando indentificadores*/

[ \t\n]+    { /* ignora espaços e quebras de linha */ }

.           { printf("Caractere desconhecido: %s\n", yytext); }

%%

/* Função auxiliar do Flex (quando termina o arquivo de entrada) */
int yywrap(void) {
    return 1;
}
