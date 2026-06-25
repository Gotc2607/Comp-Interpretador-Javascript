# Comp-Interpretador-Javascript

Interpretador de JavaScript (versão de estudo) desenvolvido para a disciplina de Compiladores utilizando **Flex** (analisador léxico), **Bison** (analisador sintático) e **C** (árvore sintática e execução).

---

## 🏗️ Arquitetura do Projeto

O fluxo de processamento do compilador é dividido nas seguintes fases:
1. **Léxico ([scanner.l](Lexer/scanner.l)):** Converte a entrada de caracteres em uma corrente de tokens.
2. **Sintático ([parser.y](Parser/parser.y)):** Reconhece as regras gramaticais e monta a Árvore de Sintaxe Abstrata (AST).
3. **Semântico e Análise ([Parser/ast.c](Parser/ast.c)):** Valida regras estáticas (declarações, restrições e otimização de constantes).
4. **Execução/Avaliação ([Parser/ast.c](Parser/ast.c)):** Avalia recursivamente os nós da AST usando a tabela de símbolos encadeada ([Parser/symboltable.c](Parser/symboltable.c)).

Se quiser ver os detalhes da estrutura da AST, consulte [docs/AST.md](docs/AST.md).

---

## 🛠️ Recursos Suportados

- **Tipos de Dados:** Inteiros (`VAL_INT`), Booleanos (`VAL_BOOL`), Nulo (`VAL_NULL`), Strings (`VAL_STRING`) e Arrays (`SYM_ARRAY`).
- **Operadores Aritméticos:** Adição (`+`), Subtração (`-`), Multiplicação (`*`), Divisão (`/`), Módulo (`%`) e Potência (`**`).
- **Operadores Lógicos e Comparação:** `&&`, `||`, `!`, `==`, `!=`, `===`, `!==`, `>`, `<`, `>=`, `<=`.
- **Variáveis e Constantes:** Declaração com `let` e `const` com escopo de bloco, `var`, atribuição composto (`+=`, `-=`, etc.) e atribuição de coalescência nula (`??=`).
- **Controle de Fluxo:** Condicionais (`if/else`), Laços (`while`, `do-while`, `for`) e desvios de loop (`break`, `continue`).
- **Estruturas switch/case:** Casamento estrito de valores, bloco `default` e cascateamento sequencial (`fallthrough`).
- **Funções:** Declaração de funções (`function`), passagem de parâmetros por valor, retorno (`return`) e suporte a recursão.
- **Saídas de Dados:** Função global `console.log()` para impressão no console.
- **Strict Mode:** Diretiva `"use strict";` no topo do arquivo que ativa validações de referência rígidas.

---

## 📋 Dependências e Instalação

Para compilar e executar o projeto, você precisará das seguintes ferramentas:
- **GCC** (Compilador C)
- **Flex** (Versão >= 2.5)
- **Bison** (Versão >= 3.0)
- **GNU Make** (Ferramenta de automação)
- **Python 3** (Opcional, apenas para rodar a suíte de testes)

No Ubuntu, Debian ou distribuições WSL equivalentes, você pode instalar todas as dependências com o seguinte comando:
```bash
sudo apt update
sudo apt install build-essential flex bison python3
```

---

## ⚙️ Como Compilar e Rodar

### Compilação Automática (Recomendado)
Para compilar o projeto utilizando o `Makefile` na raiz:
```bash
make
```
Para limpar todos os binários gerados e arquivos gerados por Bison/Flex:
```bash
make clean
```

### Compilação Manual (Alternativa)
Se preferir compilar manualmente sem o `make`:
```bash
flex -o Lexer/lex.yy.c Lexer/scanner.l
bison -d -o Parser/parser.tab.c Parser/parser.y
gcc -Wall -I./Parser Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c Parser/symboltable.c -o interpretador -lfl -lm
```

---

## 🚀 Execução e Testes

### Execução Interativa (Manual)
Rode o executável sem argumentos:
```bash
./interpretador
```
Digite comandos JavaScript terminados em ponto e vírgula `;`. 

> [!IMPORTANT]
> A avaliação da AST ocorre apenas quando a entrada de texto encontra o final do arquivo (**EOF**).
> No console, após digitar suas instruções, pressione **Ctrl+D** para finalizar a entrada e disparar o interpretador.

Exemplo de entrada manual:
```js
let a = 10;
let b = 20;
console.log(a + b);
```
*(Pressione Ctrl+D para ver o resultado no console)*

### Execução via Pipe
Você pode passar um arquivo ou instrução via stream para o executável:
```bash
printf 'let x = 15; console.log(x * 2);\n' | ./interpretador
```

### Rodar Suíte de Testes
O projeto conta com mais de 40 testes de unidade e integração contidos na pasta [testes/](testes). Para rodar a suíte inteira automaticamente:
```bash
python3 executar_testes.py
```

---

## 💡 Exemplos de Código

### 1. Funções e Recursão
```js
function fatorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * fatorial(n - 1);
}
console.log(fatorial(5)); // Imprime 120
```

### 2. Switch Case com Fallthrough
```js
let x = 2;
switch (x) {
    case 1:
        console.log("Um");
    case 2:
        console.log("Dois"); // Casou e faz fallthrough
    case 3:
        console.log("Três"); // Executado pelo fallthrough
        break;
    default:
        console.log("Outro");
}
// Saída: "Dois", "Três"
```

### 3. Strict Mode e TypeError
```js
"use strict";
let x = 10;
y = 20; // ReferenceError: y is not defined (Strict Mode)
```

---

## 🗺️ Guia de Compatibilidade com JavaScript

Embora o interpretador siga a sintaxe geral do JavaScript, há particularidades semânticas importantes a serem notadas:
- **Tipagem Aritmética Rígida:** Diferente do JavaScript oficial que tenta converter strings para números em operações como `-` ou `*`, nosso interpretador rejeita operações incompatíveis lançando um `TypeError` imediato (ex: `"texto" * 5` falha).
- **Escopo simplificado de `var`:** A variável declarada com `var` é inserida sob o escopo do bloco atual, diferentemente do escopo funcional tradicional do ECMAScript.
- **Validações estritas de `const`:** Atribuições diretas e compostas (ex: `+=`) em constantes são estritamente rejeitadas lançando erro de reatribuição.

Para um guia completo de compatibilidade detalhado, consulte [docs/compatibilidade.md](docs/compatibilidade.md).