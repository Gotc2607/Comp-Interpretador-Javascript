# Atribuição Nullish (`??=`)

Este documento descreve como o operador `??=` foi implementado no interpretador.

## Objetivo

O operador `??=` atribui um valor somente quando a variável ainda não existe ou não tem um valor válido.

Exemplo suportado:

```js
x ??= 10;
```

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.c`
- `Parser/ast.h`

## 1. Lexer

No arquivo `Lexer/scanner.l`, o token `??=` é reconhecido e transformado em `OP_atribuicao_nullish`:

```c
"??="       { return OP_atribuicao_nullish; }
```

Isso permite que o parser identifique o operador como um símbolo único.

## 2. Parser

No arquivo `Parser/parser.y`, o token é declarado e integrado à gramática de expressão:

```yacc
%token OP_atribuicao_nullish
%right OP_atribuicao_nullish
```

A regra de atribuição passa a aceitar:

```yacc
| IDENT OP_atribuicao_nullish expressao { $$ = ast_assign(OP_atribuicao_nullish, $1, $3); }
```

Assim, `x ??= 10` produz um nó `AST_ASSIGN` com a operação `OP_atribuicao_nullish`.

## 3. AST

No arquivo `Parser/ast.h` não foi necessário criar um novo tipo de nó, pois `??=` é tratado como um caso especial de `AST_ASSIGN`.

O construtor de atribuição já existente é usado:

```c
ASTNode *ast_assign(int op, char *name, ASTNode *expression);
```

## 4. Avaliação

A lógica de `??=` fica em `Parser/ast.c`, dentro de `eval_assign`.

### Comportamento implementado

- Se a variável já existe e tem tipo `SYM_INT` ou `SYM_STRING`, o valor existente é mantido.
- Caso contrário, a variável recebe o novo valor.
- A expressão retorna o valor final da variável.

### Código relevante

```c
if (node->op == OP_atribuicao_nullish) {
    if (sym_exists(node->text) && (tipo_atual == SYM_INT || tipo_atual == SYM_STRING)) {
        if (tipo_atual == SYM_STRING) {
            result.type = VAL_STRING;
            result.sval = sym_get_str(node->text);
        } else {
            result.type = VAL_INT;
            result.ival = sym_get_int(node->text);
        }
        return result;
    }
    if (value.type == VAL_STRING) {
        sym_set_str(node->text, value.sval ? value.sval : "");
    } else {
        sym_set_int(node->text, value.ival);
    }
    return result;
}
```

### Observações

- A implementação atual considera `int` e `string` como valores "existentes" válidos.
- Se a variável existir com outro tipo, o código atual atribui o novo valor mesmo assim.
- O operador `??=` não é um nó separado na AST; ele é apenas um caso especial de `AST_ASSIGN`.

## 5. Exemplo de uso

```js
x ??= 1;    // x é criado com 1
x ??= 2;    // x permanece 1

s ??= "ola";   // s é criado com "ola"
s ??= "tchau"; // s permanece "ola"
```

## 6. Limitações atuais

- O operador só foi implementado para variáveis simples (`IDENT`), não para acessos a propriedade ou arrays.
- O código não diferencia `null` de variáveis inexistentes; ele apenas verifica se a variável existe e é int/string.
- O tipo de retorno é `RuntimeValue`, então strings e inteiros são tratados corretamente ao final da avaliação.
