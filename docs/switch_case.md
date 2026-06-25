# `switch/case` no Interpretador JavaScript

Este documento explica como o `switch` foi implementado no interpretador e como ele se comporta.

## O que o `switch` faz

No interpretador, o `switch` avalia uma expressão de controle e compara esse valor com cada `case` disponível.
Quando um `case` coincide, o corpo daquele case é executado. Se nenhum `case` combinar, o `default` é executado quando presente.

A sintaxe suportada é semelhante a:

```js
switch (expressao) {
    case valor1: {
        // comandos
    }
    case valor2: {
        // comandos
    }
    default: {
        // comandos
    }
}
```

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.c`
- `Parser/ast.h`

## 1. Reconhecimento léxico

O lexer reconhece as palavras-chave do `switch`:

```lex
"switch"    { return SWITCH; }
"case"      { return CASE; }
"default"   { return DEFAULT; }
```

## 2. Gramática no parser

Em `Parser/parser.y`, o `switch` é aceito como um elemento da linguagem:

```bison
| SWITCH '(' expressao ')' '{' lista_cases '}' { $$ = ast_switch($3, $6); }
```

A lista de cases é composta por zero ou mais blocos de case:

```bison
lista_cases:
      /* vazio */        { $$ = NULL; }
    | lista_cases bloco_case { $$ = ast_sequence($1, $2); }
    ;

bloco_case:
      CASE expressao ':' lista_linhas { $$ = ast_case_block($2, $4); }
    | DEFAULT ':' lista_linhas        { $$ = ast_case_block(NULL, $3); }
    ;
```

O `default` é representado como um caso especial com `case_expr == NULL`.

## 3. AST

No `Parser/ast.h`, existem dois novos nós:

- `AST_SWITCH`
- `AST_CASE_BLOCK`

Os construtores em `Parser/ast.c` criam esses nós:

```c
ASTNode *ast_switch(ASTNode *control_expr, ASTNode *cases_list);
ASTNode *ast_case_block(ASTNode *case_expr, ASTNode *body);
```

## 4. Avaliação do `switch`

A lógica de execução do `switch` está em `Parser/ast.c` dentro de `ast_eval`.

Passos principais:

1. Avalia `node->left` para obter o valor de controle.
2. Coleta e ordena os casos da esquerda para a direita (primeiro para o último) resolvendo a recursividade à esquerda do parser.
3. Percorre os casos em ordem para encontrar o primeiro cuja expressão seja estritamente igual ao valor de controle (utilizando `verificar_igualdade_estrita`).
4. Se nenhum caso for compatível, seleciona o bloco `default` (se presente) como ponto de partida.
5. Inicia a execução em cascata a partir do caso selecionado (ou `default`), avaliando seus comandos sequencialmente (fallthrough).
6. Interrompe a execução em cascata do `switch` apenas se um comando `break` (que retorna `CTRL_BREAK`) for avaliado, limpando esse sinalizador para que a execução do programa continue normalmente.

### Suporte a Fallthrough e Break

O interpretador agora suporta **fallthrough** por padrão. Se um `case` coincidir e seu bloco não contiver um comando `break`, a execução continuará nos blocos dos casos subsequentes (inclusive do `default` se este estiver posicionado após o caso correspondente), até que um `break` seja encontrado ou a lista de casos termine.

## 5. Escopo de blocos

Atualmente, blocos como `{ ... }` dentro de `case` não criam um escopo separado no interpretador.
Isto é intencional para que variáveis atribuídas em cases continuem visíveis depois do `switch`.

## 6. Exemplo de uso

### Com uso de `break` (comportamento tradicional)
```js
let x = 2;
let y = 0;
switch (x) {
    case 1: {
        y = 10;
        break;
    }
    case 2: {
        y = 20;
        break;
    }
    default: {
        y = 99;
    }
}
console.log(y);
```

Saída esperada:
```txt
20
```

### Com Fallthrough (sem `break`)
```js
let x = 2;
let y = 0;
switch (x) {
    case 1:
        y += 1;
    case 2:
        y += 10;
    case 3:
        y += 100;
        break;
    case 4:
        y += 1000;
}
console.log(y);
```

Saída esperada (o caso 2 executa e faz fallthrough para o caso 3):
```txt
110
```

## 7. Limitações atuais

- `switch` aceita apenas comparações de valor estrito (equivalente a `===`) entre os tipos suportados (inteiros, booleanos e strings).
- O conteúdo do `switch` deve estar dentro de chaves `{}` e os cases/default devem terminar com `:`.

