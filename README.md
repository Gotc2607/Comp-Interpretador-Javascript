# Comp-Interpretador-Javascript

Interpretador de JavaScript (versao de estudo) para a materia de Compiladores, usando Flex (lexer) e Bison (parser).

## O que funciona atualmente

- Numeros inteiros
- Operadores: `+`, `*`, `==`
- Precedencia: `*` > `+` > `==`
- Leitura de multiplas expressoes finalizadas com `;`

## Comandos para build e execucao

Execute na raiz do projeto:

```bash
bison -d Parser/parser.y -o Parser/parser.tab.c
flex -o Lexer/lex.yy.c Lexer/scanner.l
gcc Lexer/lex.yy.c Parser/parser.tab.c -o interpretador -I ./Parser -lfl
./interpretador
```

## Teste rapido (entrada manual)

Depois de rodar `./interpretador`, digite expressoes como:

```txt
2 + 3 * 4;
2 + 3 == 5;
2 * 3 == 6;
```

Saidas esperadas:

- `Resultado: 14`
- `Resultado: 1`
- `Resultado: 1`

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
- `IDENT` ja e reconhecido no lexer, mas semantica de variaveis ainda nao foi implementada no parser.
- Em caso de erro de sintaxe, o parser exibira mensagem de erro.