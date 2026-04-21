%{
/* Prologo C: includes e declaracoes auxiliares. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_VARS 256

typedef struct {
    char nome[64];
    int valor;
} Variavel;

typedef enum { TIPO_NUMBER, TIPO_STRING, TIPO_BOOLEAN, TIPO_NULL } Type;

typedef struct {
    Type tipo;
    union {
        double valor_numerico;
        char* valor_string;
        int valor_bool;
    } dado;
} JSObject;

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

int avaliar_igualdade_estrita(JSObject a, JSObject b) {
    // 1. Verificação de Tipo: Se tipos diferentes, retorna Falso (0)
    if (a.tipo != b.tipo) {
        return 0; 
    }

    // 2. Verificação de Valor: Tipos são iguais, agora compara o conteúdo
    switch (a.tipo) {
        case TIPO_NUMBER:
            return a.dado.valor_numerico == b.dado.valor_numerico;
        
        case TIPO_STRING:
            return strcmp(a.dado.valor_string, b.dado.valor_string) == 0;
        
        case TIPO_BOOLEAN:
            return a.dado.valor_bool == b.dado.valor_bool;
            
        case TIPO_NULL:
            return 1; // null === null é sempre verdade

        default:
            return 0;
    }
}

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
%token OP_Igualdade_estrito
%token OP_Diferente_estrito

%type <ival> expressao

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
%left OP_Igualdade_estrito OP_Diferente_estrito


%%
/* Regras gramaticais + acoes semanticas. */
programa:
    | programa elemento
;

elemento:
    Linha
    | Bloco
;

Linha:
    expressao ';' { printf("Resultado: %d\n", $1); }
;

Bloco:
    '{' lista_linhas '}'
;

lista_linhas:
    | lista_linhas elemento
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
    | IDENT OP_atribuicao_subtracao expressao {
        int novo = get_var($1) - $3;
        set_var($1, novo);
        $$ = novo;
        free($1);
    }
    | IDENT OP_atribuicao_potencia expressao {
        int novo = (int) pow(get_var($1), $3);
        set_var($1, novo);
        $$ = novo;
        free($1);
    }
    | IDENT OP_atribuicao_multiplicacao expressao {
        int novo = get_var($1) * $3;
        set_var($1, novo);
        $$ = novo;
        free($1);
    }
    | IDENT OP_atribuicao_divisao expressao {
        int novo = get_var($1) / $3;
        set_var($1, novo);
        $$ = novo;
        free($1);
    }
    | IDENT OP_atribuicao_resto expressao {
        int novo = get_var($1) % $3;
        set_var($1, novo);
        $$ = novo;
        free($1);
    }

    | expressao OP_AND expressao { $$ = ($1 && $3); }
    | expressao OP_OR expressao { $$ = ($1 || $3); }
    | expressao OP_Igualdade expressao { $$ = ($1 == $3); }
    | expressao OP_Diferente expressao { $$ = ($1 != $3); }
    | expressao OP_Igualdade_estrito expressao { $$ = avaliar_igualdade_estrita((JSObject){.tipo=TIPO_NUMBER, .dado.valor_numerico=$1}, (JSObject){.tipo=TIPO_NUMBER, .dado.valor_numerico=$3}); }
    | expressao OP_Diferente_estrito expressao { $$ = !avaliar_igualdade_estrita((JSObject){.tipo=TIPO_NUMBER, .dado.valor_numerico=$1}, (JSObject){.tipo=TIPO_NUMBER, .dado.valor_numerico=$3}); }
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
    | '!' expressao { $$ = !$2; }
    ;
    

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    return yyparse();
}