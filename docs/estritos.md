# Igualdade Estrita e Desigualdade Estrita

Este documento descreve como os operadores de igualdade estrita `===` e desigualdade estrita `!==` funcionam no interpretador.

## Objetivo

- `===` deve comparar valor e tipo.
- `!==` deve ser o inverso de `===`.

Esses operadores são usados para evitar comparações com conversão implícita de tipos.

## Suporte no interpretador

### 1. Lexer
No `Lexer/scanner.l`, os operadores são reconhecidos como tokens distintos:

```lex
"==="       { return OP_IgualdadeEstrita; }
"!=="       { return OP_DiferenteEstrita; }
```

A ordem importa: padrões mais longos devem vir antes de padrões mais curtos como `==` e `!=`.

### 2. Parser
No `Parser/parser.y`, os tokens são declarados e usados na gramática:

```bison
%token OP_IgualdadeEstrita
%token OP_DiferenteEstrita
```

E a expressão de comparação é aceita assim:

```bison
| expressao OP_IgualdadeEstrita expressao { $$ = ast_binary(OP_IgualdadeEstrita, $1, $3); }
| expressao OP_DiferenteEstrita expressao { $$ = ast_binary(OP_DiferenteEstrita, $1, $3); }
```

### 3. Avaliação na AST
Em `Parser/ast.c`, os operadores são tratados em `ast_eval` dentro do caso `AST_BINARY`.

#### Igualdade estrita `===`
A função compara o tipo primeiro e, quando o tipo é string, compara o conteúdo:

```c
case OP_IgualdadeEstrita:
    result.ival = verificar_igualdade_estrita(left, right);
    return result;
```

#### Desigualdade estrita `!==`
A desigualdade estrita é implementada como o inverso lógico de `===`:

```c
case OP_DiferenteEstrita:
    result.ival = !verificar_igualdade_estrita(left, right);
    return result;
```

### 4. Função auxiliar
A função `verificar_igualdade_estrita` garante o comportamento correto:

```c
static int verificar_igualdade_estrita(RuntimeValue left, RuntimeValue right) {
    if (left.type != right.type) return 0;
    if (left.type == VAL_STRING) {
        return strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") == 0;
    }
    return left.ival == right.ival;
}
```

Se os tipos forem diferentes, a igualdade estrita retorna `0`.
Se ambos forem strings, a comparação usa `strcmp`.
Para valores numéricos, `ival` é comparado diretamente.

## Exemplos

### Igualdade estrita

```js
1 === 1;      // verdadeiro (1)
"x" === "x"; // verdadeiro (1)
1 === "1";    // falso (0)
```

### Desigualdade estrita

```js
1 !== 2;      // verdadeiro (1)
"x" !== "y"; // verdadeiro (1)
1 !== "1";    // verdadeiro (1)
```

## Por que isso importa

Sem `===`/`!==`, o interpretador poderia comparar valores apenas por valor numérico/textual, aceitando conversões implícitas.

Com `===` e `!==`, a verificação respeita o tipo e evita resultados inesperados em comparações entre números e strings.

## Observações

- Este interpretador atualmente suporta apenas `int` e `string` como tipos de valor.
- Se quiser estender para outros tipos, a função `verificar_igualdade_estrita` deve ser atualizada.
- A sintaxe de arrays e objetos não interfere diretamente nesses operadores, pois eles só operam em valores já avaliados.
