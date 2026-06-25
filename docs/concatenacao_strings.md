# Concatenação de Strings (`+`)

Este documento descreve como a concatenação de strings usando o operador `+` foi implementada no interpretador JavaScript.

## Objetivo

Permitir que strings sejam concatenadas dinamicamente usando o operador binário `+`. Se pelo menos um dos operandos da soma for uma string, o interpretador realiza a conversão implícita dos operandos para texto e faz a concatenação, exatamente como ocorre em JavaScript convencional.

## Exemplo Suportado

```js
console.log("a" + "b"); // Imprime "ab"
console.log("Resultado: " + 42); // Imprime "Resultado: 42"
console.log(true + "!"); // Imprime "true!"
```

## Arquivos Envolvidos

- `Parser/ast.c` (Lógica de avaliação da AST)

## Funcionamento Técnico

No tratamento do nó `AST_BINARY` para a operação `+` (em `Parser/ast.c`), a lógica do interpretador verifica se o operando esquerdo ou o operando direito possui o tipo `VAL_STRING`:

```c
case '+':
    if (left.type == VAL_STRING || right.type == VAL_STRING) {
        char buf_l[32] = "";
        char buf_r[32] = "";
        char *s_l = "";
        char *s_r = "";

        // Conversão implícita do operando esquerdo para string
        if (left.type == VAL_STRING) {
            s_l = left.sval ? left.sval : "";
        } else if (left.type == VAL_INT) {
            snprintf(buf_l, sizeof(buf_l), "%d", left.ival);
            s_l = buf_l;
        } else if (left.type == VAL_BOOL) {
            s_l = left.ival ? "true" : "false";
        } else if (left.type == VAL_NULL) {
            s_l = "null";
        }

        // Conversão implícita do operando direito para string
        if (right.type == VAL_STRING) {
            s_r = right.sval ? right.sval : "";
        } else if (right.type == VAL_INT) {
            snprintf(buf_r, sizeof(buf_r), "%d", right.ival);
            s_r = buf_r;
        } else if (right.type == VAL_BOOL) {
            s_r = right.ival ? "true" : "false";
        } else if (right.type == VAL_NULL) {
            s_r = "null";
        }

        // Alocação e cópia dos dados
        int len = strlen(s_l) + strlen(s_r) + 1;
        char *res_str = malloc(len);
        if (res_str) {
            strcpy(res_str, s_l);
            strcat(res_str, s_r);
        }
        result.type = VAL_STRING;
        result.sval = res_str;
        return result;
    }
    result.ival = left.ival + right.ival;
    return result;
```

### Conversão Implícita (Coerção)

- **Inteiros (`VAL_INT`):** Convertidos via `snprintf` usando a formatação `%d`.
- **Booleanos (`VAL_BOOL`):** Convertidos para `"true"` ou `"false"` com base em seu valor numérico (`0` ou `1`).
- **Null (`VAL_NULL`):** Convertido para o texto `"null"`.
