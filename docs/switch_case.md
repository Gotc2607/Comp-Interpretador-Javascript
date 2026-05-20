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
2. Percorre a lista encadeada de cases em `node->right`.
3. Para cada `AST_CASE_BLOCK`, avalia o valor do case e compara com o valor de controle.
4. Se encontrar correspondência, executa o corpo do case e para.
5. Se não encontrar nenhum case e houver `default`, executa o body do default.

### Importante

O interpretador atual não faz "fallthrough" entre cases. Ou seja, quando um case combina, apenas o corpo desse case é executado e a busca termina.

## 5. Escopo de blocos

Atualmente, blocos como `{ ... }` dentro de `case` não criam um escopo separado no interpretador.
Isto é intencional para que variáveis atribuídas em cases continuem visíveis depois do `switch`.

## 6. Exemplo de uso

```js
x = 2;
switch (x) {
    case 1: {
        y = 10;
    }
    case 2: {
        y = 20;
    }
    default: {
        y = 99;
    }
}
y;
```

Saída esperada:

```txt
Resultado: 2
Resultado: 20
Resultado: 20
```

## 7. Limitações atuais

- `switch` aceita apenas comparações de valor exato entre inteiros e strings.
- `fallthrough` não é implementado; cada `case` é isolado.
- O `default` só é executado quando nenhum `case` coincide.
- O conteúdo do `switch` deve estar dentro de chaves `{}` e os cases devem terminar com `:`.

## 8. Como adicionar casos adicionais

Para suportar `fallthrough` ou `break`, seria preciso:
- estender o parser para reconhecer `break`;
- alterar a avaliação para continuar após um case combinado, a menos que haja `break`;
- ajustar a lógica de `default` para impedir sua execução quando algum case já foi executado.
