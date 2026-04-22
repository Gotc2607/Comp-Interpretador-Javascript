#ifndef AST_H
#define AST_H

typedef struct ASTNode ASTNode;

typedef enum {
    AST_SEQUENCE,
    AST_PRINT,
    AST_BLOCK,
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_ASSIGN,
    AST_BINARY,
    AST_UNARY
} ASTKind;

ASTNode *ast_sequence(ASTNode *left, ASTNode *right);
ASTNode *ast_print_stmt(ASTNode *expression);
ASTNode *ast_block(ASTNode *body);
ASTNode *ast_number(int value);
ASTNode *ast_identifier(char *name);
ASTNode *ast_assign(int op, char *name, ASTNode *expression);
ASTNode *ast_binary(int op, ASTNode *left, ASTNode *right);
ASTNode *ast_unary(int op, ASTNode *child);

int ast_eval(ASTNode *node);
void ast_free(ASTNode *node);
void ast_dump(const ASTNode *node, int indent);

#endif