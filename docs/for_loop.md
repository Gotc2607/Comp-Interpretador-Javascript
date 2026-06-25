# Laços `for` no Interpretador JavaScript

Este documento detalha o suporte a estruturas de repetição `for` implementado no interpretador JavaScript.

---

## 1. Sintaxe de Declaração

O laço `for` do interpretador aceita três seções opcionais (inicialização, condição de parada e expressão de incremento) separadas por ponto e vírgula, seguidas pelo corpo do laço:

```js
let i = 0;
for (i = 0; i < 3; i = i + 1) {
    console.log(i);
}
```

- **Inicialização:** Avaliada apenas uma vez antes do início da primeira iteração (ex: `i = 0`).
- **Condição:** Expressão booleana avaliada no início de cada iteração. Se retornar um valor falso (`0`), a execução do laço é encerrada. Se omitida, é considerada verdadeira.
- **Incremento/Atualização:** Executada no final de cada iteração, logo após o corpo do laço ser executado.

---

## 2. Controle de Fluxo (`break` e `continue`)

O laço `for` é totalmente compatível com comandos de controle de fluxo de laços:

### Comando `break`
Interrompe imediatamente o loop e transfere o controle do programa para a instrução seguinte ao bloco `for`.
```js
let i = 0;
for (i = 0; i < 10; i = i + 1) {
    if (i == 3) {
        break;
    }
    console.log(i);
}
// Saída: 0, 1, 2
```

### Comando `continue`
Interrompe a iteração atual, executa a **expressão de incremento** (se presente) e reavalia a condição para a próxima iteração.
```js
let i = 0;
for (i = 0; i < 5; i = i + 1) {
    if (i == 2) {
        continue;
    }
    console.log(i);
}
// Saída: 0, 1, 3, 4
```

---

## 3. Detalhes de Implementação (AST)

No arquivo [Parser/ast.c](../Parser/ast.c), o nó `AST_FOR` é estruturado da seguinte forma:
- `node->left`: Nó de inicialização.
- `node->mid`: Nó de condição.
- `node->extra`: Nó de incremento/atualização.
- `node->right`: Nó do corpo do laço.

A execução ocorre recursivamente em `ast_eval` de forma a respeitar a precedência e o fluxo tradicional do loop `for`, propagando e limpando os sinais de controle `CTRL_BREAK` e `CTRL_CONTINUE`.
