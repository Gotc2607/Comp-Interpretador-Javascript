# Comp-Interpretador-Javascript
Interpretador de javascript para a materia de compiladores


comandos para teste:

bison -d Parser/parser.y -o Parser/parser.tab.c
flex -o Lexer/lex.yy.c Lexer/scanner.l
gcc Lexer/lex.yy.c Parser/parser.tab.c -o interpretador -I ./Parser -lfl
./interpretador