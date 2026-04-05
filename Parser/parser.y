%{
/* Prologo C: includes e declaracoes auxiliares. */

#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);

%}


%union {
    int ival;
    char *sval;
}

/*
 * Declaracoes do Bison (ordem recomendada no operadores.md):
 * 1) %union
 * 2) %token
 * 3) %type
 * 4) precedencia
 */

%token <ival> NUMBER
%token <sval> IDENT
%token OP_Igualdade
%token '+'
%token '*'

%type <ival> expressao

/* Menor -> maior precedencia. */

%left OP_Igualdade
%left '+'
%left '*'

%%
/* Regras gramaticais + acoes semanticas. */
programa:
    | programa Linha
    ;

Linha:
    expressao ';' { printf("Resultado: %d\n", $1); }
    ;

/* Nesta etapa, a semantica calcula expressoes numericas. */
expressao:
    NUMBER { $$ = $1; }
    | expressao OP_Igualdade expressao { $$ = ($1 == $3); }
    | expressao '+' expressao { $$ = $1 + $3; }
    | expressao '*' expressao { $$ = $1 * $3; }
    ;

%%


void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    printf("Interpretador JS v0.1 iniciado.\nDigite suas expressões (ex: x + 10 == 20;)\n\n");
    return yyparse();
}