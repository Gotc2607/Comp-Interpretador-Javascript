%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

int yylex(void);
void yyerror(const char *s);

extern int yylineno;
extern int yycolumn;
extern int yycolumn_last_token;
extern char *yytext;
/* current_line_buffer é declarado (com tamanho completo) em ast.h */

static ASTNode *raiz = NULL;
static int parse_error_count = 0;

static const char *token_descr(void) {
    if (!yytext || yytext[0] == '\0') return "<fim de entrada>";
    return yytext;
}

/*
 * Remove espacos em branco do final de 'str' (em-place) e retorna o
 * novo comprimento. Usado para extrair o trecho relevante da linha
 * atual (ex.: "let x = 20") ao montar mensagens de erro mais ricas.
 */
static size_t rtrim(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' ||
                        str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    return len;
}

/*
 * Verifica se a mensagem verbosa do Bison (parse.error verbose) indica
 * que ';' era um dos tokens esperados nesse ponto. Mensagens tem o
 * formato: "syntax error, unexpected X, expecting Y or Z or ...".
 */
static int esperava_ponto_e_virgula(const char *bison_msg) {
    if (!bison_msg) return 0;
    const char *p = strstr(bison_msg, "expecting");
    if (!p) return 0;
    return strstr(p, "';'") != NULL;
}

%}

%code requires {
#include "ast.h"
typedef struct { char *name; int line; int col; } IdentTok;
}

%define parse.error verbose

%union {
    int ival;
    char *sval;
    ASTNode *node;
    IdentTok ident;
}

%token <ival> NUMBER
%token <sval> STRING
%token <ident> IDENT
%token OP_Igualdade
%token OP_OR
%token OP_AND
%token FOR
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
%token '='
%token OP_Potencia
%token OP_Incremento
%token OP_Decremento
%token OP_MaiorIgual
%token OP_MenorIgual
%token '%'
%token OP_IgualdadeEstrita
%token OP_DiferenteEstrita
%token WHILE DO IF ELSE
%token LET CONST VAR
%token OP_atribuicao_nullish
%token CONSOLE_LOG
%token '[' ']'
%token SWITCH CASE DEFAULT ':'
%token BREAK CONTINUE
%token FUNCTION RETURN ','
%token KW_USE_STRICT

%type <node> programa elementos elemento Linha Bloco lista_linhas expressao lista_cases bloco_case
%type <node> expressao_opt params_opt param_list args_opt arg_list

%right OP_atribuicao_nullish
%right OP_atribuicao_soma
%right OP_atribuicao_subtracao
%right OP_atribuicao_potencia
%right OP_atribuicao_multiplicacao
%right OP_atribuicao_divisao
%right OP_atribuicao_resto
%left OP_OR
%left OP_AND
%left OP_Igualdade OP_Diferente OP_IgualdadeEstrita OP_DiferenteEstrita
%left '<' '>' OP_MaiorIgual OP_MenorIgual
%left '+' '-'
%left '*' '/'
%right '!'
%left '(' ')'
%left '[' ']'

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

programa:
      KW_USE_STRICT ';' elementos { ativar_strict_mode(); raiz = $3; $$ = $3; $$ = NULL;}
    | elementos { raiz = $1; $$ = $1; }
;

elementos:
    /* vazio */ { $$ = NULL; }
    | elementos elemento { $$ = ast_sequence($1, $2); }
;

elemento:
    Linha
    | Bloco
    | WHILE '(' expressao ')' elemento { $$ = ast_while($3, $5); }
    | FOR '(' expressao_opt ';' expressao_opt ';' expressao_opt ')' elemento { $$ = ast_for($3, $5, $7, $9); }
    | DO Bloco WHILE '(' expressao ')' ';' { $$ = ast_do_while($5, $2); }
    | SWITCH '(' expressao ')' '{' lista_cases '}' { $$ = ast_switch($3, $6); }
    | BREAK ';' { $$ = ast_break_stmt(); }
    | CONTINUE ';' { $$ = ast_continue_stmt(); }
    | IF '(' expressao ')' elemento %prec LOWER_THAN_ELSE { $$ = ast_if($3, $5, NULL); }
    | IF '(' expressao ')' elemento ELSE elemento { $$ = ast_if($3, $5, $7); }
    | CONSOLE_LOG '(' expressao ')' ';' { $$ = ast_console_log($3); }
    | FUNCTION IDENT '(' params_opt ')' Bloco { $$ = ast_func_decl($2.name, $4, $6); }
    | RETURN expressao_opt ';' { $$ = ast_return($2); }
    | error ';' { $$ = NULL; yyerrok; }
    | error '}' { $$ = NULL; yyerrok; }
;

Linha:
    expressao ';' { $$ = $1; }
    | LET IDENT '=' expressao ';'   { ast_current_line = $2.line; ast_current_col = $2.col; $$ = ast_declare(0, $2.name, $4); }
    | LET IDENT ';'                 { ast_current_line = $2.line; ast_current_col = $2.col; $$ = ast_declare(0, $2.name, NULL); }
    | CONST IDENT '=' expressao ';' { ast_current_line = $2.line; ast_current_col = $2.col; $$ = ast_declare(1, $2.name, $4); }
    | VAR IDENT '=' expressao ';'   { ast_current_line = $2.line; ast_current_col = $2.col; $$ = ast_declare(0, $2.name, $4); }
    | VAR IDENT ';'                 { ast_current_line = $2.line; ast_current_col = $2.col; $$ = ast_declare(0, $2.name, NULL); }
;

Bloco:
    '{' lista_linhas '}' { $$ = ast_block($2); }
;

lista_linhas:
    /* vazio */ { $$ = NULL; }
    | lista_linhas elemento { $$ = ast_sequence($1, $2); }
;

lista_cases:
      /* vazio */        { $$ = NULL; }
    | lista_cases bloco_case { $$ = ast_sequence($1, $2); } 
    ;

expressao_opt:
        /* vazio */ { $$ = NULL; }
        | expressao { $$ = $1; }
;

bloco_case:
      CASE expressao ':' lista_linhas { $$ = ast_case_block($2, $4); }
    | DEFAULT ':' lista_linhas        { $$ = ast_case_block(NULL, $3); }
    ;

params_opt:
      /* vazio */ { $$ = NULL; }
    | param_list  { $$ = $1; }
;

param_list:
      IDENT                  { $$ = ast_param_list($1.name, NULL); }
    | IDENT ',' param_list   { $$ = ast_param_list($1.name, $3); }
;

args_opt:
      /* vazio */ { $$ = NULL; }
    | arg_list    { $$ = $1; }
;

arg_list:
      expressao                { $$ = ast_arg_list($1, NULL); }
    | expressao ',' arg_list   { $$ = ast_arg_list($1, $3); }
;

expressao:
    NUMBER { $$ = ast_number($1); }
    | STRING { $$ = ast_string($1); }
    | IDENT  { $$ = ast_identifier($1.name); }
    | IDENT '(' args_opt ')' { $$ = ast_func_call($1.name, $3); }
    | IDENT OP_atribuicao_soma expressao { $$ = ast_assign(OP_atribuicao_soma, $1.name, $3); }
    | IDENT OP_atribuicao_subtracao expressao { $$ = ast_assign(OP_atribuicao_subtracao, $1.name, $3); }
    | IDENT OP_atribuicao_potencia expressao { $$ = ast_assign(OP_atribuicao_potencia, $1.name, $3); }
    | IDENT OP_atribuicao_multiplicacao expressao { $$ = ast_assign(OP_atribuicao_multiplicacao, $1.name, $3); }
    | IDENT OP_atribuicao_divisao expressao { $$ = ast_assign(OP_atribuicao_divisao, $1.name, $3); }
    | IDENT OP_atribuicao_resto expressao { $$ = ast_assign(OP_atribuicao_resto, $1.name, $3); }
    | IDENT '=' expressao { $$ = ast_assign('=', $1.name, $3); }
    | IDENT OP_Incremento { $$ = ast_unary(OP_Incremento, ast_identifier($1.name)); }
    | IDENT OP_Decremento { $$ = ast_unary(OP_Decremento, ast_identifier($1.name)); }
    | IDENT OP_atribuicao_nullish expressao { $$ = ast_assign(OP_atribuicao_nullish, $1.name, $3); }
    | expressao '[' expressao ']' { $$ = ast_array_access($1, $3); }
    | expressao OP_AND expressao { $$ = ast_binary(OP_AND, $1, $3); }
    | expressao OP_OR expressao { $$ = ast_binary(OP_OR, $1, $3); }
    | expressao OP_Igualdade expressao { $$ = ast_binary(OP_Igualdade, $1, $3); }
    | expressao OP_Diferente expressao { $$ = ast_binary(OP_Diferente, $1, $3); }
    | expressao '+' expressao { $$ = ast_binary('+', $1, $3); }
    | expressao '*' expressao { $$ = ast_binary('*', $1, $3); }
    | expressao OP_Potencia expressao { $$ = ast_binary(OP_Potencia, $1, $3); }
    | expressao '%' expressao { $$ = ast_binary('%', $1, $3); }
    | expressao '-' expressao { $$ = ast_binary('-', $1, $3); }
    | expressao '>' expressao { $$ = ast_binary('>', $1, $3); }
    | expressao OP_MaiorIgual expressao { $$ = ast_binary(OP_MaiorIgual,$1,$3);}
    | expressao OP_MenorIgual expressao { $$ = ast_binary(OP_MenorIgual,$1,$3);}
    | expressao '<' expressao { $$ = ast_binary('<', $1, $3); }
    | expressao '/' expressao { $$ = ast_binary('/', $1, $3); }
    | '!' expressao { $$ = ast_unary('!', $2); }
    | '(' expressao ')' { $$ = $2; }
    | expressao OP_IgualdadeEstrita expressao { $$ = ast_binary(OP_IgualdadeEstrita, $1, $3); }
    | expressao OP_DiferenteEstrita expressao { $$ = ast_binary(OP_DiferenteEstrita, $1, $3); }
    | expressao '[' expressao ']' '=' expressao { 
          ASTNode *acesso = ast_array_access($1, $3);
          $$ = ast_array_assign(acesso, $6); 
      }
    ;
    | OP_Incremento IDENT { $$ = ast_unary(OP_Incremento, ast_identifier($2.name)); }
    | OP_Decremento IDENT { $$ = ast_unary(OP_Decremento, ast_identifier($2.name)); }

%%

/*
 * Monta e imprime a mensagem "esperado ';' depois de '<trecho>'", usando
 * o texto acumulado da linha atual e apontando a coluna logo após esse
 * trecho (onde o ';' deveria estar).
 */
static void reportar_ponto_e_virgula_faltante(int linha) {
    char trecho[LINE_BUFFER_SIZE];
    strncpy(trecho, current_line_buffer, sizeof(trecho) - 1);
    trecho[sizeof(trecho) - 1] = '\0';
    size_t len = rtrim(trecho);

    fprintf(stderr,
        "Erro de sintaxe [linha %d, col %zu]: esperado ';' depois de '%s'.\n",
        linha, len + 1, trecho);
}

/*
 * Quando a mensagem verbosa do Bison existe mas foi truncada (sem a
 * parte "expecting ...", o que o Bison faz quando há muitos tokens
 * possíveis), não temos como saber com certeza quais tokens eram
 * esperados. Nesse caso, ainda assim consideramos ';' como candidato
 * razoável apenas se o token problemático for o fim de arquivo, já
 * que esse é o cenário típico de um ';' faltando no final do programa.
 */
static int mensagem_sem_lista_de_esperados(const char *bison_msg) {
    return bison_msg && strstr(bison_msg, "expecting") == NULL;
}

void yyerror(const char *s) {
    const char *tok = token_descr();

    /*
     * yycolumn_last_token aponta para o início do token problemático.
     * yylineno é a linha onde o token foi lido.
     */
    int linha = yylineno;
    int col   = yycolumn_last_token;

    /*
     * Caso especial: quando o Bison (com parse.error verbose) indica que
     * ';' era um dos tokens esperados, preferimos uma mensagem mais
     * informativa, citando o trecho da linha já lido e apontando a
     * coluna imediatamente após esse trecho (onde o ';' deveria estar),
     * em vez de simplesmente reportar o token/EOF inesperado.
     *
     * Se a lista de tokens esperados não veio na mensagem (Bison a
     * omitiu por haver muitas alternativas) e o token problemático é o
     * fim de arquivo, assumimos o mesmo cenário: é o caso mais comum de
     * EOF inesperado nesta gramática (declaração/expressão sem ';'
     * final).
     *
     * Este caso é tratado como aviso, não como erro fatal: reporta a
     * mensagem no stderr, mas não incrementa parse_error_count, então
     * não aparece o resumo "N erro(s) de sintaxe encontrado(s)" e o
     * programa não termina com código de erro por causa dele.
     */
    if (esperava_ponto_e_virgula(s) ||
        (mensagem_sem_lista_de_esperados(s) &&
         strcmp(tok, "<fim de entrada>") == 0 &&
         current_line_buffer[0] != '\0')) {
        reportar_ponto_e_virgula_faltante(linha);
        return;
    }

    parse_error_count++;

    /* Tenta montar uma mensagem específica baseada no token atual. */
    if (strcmp(tok, "<fim de entrada>") == 0) {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: fim inesperado do arquivo.\n",
            linha, col);
    } else if (tok[0] == ';') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: ';' inesperado.\n",
            linha, col);
    } else if (tok[0] == '}') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: '}' inesperado.\n",
            linha, col);
    } else if (tok[0] == '{') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: '{' inesperado.\n",
            linha, col);
    } else if (tok[0] == ')') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: ')' inesperado.\n",
            linha, col);
    } else if (tok[0] == '(') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: '(' inesperado.\n",
            linha, col);
    } else if (tok[0] == '=') {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: '=' inesperado.\n",
            linha, col);
    } else {
        fprintf(stderr,
            "Erro de sintaxe [linha %d, col %d]: token inesperado '%s'.\n",
            linha, col, tok);
    }
}

int main(void) {
    int parse_ok = (yyparse() == 0);

    /* Erros de sintaxe */
    if (parse_error_count > 0) {
        fprintf(stderr,
                "\n%d erro(s) de sintaxe encontrado(s). Corrija-os antes de executar o programa.\n",
                parse_error_count);

        if (raiz) {
            ast_free(raiz);
            raiz = NULL;
        }

        return 1;
    }

    if (parse_ok && raiz != NULL) {

        /* Faz apenas a análise semântica. Não altera o código de retorno. */
        if (ast_check(raiz)) {
            ast_eval(raiz);
        }

        ast_free(raiz);
        raiz = NULL;
    }

    return 0;
}