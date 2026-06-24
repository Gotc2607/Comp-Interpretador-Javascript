# Otimizações no Interpretador JavaScript

Este documento descreve as otimizações implementadas no interpretador JavaScript para aumentar o desempenho geral de execução e garantir a correta liberação de recursos em memória heap.

---

## 1. Correção de Vazamento de Memória (Memory Leak) em Arrays Locais

### O Problema
Ao declarar um Array em escopos internos (como loops ou funções), o interpretador aloca espaço para a estrutura do símbolo e para o buffer de elementos do array (`s->arr_vals`).

Ao fechar um bloco `{ ... }`, o método `scope_pop()` é acionado para desalocar e remover as variáveis locais do escopo que está sendo encerrado. No entanto, na implementação inicial, se a variável fosse do tipo `SYM_ARRAY`, apenas a estrutura do símbolo era desalocada. O buffer `s->arr_vals` permanecia alocado em memória heap, resultando em vazamento de memória progressivo.

### A Solução
Adicionamos em [Parser/symboltable.c](file://wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/Parser/symboltable.c#L79-L83) a liberação explícita do buffer de elementos de arrays durante o descarte de variáveis locais em `scope_pop()`:

```c
void scope_pop() {
    ...
    for (int i = 0; i < HASH_SIZE; i++) {
        Symbol *s = current->buckets[i];
        while (s) {
            Symbol *next = s->next;
            if (s->type == SYM_STRING && s->sval)
                free(s->sval);
            if (s->type == SYM_ARRAY && s->arr_vals)
                free(s->arr_vals); // Liberação do buffer do array local
            free(s);
            s = next;
        }
    }
    ...
}
```

---

## 2. Redução de Lookups Redundantes na Tabela de Símbolos

### O Problema
Nas operações de avaliação da AST (`ast_eval`), o acesso e mutação de variáveis geravam múltiplos lookups por tabela de dispersão (hash) redundantes para o mesmo identificador de variável.

Por exemplo, ao avaliar a expressão de atribuição composta `x = x + 1`:
1. **Fase de Leitura de `x`:** chamava `sym_get_type(x)` (1 lookup) e em seguida `sym_get_int(x)` (mais 1 lookup).
2. **Fase de Atribuição em `x`:** chamava `sym_get_type(x)` (1 lookup), `sym_exists(x)` (1 lookup), `sym_is_initialized(x)` (1 lookup) e `sym_set_int(x)` (1 lookup).

Isso acarretava **6 buscas por hashing** na tabela de símbolos para uma única operação.

### A Solução
Expostos métodos de busca e modificação diretos na API em [Parser/symboltable.h](file://wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/Parser/symboltable.h#L58-L64) e [Parser/symboltable.c](file://wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/Parser/symboltable.c#L242-L278):
- `Symbol *sym_lookup(const char *name);`
- `void sym_set_int_direct(Symbol *s, int value);`
- `void sym_set_str_direct(Symbol *s, char *value);`

No arquivo [Parser/ast.c](file://wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/Parser/ast.c#L443-L519), a avaliação de `AST_IDENTIFIER` e `eval_assign` foi refatorada para fazer apenas um `sym_lookup` no início do comando. Toda a leitura e a escrita ocorrem diretamente através do ponteiro do símbolo `Symbol *s` retornado, eliminando as buscas repetidas. O número de buscas por hashing para `x = x + 1` foi reduzido de **6 lookups para 2 lookups**.

---

## 3. Constant Folding (Simplificação de Constantes)

### O Problema
Expressões formadas unicamente por valores literais constantes (como `2 + 3` ou `5 * 4`) eram avaliadas repetidamente no `ast_eval` a cada ciclo de execução do runtime (especialmente ineficiente dentro de laços de repetição).

### A Solução
Implementamos o **Constant Folding** durante a fase de análise estática do compilador no método `ast_check_node` do arquivo [Parser/ast.c](file://wsl.localhost/Ubuntu/home/artur/Comp-Interpretador-Javascript/Parser/ast.c#L372-L409).

Ao analisar um nó `AST_BINARY`:
1. O analisador verifica recursivamente se os nós filhos (esquerdo e direito) foram resolvidos como literais do tipo `AST_NUMBER`.
2. Se ambos forem numéricos, a operação matemática correspondente (`+`, `-`, `*`, `/`, `%`, `**`) é resolvida estaticamente na análise.
3. Os nós filhos redundantes são devidamente desalocados com `ast_free`.
4. O nó pai original é reconfigurado para ser diretamente um literal `AST_NUMBER` contendo o resultado pré-calculado.

Isso acelera o tempo de execução no runtime, diminui o consumo de memória da árvore sintática na heap, e permite otimizar a performance de loops que contêm operações aritméticas de constantes literais.

---

## ⚙️ Como Validar

Para atestar a integridade física do interpretador após as otimizações, recompile o executável e execute os testes automatizados da suíte:

```bash
make clean
make
python3 executar_testes.py
```
*(Ou via terminal WSL)*
```bash
wsl sh -c "make clean && make && python3 executar_testes.py"
```

Todos os **44 testes devem passar com sucesso**, confirmando que as otimizações preservam as especificações de semântica e tipagem da linguagem.
