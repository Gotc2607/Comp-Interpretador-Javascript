# `console.log`

Esta documentação descreve como o comando `console.log` funciona no interpretador.

## Objetivo

Permitir a impressão de valores apenas quando o usuário usa explicitamente `console.log(...)`, evitando saídas implícitas de expressões não solicitadas.

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.h`
- `Parser/ast.c`

## Como funciona

- O lexer reconhece `console.log` como um token especial `CONSOLE_LOG`.
- O parser transforma `console.log(expr);` em um nó AST do tipo `AST_CONSOLE_LOG`.
- Durante a avaliação, o nó `AST_CONSOLE_LOG` avalia a expressão interna e imprime o resultado.

## Sintaxe suportada

```js
console.log(123);
console.log("texto");
console.log(x + 5);
```

Observação: a forma deve ser exatamente `console.log(...)` como um único token, sem espaços dentro do `console.log`.

## Comportamento

- Se a expressão for string, imprime o conteúdo textual.
- Se a expressão for número, imprime o valor inteiro.
- Outras expressões são avaliadas pelo interpretador e o resultado é impresso.
- Se a expressão usar variável, a variável deve ser declarada anteriormente.

## Exemplo

```js
let saudacao = "Testando o console.log!";
console.log(saudacao);
console.log("O resultado da expressao e:");
console.log(100 + 50 * 2);
```

Saída esperada:

```
Testando o console.log!
O resultado da expressao e:
200
```

## Detalhes de implementação

No lexer (`Lexer/scanner.l`):

```c
"console.log" { return CONSOLE_LOG; }
```

No parser (`Parser/parser.y`):

```yacc
| CONSOLE_LOG '(' expressao ')' ';' { $$ = ast_console_log($3); }
```

No AST (`Parser/ast.c`):

```c
case AST_CONSOLE_LOG:
    left = ast_eval(node->left);
    if (left.type == VAL_STRING) {
        printf("%s\n", left.sval ? left.sval : "");
    } else {
        printf("%d\n", left.ival);
    }
    return left;
```

## Nota

A implementação mantém `console.log` como única forma de saída do interpretador, o que torna o comportamento mais previsível e fácil de testar.
