CC = gcc
FLEX = flex
BISON = bison

CFLAGS = -Wall -I./Parser

all: interpretador

interpretador: Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c Parser/parser_bib.c
	$(CC) $(CFLAGS) \
	Lexer/lex.yy.c \
	Parser/parser.tab.c \
	Parser/ast.c \
	Parser/parser_bib.c \
	-o interpretador -lfl -lm

Lexer/lex.yy.c: Lexer/scanner.l Parser/parser.tab.h
	$(FLEX) -o Lexer/lex.yy.c Lexer/scanner.l

Parser/parser.tab.c Parser/parser.tab.h: Parser/parser.y
	$(BISON) -d -o Parser/parser.tab.c Parser/parser.y

clean:
	rm -f interpretador
	rm -f Lexer/lex.yy.c
	rm -f Parser/parser.tab.c
	rm -f Parser/parser.tab.h