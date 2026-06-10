# Análise Semântica

Esta documentação descreve a implementação do analisador semântico no interpretador.

## Objetivo

A análise semântica verifica o uso correto de identificadores e escopos antes da execução da AST, evitando que comandos inválidos cheguem ao avaliador.

## Qual é o benefício

- Detecta variáveis não declaradas antes da execução.
- Detecta atribuição em variáveis não declaradas.
- Impede redeclaração no mesmo escopo.
- Mantém o interpretador mais seguro e previsível.

## Arquivos envolvidos

- `Parser/ast.h`
- `Parser/ast.c`
- `Parser/parser.y`
- `Parser/symboltable.c`

## Como funciona

1. O parser gera uma AST a partir do código-fonte.
2. Antes de chamar `ast_eval`, o `main()` em `Parser/parser.y` chama `ast_check(raiz)`.
3. `ast_check()` percorre recursivamente a AST e valida cada nó.
4. Se houver erro semântico, a execução é interrompida e uma mensagem é exibida.

## Regras de verificação

- Identificadores usados em expressões devem ter sido declarados antes.
- Uma atribuição a uma variável exige que ela exista no escopo atual ou em um escopo pai.
- A declaração de variáveis usa um escopo semântico independente da tabela de símbolos de execução.
- Cada bloco `{ ... }` cria um novo escopo semântico.

## Exemplo

Código inválido:

```js
console.log(a);
```

Saída gerada antes da execução:

```
Erro semantico: variavel 'a' nao declarada
```

Código válido:

```js
let a = 10;
console.log(a);
```

Saída:

```
10
```

## Detalhes de implementação

No arquivo `Parser/ast.c`:

- `sem_scope_push()` e `sem_scope_pop()` gerenciam escopos semânticos.
- `sem_declare()` registra declarações no escopo atual.
- `sem_exists()` verifica se uma variável existe em algum escopo válido.
- `ast_check_node()` percorre a AST e valida nós como `AST_IDENTIFIER`, `AST_ASSIGN`, `AST_DECLARE`, `AST_BLOCK`, etc.

A implementação usa uma tabela semântica própria para não misturar declarações com a tabela de símbolos de execução.
