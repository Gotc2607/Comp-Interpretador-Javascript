# Suíte de Testes

Esta documentação analisa a suíte de testes atual do interpretador e propõe melhorias para torná-la mais robusta.

## Estado atual

A suíte é baseada em arquivos `testes/teste_*.js` com gabaritos correspondentes `testes/teste_*.out`. O script `executar_testes.py` executa cada arquivo `.js` no interpretador e compara a saída padrão (`stdout`) com o arquivo `.out`.

### Pontos fortes

- O formato de testes emparelhados (`.js` + `.out`) é simples e fácil de manter.
- O script já ordena e executa todos os casos automaticamente.
- Testes existem para várias áreas: soma, matemática, comparação, lógica, loops, if, arrays, switch, `console.log`.

### Problemas identificados

1. **Cobertura inconsistente com a semântica atual**
   - Vários arquivos de teste usam expressões isoladas como `10 > 5;` esperando saída implícita.
   - O interpretador foi alterado para imprimir apenas `console.log(...)`, portanto esses testes agora falham.

2. **Saída ambígua / misturada**
   - O arquivo de gabarito `teste_05_erro.out` espera saída de erro e valor juntos.
   - Atualmente o interpretador imprime erros em `stdout`; separar `stdout` e `stderr` tornaria os testes mais precisos.

3. **Falta de testes negativos estruturados**
   - Só há um teste de erro de divisão por zero.
   - Não há testes específicos para erros semânticos recentes, como variável não declarada ou re-declaração.

4. **Formato do script de testes limitado**
   - O script só compara `stdout`; não considera `stderr` ou código de saída (`return code`).
   - Não há meta-dados de teste, categorias ou descrição do que cada caso verifica.

5. **Cobertura de linguagem incompleta**
   - Falta cobertura para uso de `console.log` em estados diferentes, strings, operações de array após declaração e erros de escopo.
   - Falta cobertura de funções, se forem adicionadas no futuro.

## Melhorias recomendadas

### 1. Atualizar a suíte para a semântica `console.log`

- Mudar casos de teste que dependem de saída implícita para chamadas explícitas de `console.log`.
- Exemplo: `10 > 5;` passa a ser `console.log(10 > 5);`.

### 2. Melhorar o script de testes

- Comparar `stdout` e `stderr` separadamente.
- Suportar arquivos `*.out`, `*.err` e `*.rc` para saída padrão, saída de erro e código de retorno.
- Exibir diffs mais claros quando houver falha.

### 3. Adicionar testes negativos e de regressão

- Variável não declarada
- Reatribuição em `const`
- Redefinição de variável no mesmo escopo
- Erros de sintaxe (parênteses não fechados, token inválido)
- Uso de `console.log` com literais de string e expressões complexas
- Comportamento de arrays e `switch` com `default`

### 4. Documentar como criar testes

- Criar doc em `docs/suite_testes.md` com instruções de como adicionar novos casos.
- Descrever o padrão de arquivos e como o runner compara saída.

## Ações implementadas

- Documento criado: `docs/suite_testes.md`.
- Serão aplicadas as seguintes correções:
  - atualização dos testes existentes para `console.log`.
  - melhoria do harness `executar_testes.py`.
  - adição de testes sintáticos e semânticos básicos.
