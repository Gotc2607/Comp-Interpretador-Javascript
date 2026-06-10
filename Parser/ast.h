#ifndef AST_H
#define AST_H

typedef struct ASTNode ASTNode;

typedef enum {
    AST_SEQUENCE,
    AST_CONSOLE_LOG,
    AST_BLOCK,
    AST_NUMBER,
    AST_STRING,
    AST_IDENTIFIER,
    AST_ASSIGN,
    AST_BINARY,
    AST_UNARY,
    AST_WHILE,
    AST_DO_WHILE,
    AST_IF,
    AST_ARRAY_ACCESS,
    AST_ARRAY_ASSIGN,
    AST_SWITCH,
    AST_CASE_BLOCK,
    AST_FOR,
    AST_DECLARE,
} ASTKind;

typedef enum {
    VAL_INT,
    VAL_BOOL,
    VAL_STRING,
    VAL_NULL,
} ValueType;

typedef struct {
    ValueType type;
    int ival;
    char *sval;
} RuntimeValue;

ASTNode *ast_sequence(ASTNode *left, ASTNode *right);
ASTNode *ast_console_log(ASTNode *expression);
ASTNode *ast_block(ASTNode *body);
ASTNode *ast_number(int value);
ASTNode *ast_string(char *value);
ASTNode *ast_identifier(char *name);
ASTNode *ast_assign(int op, char *name, ASTNode *expression);
ASTNode *ast_binary(int op, ASTNode *left, ASTNode *right);
ASTNode *ast_unary(int op, ASTNode *child);
ASTNode *ast_while(ASTNode *cond, ASTNode *body);
ASTNode *ast_do_while(ASTNode *cond, ASTNode *body);
ASTNode *ast_if(ASTNode *cond, ASTNode *then_body, ASTNode *else_body);
ASTNode *ast_array_access(ASTNode *array, ASTNode *index);
ASTNode *ast_array_assign(ASTNode *array_access, ASTNode *expression);
ASTNode *ast_switch(ASTNode *control_expr, ASTNode *cases_list);
ASTNode *ast_case_block(ASTNode *case_expr, ASTNode *body);
ASTNode *ast_for(ASTNode *init, ASTNode *cond, ASTNode *update, ASTNode *body);
ASTNode *ast_declare(int is_const, char *name, ASTNode *expression);
RuntimeValue ast_eval(ASTNode *node);
void ast_free(ASTNode *node);
void ast_dump(const ASTNode *node, int indent);

#endif