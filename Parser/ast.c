#include "ast.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"
#include "parser.tab.h"

#define MAX_VARS 256
#define MAX_LOOP_ITERATIONS 1000000

static int call_depth = 0;

static int verificar_igualdade_estrita(RuntimeValue left, RuntimeValue right);

/* Localização corrente: o parser atualiza antes de criar cada nó */
int ast_current_line = 0;
int ast_current_col  = 0;

struct ASTNode {
    ASTKind kind;
    int op;
    int value;
    char *text;
    ASTNode *left;
    ASTNode *mid;
    ASTNode *right;
    ASTNode *extra;
    /* localização no fonte */
    int line;
    int col;
};

int ast_node_line(const ASTNode *node) { return node ? node->line : 0; }
int ast_node_col (const ASTNode *node) { return node ? node->col  : 0; }

/* ── Análise semântica ─────────────────────────────────────────────────── */

typedef struct SemSymbol {
    char name[64];
    struct SemSymbol *next;
} SemSymbol;

typedef struct SemScope {
    SemSymbol *symbols;
    struct SemScope *parent;
} SemScope;

static SemScope sem_global = { NULL, NULL };
static SemScope *sem_current = &sem_global;

static int in_loop_depth = 0;

static SemSymbol *sem_find_in_current(const char *name) {
    SemSymbol *sym = sem_current->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0) return sym;
        sym = sym->next;
    }
    return NULL;
}

static int sem_exists(const char *name) {
    SemScope *scope = sem_current;
    while (scope) {
        SemSymbol *sym = scope->symbols;
        while (sym) {
            if (strcmp(sym->name, name) == 0) return 1;
            sym = sym->next;
        }
        scope = scope->parent;
    }
    return 0;
}

static void semantic_error(const char *format, ...);

static int sem_declare(const char *name, int line, int col) {
    if (sem_find_in_current(name)) {
        semantic_error("Erro de sintaxe [linha %d, col %d]: identificador '%s' ja declarado neste escopo.",
            line, col, name);
        return 0;
    }
    SemSymbol *sym = calloc(1, sizeof(SemSymbol));
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->next = sem_current->symbols;
    sem_current->symbols = sym;
    return 1;
}

static void sem_scope_push(void) {
    SemScope *novo = calloc(1, sizeof(SemScope));
    novo->parent = sem_current;
    sem_current = novo;
}

static void sem_scope_pop(void) {
    SemSymbol *sym = sem_current->symbols;
    while (sym) {
        SemSymbol *next = sym->next;
        free(sym);
        sym = next;
    }
    SemScope *old = sem_current;
    sem_current = sem_current->parent;
    free(old);
}

/* ── Construtores de nó ─────────────────────────────────────────────────── */

static ASTNode *ast_new(ASTKind kind) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Erro: memoria insuficiente ao criar AST.\n");
        exit(EXIT_FAILURE);
    }
    node->kind = kind;
    node->line = ast_current_line;
    node->col  = ast_current_col;
    return node;
}

ASTNode *ast_sequence(ASTNode *left, ASTNode *right) {
    if (!left) return right;
    if (!right) return left;
    ASTNode *node = ast_new(AST_SEQUENCE);
    node->left = left;
    node->right = right;
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

ASTNode *ast_for(ASTNode *init, ASTNode *cond, ASTNode *update, ASTNode *body) {
    ASTNode *node = ast_new(AST_FOR);
    node->left  = init;
    node->mid   = cond;
    node->extra = update;
    node->right = body;
    return node;
}

ASTNode *ast_break_stmt(void)    { return ast_new(AST_BREAK); }
ASTNode *ast_continue_stmt(void) { return ast_new(AST_CONTINUE); }

ASTNode *ast_array_access(ASTNode *array, ASTNode *index) {
    ASTNode *node = ast_new(AST_ARRAY_ACCESS);
    node->left  = array;
    node->right = index;
    return node;
}

ASTNode *ast_array_assign(ASTNode *array_access, ASTNode *expression) {
    ASTNode *node = ast_new(AST_ARRAY_ASSIGN);
    node->left  = array_access;
    node->right = expression;
    return node;
}

ASTNode *ast_switch(ASTNode *control_expr, ASTNode *cases_list) {
    ASTNode *node = ast_new(AST_SWITCH);
    node->left  = control_expr;
    node->right = cases_list;
    return node;
}

ASTNode *ast_case_block(ASTNode *case_expr, ASTNode *body) {
    ASTNode *node = ast_new(AST_CASE_BLOCK);
    node->left  = case_expr;
    node->right = body;
    return node;
}

ASTNode *ast_if(ASTNode *cond, ASTNode *then_body, ASTNode *else_body) {
    ASTNode *node = ast_new(AST_IF);
    node->left  = cond;
    node->right = ast_sequence(then_body, else_body);
    return node;
}

ASTNode *ast_declare(int is_const, char *name, ASTNode *expression) {
    ASTNode *node = ast_new(AST_DECLARE);
    node->op   = is_const;
    node->text = name;
    node->left = expression;
    return node;
}

ASTNode *ast_console_log(ASTNode *expression) {
    ASTNode *node = ast_new(AST_CONSOLE_LOG);
    node->left = expression;
    return node;
}

ASTNode *ast_func_decl(char *name, ASTNode *params, ASTNode *body) {
    ASTNode *node = ast_new(AST_FUNC_DECL);
    node->text  = name;
    node->left  = params;
    node->right = body;
    return node;
}

ASTNode *ast_func_call(char *name, ASTNode *args) {
    ASTNode *node = ast_new(AST_FUNC_CALL);
    node->text = name;
    node->left = args;
    return node;
}

ASTNode *ast_return(ASTNode *expr) {
    ASTNode *node = ast_new(AST_RETURN);
    node->left = expr;
    return node;
}

ASTNode *ast_param_list(char *name, ASTNode *next) {
    ASTNode *node = ast_new(AST_PARAM_LIST);
    node->text  = name;
    node->right = next;
    return node;
}

ASTNode *ast_arg_list(ASTNode *expr, ASTNode *next) {
    ASTNode *node = ast_new(AST_ARG_LIST);
    node->left  = expr;
    node->right = next;
    return node;
}

/* ── Erros semânticos ───────────────────────────────────────────────────── */

/*
 * Emite um erro semântico no formato padrão:
 *   Erro de <tipo> [linha X, col Y]: <mensagem>
 *
 * O chamador passa a mensagem já formatada (sem prefixo de linha/col);
 * esta função adiciona o cabeçalho e a quebra de linha.
 */
static void semantic_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void semantic_error_loc(int line, int col, const char *format, ...) {
    /* Extraímos o tipo do erro do início da string de formato para montar
     * o cabeçalho no padrão "Erro de <tipo> [linha X, col Y]: <rest>".
     * Como as chamadas existentes já embutem o tipo na mensagem, apenas
     * prefixamos com a localização e repassamos o restante. */
    if (line > 0) {
        /* Procura o primeiro ':' para separar o prefixo ("Erro de Xxx")
         * da descrição, e insere [linha X, col Y] entre eles. */
        char buf[2048];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        /* Tenta injetar a localização depois do primeiro ':' */
        char *colon = strchr(buf, ':');
        if (colon) {
            /* Imprime a parte antes do ':' */
            int prefix_len = (int)(colon - buf);
            fprintf(stderr, "%.*s [linha %d, col %d]:%s\n",
                    prefix_len, buf, line, col, colon + 1);
        } else {
            fprintf(stderr, "[linha %d, col %d] %s\n", line, col, buf);
        }
    } else {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
}

/* ── Verificação semântica ──────────────────────────────────────────────── */

static int ast_check_node(ASTNode *node) {
    if (!node) return 1;

    int ok = 1;
    int ln = node->line;
    int cl = node->col;

    switch (node->kind) {
        case AST_SEQUENCE:
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_CONSOLE_LOG:
            return ast_check_node(node->left);

        case AST_BLOCK:
            sem_scope_push();
            ok &= ast_check_node(node->left);
            sem_scope_pop();
            return ok;

        case AST_WHILE:
        case AST_DO_WHILE:
            in_loop_depth++;
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            in_loop_depth--;
            return ok;

        case AST_FOR:
            in_loop_depth++;
            if (node->left)  ok &= ast_check_node(node->left);
            if (node->mid)   ok &= ast_check_node(node->mid);
            if (node->extra) ok &= ast_check_node(node->extra);
            ok &= ast_check_node(node->right);
            in_loop_depth--;
            return ok;

        case AST_SWITCH:
            in_loop_depth++;
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            in_loop_depth--;
            return ok;

        case AST_BREAK:
            if (in_loop_depth == 0) {
                semantic_error_loc(ln, cl,
                    "Erro de sintaxe: 'break' usado fora de um laco ou switch.");
                return 0;
            }
            return 1;

        case AST_CONTINUE:
            if (in_loop_depth == 0) {
                semantic_error_loc(ln, cl,
                    "Erro de sintaxe: 'continue' usado fora de um laco de repeticao.");
                return 0;
            }
            return 1;

        case AST_IF:
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_NUMBER:
        case AST_STRING:
            return 1;

        case AST_IDENTIFIER:
            if (!sem_exists(node->text)) {
                semantic_error_loc(ln, cl,
                    "Erro de referencia: variavel '%s' usada sem ser declarada.",
                    node->text);
                return 0;
            }
            return 1;

        case AST_ASSIGN:
            if (!sem_exists(node->text)) {
                semantic_error_loc(ln, cl,
                    "Erro de referencia: atribuicao para variavel nao declarada '%s'.",
                    node->text);
                ok = 0;
            }
            ok &= ast_check_node(node->left);
            return ok;

        case AST_BINARY:
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_UNARY:
            return ast_check_node(node->left);

        case AST_ARRAY_ACCESS:
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_ARRAY_ASSIGN:
            ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_CASE_BLOCK:
            if (node->left) ok &= ast_check_node(node->left);
            ok &= ast_check_node(node->right);
            return ok;

        case AST_DECLARE:
            sem_declare(node->text, ln, cl);
            ok &= ast_check_node(node->left);
            return ok;

        case AST_FUNC_DECL:
            sem_declare(node->text, ln, cl);
            sem_scope_push();
            {
                ASTNode *p = node->left;
                while (p) {
                    sem_declare(p->text, p->line, p->col);
                    p = p->right;
                }
            }
            ok &= ast_check_node(node->right);
            sem_scope_pop();
            return ok;

        case AST_FUNC_CALL:
            if (!sem_exists(node->text)) {
                semantic_error_loc(ln, cl,
                    "Erro de referencia: funcao '%s' chamada sem ser declarada.",
                    node->text);
                ok = 0;
            }
            {
                ASTNode *a = node->left;
                while (a) {
                    ok &= ast_check_node(a->left);
                    a = a->right;
                }
            }
            return ok;

        case AST_RETURN:
            if (node->left) ok &= ast_check_node(node->left);
            return ok;

        default:
            return ok;
    }
}

int ast_check(ASTNode *node) {
    return ast_check_node(node);
}

/* ── Avaliação ──────────────────────────────────────────────────────────── */

static RuntimeValue eval_assign(ASTNode *node, RuntimeValue value) {
    RuntimeValue result = value;
    SymbolType tipo_atual = sym_get_type(node->text);
    int ln = node->line, cl = node->col;

    if (node->op == OP_atribuicao_nullish) {
        if (sym_exists(node->text) && sym_is_initialized(node->text) &&
            (tipo_atual == SYM_INT || tipo_atual == SYM_STRING)) {
            if (tipo_atual == SYM_STRING) {
                result.type = VAL_STRING;
                result.sval = sym_get_str(node->text);
            } else {
                result.type = VAL_INT;
                result.ival = sym_get_int(node->text);
            }
            return result;
        }
        if (value.type == VAL_STRING)
            sym_set_str(node->text, value.sval ? value.sval : "");
        else
            sym_set_int(node->text, value.ival);
        return result;
    }

    int atual = sym_get_int(node->text);
    int novo;

    if (value.type == VAL_STRING) {
        if (node->op != '=') {
            fprintf(stderr,
                "Erro [linha %d, col %d]: atribuicao composta nao suporta string em '%s'.\n",
                ln, cl, node->text);
            result.type = VAL_ERROR;
            return result;
        }
        sym_set_str(node->text, value.sval ? value.sval : "");
        return result;
    }

    switch (node->op) {
        case OP_atribuicao_soma:           novo = atual + value.ival; break;
        case OP_atribuicao_subtracao:      novo = atual - value.ival; break;
        case OP_atribuicao_potencia:       novo = (int)pow(atual, value.ival); break;
        case OP_atribuicao_multiplicacao:  novo = atual * value.ival; break;
        case OP_atribuicao_divisao:
            if (value.ival == 0) {
                fprintf(stderr, "Erro: Divisao por zero!\n");
                result.type = VAL_ERROR;
                return result;
            }
            novo = atual / value.ival; break;
        case OP_atribuicao_resto:
            if (value.ival == 0) {
                fprintf(stderr, "Erro: Divisao por zero!\n");
                result.type = VAL_ERROR;
                return result;
            }
            novo = atual % value.ival; break;
        default: novo = value.ival; break;
    }

    sym_set_int(node->text, novo);
    result.ival = novo;
    return result;
}

#define IS_TRUTHY(v) \
    (((v).type == VAL_INT && (v).ival != 0) || \
     ((v).type == VAL_STRING && (v).sval && (v).sval[0] != '\0'))

RuntimeValue ast_eval(ASTNode *node) {
    RuntimeValue result = {VAL_INT, 0, NULL, CTRL_NONE};
    RuntimeValue left, right;

    if (!node) return result;

    int ln = node->line;
    int cl = node->col;

    switch (node->kind) {
        case AST_SEQUENCE:
            left = ast_eval(node->left);
            if (left.control_flow != CTRL_NONE) return left;
            if (node->right) {
                right = ast_eval(node->right);
                return right;
            }
            return left;

        case AST_CONSOLE_LOG:
            left = ast_eval(node->left);
            if (left.type == VAL_ERROR) return left;
            if (left.type == VAL_STRING)
                printf("%s\n", left.sval ? left.sval : "");
            else
                printf("%d\n", left.ival);
            return left;

        case AST_BLOCK:
            return ast_eval(node->left);

        case AST_WHILE: {
            RuntimeValue cond_val;
            RuntimeValue last_val = {VAL_NULL, 0, NULL, CTRL_NONE};
            int loop_iters = 0;
            while (1) {
                if (++loop_iters > MAX_LOOP_ITERATIONS) {
                    fprintf(stderr,
                        "Erro de Seguranca [linha %d, col %d]: Limite de iteracoes excedido no 'while' (possivel loop infinito).\n",
                        ln, cl);
                    last_val.type = VAL_ERROR;
                    return last_val;
                }
                cond_val = ast_eval(node->left);
                if (!IS_TRUTHY(cond_val)) break;
                last_val = ast_eval(node->right);
                if (last_val.control_flow == CTRL_RETURN) return last_val;
                if (last_val.control_flow == CTRL_BREAK)  { last_val.control_flow = CTRL_NONE; break; }
                if (last_val.control_flow == CTRL_CONTINUE) {
                    last_val.control_flow = CTRL_NONE;
                    if (node->extra) ast_eval(node->extra);
                    continue;
                }
                if (node->extra) ast_eval(node->extra);
            }
            return last_val;
        }

        case AST_FOR: {
            RuntimeValue last_val = {VAL_NULL, 0, NULL, CTRL_NONE};
            if (node->left) ast_eval(node->left);
            int loop_iters = 0;
            while (1) {
                if (++loop_iters > MAX_LOOP_ITERATIONS) {
                    fprintf(stderr,
                        "Erro de Seguranca [linha %d, col %d]: Limite de iteracoes excedido no 'for' (possivel loop infinito).\n",
                        ln, cl);
                    last_val.type = VAL_ERROR;
                    return last_val;
                }
                RuntimeValue cond_val;
                if (node->mid) {
                    cond_val = ast_eval(node->mid);
                } else {
                    cond_val.type = VAL_INT;
                    cond_val.ival = 1;
                }
                if (!IS_TRUTHY(cond_val)) break;
                last_val = ast_eval(node->right);
                if (last_val.control_flow == CTRL_RETURN) return last_val;
                if (last_val.control_flow == CTRL_BREAK)  { last_val.control_flow = CTRL_NONE; break; }
                if (last_val.control_flow == CTRL_CONTINUE) {
                    last_val.control_flow = CTRL_NONE;
                    if (node->extra) ast_eval(node->extra);
                    continue;
                }
                if (node->extra) ast_eval(node->extra);
            }
            return last_val;
        }

        case AST_DO_WHILE: {
            RuntimeValue cond_val;
            RuntimeValue last_val = {VAL_NULL, 0, NULL, CTRL_NONE};
            int loop_iters = 0;
            do {
                if (++loop_iters > MAX_LOOP_ITERATIONS) {
                    fprintf(stderr,
                        "Erro de Seguranca [linha %d, col %d]: Limite de iteracoes excedido no 'do...while' (possivel loop infinito).\n",
                        ln, cl);
                    last_val.type = VAL_ERROR;
                    return last_val;
                }
                last_val = ast_eval(node->right);
                if (last_val.control_flow == CTRL_RETURN) return last_val;
                if (last_val.control_flow == CTRL_BREAK)  { last_val.control_flow = CTRL_NONE; break; }
                if (last_val.control_flow == CTRL_CONTINUE) last_val.control_flow = CTRL_NONE;
                cond_val = ast_eval(node->left);
                if (!IS_TRUTHY(cond_val)) break;
            } while (1);
            return last_val;
        }

        case AST_IF: {
            RuntimeValue cond_val = ast_eval(node->left);
            if (IS_TRUTHY(cond_val))
                return ast_eval(node->right->left);
            else if (node->right->right)
                return ast_eval(node->right->right);
            result.type = VAL_NULL;
            return result;
        }

        case AST_BREAK:
            result.control_flow = CTRL_BREAK;
            return result;

        case AST_CONTINUE:
            result.control_flow = CTRL_CONTINUE;
            return result;

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
            if (left.type == VAL_ERROR) return left;
            return eval_assign(node, left);

        case AST_BINARY:
            left = ast_eval(node->left);
            if (left.type == VAL_ERROR) return left;
            right = ast_eval(node->right);
            if (right.type == VAL_ERROR) return right;

            switch (node->op) {
                case OP_AND:
                    result.ival = IS_TRUTHY(left) && IS_TRUTHY(right);
                    return result;
                case OP_OR:
                    result.ival = IS_TRUTHY(left) || IS_TRUTHY(right);
                    return result;
                case OP_Igualdade:
                    if (left.type == VAL_STRING && right.type == VAL_STRING)
                        result.ival = strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") == 0;
                    else
                        result.ival = left.ival == right.ival;
                    return result;
                case OP_Diferente:
                    if (left.type == VAL_STRING && right.type == VAL_STRING)
                        result.ival = strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") != 0;
                    else
                        result.ival = left.ival != right.ival;
                    return result;
                case '+': result.ival = left.ival + right.ival; return result;
                case '*': result.ival = left.ival * right.ival; return result;
                case '%':
                    if (right.ival == 0) {
                    fprintf(stderr, "Erro: Divisao por zero!\n");
                        result.type = VAL_ERROR;
                        return result;
                    }
                    result.ival = left.ival % right.ival;
                    return result;
                case OP_Potencia:
                    result.ival = (int)pow(left.ival, right.ival);
                    return result;
                case '-': result.ival = left.ival - right.ival; return result;
                case OP_MaiorIgual: result.ival = left.ival >= right.ival; return result;
                case OP_MenorIgual: result.ival = left.ival <= right.ival; return result;
                case '>': result.ival = left.ival > right.ival; return result;
                case '<': result.ival = left.ival < right.ival; return result;
                case '/':
                    if (right.ival == 0) {
                        fprintf(stderr, "Erro: Divisao por zero!\n");
                        result.type = VAL_ERROR;
                        return result;
                    }
                    result.ival = left.ival / right.ival;
                    return result;
                case OP_IgualdadeEstrita:
                    result.ival = verificar_igualdade_estrita(left, right);
                    return result;
                case OP_DiferenteEstrita:
                    result.ival = !verificar_igualdade_estrita(left, right);
                    return result;
                default: return result;
            }

        case AST_SWITCH: {
            RuntimeValue valor_controle = ast_eval(node->left);
            ASTNode *case_atual = node->right;
            ASTNode *no_default = NULL;
            int match_encontrado = 0;
            RuntimeValue last_val = {VAL_NULL, 0, NULL, CTRL_NONE};

            while (case_atual) {
                ASTNode *bloco = case_atual;
                if (case_atual->kind == AST_SEQUENCE) {
                    bloco = case_atual->right;
                    case_atual = case_atual->left;
                } else {
                    case_atual = NULL;
                }
                if (bloco && bloco->kind == AST_CASE_BLOCK) {
                    if (bloco->left == NULL) {
                        no_default = bloco->right;
                        continue;
                    }
                    RuntimeValue valor_case = ast_eval(bloco->left);
                    if (valor_controle.type == valor_case.type && valor_controle.ival == valor_case.ival) {
                        match_encontrado = 1;
                        if (bloco->right) {
                            last_val = ast_eval(bloco->right);
                            if (last_val.control_flow == CTRL_BREAK)
                                last_val.control_flow = CTRL_NONE;
                        }
                        break;
                    }
                }
            }
            if (!match_encontrado && no_default) {
                last_val = ast_eval(no_default);
                if (last_val.control_flow == CTRL_BREAK)
                    last_val.control_flow = CTRL_NONE;
            }
            return last_val;
        }

        case AST_UNARY:
            switch (node->op) {
                case OP_Decremento: {
                    int val = sym_get_int(node->left->text) - 1;
                    sym_set_int(node->left->text, val);
                    result.ival = val;
                    return result;
                }
                case OP_Incremento: {
                    int val = sym_get_int(node->left->text) + 1;
                    sym_set_int(node->left->text, val);
                    result.ival = val;
                    return result;
                }
                default:
                    left = ast_eval(node->left);
                    result.ival = !left.ival;
                    return result;
            }

        case AST_ARRAY_ACCESS: {
            if (node->left->kind == AST_IDENTIFIER) {
                char *nome = node->left->text;
                RuntimeValue right_val = ast_eval(node->right);
                if (right_val.type != VAL_INT) {
                    fprintf(stderr,
                        "Erro [linha %d, col %d]: o indice de '%s' precisa ser um numero inteiro.\n",
                        ln, cl, nome);
                    result.type = VAL_ERROR;
                    return result;
                }
                if (sym_get_type(nome) == SYM_STRING) {
                    char *s = sym_get_str(nome);
                    int tam = strlen(s ? s : "");
                    if (right_val.ival >= 0 && right_val.ival < tam) {
                        char *caractere = (char *)malloc(2);
                        caractere[0] = s[right_val.ival];
                        caractere[1] = '\0';
                        result.type = VAL_STRING;
                        result.sval = caractere;
                        return result;
                    }
                    result.type = VAL_NULL;
                    return result;
                } else {
                    result.type = VAL_INT;
                    result.ival = sym_get_array_element(nome, right_val.ival);
                    return result;
                }
            } else {
                RuntimeValue left_val  = ast_eval(node->left);
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
                fprintf(stderr,
                    "Erro [linha %d, col %d]: operacao invalida com colchetes.\n",
                    ln, cl);
                result.type = VAL_ERROR;
                return result;
            }
        }

        case AST_ARRAY_ASSIGN: {
            ASTNode *acesso = node->left;
            char *nome_array = acesso->left->text;
            RuntimeValue idx_val  = ast_eval(acesso->right);
            RuntimeValue expr_val = ast_eval(node->right);
            if (idx_val.type == VAL_INT && expr_val.type == VAL_INT)
                sym_set_array_element(nome_array, idx_val.ival, expr_val.ival);
            return expr_val;
        }

        case AST_DECLARE: {
            RuntimeValue val = {VAL_NULL, 0, NULL, CTRL_NONE};
            if (node->left) val = ast_eval(node->left);
            sym_declare(node->text, node->op);
            if (node->left) {
                if (val.type == VAL_STRING)
                    sym_set_str(node->text, val.sval ? val.sval : "");
                else
                    sym_set_int(node->text, val.ival);
            }
            return val;
        }

        case AST_FUNC_DECL:
            sym_set_func(node->text, node);
            result.type = VAL_NULL;
            return result;

        case AST_FUNC_CALL: {
            if (strcmp(node->text, "eval")     == 0 ||
                strcmp(node->text, "exec")     == 0 ||
                strcmp(node->text, "system")   == 0 ||
                strcmp(node->text, "Function") == 0) {
                fprintf(stderr,
                    "Erro de Seguranca [linha %d, col %d]: chamada bloqueada para funcao '%s'.\n",
                    ln, cl, node->text);
                result.type = VAL_ERROR;
                return result;
            }

            ASTNode *func_node = sym_get_func(node->text);
            if (!func_node) {
                fprintf(stderr,
                    "Erro [linha %d, col %d]: funcao '%s' nao definida.\n",
                    ln, cl, node->text);
                result.type = VAL_ERROR;
                return result;
            }

            if (++call_depth > 1000) {
                fprintf(stderr,
                    "Erro de Seguranca [linha %d, col %d]: Stack Overflow ao chamar '%s' (limite de recursao excedido).\n",
                    ln, cl, node->text);
                result.type = VAL_ERROR;
                call_depth--;
                return result;
            }

            RuntimeValue arg_vals[256];
            int arg_count = 0;
            ASTNode *arg = node->left;
            while (arg && arg_count < 256) {
                arg_vals[arg_count++] = ast_eval(arg->left);
                arg = arg->right;
            }

            scope_push();

            ASTNode *param = func_node->left;
            int p_idx = 0;
            while (param) {
                sym_declare(param->text, 0);
                if (p_idx < arg_count) {
                    if (arg_vals[p_idx].type == VAL_STRING)
                        sym_set_str(param->text, arg_vals[p_idx].sval);
                    else
                        sym_set_int(param->text, arg_vals[p_idx].ival);
                } else {
                    sym_set_int(param->text, 0);
                }
                param = param->right;
                p_idx++;
            }

            RuntimeValue ret_val = ast_eval(func_node->right);

            if (ret_val.control_flow == CTRL_RETURN)
                ret_val.control_flow = CTRL_NONE;
            else {
                ret_val.type = VAL_NULL;
                ret_val.ival = 0;
            }

            scope_pop();
            call_depth--;
            return ret_val;
        }

        case AST_RETURN: {
            if (node->left) result = ast_eval(node->left);
            else { result.type = VAL_NULL; result.ival = 0; }
            result.control_flow = CTRL_RETURN;
            return result;
        }

        default:
            return result;
    }
}

/* ── Liberação de memória ────────────────────────────────────────────────── */

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->mid);
    ast_free(node->right);
    ast_free(node->extra);
    free(node->text);
    free(node);
}

/* ── Dump ───────────────────────────────────────────────────────────────── */

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) putchar(' ');
}

static int verificar_igualdade_estrita(RuntimeValue left, RuntimeValue right) {
    if (left.type != right.type) return 0;
    if (left.type == VAL_STRING)
        return strcmp(left.sval ? left.sval : "", right.sval ? right.sval : "") == 0;
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
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent + 2);
            break;
        case AST_BLOCK:
            puts("BLOCK");
            ast_dump(node->left, indent + 2);
            break;
        case AST_WHILE:
            puts("WHILE");
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent + 2);
            break;
        case AST_FOR:
            puts("FOR");
            ast_dump(node->left,  indent + 4);
            ast_dump(node->mid,   indent + 4);
            ast_dump(node->extra, indent + 4);
            ast_dump(node->right, indent + 2);
            break;
        case AST_DO_WHILE:
            puts("DO_WHILE");
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent + 2);
            break;
        case AST_IF:
            puts("IF");
            ast_dump(node->left,         indent + 4);
            ast_dump(node->right->left,  indent + 4);
            if (node->right->right) ast_dump(node->right->right, indent + 4);
            break;
        case AST_NUMBER:     printf("NUMBER(%d)\n", node->value); break;
        case AST_STRING:     printf("STRING(%s)\n", node->text);  break;
        case AST_IDENTIFIER: printf("IDENT(%s)\n",  node->text);  break;
        case AST_ASSIGN:
            printf("ASSIGN(%s, %d)\n", node->text, node->op);
            ast_dump(node->left, indent + 2);
            break;
        case AST_BINARY:
            printf("BINARY(%d)\n", node->op);
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent + 2);
            break;
        case AST_UNARY:
            printf("UNARY(%d)\n", node->op);
            ast_dump(node->left, indent + 2);
            break;
        case AST_DECLARE:
            printf("DECLARE(%s, is_const=%d)\n", node->text, node->op);
            ast_dump(node->left, indent + 2);
            break;
        case AST_CONSOLE_LOG:
            puts("CONSOLE_LOG");
            ast_dump(node->left, indent + 2);
            break;
        case AST_FUNC_DECL:
            printf("FUNC_DECL(%s)\n", node->text);
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent + 2);
            break;
        case AST_FUNC_CALL:
            printf("FUNC_CALL(%s)\n", node->text);
            ast_dump(node->left, indent + 2);
            break;
        case AST_RETURN:
            puts("RETURN");
            ast_dump(node->left, indent + 2);
            break;
        case AST_PARAM_LIST:
            printf("PARAM(%s)\n", node->text);
            ast_dump(node->right, indent);
            break;
        case AST_ARG_LIST:
            puts("ARG");
            ast_dump(node->left,  indent + 2);
            ast_dump(node->right, indent);
            break;
        default: puts("UNKNOWN_NODE"); break;
    }
}