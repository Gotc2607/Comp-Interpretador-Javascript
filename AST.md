# Arquitetura AST

Este projeto passou a usar uma AST basica entre o parser e a avaliacao.

## Fluxo

1. O Flex reconhece os tokens e preenche os valores de `NUMBER` e `IDENT`.
2. O Bison monta a arvore sintatica abstrata em vez de calcular tudo diretamente nas regras.
3. O modulo `Parser/ast.c` percorre a arvore, aplica a tabela de simbolos e imprime os resultados das linhas.
4. Ao final da parse, a arvore inteira e liberada com `ast_free`.

## Tipos de no

- `AST_SEQUENCE`: encadeia elementos na ordem em que aparecem.
- `AST_PRINT`: representa uma linha terminada por `;` e imprime `Resultado: ...`.
- `AST_BLOCK`: representa um bloco entre `{` e `}`.
- `AST_NUMBER`: valor literal inteiro.
- `AST_IDENTIFIER`: leitura de variavel.
- `AST_ASSIGN`: atribuicoes compostas como `+=`, `-=`, `**=`, `*=`, `/=` e `%=`.
- `AST_BINARY`: operacoes binarias como `+`, `*`, `==`, `&&`, `||`, `<`, `>`, `/`.
- `AST_UNARY`: operador unario `!`.

## Arquivos

- [Parser/parser.y](Parser/parser.y): gramática e construcao dos nos.
- [Parser/ast.h](Parser/ast.h): declaracoes publicas da AST.
- [Parser/ast.c](Parser/ast.c): criacao, avaliacao, liberacao e tabela de simbolos.
- [Lexer/scanner.l](Lexer/scanner.l): analise lexica e preenchimento de valores.

## Build

```bash
bison -d Parser/parser.y -o Parser/parser.tab.c
flex -o Lexer/lex.yy.c Lexer/scanner.l
gcc Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c -o interpretador -I ./Parser -lfl -lm
```

## Observacao de manutencao

Se um novo operador for adicionado, o caminho correto e este:

1. reconhecer o simbolo no lexer;
2. declarar o token no parser;
3. construir o no correspondente;
4. ensinar `ast_eval` a interpretar o novo no.