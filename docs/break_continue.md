# `break` e `continue` no Interpretador JavaScript

Este documento explica como o `break` e o `continue` foram implementados no interpretador e como eles se comportam.

## O que o `break` e o `continue` fazem

No interpretador, `break` e `continue` são instruções de controle de fluxo que funcionam dentro de laços (`while`, `do...while`, `for`).

- **`break`**: interrompe imediatamente o laço em execução e transfere o controle para o código após o laço.
- **`continue`**: pula o restante do corpo do laço na iteração atual e avança para a próxima verificação de condição (e, no `for`, executa a expressão de atualização antes de checar a condição).

A sintaxe suportada é:

```js
while (condicao) {
    if (algo) break;
    if (outraCoisas) continue;
    // restante do corpo
}
```

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.c`
- `Parser/ast.h`

## 1. Reconhecimento léxico

O lexer reconhece as palavras-chave:

```lex
"break"     { return BREAK; }
"continue"  { return CONTINUE; }
```

## 2. Gramática no parser

Em `Parser/parser.y`, `break` e `continue` são aceitos como elementos da linguagem:

```bison
| BREAK ';'    { $$ = ast_break_stmt(); }
| CONTINUE ';' { $$ = ast_continue_stmt(); }
```

Ambos produzem um nó de AST folha sem filhos — são instruções sem expressão associada.

## 3. AST

No `Parser/ast.h`, existem dois tipos de nó correspondentes:

- `AST_BREAK`
- `AST_CONTINUE`

Os construtores em `Parser/ast.c` criam esses nós:

```c
ASTNode *ast_break_stmt(void);
ASTNode *ast_continue_stmt(void);
```

Cada função simplesmente aloca um nó do tipo correspondente sem filhos (`ast_new(AST_BREAK)` / `ast_new(AST_CONTINUE)`).

## 4. Mecanismo de sinalização via `control_flow`

O controle de fluxo é propagado pelo campo `control_flow` da struct `RuntimeValue`:

```c
typedef struct {
    ValueType type;
    int ival;
    char *sval;
    int control_flow;
} RuntimeValue;

#define CTRL_NONE     0
#define CTRL_BREAK    1
#define CTRL_CONTINUE 2
```

Quando o avaliador encontra um nó `AST_BREAK` ou `AST_CONTINUE`, ele retorna um `RuntimeValue` com o campo `control_flow` preenchido:

```c
case AST_BREAK:
    result.control_flow = CTRL_BREAK;
    return result;

case AST_CONTINUE:
    result.control_flow = CTRL_CONTINUE;
    return result;
```

Esse sinal se propaga pela árvore até o laço mais próximo, que o intercepta e toma a ação adequada.

## 5. Avaliação nos laços

### `while`

```c
case AST_WHILE: {
    while (1) {
        cond_val = ast_eval(node->left);
        if (!is_true) break;

        last_val = ast_eval(node->right);

        if (last_val.control_flow == CTRL_BREAK) {
            last_val.control_flow = CTRL_NONE;
            break;
        }

        if (last_val.control_flow == CTRL_CONTINUE) {
            last_val.control_flow = CTRL_NONE;
            continue;
        }
    }
}
```

### `do...while`

```c
case AST_DO_WHILE: {
    do {
        last_val = ast_eval(node->right);

        if (last_val.control_flow == CTRL_BREAK) {
            last_val.control_flow = CTRL_NONE;
            break;
        }

        if (last_val.control_flow == CTRL_CONTINUE) {
            last_val.control_flow = CTRL_NONE;
        }

        cond_val = ast_eval(node->left);
        if (!IS_TRUTHY(cond_val)) break;
    } while (1);
}
```

### `for`

```c
case AST_FOR: {
    ast_eval(node->left); // inicialização

    while (1) {
        cond_val = ast_eval(node->mid);
        if (!is_true) break;

        last_val = ast_eval(node->right);

        if (last_val.control_flow == CTRL_BREAK) {
            last_val.control_flow = CTRL_NONE;
            break;
        }

        if (last_val.control_flow == CTRL_CONTINUE) {
            last_val.control_flow = CTRL_NONE;
            if (node->extra) ast_eval(node->extra); // atualização
            continue;
        }

        if (node->extra) ast_eval(node->extra); // atualização
    }
}
```

### Importante

Em todos os laços, após interceptar o sinal de `break` ou `continue`, o campo `control_flow` é zerado (`CTRL_NONE`) para evitar que o sinal continue se propagando para laços externos inadvertidamente.

No `continue` dentro de um `for`, a expressão de atualização (terceira parte do `for`) é executada antes de avançar para a próxima iteração, o que está de acordo com o comportamento do JavaScript.

## 6. Propagação em `AST_SEQUENCE`

> ⚠️ **Bug conhecido:** a propagação em `AST_SEQUENCE` está incompleta. Veja a seção 9 para detalhes e a correção.

Em `AST_SEQUENCE`, o sinal de controle é propagado para cima apenas quando detectado no **nó da direita**:

```c
case AST_SEQUENCE:
    left = ast_eval(node->left);

    if (node->right) {
        right = ast_eval(node->right);  // executado mesmo que left tenha CTRL_BREAK/CONTINUE

        if (right.control_flow != CTRL_NONE) {
            return right;
        }

        return right;
    }

    return left;
```

O problema é que `left.control_flow` nunca é verificado antes de avaliar `node->right`. Quando o `break` ou `continue` aparece no nó esquerdo da sequência, o nó direito ainda é avaliado — ou seja, instruções que deveriam ser puladas acabam sendo executadas.

## 7. Exemplos de uso

### `break` em `while`

```js
i = 0;
while (i < 10) {
    if (i == 3) break;
    i = i + 1;
}
i;
```

Saída esperada:

```txt
Resultado: 3
```

### `continue` em `for`

```js
for (i = 0; i < 5; i = i + 1) {
    if (i == 2) continue;
    i;
}
```

Saída esperada:

```txt
Resultado: 0
Resultado: 1
Resultado: 3
Resultado: 4
```

### `break` em `do...while`

```js
i = 0;
do {
    if (i == 2) break;
    i = i + 1;
} while (i < 10);
i;
```

Saída esperada:

```txt
Resultado: 2
```

## 8. Limitações atuais

- `break` e `continue` **não suportam labels**: não é possível escrever `break label;` ou `continue label;` para sair de laços externos em estruturas aninhadas.
- Dentro de um `switch`, o `break` ainda não é utilizado para encerrar um `case` — o interpretador já para automaticamente no primeiro `case` que coincide (sem fallthrough). O `break` em um `switch` encerraria o laço pai, não o `switch` em si.
- Usar `break` ou `continue` fora de um laço causa comportamento indefinido: o sinal `control_flow` se propaga até o topo sem ser interceptado.

## 9. Bug: `AST_SEQUENCE` não verifica `control_flow` do nó esquerdo

### Descrição

Em `Parser/ast.c`, o caso `AST_SEQUENCE` avalia `node->right` sem antes verificar se `node->left` gerou um sinal de `break` ou `continue`:

```c
// código atual — com bug
case AST_SEQUENCE:
    left = ast_eval(node->left);

    if (node->right) {
        right = ast_eval(node->right);  // ← deveria ser condicional

        if (right.control_flow != CTRL_NONE) {
            return right;
        }

        return right;
    }

    return left;
```

### Impacto

Se o `break` ou `continue` for o nó esquerdo de uma sequência, as instruções à direita são executadas mesmo assim. Dependendo de como o Bison constrói a árvore para um bloco específico, isso pode fazer com que código após o `break`/`continue` rode indevidamente.

### Correção

Adicionar a verificação de `left.control_flow` antes de avaliar `node->right`:

```c
// código corrigido
case AST_SEQUENCE:
    left = ast_eval(node->left);

    if (left.control_flow != CTRL_NONE) {
        return left;
    }

    if (node->right) {
        right = ast_eval(node->right);

        if (right.control_flow != CTRL_NONE) {
            return right;
        }

        return right;
    }

    return left;
```