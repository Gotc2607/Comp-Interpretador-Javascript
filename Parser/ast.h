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
    AST_BREAK,
    AST_CONTINUE,
    AST_FUNC_DECL,
    AST_FUNC_CALL,
    AST_RETURN,
    AST_PARAM_LIST,
    AST_ARG_LIST,
} ASTKind;

typedef enum {
    VAL_INT,
    VAL_BOOL,
    VAL_STRING,
    VAL_NULL,
    VAL_ERROR,
} ValueType;

typedef struct {
    ValueType type;
    int ival;
    char *sval;
    int control_flow;
} RuntimeValue;

#define CTRL_NONE 0
#define CTRL_BREAK 1
#define CTRL_CONTINUE 2
#define CTRL_RETURN 3

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
ASTNode *ast_break_stmt(void);
ASTNode *ast_continue_stmt(void);
ASTNode *ast_declare(int is_const, char *name, ASTNode *expression);
int ast_check(ASTNode *node);
RuntimeValue ast_eval(ASTNode *node);
void ast_free(ASTNode *node);
void ast_dump(const ASTNode *node, int indent);
ASTNode *ast_func_decl(char *name, ASTNode *params, ASTNode *body);
ASTNode *ast_func_call(char *name, ASTNode *args);
ASTNode *ast_return(ASTNode *expr);
ASTNode *ast_param_list(char *name, ASTNode *next);
ASTNode *ast_arg_list(ASTNode *expr, ASTNode *next);

#endif