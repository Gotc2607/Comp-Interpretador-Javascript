%{
/* Prologo C: includes e declaracoes auxiliares. */

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

int yylex(void);
void yyerror(const char *s);

static ASTNode *raiz = NULL;

%}

%code requires {
#include "ast.h"
}


%union {
    int ival;
    char *sval;
    ASTNode *node;
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
%token OP_OR
%token OP_AND
%token '+'
%token '*'
%token '{' '}'
%token OP_atribuicao_soma
%token OP_atribuicao_subtracao
%token OP_atribuicao_potencia
%token OP_atribuicao_multiplicacao
%token OP_atribuicao_divisao
%token OP_atribuicao_resto
%token '-'
%token '/'
%token '>'
%token '<'
%token OP_Diferente
%token '!'
%token '(' ')'
%token ';'

%type <node> programa elementos elemento Linha Bloco lista_linhas expressao

/* Menor -> maior precedencia. */

%right OP_atribuicao_soma
%right OP_atribuicao_subtracao
%right OP_atribuicao_potencia
%right OP_atribuicao_multiplicacao
%right OP_atribuicao_divisao
%right OP_atribuicao_resto
%left OP_OR
%left OP_AND
%left OP_Igualdade OP_Diferente
%left '<' '>'
%left '+' '-'
%left '*' '/'
%right '!'
%left '(' ')'



%%
/* Regras gramaticais + acoes semanticas. */
programa:
    elementos { raiz = $1; $$ = $1; }
;

elementos:
    /* vazio */ { $$ = NULL; }
    | elementos elemento { $$ = ast_sequence($1, $2); }
;

elemento:
    Linha
    | Bloco
;

Linha:
    expressao ';' { $$ = ast_print_stmt($1); }
;

Bloco:
    '{' lista_linhas '}' { $$ = ast_block($2); }
;

lista_linhas:
    /* vazio */ { $$ = NULL; }
    | lista_linhas elemento { $$ = ast_sequence($1, $2); }
;

/* A AST representa expressoes, atribuicoes e operacoes logicas. */
expressao:
    NUMBER { $$ = ast_number($1); }
    | IDENT  { $$ = ast_identifier($1); }
    | IDENT OP_atribuicao_soma expressao { $$ = ast_assign(OP_atribuicao_soma, $1, $3); }
    | IDENT OP_atribuicao_subtracao expressao { $$ = ast_assign(OP_atribuicao_subtracao, $1, $3); }
    | IDENT OP_atribuicao_potencia expressao { $$ = ast_assign(OP_atribuicao_potencia, $1, $3); }
    | IDENT OP_atribuicao_multiplicacao expressao { $$ = ast_assign(OP_atribuicao_multiplicacao, $1, $3); }
    | IDENT OP_atribuicao_divisao expressao { $$ = ast_assign(OP_atribuicao_divisao, $1, $3); }
    | IDENT OP_atribuicao_resto expressao { $$ = ast_assign(OP_atribuicao_resto, $1, $3); }

    | expressao OP_AND expressao { $$ = ast_binary(OP_AND, $1, $3); }
    | expressao OP_OR expressao { $$ = ast_binary(OP_OR, $1, $3); }
    | expressao OP_Igualdade expressao { $$ = ast_binary(OP_Igualdade, $1, $3); }
    | expressao OP_Diferente expressao { $$ = ast_binary(OP_Diferente, $1, $3); }
    | expressao '+' expressao { $$ = ast_binary('+', $1, $3); }
    | expressao '*' expressao { $$ = ast_binary('*', $1, $3); }
    | expressao '-' expressao { $$ = ast_binary('-', $1, $3); }
    | expressao '>' expressao { $$ = ast_binary('>', $1, $3); }
    | expressao '<' expressao { $$ = ast_binary('<', $1, $3); }
    | expressao '/' expressao { $$ = ast_binary('/', $1, $3); }
    | '!' expressao { $$ = ast_unary('!', $2); }
    | '(' expressao ')' { $$ = $2; }
    ;
    

%%


void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    printf("Interpretador JS v0.1 iniciado.\nDigite suas expressões (ex: x + 10 == 20;)\n\n");
    if (yyparse() == 0 && raiz != NULL) {
        ast_eval(raiz);
        ast_free(raiz);
        raiz = NULL;
    }

    return 0;
}