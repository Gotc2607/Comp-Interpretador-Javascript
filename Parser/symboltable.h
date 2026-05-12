#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef struct ASTNode ASTNode;

typedef enum {
    SYM_INT,
    SYM_STRING,
    SYM_FUNCTION
} SymbolType;

#define HASH_SIZE 256

typedef struct Symbol {
    char        name[64];
    SymbolType  type;
    int         ival;
    char       *sval;
    ASTNode    *func_body;
    struct Symbol *next;   
} Symbol;

typedef struct Scope {
    Symbol      *buckets[HASH_SIZE];  
    struct Scope *parent;
} Scope;

// Escopos
void      scope_push();
void      scope_pop();

// Inteiros
void      sym_set_int(const char *name, int value);
int       sym_get_int(const char *name);

// Strings
void      sym_set_str(const char *name, char *value);
char     *sym_get_str(const char *name);

// Funções
void      sym_set_func(const char *name, ASTNode *body);
ASTNode  *sym_get_func(const char *name);

// Utilitários
int       sym_exists(const char *name);
SymbolType sym_get_type(const char *name);

#endif