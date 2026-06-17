# Estruturas de Repetição (`while` e `do-while`)

Este documento descreve como os laços de repetição `while` e `do-while` foram implementados no interpretador.

## Objetivo

O objetivo é permitir a execução repetida de um bloco de código com base em uma condição (verdadeiro/falso). No `while`, a condição é avaliada antes; no `do-while`, o corpo é executado pelo menos uma vez antes da avaliação.

Exemplo suportado:

```js
while (x < 10) { x = x + 1; }
do { y = y - 1; } while (y > 0);
```

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.h`
- `Parser/ast.c`

## 1. Lexer

No arquivo `Lexer/scanner.l`, os tokens são reconhecidos:

```c
"while"     { return WHILE; }
"do"        { return DO; }
```

## 2. Parser

No arquivo `Parser/parser.y`, as palavras são declaradas como tokens e integradas às regras:

```yacc
%token WHILE DO

| WHILE '(' expressao ')' elemento { $$ = ast_while($3, $5); }
| DO Bloco WHILE '(' expressao ')' ';' { $$ = ast_do_while($5, $2); }
```

## 3. AST

No arquivo `Parser/ast.h`, foram criados dois novos tipos de nós na enumeração `ASTKind`:

```c
AST_WHILE,
AST_DO_WHILE,
```

Os construtores recebem a condição e o corpo do laço:

```c
ASTNode *ast_while(ASTNode *cond, ASTNode *body);
ASTNode *ast_do_while(ASTNode *cond, ASTNode *body);
```

## 4. Avaliação

A lógica de execução fica em `Parser/ast.c`. Foi criada uma macro `IS_TRUTHY` para avaliar se a condição é verdadeira (números `!= 0` ou strings não vazias).

- O `AST_WHILE` entra num `while (1)` no C, avalia a condição primeiro e dá `break` se for falsa.
- O `AST_DO_WHILE` utiliza um `do { ... } while (1)` no C, executando o corpo antes de avaliar a condição.

Código relevante (`do-while`):

```c
case AST_DO_WHILE: {
    RuntimeValue cond_val;
    RuntimeValue last_val = {VAL_NULL, 0, NULL};
    do {
        last_val = ast_eval(node->right);
        cond_val = ast_eval(node->left);
        if (!IS_TRUTHY(cond_val)) break;
    } while (1);
    return last_val;
}
```