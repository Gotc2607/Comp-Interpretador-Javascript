# Funções no Interpretador JavaScript

Este documento detalha o suporte a funções implementado no interpretador JavaScript.

---

## 1. Sintaxe de Declaração e Chamada

As funções são declaradas utilizando a palavra-chave `function` e podem receber zero ou mais parâmetros delimitados por vírgulas. O corpo da função deve ser delimitado por chaves `{ ... }`.

### Declaração:
```js
function soma(a, b) {
    return a + b;
}
```

### Chamada:
```js
let resultado = soma(5, 10);
console.log(resultado); // Imprime 15
```

---

## 2. Passagem de Argumentos e Parâmetros

1. **Passagem por Valor:** Todos os argumentos em chamadas de função são avaliados no escopo do chamador e passados por cópia de valor aos parâmetros formais.
2. **Argumentos Faltantes:** Caso uma função seja chamada com menos argumentos do que os parâmetros declarados, os parâmetros excedentes são inicializados automaticamente com o valor padrão `0` (do tipo `VAL_INT`).
3. **Argumentos Excedentes:** Argumentos passados além dos parâmetros declarados são avaliados mas ignorados (não vinculados a nenhuma variável local).

---

## 3. Retorno de Valores (`return`)

O comando `return` é opcional. Ele interrompe a execução imediata da função e propaga o valor da expressão avaliada de volta para o chamador.

- **Comportamento:** Ao avaliar o nó `AST_RETURN`, o interpretador gera um valor com a flag de fluxo `control_flow` definida como `CTRL_RETURN`.
- **Desvio de Fluxo:** Laços de repetição (`while`, `do-while`, `for`) e blocos sequenciais propagam o sinal de `CTRL_RETURN` para interromper a execução até que o controle retorne ao chamador da função (em `AST_FUNC_CALL`), onde o sinalizador de fluxo é limpo (`CTRL_NONE`).
- **Sem Expressão:** Se o `return;` for omitido ou chamado sem valor, o interpretador retorna `0` (`VAL_INT`).

---

## 4. Regras de Escopo

- **Escopo Local de Função:** Cada chamada de função invoca o isolamento de escopo empilhando um novo nível de variáveis (`scope_push()`).
- **Parâmetros Locais:** Os parâmetros da função são declarados como variáveis locais nesse novo escopo e recebem os valores dos argumentos.
- **Liberação de Memória:** Ao fim da execução da função, o escopo local é removido (`scope_pop()`), liberando todas as variáveis e strings locais declaradas.
- **Lookup Dinâmico:** Se uma variável não for encontrada no escopo da função, o interpretador realiza a busca nos escopos pais (escopo dinâmico/global).

---

## 5. Exemplo de Recursão

Devido ao empilhamento e desempilhamento correto de escopos de variáveis para cada chamada de função, o interpretador suporta recursão:

```js
function fatorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * fatorial(n - 1);
}

console.log(fatorial(5)); // Imprime 120
```
