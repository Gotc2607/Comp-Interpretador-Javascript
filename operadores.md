# Guia de Adicao de Operadores (Flex + Bison)

Este guia mostra, passo a passo, como adicionar novos operadores no interpretador.

Estrutura considerada:

- `Lexer/scanner.l` (analise lexica)
- `Parser/parser.y` (analise sintatica + acoes semanticas)

---

## Visao Geral do Fluxo

1. O **Flex** reconhece um simbolo (ex.: `*`) e retorna um token para o parser.
2. O **Bison** declara esse token e define em quais regras ele aparece.
3. O **Bison** recebe e combina valores semanticos via `%union`, `yylval`, `$$`, `$1`, `$3`.
4. A precedencia (`%left`, `%right`) evita conflitos de **Shift/Reduce** e define a hierarquia de operadores.

---

## Parte 1: Validacao (Lexica e Sintatica)

Objetivo desta parte: garantir que o parser **reconheca** o novo operador e aceite a sintaxe corretamente.
Nesta fase, foque apenas em token, regra gramatical e precedencia (sem calculo em `$$` ainda).

### 1) Definir o simbolo no Flex (`scanner.l`)

No bloco de regras (`%% ... %%`), adicione o operador desejado.

Exemplo para multiplicacao:

```lex
"*"         { return '*'; }
```

Dica:
- Para operadores simples de 1 caractere (`+`, `-`, `*`, `/`, `(`, `)`), usar o proprio caractere como token simplifica o parser.
- Para operadores compostos (`==`, `!=`, `<=`, `>=`), normalmente e melhor usar token nomeado (`OP_Igualdade`, etc.).

### 2) Declarar token no Bison (`parser.y`)

No cabecalho de declaracoes, garanta que o parser conhece o operador.

Para `*` como literal de caractere:

```bison
%token '+'
%token '*'
%token OP_Igualdade NUMBER IDENT
```

Observacao:
- Se preferir token nomeado para `*`, use `%token OP_MULT` e no Flex retorne `OP_MULT`.
- `+` e `*` podem ser declarados no mesmo `%token` ou em linhas separadas; neste guia, os exemplos ficam separados por didatica.

### 3) Definir precedencia e associatividade

**O que e precedencia?**

Precedencia e a "prioridade" entre operadores em uma expressao.
Ela define qual operacao deve ser avaliada primeiro quando ha mais de um operador na mesma linha.

Exemplo:

- Em `2 + 3 * 4`, a multiplicacao tem precedencia maior que a soma, entao calculamos primeiro `3 * 4`.
- Resultado: `2 + 12 = 14`.

**E associatividade?**

Associatividade define como operadores de **mesma precedencia** sao agrupados.

- `%left`: agrupa da esquerda para a direita (ex.: `10 - 3 - 2` vira `(10 - 3) - 2`).
- `%right`: agrupa da direita para a esquerda (ex.: exponenciacao em muitas linguagens).

No Bison, a ordem das declaracoes de precedencia vai de **menor para maior precedencia**.

Para que `*` tenha precedencia maior que `+`, e `+` maior que `==`:

```bison
%left OP_Igualdade
%left '+'
%left '*'
```

Resultado esperado:
- `2 + 3 * 4` vira `2 + (3 * 4)`
- `2 + 3 == 5` vira `(2 + 3) == 5`

### 4) Como colocar o novo operador na area de `expressao` (Bison)

Depois de declarar o token e a precedencia, o proximo passo e incluir o operador na regra gramatical `expressao`.

Passo a passo:

1. Encontre o bloco da regra `expressao:` no `parser.y`.
2. Adicione uma nova alternativa com o formato geral:

```bison
| expressao OPERADOR expressao
```

3. Nesta etapa de validacao, mantenha a regra sem acao semantica.
4. Teste se o parser aceita entradas com o novo operador sem erro de sintaxe.

Exemplo com multiplicacao (`*`):

Antes:

```bison
expressao:
            NUMBER
        | expressao OP_Igualdade expressao
        | expressao '+' expressao
        ;
```

Depois:

```bison
expressao:
            NUMBER
        | expressao OP_Igualdade expressao
        | expressao '+' expressao
        | expressao '*' expressao
        ;
```

Relacao com Shift/Reduce:

Sem precedencia, a gramatica acima pode ser ambigua.
Ao ler `2 + 3 * 4`, o parser pode ficar em duvida se reduz `2 + 3` antes de ler `*`.
As diretivas `%left`/`%right` (na ordem correta) resolvem isso e garantem a hierarquia esperada.

---

## Parte 2: Semantica (Acao)

Objetivo desta parte: agora que a validacao sintatica funciona, adicionar significado aos operadores.
Aqui voce vai carregar valores no lexer e calcular no parser.

## 2.1 Definir `%union` no Bison

A `%union` diz quais tipos de valor semantico circulam entre tokens e regras.

```bison
%union {
    int ival;
    char *sval;
}
```

O que significam esses campos:

- `int ival`: e um campo para guardar valores inteiros (por exemplo, o valor numerico de `NUMBER`, como `42`).
- `char *sval`: e um ponteiro para texto (string), usado para guardar lexemas como identificadores (`IDENT`), por exemplo `nome` ou `x`.

Em outras palavras, a `%union` funciona como uma "caixa" de valores possiveis do Bison:

- quando o token for numero, voce preenche `yylval.ival`;
- quando o token for identificador, voce preenche `yylval.sval`.

Assim, cada token usa o campo que corresponde ao seu tipo semantico.

Depois associe tipos aos tokens/nao-terminais:

```bison
%token <ival> NUMBER
%token <sval> IDENT
%token OP_Igualdade
%token '+' 
%token '*'

%type <ival> expressao
```

Dica de organizacao:

- O mais comum e declarar `%type <ival> expressao` logo abaixo dos `%token`.
- Nao e obrigatorio estar exatamente "embaixo", mas ele precisa estar na area de declaracoes do Bison (antes do primeiro `%%`).

Ordem recomendada para ficar organizado:

1. `%union`
2. `%token`
3. `%type`
4. precedencia (`%left`, `%right`, `%nonassoc`)

- `NUMBER` carrega inteiro (`ival`)
- `IDENT` pode carregar string (`sval`)
- `expressao` vai produzir inteiro (`ival`) neste exemplo

## 2.2 Capturar valor no Flex com `yylval`

No `scanner.l`, inclua `stdlib.h` e preencha `yylval` antes de `return`.

```lex
%{
#include <stdio.h>
#include <stdlib.h>
#include "../Parser/parser.tab.h"
%}

[0-9]+      {
    yylval.ival = atoi(yytext);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.sval = strdup(yytext);
    return IDENT;
}
```

Observacao importante:
- Se usar `strdup`, depois libere memoria no parser (ou na tabela de simbolos), para evitar vazamento.

## 2.3 Calculo no Bison com `$$`, `$1`, `$3`

Agora sim entra a semantica da regra de `expressao`.
Depois da validacao, adicione as acoes para produzir resultado:

Em acoes semanticas:

- `$1` = valor do primeiro simbolo da regra
- `$3` = valor do terceiro simbolo
- `$$` = valor produzido pela regra inteira

Exemplo:

```bison
expressao:
      NUMBER
        { $$ = $1; }
    | expressao '+' expressao
        { $$ = $1 + $3; }
    | expressao '*' expressao
        { $$ = $1 * $3; }
    | expressao OP_Igualdade expressao
        { $$ = ($1 == $3); }
    ;
```

- Em igualdade, o resultado vira `1` (verdadeiro) ou `0` (falso).

## 2.4 Operadores com estado (ex.: `+=`, `-=`, `=`)

Alguns operadores nao apenas calculam, eles tambem **atualizam estado**.
Esse e o caso de operadores de atribuicao composta como `+=`.

Para esse tipo de operador, alem de `%union` e `yylval`, voce precisa de uma tabela de simbolos.

### O que a tabela de simbolos faz

- Guarda pares `nome -> valor` (ex.: `x -> 11`).
- Permite ler o valor atual (`get_var`).
- Permite atualizar/criar valor (`set_var`).

Sem isso, `IDENT += expressao` nao tem como funcionar, porque o parser nao sabe onde guardar o novo valor da variavel.

### Fluxo semantico de `x += 5`

1. Ler valor atual de `x` com `get_var("x")`.
2. Somar com valor da direita (`+ 5`).
3. Gravar de volta com `set_var("x", novo_valor)`.
4. Retornar o resultado em `$$`.

### Regra no Bison (ideia geral)

```bison
| IDENT OP_atribuicao_soma expressao {
    int novo = get_var($1) + $3;
    set_var($1, novo);
    $$ = novo;
    free($1);
}
```

Observacao:

- Como `IDENT` vem de `strdup` no lexer, use `free($1)` depois da acao para evitar vazamento de memoria.

### Quando usar `IDENT` explicito na gramatica

Regra pratica para novos operadores:

1. Use `IDENT` explicito quando o operador altera diretamente uma variavel.
2. Use `expressao OPERADOR expressao` quando o operador apenas combina/calcula valores.

Exemplos de operadores que alteram variavel (lado esquerdo com `IDENT`):

```bison
| IDENT '=' expressao
| IDENT OP_atribuicao_soma expressao
| IDENT OP_atribuicao_sub expressao
```

Exemplos de operadores de calculo geral (sem `IDENT` explicito):

```bison
| expressao '+' expressao
| expressao '*' expressao
| expressao OP_Igualdade expressao
```

Motivo:

- Em operadores de calculo, `IDENT` ja entra de forma indireta pela regra base da expressao (ex.: `expressao: IDENT { ... }`).
- Em operadores de atribuicao, a gramatica precisa identificar explicitamente qual variavel sera atualizada.

---

## Exemplo Pratico Completo: Operador de Multiplicacao (`*`)

A seguir, um exemplo direto de como integrar `*` no seu projeto.

### Etapa 1: Somente validacao (semantica ainda nao implementada)

Use esta etapa para confirmar que o parser reconhece `*` com a precedencia correta.

## A) Alteracoes em `Lexer/scanner.l`

Trecho recomendado:

```lex
%{
#include <stdio.h>
#include <stdlib.h>
#include "../Parser/parser.tab.h"
%}

%%
"=="        { return OP_Igualdade; }
"+"         { return '+'; }
"*"         { return '*'; }
";"         { return ';'; }

[0-9]+      { return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { return IDENT; }

[ \t\n]+    { /* ignora espacos */ }
.           { printf("Caractere desconhecido: %s\n", yytext); }
%%
```

## B) Alteracoes em `Parser/parser.y`

Trecho recomendado:

```bison
%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

%token NUMBER IDENT
%token OP_Igualdade
%token '+'
%token '*'

/* Menor -> maior precedencia */
%left OP_Igualdade
%left '+'
%left '*'

%%
programa:
      /* vazio */
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
        | expressao '*' expressao
    ;
%%
```

### Etapa 2: Adicionar semantica e acao

Depois de validar a sintaxe, evolua para a versao com `%union`, `yylval` e acoes `$$`, `$1`, `$3`.
Esta etapa corresponde aos exemplos da Parte 2 deste guia.

## C) Alteracoes em `Lexer/scanner.l` (com semantica)

Trecho recomendado:

```lex
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Parser/parser.tab.h"
%}

%%
"=="        { return OP_Igualdade; }
"+"         { return '+'; }
"*"         { return '*'; }
";"         { return ';'; }

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
```

## D) Alteracoes em `Parser/parser.y` (com semantica)

Trecho recomendado:

```bison
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);
%}

%union {
    int ival;
    char *sval;
}

%token <ival> NUMBER
%token <sval> IDENT
%token OP_Igualdade
%token '+'
%token '*'

%type <ival> expressao

/* Menor -> maior precedencia */
%left OP_Igualdade
%left '+'
%left '*'

%%
programa:
      /* vazio */
    | programa Linha
    ;

Linha:
      expressao ';' { printf("Resultado: %d\n", $1); }
    ;

expressao:
      NUMBER                          { $$ = $1; }
    | expressao OP_Igualdade expressao { $$ = ($1 == $3); }
    | expressao '+' expressao          { $$ = $1 + $3; }
    | expressao '*' expressao          { $$ = $1 * $3; }
    ;
%%
```

Observacao:

- Se `IDENT` for retornado com `strdup`, libere a memoria quando nao precisar mais dele.

## E) Hierarquia de precedencia (onde `*` se encaixa)

Use esta ordem:

1. `OP_Igualdade` (`==`) - menor precedencia
2. `'+'` - intermediaria
3. `'*'` - maior precedencia (entre as tres)

Em Bison, isso significa declarar nessa sequencia (de cima para baixo):

```bison
%left OP_Igualdade
%left '+'
%left '*'
```

## Exemplo Adicional: Operador `+=`

Este exemplo mostra como adicionar um operador de atribuicao composta, que exige estado.

### 1) Flex (`scanner.l`)

```lex
"+="        { return OP_atribuicao_soma; }
"+"         { return '+'; }
```

### 2) Bison: token e precedencia (`parser.y`)

```bison
%token OP_atribuicao_soma

/* Menor -> maior precedencia */
%right OP_atribuicao_soma
%left OP_Igualdade
%left '+'
%left '*'
```

### 3) Bison: tabela de simbolos minima

```c
#define MAX_VARS 256

typedef struct {
        char nome[64];
        int valor;
} Variavel;

static Variavel tabela[MAX_VARS];
static int qtd_vars = 0;

static int find_var(const char *nome);
static int get_var(const char *nome);
static void set_var(const char *nome, int valor);
```

### 4) Bison: regra semantica de `+=`

```bison
expressao:
            NUMBER { $$ = $1; }
        | IDENT  { $$ = get_var($1); free($1); }
        | IDENT OP_atribuicao_soma expressao {
                    int novo = get_var($1) + $3;
                    set_var($1, novo);
                    $$ = novo;
                    free($1);
            }
        | expressao '+' expressao { $$ = $1 + $3; }
        | expressao '*' expressao { $$ = $1 * $3; }
        | expressao OP_Igualdade expressao { $$ = ($1 == $3); }
        ;
```

### 5) Teste sugerido para `+=`

```txt
x += 5;
x += 2 * 3;
x + 1 == 12;
```

Resultados esperados:

- `x += 5;` -> `5`
- `x += 2 * 3;` -> `11`
- `x + 1 == 12;` -> `1`

---

## Checklist Rapido para Novo Operador

1. Adicionar regra no Flex retornando token correto.
2. Declarar token no Bison (`%token`).
3. Inserir operador em regra(s) de `expressao`.
4. Ajustar precedencia com `%left`/`%right` na ordem correta.
5. Se houver valor semantico, mapear em `%union` e usar `yylval`.
6. Implementar acao com `$$`, `$1`, `$3`.
7. Testar entradas com e sem mistura de operadores para validar precedencia.

---

## Comandos de Teste Sugeridos

Depois de alterar, regenere lexer/parser e compile:

```bash
cd Lexer && flex -o lex.yy.c scanner.l
cd ../Parser && bison -d -o parser.tab.c parser.y
cd .. && gcc Lexer/lex.yy.c Parser/parser.tab.c -o interpretador -lfl
./interpretador
```

Teste com:

```txt
2 + 3 * 4;
2 + 3 == 5;
2 * 3 == 6;
```

Saidas esperadas (sem tabela de simbolos):

- `2 + 3 * 4;` -> `14`
- `2 + 3 == 5;` -> `1`
- `2 * 3 == 6;` -> `1`


## Execucao de Testes Automatizados
Para testar a validade das operacoes:
1. Crie o arquivo de entrada em `testes/nome.js`.
2. Crie o arquivo com a saida esperada em `testes/nome.out`.
3. Execute no terminal: `python3 executar_testes.py`.