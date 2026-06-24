# Strict Mode ("use strict";) no Interpretador JavaScript

Este documento descreve como o **Strict Mode** (Modo Estrito) foi implementado e como ele se comporta no interpretador.

## O que é o Strict Mode?

O Strict Mode é uma funcionalidade que permite colocar o interpretador em um modo de execução mais rígido e seguro.
Atualmente, o principal impacto do Strict Mode é **impedir a criação automática de variáveis globais** (atribuições a variáveis sem declaração prévia via `let`, `const` ou `var`).

## Como ativar

Para ativar o Strict Mode, adicione a diretiva `"use strict";` no topo do seu programa:

```js
"use strict";

let x = 10; // Correto
y = 20;     // Lança ReferenceError: y is not defined (Strict Mode)
```

## Arquivos Envolvidos

- `Lexer/scanner.l` (Reconhecimento do token da diretiva)
- `Parser/parser.y` (Tratamento sintático da diretiva opcional)
- `Parser/ast.h` e `Parser/ast.c` (Estado do Strict Mode e verificação de referência na atribuição)

## Detalhes de Implementação

### 1. Lexer (`Lexer/scanner.l`)

O lexer reconhece a diretiva `"use strict"` e retorna o token `KW_USE_STRICT`:

```lex
"\"use strict\"" { return KW_USE_STRICT; }
```

### 2. Parser (`Parser/parser.y`)

Declaramos o token e ajustamos a regra principal de entrada (`programa`) para aceitar a diretiva de modo estrito opcionalmente no início do arquivo:

```bison
%token KW_USE_STRICT

programa:
      KW_USE_STRICT ';' elementos { ativar_strict_mode(); raiz = $3; $$ = $3; }
    | elementos { raiz = $1; $$ = $1; }
    ;
```

### 3. Controle de Estado e Validação (`Parser/ast.c`)

1. **Flag global de estado:**
   ```c
   static int strict_mode_ativo = 0;

   void ativar_strict_mode(void) {
       strict_mode_ativo = 1;
   }
   ```

2. **Bypass da Análise Semântica:**
   Na análise semântica (`ast_check_node`), quando encontramos um nó de atribuição (`AST_ASSIGN`), declaramos a variável implicitamente no escopo de checagem. Isso permite que a compilação/validação inicial passe, delegando a verificação de segurança para o tempo de execução (runtime), onde o erro correto de referência deve ser disparado.

3. **Verificação de Referência na Atribuição:**
   Na função `eval_assign`, se o modo estrito estiver ativo e a variável que está recebendo a atribuição não existir na tabela de símbolos runtime, o programa é interrompido imediatamente com um erro e código de retorno de falha:

   ```c
   if (strict_mode_ativo && !sym_exists(node->text)) {
       fprintf(stderr, "ReferenceError: %s is not defined (Strict Mode)\n", node->text);
       exit(EXIT_FAILURE);
   }
   ```
