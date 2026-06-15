# Declaração de Variáveis (`let`, `const`, `var`)

Este documento descreve como a declaração formal de variáveis com controle de mutabilidade foi implementada no interpretador.

## Objetivo

O objetivo é permitir que variáveis sejam criadas utilizando as palavras-chave padrão do JavaScript (`let`, `const`, `var`) e garantir que variáveis declaradas como `const` não possam ser reatribuídas após receberem seu valor inicial.

Exemplo suportado:

```js
let x = 10;
const pi = 3;
```

## Arquivos envolvidos

- `Lexer/scanner.l`
- `Parser/parser.y`
- `Parser/ast.h`
- `Parser/ast.c`
- `Parser/symboltable.h`
- `Parser/symboltable.c`

## 1. Lexer

No arquivo `Lexer/scanner.l`, as palavras-chave foram adicionadas para retornar seus respectivos tokens:

```c
"let"       { return LET; }
"const"     { return CONST; }
"var"       { return VAR; }
```

## 2. Parser

No arquivo `Parser/parser.y`, os tokens foram declarados e integrados à regra gramatical de `Linha`:

```yacc
%token LET CONST VAR

| LET IDENT '=' expressao ';'   { $$ = ast_declare(0, $2, $4); }
| CONST IDENT '=' expressao ';' { $$ = ast_declare(1, $2, $4); }
| VAR IDENT '=' expressao ';'   { $$ = ast_declare(0, $2, $4); }
```

## 3. AST

No arquivo `Parser/ast.h`, foi criado o nó na enumeração `ASTKind`:

```c
AST_DECLARE
```

O construtor guarda o sinalizador de const no campo `op`:

```c
ASTNode *ast_declare(int is_const, char *name, ASTNode *expression) {
    ASTNode *node = ast_new(AST_DECLARE);
    node->op = is_const; 
    node->text = name;
    node->left = expression;
    return node;
}
```

## 4. Avaliação e Memória

A lógica fica dividida entre a execução na AST (`ast.c`) e o gerenciamento na Tabela de Símbolos (`symboltable.c`).

- O nó `AST_DECLARE` registra a variável na memória chamando `sym_declare`.
- Se a variável for `const`, ela é registrada com um estado temporário (`2`), aguardando o primeiro valor.
- Ao receber a primeira atribuição via `sym_set_int` ou `sym_set_str`, a porta se tranca (o estado vira `1`).
- Se houver nova atribuição, emite `TypeError`.

Código relevante (`symboltable.c`):

```c
void sym_set_int(const char *name, int value) {
    Symbol *s = find_symbol(name);
    if (s) {
        if (s->is_const == 1) {
            printf("TypeError: Atribuicao a variavel constante '%s'.\n", name);
            return;
        }
        if (s->is_const == 2) {
            s->is_const = 1;
        }
        s->type = SYM_INT;
        s->ival = value;
        return;
    }
    // ... criação padrão ...
}
```