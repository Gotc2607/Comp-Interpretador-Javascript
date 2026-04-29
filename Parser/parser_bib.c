#include "parser_bib.h"

ASTNode *raiz = NULL;

void yyerror(const char *s)
{
    fprintf(stderr, "Erro sintático: %s\n", s);
}