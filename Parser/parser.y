%{
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

%token <ival> NUMBER
%token <sval> STRING
%token <sval> IDENT
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
    elementos { raiz = $1; $$ = $1; }
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
    | FUNCTION IDENT '(' params_opt ')' Bloco { $$ = ast_func_decl($2, $4, $6); }
    | RETURN expressao_opt ';' { $$ = ast_return($2); }
;

Linha:
    expressao ';' { $$ = $1; }
    | LET IDENT '=' expressao ';'   { $$ = ast_declare(0, $2, $4); }
    | LET IDENT ';'                 { $$ = ast_declare(0, $2, NULL); }
    | CONST IDENT '=' expressao ';' { $$ = ast_declare(1, $2, $4); }
    | VAR IDENT '=' expressao ';'   { $$ = ast_declare(0, $2, $4); }
    | VAR IDENT ';'                 { $$ = ast_declare(0, $2, NULL); }
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
      IDENT                  { $$ = ast_param_list($1, NULL); }
    | IDENT ',' param_list   { $$ = ast_param_list($1, $3); }
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
    | IDENT  { $$ = ast_identifier($1); }
    | IDENT '(' args_opt ')' { $$ = ast_func_call($1, $3); }
    | IDENT OP_atribuicao_soma expressao { $$ = ast_assign(OP_atribuicao_soma, $1, $3); }
    | IDENT OP_atribuicao_subtracao expressao { $$ = ast_assign(OP_atribuicao_subtracao, $1, $3); }
    | IDENT OP_atribuicao_potencia expressao { $$ = ast_assign(OP_atribuicao_potencia, $1, $3); }
    | IDENT OP_atribuicao_multiplicacao expressao { $$ = ast_assign(OP_atribuicao_multiplicacao, $1, $3); }
    | IDENT OP_atribuicao_divisao expressao { $$ = ast_assign(OP_atribuicao_divisao, $1, $3); }
    | IDENT OP_atribuicao_resto expressao { $$ = ast_assign(OP_atribuicao_resto, $1, $3); }
    | IDENT '=' expressao { $$ = ast_assign('=',$1,$3);}
    | IDENT OP_Incremento { $$ = ast_unary(OP_Incremento,ast_identifier($1));}
    | IDENT OP_Decremento { $$ = ast_unary(OP_Decremento,ast_identifier($1));}
    | IDENT OP_atribuicao_nullish expressao { $$ = ast_assign(OP_atribuicao_nullish, $1, $3); }
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
    | OP_Incremento IDENT { $$ = ast_unary(OP_Incremento, ast_identifier($2)); }
    | OP_Decremento IDENT { $$ = ast_unary(OP_Decremento, ast_identifier($2)); }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    if (yyparse() == 0 && raiz != NULL) {
        if (ast_check(raiz)) {
            ast_eval(raiz);
        }
        ast_free(raiz);
        raiz = NULL;
    }
    return 0;
}