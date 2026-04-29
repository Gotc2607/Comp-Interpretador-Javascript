#ifndef PARSER_SUPPORT_H
#define PARSER_SUPPORT_H

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);

void yyerror(const char *s);

extern ASTNode *raiz;

#endif