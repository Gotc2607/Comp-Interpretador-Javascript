# Documentação da Implementação de Colchetes

Esta documentação descreve como os colchetes (`[]`) foram implementados no interpretador JavaScript.

## Objetivo

Permitir:
- acesso a elementos de arrays com `x[i]`
- atribuição a elementos de arrays com `x[i] = valor`

## Arquivos Principais

- `Parser/parser.y`
- `Parser/ast.c`
- `Parser/ast.h`
- `Parser/symboltable.c`
- `Parser/symboltable.h`

## 1. Gramática no Parser

No arquivo `Parser/parser.y`, foram adicionadas duas regras principais:

- A expressão de acesso a array:

```yacc
| expressao '[' expressao ']' { $$ = ast_array_access($1, $3); }
```

- A expressão de atribuição a array:

```yacc
| expressao '[' expressao ']' '=' expressao {
      ASTNode *acesso = ast_array_access($1, $3);
      $$ = ast_array_assign(acesso, $6);
  }
```

Essas regras permitem que uma expressão de array seja usada tanto para leitura quanto para escrita.

## 2. Nó AST para Arrays

No arquivo `Parser/ast.h`, foram declarados dois novos construtores:

```c
ASTNode *ast_array_access(ASTNode *array, ASTNode *index);
ASTNode *ast_array_assign(ASTNode *array_access, ASTNode *expression);
```

Em `Parser/ast.c`, os nós foram implementados:

```c
ASTNode *ast_array_access(ASTNode *array, ASTNode *index) {
    ASTNode *node = ast_new(AST_ARRAY_ACCESS);
    node->left = array;
    node->right = index;
    return node;
}

ASTNode *ast_array_assign(ASTNode *array_access, ASTNode *expression) {
    ASTNode *node = ast_new(AST_ARRAY_ASSIGN);
    node->left = array_access;
    node->right = expression;
    return node;
}
```

## 3. Execução no AST

A função `ast_eval` em `Parser/ast.c` foi estendida para tratar os nós de array.

### Leitura de elemento de array

```c
case AST_ARRAY_ACCESS: {
    if (node->left->kind != AST_IDENTIFIER) {
        // Caso especial para string[index]
    }

    char *nome_array = node->left->text;
    right = ast_eval(node->right);

    if (right.type != VAL_INT) {
        printf("Erro: O índice do array precisa ser um número inteiro.\n");
        result.type = VAL_NULL;
        return result;
    }

    result.type = VAL_INT;
    result.ival = sym_get_array_element(nome_array, right.ival);
    return result;
}
```

### Escrita em elemento de array

```c
case AST_ARRAY_ASSIGN: {
    ASTNode *acesso = node->left;
    char *nome_array = acesso->left->text;

    RuntimeValue idx_val = ast_eval(acesso->right);
    RuntimeValue expr_val = ast_eval(node->right);

    if (idx_val.type == VAL_INT && expr_val.type == VAL_INT) {
        sym_set_array_element(nome_array, idx_val.ival, expr_val.ival);
    }

    return expr_val;
}
```

A atribuição retorna o próprio valor atribuído, conforme comportamento esperado em JavaScript.

## 4. Tabela de Símbolos para Arrays

No arquivo `Parser/symboltable.h` foram declaradas as funções de array:

```c
void sym_set_array_element(const char *name, int index, int value);
int sym_get_array_element(const char *name, int index);
```

`Parser/symboltable.c` implementa o armazenamento dinâmico:

- Cria o símbolo do array se não existir
- Redimensiona o array quando o índice é maior do que o tamanho atual
- Inicializa novos elementos com `0`

```c
void sym_set_array_element(const char *name, int index, int value) {
    Symbol *s = find_symbol(name);
    if (!s) {
        s = create_symbol(name);
        s->type = SYM_ARRAY;
        s->arr_vals = NULL;
        s->arr_size = 0;
    }

    if (index >= s->arr_size) {
        int novo_tamanho = index + 1;
        s->arr_vals = realloc(s->arr_vals, novo_tamanho * sizeof(int));
        for (int i = s->arr_size; i < novo_tamanho; i++) {
            s->arr_vals[i] = 0;
        }
        s->arr_size = novo_tamanho;
    }
    s->arr_vals[index] = value;
}

int sym_get_array_element(const char *name, int index) {
    Symbol *s = find_symbol(name);
    if (s && s->type == SYM_ARRAY && index >= 0 && index < s->arr_size) {
        return s->arr_vals[index];
    }
    return 0;
}
```

## 5. Reconhecimento de `[` e `]` no scanner

No `Lexer/scanner.l`, foram definidos os tokens dos colchetes:

```c
"["         { return '['; }
"]"         { return ']'; }
```

Esses tokens são enviados ao parser para montar a árvore sintática.

## 6. Exemplo de uso

```js
x[0] = 10;
x[1] = 20;

x[0]; // lê o valor 10
x[1]; // lê o valor 20

x[1] = 99;
x[1]; // 99
```

## 7. Comportamento de indices e valores

- Apenas índices inteiros são aceitos.
- Quando o índice é maior que o tamanho atual, o array cresce automaticamente.
- Valores não atribuídos são retornados como `0`.

## 8. Observações

- A implementação atual trata o array como um vetor de inteiros.
- O acesso via string `str[i]` também é suportado em `AST_ARRAY_ACCESS`, mas somente para leitura de caractere.
- A atribuição `x[i] = valor` cria o array automaticamente se `x` não existir.
