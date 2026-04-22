# Comp-Interpretador-Javascript

Interpretador de JavaScript (versao de estudo) para a materia de Compiladores, usando Flex (lexer) e Bison (parser).

## Arquitetura atual

O fluxo foi reorganizado para usar uma AST basica entre o parser e a avaliacao. O Bison monta a arvore em vez de calcular tudo diretamente, e `Parser/ast.c` faz a interpretacao final, incluindo a tabela de simbolos.

Se quiser ver o desenho da arquitetura, consulte [AST.md](AST.md).

## O que funciona atualmente

- Numeros inteiros
- Operadores: `+`, `*`, `==`
- Precedencia: `*` > `+` > `==`
- Leitura de multiplas expressoes finalizadas com `;`
- AST basica com avaliacao posterior

## Comandos para build e execucao

Execute na raiz do projeto:

```bash
bison -d Parser/parser.y -o Parser/parser.tab.c
flex -o Lexer/lex.yy.c Lexer/scanner.l
gcc Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c -o interpretador -I ./Parser -lfl -lm
./interpretador
```

## Teste rapido (entrada manual)

Depois de rodar `./interpretador`, digite expressoes como:

Importante: os resultados sao exibidos apenas quando a entrada termina em EOF.
No Linux, pressione `Ctrl+D` para finalizar a entrada e disparar a avaliacao da AST.

```txt
2 + 3 * 4;
2 + 3 == 5;
2 * 3 == 6;
2 + 2 * 5 + 4 == 16;
x += 5;
x += 2 * 3;
x + 1 == 12;
1 == 1 && 2 == 2;
1 == 0 && 2 == 2;
```

Saidas esperadas:

- `Resultado: 14`
- `Resultado: 1`
- `Resultado: 1`
- `Resultado: 1`
- `Resultado: 5`
- `Resultado: 11`
- `Resultado: 1`
- `Resultado: 1`
- `Resultado: 0`

## Teste automatico por pipe

```bash
printf '2 + 3 * 4;\n2 + 3 == 5;\n2 * 3 == 6;\n' | ./interpretador
```

## Mais expressoes para validar precedencia

```txt
1 + 2 * 3 == 7;   // esperado: 1
1 + 2 * 3 == 9;   // esperado: 0
4 * 2 + 1 == 9;   // esperado: 1
4 * (2 + 1);      // ainda nao suportado (parenteses nao implementados)
```

## Observacoes

- Cada comando deve terminar com `;`.
- A avaliacao acontece no fim da entrada (EOF), nao a cada linha digitada.
- `IDENT` agora participa da AST e da tabela de simbolos durante a avaliacao.
- Em caso de erro de sintaxe, o parser exibira mensagem de erro.

## Infraestrutura e Testes
- **Build:** Implementado arquivo `Makefile`. Utilize o comando `make` para compilar e `make clean` para limpar binarios gerados.
- **Testes Automatizados:** Adicionada suite de testes em Python. Para executar, rode `python3 executar_testes.py`. O script processa os arquivos em `testes/*.js` e compara a saida com os arquivos `*.out` correspondentes.