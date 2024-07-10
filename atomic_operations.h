#ifndef MTX_ATOMIC_OP_H
#define MTX_ATOMIC_OP_H

#include "matrix.h"

// Cria um alias de uma matriz, copiando as bordas e referenciando os dados.
#define CREATE_OUTPUT_ALIAS(name, output) mtx_matrix_t name = *(output)

// Caso rules == true, executa action() com argumentos na va_args. Recebe uma
// mtx_matrix_t alias e uma mtx_matrix_t * output.
#define GET_SAFE_OUTPUT_IN_DETAILS(alias, output, rules, action, ...)          \
  do {                                                                         \
    if (rules) {                                                               \
      alias.data = NULL;                                                       \
      action(&(alias), __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

// Caso rules == true , aloca uma nova matriz de output para alias, que deixará
// de ser um alias e terá dados alocados para si.
#define GET_SAFE_OUTPUT_RULES(alias, output, rules)                            \
  GET_SAFE_OUTPUT_IN_DETAILS(alias, output, rules, mtx_matrix_init,            \
                             (output)->dy, (output)->dx)

// Caso output convegir com input, aloca uma nova matriz com as mesmas dimensões
// de input e guarda na mtx_matrix_t alias.
#define GET_SAFE_OUTPUT(alias, output, input)                                  \
  GET_SAFE_OUTPUT_RULES(alias, output, input, MTX_OVERLAP((input), (output)))

// Caso mtx_matrix_t alias e mtx_matrix_t * output não sejam a mesma matriz,
// copia o que tem em alias para output. Usar somente depois de
// CREATE_OUTPUT_ALIAS e GET_SAFE_OUTPUT*.
#define COMMIT_OUTPUT(alias, output)                                           \
  do {                                                                         \
    if (!MTX_MATRIX_ARE_SAME(&alias, output)) {                                \
      mtx_matrix_copy(output, &alias);                                         \
      mtx_matrix_free(&alias);                                                 \
    }                                                                          \
  } while (0)

#endif
