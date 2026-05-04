# Correção de Erros de Tipo no Interpretador JavaScript

## Visão Geral

Este documento explica as correções de tipo realizadas no arquivo `Parser/ast.c` e os motivos que tornaram essas alterações necessárias para implementar a igualdade estrita (`===`).

---

## Problemas Identificados

### 1. Retorno de Tipo Incompatível

A função `ast_eval` foi declarada para retornar `RuntimeValue`, mas várias partes do código retornavam `int` diretamente:

```c
// ERRADO - retornando int onde RuntimeValue é esperado
return 0;
return node->value;
return get_var(node->text);
```

### 2. Variáveis Não Declaradas

Variáveis `left` e `right` eram usadas sem declaração prévia em alguns casos:

```c
// ERRADO - variáveis não declaradas
left = ast_eval(node->left);
right = ast_eval(node->right);
```

### 3. Operações Aritméticas em Structs

O código tentava fazer operações aritméticas diretamente em structs `RuntimeValue`:

```c
// ERRADO - não pode operar em structs diretamente
return left + right;
return left && right;
return left == right;
```

### 4. Campos Inexistentes

O código referenciava campos que não existiam na struct:

```c
// ERRADO - campos left_val e right_val não existem
return left_val.data.i_val == right_val.data.i_val;
```

---

## Correções Aplicadas

### 1. Declaração de Variáveis

Todas as variáveis `left` e `right` passaram a ser declaradas como `RuntimeValue`:

```c
RuntimeValue result = {VAL_INT, 0};
RuntimeValue left, right;
```

### 2. Uso do Campo `.ival`

As operações passaram a acessar o valor interno através do campo `ival`:

```c
// CORRIGIDO - usando .ival para operações
result.ival = left.ival + right.ival;
result.ival = left.ival && right.ival;
result.ival = left.ival == right.ival;
```

### 3. Retorno de RuntimeValue

Todos os caminhos de retorno agora retornam um `RuntimeValue` válido:

```c
// CORRIGIDO - retornando struct completa
result.ival = node->value;
return result;
```

---

## Implementação da Igualdade Estrita (`===`)

### O que é Igualdade Estrita?

A igualdade estrita (`===`) difere da igualdade comum (`==`) porque:
- **`==`**: Compara valores após conversão de tipo (ex: `"5" == 5` é verdadeiro)
- **`===`**: Compara valores **E** tipos (ex: `"5" === 5` é falso)

### Implementação no Código

```c
case OP_IgualdadeEstrita:
    if (left.type != right.type) {
        result.ival = 0;  // Tipos diferentes → Falso
        return result;
    }
    result.ival = left.ival == right.ival;  // Compara valores
    return result;
```

### Por que foi necessário corrigir os tipos?

Antes das correções, o código tentava:
1. Comparar structs diretamente → erro de tipo
2. Acessar campos inexistentes → `left_val`, `right_val`
3. Retornar `int` → incompatível com retorno `RuntimeValue`

Após as correções, podemos:
1. Acessar `.type` para comparar tipos
2. Acessar `.ival` para comparar valores
3. Retornar `RuntimeValue` corretamente

---

## Estrutura RuntimeValue

```c
typedef struct {
    ValueType type;  // VAL_INT, VAL_BOOL, VAL_STRING, VAL_NULL
    int ival;        // Valor inteiro (usado para bool também)
} RuntimeValue;
```

### Campos:
- **type**: Indica o tipo do valor armazenado
- **ival**: Armazena o valor inteiro (ou booleano)

---

## Resumo das Alterações

| Problema | Solução |
|----------|---------|
| Retorno de `int` | Usar variável `result` e retornar struct |
| Variáveis não declaradas | Declarar `RuntimeValue left, right;` |
| Operações em structs | Usar `.ival` para valores |
| Campos inexistentes | Remover e usar `.ival` e `.type` |

---

## Conclusão

As correções de tipo foram essenciais para que a implementação da igualdade estrita (`===`) funcionasse corretamente. A linguagem C exige tipos compatíveis, e a struct `RuntimeValue` precisa ter seus campos acessados explicitamente para realizar operações aritméticas e lógicas.