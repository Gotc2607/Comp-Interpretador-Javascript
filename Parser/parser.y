%{
/* 1. PROLÓGO (Código C) */
// Aqui você coloca os #include e declarações de funções que o C precisa.

#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* 2. DECLARAÇÕES DO BISON */
// Aqui você define os "Tokens" (as palavras que o Flex envia)
// e a precedência (quem manda mais na matemática).


%token OP_Igualdade NUMBER IDENT
%token '+'

/* Colocar a Procedencia: sentido no qual o comando é lido*/
%left OP_Igualdade
%left '+'

%%
/* 3. REGRAS GRAMATICAIS */
// Aqui você define a estrutura da linguagem (o "cérebro").
programa:
    | programa Linha
    ;

Linha:
    expressao ';' { printf("Comando aceito!\n"); }
    ;

expressao:
    NUMBER
    | IDENT
    | expressao OP_Igualdade expressao
    | expressao '+' expressao
    ;

%%


void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    printf("Interpretador JS v0.1 iniciado.\nDigite suas expressões (ex: x + 10 == 20;)\n\n");
    return yyparse();
}