%{
/* Prologo C: includes e declaracoes auxiliares. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 256

typedef struct {
    char nome[64];
    int valor;
} Variavel;

static Variavel tabela[MAX_VARS];
static int qtd_vars = 0;

static int find_var(const char *nome) {
    int i;
    for (i = 0; i < qtd_vars; i++) {
        if (strcmp(tabela[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;
}

static int get_var(const char *nome) {
    int idx = find_var(nome);
    if (idx >= 0) {
        return tabela[idx].valor;
    }
    return 0;
}

static void set_var(const char *nome, int valor) {
    int idx = find_var(nome);
    if (idx >= 0) {
        tabela[idx].valor = valor;
        return;
    }

    if (qtd_vars < MAX_VARS) {
        strncpy(tabela[qtd_vars].nome, nome, sizeof(tabela[qtd_vars].nome) - 1);
        tabela[qtd_vars].nome[sizeof(tabela[qtd_vars].nome) - 1] = '\0';
        tabela[qtd_vars].valor = valor;
        qtd_vars++;
    }
}

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
%token OP_AND
%token '+'
%token '*'
%token OP_atribuicao_soma
%token '-'
%token '/'
%token '>'
%token '<'
%token OP_Diferente

%type <ival> expressao

/* Menor -> maior precedencia. */

%right OP_atribuicao_soma
%left OP_AND
%left OP_Igualdade OP_Diferente
%left '<' '>'
%left '+' '-'
%left '*' '/'


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
    | IDENT  { $$ = get_var($1); free($1); }
    | IDENT OP_atribuicao_soma expressao {
        int novo = get_var($1) + $3;
        set_var($1, novo);
        $$ = novo;
        free($1);
    }
    | expressao OP_AND expressao { $$ = ($1 && $3); }
    | expressao OP_Igualdade expressao { $$ = ($1 == $3); }
    | expressao OP_Diferente expressao { $$ = ($1 != $3); }
    | expressao '+' expressao { $$ = $1 + $3; }
    | expressao '*' expressao { $$ = $1 * $3; }
    | expressao '-' expressao { $$ = $1 - $3; }
    | expressao '>' expressao { $$ = ($1 > $3); }
    | expressao '<' expressao { $$ = ($1 < $3); }
    | expressao '/' expressao { 
        if ($3 == 0) {
            printf("Erro: Divisao por zero!\n");
            $$ = 0; 
        } else {
            $$ = $1 / $3; 
        }
    }
    ;
    

%%


void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    printf("Interpretador JS v0.1 iniciado.\nDigite suas expressões (ex: x + 10 == 20;)\n\n");
    return yyparse();
}