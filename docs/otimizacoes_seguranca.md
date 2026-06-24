# Otimizações de Segurança no Interpretador JavaScript

Este documento descreve as mitigações de segurança, estabilidade e prevenção de ataques de negação de serviço (Denial of Service - DoS) implementadas no núcleo do interpretador. 

As otimizações focam em garantir que a execução de scripts JavaScript seja estrita e segura, sem comprometer a máquina hospedeira, mesmo perante a execução de códigos maliciosos ou scripts com erros.

## 1. Bloqueio de Execução Indireta e Injeção de Código

**Arquivo:** `ast.c`
**Função afetada:** `AST_FUNC_CALL`

Chamadas diretas a funções de avaliação de strings e execução de comandos do sistema operacional podem servir como vetores de injeção de código severos se o interpretador for executado em um ambiente de sandbox.
Implementou-se um bloqueio para as seguintes palavras-chave em chamadas de função:
- `eval`
- `exec`
- `system`
- `Function`

Qualquer tentativa de utilizar essas funções acarretará a interrupção do bloco com o erro: `Erro de Seguranca: Execucao indireta ou injecao bloqueada`.

## 2. Prevenção contra Corrupção de Memória (Array Bounds Check)

**Arquivo:** `symboltable.c`
**Função afetada:** `sym_set_array_element`

A alocação de índices em arrays no JavaScript é altamente flexível, mas, na conversão para arrays C usando alocação dinâmica (`realloc`), o uso de índices negativos ou exorbitantes (como `arr[-10] = 5` ou `arr[9999999999] = 1`) poderia resultar em um _Segmentation Fault_ imediato ou em manipulação de áreas vitais de memória (Buffer Overflow/Underflow).
- **Ajuste implementado:** Índices negativos ou maiores que 1.000.000 são instantaneamente barrados.

## 3. Prevenção de Stack Overflow (Controle de Recursividade)

**Arquivo:** `ast.c`
**Função afetada:** `AST_FUNC_CALL`

Sem limite na pilha de execução de funções (Call Stack), uma função recursiva infinita em JavaScript causaria um estouro de pilha na linguagem C base (Stack Overflow), culminando na queda irrecuperável de todo o interpretador.
- **Ajuste implementado:** Implementação da variável `call_depth` para rastrear a profundidade da recursão. A profundidade máxima permitida é de **1.000 chamadas**. Atingir esse limite interrompe a árvore de execução recursiva com a mensagem: `Erro de Seguranca: Stack Overflow (Limite de recursao excedido)`.

## 4. Mitigação de Loops Infinitos (DoS por Thread Hanging)

**Arquivo:** `ast.c`
**Estruturas afetadas:** `AST_WHILE`, `AST_FOR`, `AST_DO_WHILE`

Scripts do tipo `while(true){}` travam _threads_ e esgotam o processamento, o que caracteriza um ataque clássico de DoS quando executados em servidores. 
- **Ajuste implementado:** Foi inserido o parâmetro global `MAX_LOOP_ITERATIONS`, limitando todas as estruturas de laço do interpretador a **1.000.000 de iterações** contínuas. Ao atingir o valor, o loop quebra o ciclo e devolve o controle via: `Erro de Seguranca: Limite de iteracoes excedido (Possivel Loop Infinito)`.

## 5. Prevenção de Desreferência de Ponteiro Nulo (Null Pointer Dereference) em OOM

**Arquivo:** `symboltable.c`
**Funções afetadas:** `create_symbol`, `scope_push`, `sym_set_array_element`, `sym_set_str`

Na eventualidade de a máquina executar sem memória RAM/Swap suficiente (Out of Memory - OOM), métodos como `calloc`, `realloc` e `strdup` retornam um ponteiro nulo (`NULL`). Tentar usar esse ponteiro na sequência (como popular um nome de variável ou ler um valor) ocasiona o travamento do sistema.
- **Ajuste implementado:** Validadores `if (!ponteiro)` foram estrategicamente colocados sempre logo após as alocações na Tabela de Símbolos (Symbol Table). O interpretador encerra a execução com uma notificação segura antes de tocar nesses blocos de memória ausentes.
