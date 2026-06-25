# Guia de Compatibilidade com a Especificação JavaScript

Este documento detalha o nível de compatibilidade do nosso interpretador com o padrão oficial do JavaScript (ECMAScript) e as decisões de design adotadas.

---

## 1. Declarações e Regras de Escopo (`var`, `let`, `const`)

| Recurso | Comportamento no ECMAScript | Comportamento no Nosso Interpretador |
| :--- | :--- | :--- |
| **Escopo de `let` e `const`** | Escopo de bloco `{ ... }`. | Totalmente suportado via pilha de escopo encadeada. |
| **Escopo de `var`** | Escopo de função ou global (não respeita bloco). | Simplificado: tratado sob o mesmo escopo de bloco atual do bloco onde foi declarado. |
| **Reatribuição de `const`** | Lança `TypeError` em runtime. | Totalmente suportado: gera `TypeError` ao tentar modificar constantes (inclusive atribuições compostas como `pi += 1`). |
| **Redeclaração no mesmo escopo** | Lança erro estático. | Totalmente suportado: impede redeclarações de `let` ou `const` no mesmo nível de escopo. |
| **Shadowing (Sombreamento)** | Escopos internos ocultam variáveis externas. | Totalmente suportado. |

---

## 2. Tipagem e Coerção Automática

O interpretador adota um modelo de **tipagem mais seguro e rígido** do que a especificação ECMAScript padrão em certas operações para prevenir bugs silenciosos.

### Operador de Adição (`+`)
- **JavaScript Padrão:** Coerge operandos não-string para string se houver uma string envolvida na soma.
- **Nosso Interpretador:** Compatível. Se um dos operandos for `VAL_STRING`, converte o outro operando (seja inteiro, booleano ou nulo) para string e efetua a concatenação dinâmica.

### Outros Operadores Matemáticos (`-`, `*`, `/`, `%`, `**`)
- **JavaScript Padrão:** Tenta converter strings contendo dígitos numéricos para números (ex: `"5" - 2` resulta em `3`) e retorna `NaN` se for impossível converter.
- **Nosso Interpretador:** Mais rígido. Qualquer operação aritmética com strings (literais ou variáveis) lança imediatamente um `TypeError` no `stderr` e interrompe o programa com código de retorno `1`.

---

## 3. Modo Estrito (`"use strict";`)

Quando a diretiva `"use strict";` é declarada no topo do programa, as seguintes regras são aplicadas:

- **Atribuições Implícitas (Globais):** No JS padrão, atribuir valor a uma variável não declarada (ex: `x = 10;`) cria uma variável global silenciosamente no modo não-estrito. No Strict Mode, isso é proibido e lança `ReferenceError`.
- **Nosso Interpretador:** Compatível. Com o Strict Mode ativo, qualquer atribuição a uma variável que não foi declarada anteriormente gera um `ReferenceError: x is not defined (Strict Mode)` e finaliza o processo.

---

## 4. O Comando `switch` e Blocos `case`

- **Igualdade Estrita:** Coerente com a especificação ECMAScript, a comparação dos valores do `switch` com as condições dos `cases` utiliza a semântica de igualdade estrita (`===`), ou seja, compara tanto o tipo quanto o valor dos dados sem coerção.
- **Fallthrough:** Totalmente suportado. Os blocos dos cases subsequentes continuam executando em cascata até encontrar um sinalizador `break;`.
- **Default:** O bloco `default` é acionado na falta de casamento direto e também suporta fallthrough para os blocos abaixo dele.

---

## 5. Otimizações e Segurança

- **Constant Folding (Simplificação de Constantes):** O interpretador calcula operações aritméticas simples entre literais numéricos na árvore de sintaxe (ex: `3 + 4 * 2`) no momento da compilação estática, uma otimização comum que reduz o consumo de memória e a latência de loops.
- **Restrição de Segurança:** O interpretador bloqueia estaticamente e dinamicamente chamadas a funções comumente abusadas para execução de código arbitrário e injeções (`eval`, `exec`, `system`, `Function`), lançando erro de segurança no console.
