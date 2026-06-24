#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"

static unsigned int hash(const char *str) {
    unsigned int h = 5381;
    while (*str)
        h = ((h << 5) + h) + (unsigned char)(*str++); 
    return h % HASH_SIZE;
}

static Scope global_scope = { .buckets = {NULL}, .parent = NULL };
static Scope *current = &global_scope;

static Symbol *find_symbol(const char *name) {
    unsigned int idx = hash(name);
    Scope *scope = current;

    while (scope) {
        Symbol *s = scope->buckets[idx];
        while (s) {
            if (strcmp(s->name, name) == 0)
                return s;
            s = s->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

static Symbol *find_in_current(const char *name) {
    unsigned int idx = hash(name);
    Symbol *s = current->buckets[idx];
    while (s) {
        if (strcmp(s->name, name) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}

static Symbol *create_symbol(const char *name) {
    Symbol *s = calloc(1, sizeof(Symbol));
    if (!s) {
        fprintf(stderr, "Erro de Seguranca: Memoria insuficiente (Out of Memory).\n");
        exit(EXIT_FAILURE);
    }
    strncpy(s->name, name, sizeof(s->name) - 1);

    unsigned int idx = hash(name);
    s->next = current->buckets[idx];   
    current->buckets[idx] = s;
    s->initialized = 0;

    return s;
}

void scope_push() {
    Scope *novo = calloc(1, sizeof(Scope));
    if (!novo) {
        fprintf(stderr, "Erro de Seguranca: Memoria insuficiente ao criar escopo.\n");
        exit(EXIT_FAILURE);
    }
    novo->parent = current;
    current = novo;
}

void scope_pop() {
    if (!current->parent) {
        fprintf(stderr, "Erro: tentou sair do escopo global\n");
        return;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        Symbol *s = current->buckets[i];
        while (s) {
            Symbol *next = s->next;
            if (s->type == SYM_STRING && s->sval)
                free(s->sval);
            free(s);
            s = next;
        }
    }

    Scope *old = current;
    current = current->parent;
    free(old);
}

void sym_declare(const char *name, int is_const) {
    Symbol *s = find_in_current(name);
    if (s) {
        // Se já existe e não for 'var' (ou seja, let ou const), gera erro de sintaxe
        if (s->is_const != 0) {
            fprintf(stderr, "Erro de Sintaxe: Identificador '%s' ja foi declarado neste escopo.\n", name);
        }
        return;
    }
    
    s = create_symbol(name);
    s->is_const = (is_const == 1) ? 2 : 0;
}

void sym_set_int(const char *name, int value) {
    Symbol *s = find_symbol(name);
    
    if (!s) {
        // Se a variável não foi declarada, cria dinamicamente (evita quebrar testes antigos)
        s = create_symbol(name);
        s->is_const = 0;
    }

    if (s->is_const == 1) {
        fprintf(stderr, "TypeError: Atribuicao a variavel constante '%s'.\n", name);
        return;
    }
    if (s->is_const == 2) {
        s->is_const = 1;
    }
    
    s->type = SYM_INT;
    s->ival = value;
    s->initialized = 1;
}

int sym_get_int(const char *name) {
    Symbol *s = find_symbol(name);
    if (s && s->type == SYM_INT)
        return s->ival;
    return 0;
}

void sym_set_str(const char *name, char *value) {
    Symbol *s = find_symbol(name);
    
    if (!s) {
        s = create_symbol(name);
        s->is_const = 0;
    }

    if (s->is_const == 1) {
        fprintf(stderr, "TypeError: Atribuicao a variavel constante '%s'.\n", name);
        return;
    }
    if (s->is_const == 2) {
        s->is_const = 1;
    }
    
    if (s->sval) free(s->sval);
    s->type = SYM_STRING;
    s->sval = strdup(value);
    if (!s->sval) {
        fprintf(stderr, "Erro de Seguranca: Memoria insuficiente para string.\n");
        exit(EXIT_FAILURE);
    }
    s->initialized = 1;
}

char *sym_get_str(const char *name) {
    Symbol *s = find_symbol(name);
    if (s && s->type == SYM_STRING)
        return s->sval;
    return "";
}

void sym_set_func(const char *name, ASTNode *body) {
    Symbol *s = find_in_current(name);
    if (!s) s = create_symbol(name);
    s->type      = SYM_FUNCTION;
    s->func_body = body;
}

ASTNode *sym_get_func(const char *name) {
    Symbol *s = find_symbol(name);
    if (s && s->type == SYM_FUNCTION)
        return s->func_body;
    return NULL;
}

int sym_exists(const char *name) {
    return find_symbol(name) != NULL;
}

SymbolType sym_get_type(const char *name) {
    Symbol *s = find_symbol(name);
    if (s) return s->type;
    return SYM_INT;
}

void sym_set_array_element(const char *name, int index, int value) {
    if (index < 0 || index > 1000000) {
        fprintf(stderr, "Erro de Seguranca: Manipulacao de memoria nao prevista. Indice %d invalido.\n", index);
        return;
    }

    Symbol *s = find_symbol(name); 
    
    if (!s) {
        s = create_symbol(name);
        s->type = SYM_ARRAY;
        s->arr_vals = NULL;
        s->arr_size = 0;
    }

    if (s->type != SYM_ARRAY) {
        s->type = SYM_ARRAY;
        s->arr_vals = NULL;
        s->arr_size = 0;
    }
    s->initialized = 1;

    if (index >= s->arr_size) {
        int novo_tamanho = index + 1;
        s->arr_vals = realloc(s->arr_vals, novo_tamanho * sizeof(int));
        if (!s->arr_vals) {
            fprintf(stderr, "Erro de Seguranca: Memoria insuficiente para array.\n");
            exit(EXIT_FAILURE);
        }
        for (int i = s->arr_size; i < novo_tamanho; i++) {
            s->arr_vals[i] = 0;
        }
        s->arr_size = novo_tamanho;
    }
    s->arr_vals[index] = value;
}

int sym_get_array_element(const char *name, int index) {
    Symbol *s = find_symbol(name);
    if (s && s->type == SYM_ARRAY && index >= 0 && index < s->arr_size) {
        return s->arr_vals[index];
    }
    return 0;
}

int sym_is_initialized(const char *name) {
    Symbol *s = find_symbol(name);
    return s ? s->initialized : 0;
}