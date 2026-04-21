#include "ast.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"

#define MAX_VARS 256

struct ASTNode {
    ASTKind kind;
    int op;
    int value;
    char *text;
    ASTNode *left;
    ASTNode *right;
};

typedef struct {
    char nome[64];
    int valor;
} Variavel;

static Variavel tabela[MAX_VARS];
static int qtd_vars = 0;

static int find_var(const char *nome) {
    int i;

    for (i = 0; i < qtd_vars; i++) {
        if (strcmp(tabela[i].nome, nome) == 0) {
            return i;
        }
    }

    return -1;
}

static int get_var(const char *nome) {
    int idx = find_var(nome);

    if (idx >= 0) {
        return tabela[idx].valor;
    }

    return 0;
}

static void set_var(const char *nome, int valor) {
    int idx = find_var(nome);

    if (idx >= 0) {
        tabela[idx].valor = valor;
        return;
    }

    if (qtd_vars < MAX_VARS) {
        strncpy(tabela[qtd_vars].nome, nome, sizeof(tabela[qtd_vars].nome) - 1);
        tabela[qtd_vars].nome[sizeof(tabela[qtd_vars].nome) - 1] = '\0';
        tabela[qtd_vars].valor = valor;
        qtd_vars++;
    }
}

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

static int eval_assign(ASTNode *node, int value) {
    int atual = get_var(node->text);
    int novo;

    switch (node->op) {
        case OP_atribuicao_soma:
            novo = atual + value;
            break;
        case OP_atribuicao_subtracao:
            novo = atual - value;
            break;
        case OP_atribuicao_potencia:
            novo = (int)pow(atual, value);
            break;
        case OP_atribuicao_multiplicacao:
            novo = atual * value;
            break;
        case OP_atribuicao_divisao:
            if (value == 0) {
                printf("Erro: Divisao por zero!\n");
                return 0;
            }
            novo = atual / value;
            break;
        case OP_atribuicao_resto:
            if (value == 0) {
                printf("Erro: Divisao por zero!\n");
                return 0;
            }
            novo = atual % value;
            break;
        default:
            novo = value;
            break;
    }

    set_var(node->text, novo);
    return novo;
}

int ast_eval(ASTNode *node) {
    int left;
    int right;

    if (!node) {
        return 0;
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
            printf("Resultado: %d\n", left);
            return left;

        case AST_BLOCK:
            return ast_eval(node->left);

        case AST_NUMBER:
            return node->value;

        case AST_IDENTIFIER:
            return get_var(node->text);

        case AST_ASSIGN:
            return eval_assign(node, ast_eval(node->left));

        case AST_BINARY:
            left = ast_eval(node->left);
            right = ast_eval(node->right);

            switch (node->op) {
                case OP_AND:
                    return left && right;
                case OP_OR:
                    return left || right;
                case OP_Igualdade:
                    return left == right;
                case OP_Diferente:
                    return left != right;
                case '+':
                    return left + right;
                case '*':
                    return left * right;
                case '-':
                    return left - right;
                case '>':
                    return left > right;
                case '<':
                    return left < right;
                case '/':
                    if (right == 0) {
                        printf("Erro: Divisao por zero!\n");
                        return 0;
                    }
                    return left / right;
                default:
                    return 0;
            }

        case AST_UNARY:
            return !ast_eval(node->left);
    }

    return 0;
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

        case AST_NUMBER:
            printf("NUMBER(%d)\n", node->value);
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