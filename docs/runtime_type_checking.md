# Checagem de Tipos em Tempo de Execução (Runtime Type Checking)

Este documento descreve como a checagem de tipos em tempo de execução foi implementada no interpretador JavaScript para evitar falhas silenciosas e assegurar que operações matemáticas inválidas lancem erros controlados.

## Objetivo

Impedir que tipos de dados incompatíveis realizem operações aritméticas que não fazem sentido matemático. No JavaScript, tentar operar strings e números em operações como subtração, multiplicação, divisão, módulo ou potência gera comportamento indefinido ou conversões silenciosas não desejadas no nosso interpretador.

No modo implementado, tentar operar esses tipos lançará um `TypeError` no `stderr` e encerrará a execução do processo com status de erro.

## Operações Controladas

O interpretador valida rigorosamente os tipos dos operandos nas seguintes operações binárias:

1. **Subtração (`-`)**
2. **Multiplicação (`*`)**
3. **Divisão (`/`)**
4. **Módulo (`%`)**
5. **Potência (`**`)**

Se o operando esquerdo ou o operando direito for do tipo `VAL_STRING` (String), um erro é disparado.

## Exemplo de Erro

```js
let x = "Texto";
let y = 5;
console.log(x - y); // Lança TypeError: Cannot subtract incompatible types
```

## Arquivos Envolvidos

- `Parser/ast.c` (Avaliação e checagem de tipos no nó `AST_BINARY`)

## Detalhes de Implementação

A validação foi colocada no início da avaliação de cada operador aritmético dentro do `case AST_BINARY` de `ast_eval`:

```c
case '-':
    if (left.type == VAL_STRING || right.type == VAL_STRING) {
        fprintf(stderr, "TypeError: Cannot subtract incompatible types\n");
        exit(EXIT_FAILURE);
    }
    result.ival = left.ival - right.ival;
    return result;

case '*':
    if (left.type == VAL_STRING || right.type == VAL_STRING) {
        fprintf(stderr, "TypeError: Cannot multiply incompatible types\n");
        exit(EXIT_FAILURE);
    }
    result.ival = left.ival * right.ival;
    return result;
```

A mesma lógica se aplica para `%` (módulo), `/` (divisão) e `OP_Potencia` (potência).

### Adição (`+`)

A operação de adição é a única exceção, pois aceita concatenação quando pelo menos um dos operandos é uma string (conforme detalhado em [docs/concatenacao_strings.md](file:///wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/docs/concatenacao_strings.md)).
