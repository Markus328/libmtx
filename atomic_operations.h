#ifndef MTX_ATOMIC_OP_H
#define MTX_ATOMIC_OP_H

#include "errors.h"
#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defined in matrix.c
extern int __mtx_cfg_fix_unsafe_overlappings;
// Defined in matrix.c
int mtx_cfg_are_unsafe_overlappings_fixed();

// Ativa ou desativa a função de consertar `unsafe overlappings`.
void mtx_cfg_fix_unsafe_overlappings(int enable);

// Cria um alias de uma matriz, copiando as bordas e referenciando os dados.
#define MTX_MAKE_OUTPUT_ALIAS(alias, output)                                   \
  mtx_matrix_t alias = *(output);                                              \
  int __##alias##_is_still_same = 1;

// Caso rules == true, executa action() com argumentos na va_args. Recebe uma
// mtx_matrix_t alias e uma mtx_matrix_t * output.
#define MTX_ENSURE_SAFE_OUTPUT_IN_DETAILS(alias, output, input, rules, action, \
                                          ...)                                 \
  do {                                                                         \
    if (__##alias##_is_still_same && rules) {                                  \
      if (!mtx_cfg_are_unsafe_overlappings_fixed()) {                          \
        MTX_OVERLAP_ERR(output, input);                                        \
      }                                                                        \
      alias.data = NULL;                                                       \
      __##alias##_is_still_same = 0;                                           \
      action(&(alias), __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

// Caso rules == true , aloca uma nova matriz de output para alias, que deixará
// de ser um alias e terá dados alocados para si.
#define MTX_ENSURE_SAFE_OUTPUT_RULES(alias, output, input, rules)              \
  MTX_ENSURE_SAFE_OUTPUT_IN_DETAILS(alias, output, input, rules,               \
                                    mtx_matrix_init, (output)->dy,             \
                                    (output)->dx)

// Caso output convegir com input, aloca uma nova matriz com as mesmas dimensões
// de input e guarda na mtx_matrix_t alias.
#define MTX_ENSURE_SAFE_OUTPUT(alias, output, input)                           \
  MTX_ENSURE_SAFE_OUTPUT_RULES(alias, output, input,                           \
                               MTX_MATRIX_OVERLAP((input), (output)))

// Caso mtx_matrix_t alias e mtx_matrix_t * output não sejam a mesma matriz,
// copia o que tem em alias para output. Usar somente depois de
// CREATE_OUTPUT_ALIAS e MTX_ENSURE_SAFE_OUTPUT*.
#define MTX_COMMIT_OUTPUT(alias, output)                                       \
  do {                                                                         \
    if (!__##alias##_is_still_same) {                                          \
      mtx_matrix_copy(output, &alias);                                         \
      mtx_matrix_free(&alias);                                                 \
    }                                                                          \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
