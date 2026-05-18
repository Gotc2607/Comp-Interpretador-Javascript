#include "ast.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"
#include "parser.tab.h"

#define MAX_VARS 256

static int verificar_igualdade_estrita(RuntimeValue left, RuntimeValue right);

struct ASTNode {
    ASTKind kind;
    int op;
    int value;
    char *text;
    ASTNode *left;
    ASTNode *right;
};

static ASTNode *ast_new(ASTKind kind) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));

    if (!node) {
        fprintf(stderr, "Erro: memoria insuficiente ao criar AST.\n");
        exit(EXIT_FAILURE);
    }

    node->kind = kind;
    return node;
}

ASTNode *ast_sequence(ASTNode *left, ASTNode *right) {
    if (!left) {
        return right;
    }

    if (!right) {
        return left;
    }

    ASTNode *node = ast_new(AST_SEQUENCE);
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_print_stmt(ASTNode *expression) {
    ASTNode *node = ast_new(AST_PRINT);
    node->left = expression;
    return node;
}

ASTNode *ast_block(ASTNode *body) {
    ASTNode *node = ast_new(AST_BLOCK);
    node->left = body;
    return node;
}

ASTNode *ast_number(int value) {
    ASTNode *node = ast_new(AST_NUMBER);
    node->value = value;
    return node;
}

ASTNode *ast_string(char *value) {
    ASTNode *node = ast_new(AST_STRING);
    node->text = value;
    return node;
}

ASTNode *ast_identifier(char *name) {
    ASTNode *node = ast_new(AST_IDENTIFIER);
    node->text = name;
    return node;
}

ASTNode *ast_assign(int op, char *name, ASTNode *expression) {
    ASTNode *node = ast_new(AST_ASSIGN);
    node->op = op;
    node->text = name;
    node->left = expression;
    return node;
}

ASTNode *ast_binary(int op, ASTNode *left, ASTNode *right) {
    ASTNode *node = ast_new(AST_BINARY);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_unary(int op, ASTNode *child) {
    ASTNode *node = ast_new(AST_UNARY);
    node->op = op;
    node->left = child;
    return node;
}

ASTNode *ast_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = ast_new(AST_WHILE);
    node->left = cond;
    node->right = body;
    return node;
}

ASTNode *ast_do_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = ast_new(AST_DO_WHILE);
    node->left = cond;
    node->right = body;
    return node;
}

ASTNode *ast_array_access(ASTNode *array, ASTNode *index) {
    ASTNode *node = ast_new(AST_ARRAY_ACCESS);
    node->left = array;
    node->right = index;
    return node;
}

ASTNode *ast_array_assign(ASTNode *array_access, ASTNode *expression) {
    ASTNode *node = ast_new(AST_ARRAY_ASSIGN);
    node->left = array_access;
    node->right = expression;
    return node;
}

static RuntimeValue eval_assign(ASTNode *node, RuntimeValue value) {
    RuntimeValue result = value;

    SymbolType tipo_atual = sym_get_type(node->text);

    if (node->op == OP_atribuicao_nullish) {
        if (sym_exists(node->text) && (tipo_atual == SYM_INT || tipo_atual == SYM_STRING)) { 
            // Se o seu interpretador considerar que variáveis existentes com tipo válido não mudam:
            if (tipo_atual == SYM_STRING) {
                result.type = VAL_STRING;
                result.sval = sym_get_str(node->text);
            } else {
                result.type = VAL_INT;
                result.ival = sym_get_int(node->text);
            }
            return result; 
        }
        
        if (value.type == VAL_STRING) {
            sym_set_str(node->text, value.sval ? value.sval : "");
        } else {
            sym_set_int(node->text, value.ival);
        }
        return result;
    }

    int atual = sym_get_int(node->text);
    int novo;

    if (value.type == VAL_STRING) {
        if (node->op != '=') {
            printf("Erro: atribuicao composta nao suporta string.\n");
            result.type = VAL_NULL;
            result.sval = NULL;
            return result;
        }

        sym_set_str(node->text, value.sval ? value.sval : "");
        return result;
    }

    switch (node->op) {
        case OP_atribuicao_soma:
            novo = atual + value.ival;
            break;
        case OP_atribuicao_subtracao:
            novo = atual - value.ival;
            break;
        case OP_atribuicao_potencia:
            novo = (int)pow(atual, value.ival);
            break;
        case OP_atribuicao_multiplicacao:
            novo = atual * value.ival;
            break;
        case OP_atribuicao_divisao:
            if (value.ival == 0) {
                printf("Erro: Divisao por zero!\n");
                result.type = VAL_NULL;
                result.sval = NULL;
                return result;
            }
            novo = atual / value.ival;
            break;
        case OP_atribuicao_resto:
            if (value.ival == 0) {
                printf("Erro: Divisao por zero!\n");
                result.type = VAL_NULL;
                result.sval = NULL;
                return result;
            }
            novo = atual % value.ival;
            break;
        default:
            novo = value.ival;
            break;
    }

    sym_set_int(node->text, novo);
    result.ival = novo;
    return result;
}

RuntimeValue ast_eval(ASTNode *node) {
    RuntimeValue result = {VAL_INT, 0};
    RuntimeValue left, right;

    if (!node) {
        return result;
    }

    switch (node->kind) {
        case AST_SEQUENCE:
            left = ast_eval(node->left);

            if (node->right) {
                right = ast_eval(node->right);
                return right;
            }

            return left;

        case AST_PRINT:
            left = ast_eval(node->left);
            if (left.type == VAL_STRING) {
                printf("Resultado: %s\n", left.sval ? left.sval : "");
            } else {
                printf("Resultado: %d\n", left.ival);
            }
            return left;

        case AST_BLOCK:
            scope_push();                    
            left = ast_eval(node->left);
            scope_pop();                     
            return left;

        case AST_WHILE: {
            RuntimeValue cond_val;
            RuntimeValue last_val = {VAL_NULL, 0, NULL};

            while (1) {
                cond_val = ast_eval(node->left);
                
                int is_true = 0;
                if (cond_val.type == VAL_INT && cond_val.ival != 0) is_true = 1;
                else if (cond_val.type == VAL_STRING && cond_val.sval && strlen(cond_val.sval) > 0) is_true = 1;

                if (!is_true) break;

                last_val = ast_eval(node->right);
            }
            return last_val;
        }

        case AST_DO_WHILE: {
            RuntimeValue cond_val;
            RuntimeValue last_val = {VAL_NULL, 0, NULL};

            do {
                last_val = ast_eval(node->right);

                cond_val = ast_eval(node->left);
                
                int is_true = 0;
                if (cond_val.type == VAL_INT && cond_val.ival != 0) is_true = 1;
                else if (cond_val.type == VAL_STRING && cond_val.sval && strlen(cond_val.sval) > 0) is_true = 1;

                if (!is_true) break;

            } while (1);
            return last_val;
        }

        case AST_NUMBER:
            result.ival = node->value;
            return result;

        case AST_STRING:
            result.type = VAL_STRING;
            result.sval = node->text;
            return result;

        case AST_IDENTIFIER:
            if (sym_get_type(node->text) == SYM_STRING) {
                result.type = VAL_STRING;
                result.sval = sym_get_str(node->text);
                return result;
            }

            result.ival = sym_get_int(node->text);
            return result;

        case AST_ASSIGN:
            left = ast_eval(node->left);
            result = eval_assign(node, left);
            return result;

        case AST_BINARY:
            left = ast_eval(node->left);
            right = ast_eval(node->right);

            switch (node->op) {
                case OP_AND:
                    result.ival = left.ival && right.ival;
                    return result;
                case OP_OR:
                    result.ival = left.ival || right.ival;
                    return result;
                case OP_Igualdade:
                    if (left.type == VAL_STRING && right.type == VAL_STRING) {
                        result.ival = strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") == 0;
                        return result;
                    }
                    result.ival = left.ival == right.ival;
                    return result;
                case OP_Diferente:
                    if (left.type == VAL_STRING && right.type == VAL_STRING) {
                        result.ival = strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") != 0;
                        return result;
                    }
                    result.ival = left.ival != right.ival;
                    return result;
                case '+':
                    result.ival = left.ival + right.ival;
                    return result;
                case '*':
                    result.ival = left.ival * right.ival;
                    return result;
                case '%':
                    if (right.ival == 0) {
                        printf("Erro: Divisao por zero!\n");
                        result.ival = 0;
                        return result;
                    }
                    result.ival = left.ival % right.ival;
                    return result;
                case OP_Potencia:
                    result.ival = (int)pow(left.ival, right.ival);
                    return result;
                case '-':
                    result.ival = left.ival - right.ival;
                    return result;
                case OP_MaiorIgual:
                    result.ival = left.ival >= right.ival;
                    return result;
                case OP_MenorIgual:
                    result.ival = left.ival <= right.ival;
                    return result;
                case '>':
                    result.ival = left.ival > right.ival;
                    return result;
                case '<':
                    result.ival = left.ival < right.ival;
                    return result;
                case '/':
                    if (right.ival == 0) {
                        printf("Erro: Divisao por zero!\n");
                        result.ival = 0;
                        return result;
                    }
                    result.ival = left.ival / right.ival;
                    return result;
                case OP_IgualdadeEstrita:
                    result.ival = verificar_igualdade_estrita(left, right);
                    return result;

                case OP_DiferenteEstrita:
                    // A desigualdade estrita é o inverso (!) da igualdade estrita
                    result.ival = !verificar_igualdade_estrita(left, right);
                    return result;
                default:
                    return result;
            }

        case AST_UNARY:
            switch (node->op) {
                case OP_Decremento:
                    result.ival = sym_get_int(node->left->text) - 1;
                    return result;
                case OP_Incremento:
                    result.ival = sym_get_int(node->left->text) + 1;
                    return result;
                default:
                    left = ast_eval(node->left);
                    result.ival = !left.ival;
                    return result;
            }
        case AST_ARRAY_ACCESS: {            
            if (node->left->kind != AST_IDENTIFIER) {
                RuntimeValue left_val = ast_eval(node->left);
                RuntimeValue right_val = ast_eval(node->right);
                
                if (left_val.type == VAL_STRING && right_val.type == VAL_INT) {
                    int tam = strlen(left_val.sval ? left_val.sval : "");
                    if (right_val.ival >= 0 && right_val.ival < tam) {
                        char *caractere = (char *)malloc(2);
                        caractere[0] = left_val.sval[right_val.ival];
                        caractere[1] = '\0';
                        result.type = VAL_STRING;
                        result.sval = caractere;
                        return result;
                    }
                }
                printf("Erro: Operação inválida com colchetes.\n");
                result.type = VAL_NULL;
                return result;
            }

            char *nome_array = node->left->text;
            right = ast_eval(node->right);

            if (right.type != VAL_INT) {
                printf("Erro: O índice do array precisa ser um número inteiro.\n");
                result.type = VAL_NULL;
                return result;
            }

            result.type = VAL_INT;
            result.ival = sym_get_array_element(nome_array, right.ival);
            return result;
        }
        // Lógica para executar a atribuição vinda de um nó customizado (ex: AST_ARRAY_ASSIGN)
        case AST_ARRAY_ASSIGN: {
            ASTNode *acesso = node->left;
            char *nome_array = acesso->left->text;

            RuntimeValue idx_val = ast_eval(acesso->right);   // Descobre a posição (ex: 0)
            RuntimeValue expr_val = ast_eval(node->right);   // Descobre o valor (ex: 50)

            if (idx_val.type == VAL_INT && expr_val.type == VAL_INT) {
                sym_set_array_element(nome_array, idx_val.ival, expr_val.ival);
            }

            return expr_val; // Atribuições em JS retornam o próprio valor atribuído
        }
    }

    return result;
}

void ast_free(ASTNode *node) {
    if (!node) {
        return;
    }

    ast_free(node->left);
    ast_free(node->right);
    free(node->text);
    free(node);
}

static void print_indent(int indent) {
    int i;

    for (i = 0; i < indent; i++) {
        putchar(' ');
    }
}

static int verificar_igualdade_estrita(RuntimeValue left, RuntimeValue right) {
    if (left.type != right.type) return 0;
    if (left.type == VAL_STRING) {
        return strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") == 0;
    }
    return left.ival == right.ival;
}

void ast_dump(const ASTNode *node, int indent) {
    if (!node) {
        print_indent(indent);
        puts("(vazio)");
        return;
    }

    print_indent(indent);

    switch (node->kind) {
        case AST_SEQUENCE:
            puts("SEQUENCE");
            ast_dump(node->left, indent + 2);
            ast_dump(node->right, indent + 2);
            break;

        case AST_PRINT:
            puts("PRINT");
            ast_dump(node->left, indent + 2);
            break;

        case AST_BLOCK:
            puts("BLOCK");
            ast_dump(node->left, indent + 2);
            break;

        case AST_WHILE:
            puts("WHILE");
            ast_dump(node->left, indent + 2);
            ast_dump(node->right, indent + 2);
            break;

        case AST_DO_WHILE:
            puts("DO_WHILE");
            ast_dump(node->left, indent + 2);
            ast_dump(node->right, indent + 2);
            break;

        case AST_NUMBER:
            printf("NUMBER(%d)\n", node->value);
            break;

        case AST_STRING:
            printf("STRING(%s)\n", node->text);
            break;

        case AST_IDENTIFIER:
            printf("IDENT(%s)\n", node->text);
            break;

        case AST_ASSIGN:
            printf("ASSIGN(%s, %d)\n", node->text, node->op);
            ast_dump(node->left, indent + 2);
            break;

        case AST_BINARY:
            printf("BINARY(%d)\n", node->op);
            ast_dump(node->left, indent + 2);
            ast_dump(node->right, indent + 2);
            break;

        case AST_UNARY:
            printf("UNARY(%d)\n", node->op);
            ast_dump(node->left, indent + 2);
            break;
    }
}