CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -I./Parser -I./AST

all: interpretador

interpretador: Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c
	$(CC) $(CFLAGS) Lexer/lex.yy.c Parser/parser.tab.c Parser/ast.c -o interpretador -lfl -lm

Lexer/lex.yy.c: Lexer/scanner.l
	$(FLEX) -o Lexer/lex.yy.c Lexer/scanner.l

Parser/parser.tab.c: Parser/parser.y
	$(BISON) -d -o Parser/parser.tab.c Parser/parser.y

clean:
	rm -f interpretador Lexer/lex.yy.c Parser/parser.tab.c Parser/parser.tab.h