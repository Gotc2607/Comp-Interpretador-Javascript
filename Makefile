CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -I ./Parser

all: interpretador

interpretador: Lexer/lex.yy.c Parser/parser.tab.c
	$(CC) Lexer/lex.yy.c Parser/parser.tab.c -o interpretador -lfl -lm

Lexer/lex.yy.c: Lexer/scanner.l
	$(FLEX) -o Lexer/lex.yy.c Lexer/scanner.l

Parser/parser.tab.c: Parser/parser.y
	$(BISON) -d -o Parser/parser.tab.c Parser/parser.y

clean:
	rm -f interpretador Lexer/lex.yy.c Parser/parser.tab.c Parser/parser.tab.h