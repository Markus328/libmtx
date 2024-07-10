#ifndef MATRIX_H
#define MATRIX_H

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct matrix_data {
  double **m;
  size_t size1;
  size_t size2;
} matrix_data_t;

typedef struct matrix {
  matrix_data_t *data;
  int dx, dy;
  int offX, offY;
} matrix_t;

typedef struct matrix_view {
  matrix_t matrix;
} matrix_view_t;

typedef matrix_t matrix_perm_t;

typedef enum error_codes {
  SUCCESS = 0,
  NULL_ERR,
  DIMEN_ERR,
  INVALID_ERR,
  BOUNDS_ERR,
  OVERLAP_ERR,
} mtx_error_t;

#define MTX_ERROR(text, error, ...)                                            \
  fprintf(stderr, "MTX ERROR in %s(): " text "\n", __func__, __VA_ARGS__);     \
  abort()

#define MTX_NULL_ERR(M) MTX_ERROR("matrix %s is unitialized.", NULL_ERR, #M)
#define MTX_DIMEN_ERR(M)                                                       \
  MTX_ERROR("matrix (%d, %d) %s has invalid dimensions.", DIMEN_ERR, (M)->dy,  \
            (M)->dx, #M)
#define MTX_INVALID_ERR(M) MTX_ERROR("matrix %s is invalid.", INVALID_ERR, #M)
#define MTX_BOUNDS_ERR(M)                                                      \
  MTX_ERROR("matrix (%d, %d) %s is out of bounds.", BOUNDS_ERR, (M)->dy,       \
            (M)->dx, #M)
#define MTX_OVERLAP_ERR(M1, M2)                                                \
  MTX_ERROR("matrix (%d, %d, %d, %d) %s has a bad overlap with matrix "        \
            "(%d, %d, %d, %d) %s.",                                            \
            OVERLAP_ERR, (M1)->offY, (M1)->offX, (M1)->dy, (M1)->dx, #M1,      \
            (M2)->offY, (M2)->offX, (M2)->dy, (M2)->dx, #M2)

#define MTX_ENSURE_INIT(M)                                                     \
  if ((M)->data == NULL) {                                                     \
    MTX_NULL_ERR(M);                                                           \
  }

static mtx_error_t _error_generic(const char *s, const matrix_t *M,
                                  mtx_error_t error) {
  fprintf(stderr, s, M);
  return error;
}

#define MTX_MAX_ROWS 1024
#define MTX_MAX_COLUMNS 1024
#define MTX_MIN_DBL_VALUE 1e-300

#define _mod(x) ((x) < 0 ? -(x) : (x))

#define MTX_SET_DBL(dbl, value) dbl = value;

#define MTX_COMPARE_DBL(dbl1, dbl2) (_mod(dbl1 - dbl2) < MTX_MIN_DBL_VALUE)

#define MTX_IS_SQUARE(M) ((M)->dx == (M)->dy)

#define matrix_at(M, i, j) (M)->data->m[(M)->offY + i][(M)->offX + j]

#define matrix_row(M, i) &(M)->data->m[(M)->offY + i][(M)->offX]

#define MTX_SAME_DIMENSIONS(M1, M2)                                            \
  ((M1)->dx == (M2)->dx && (M1)->dy == (M2)->dy)

#define MTX_ARE_SHARED(M1, M2) ((M1)->data->m == (M2)->data->m)

#define MTX_ARE_SAME(M1, M2)                                                   \
  (MTX_ARE_SHARED(M1, M2) && MTX_SAME_DIMENSIONS(M1, M2) &&                    \
   (M1)->offY == (M2)->offY && (M1)->offX == (M2)->offX)

#define MTX_IS_VIEW(M)                                                         \
  ((M)->offX > 0 || (M)->offY > 0 || (M)->dy != (M)->data->size1 ||            \
   (M)->dx != (M)->data->size2)

#define MTX_OVERLAP_OF(M, M_OF)                                                \
  ((M)->offY + (M)->dy - 1 >= (M_OF)->offY &&                                  \
   (M)->offX + (M)->dx - 1 >= (M_OF)->offX)
#define MTX_OVERLAP(M1, M2)                                                    \
  (MTX_ARE_SHARED(M1, M2) && (MTX_OVERLAP_OF(M1, M2) && MTX_OVERLAP_OF(M2, M1)))

#define MTX_OVERLAP_AFTER(M1, M2)                                              \
  (MTX_OVERLAP(M1, M2) &&                                                      \
   ((M2)->offY > (M1)->offY ||                                                 \
    ((M2)->offX > (M1)->offX && (M2)->offY == (M1)->offY)))

// void matrix_init0(matrix_t *M, unsigned int dy, unsigned int dx) {
//   _m_init_dy(M, dy);
//   M->dx = dx;

//   for (int i = 0; i < dy; ++i) {
//     M->m[i] = (double *)calloc(dx, sizeof(double));
//   }
// }

// Cria um alias de uma matriz, copiando as bordas e referenciando os dados.
#define CREATE_OUTPUT_ALIAS(name, output) matrix_t name = *(output)

// Caso rules == true, executa action() com argumentos na va_args. Recebe uma
// matrix_t alias e uma matrix_t * output.
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
  GET_SAFE_OUTPUT_IN_DETAILS(alias, output, rules, matrix_init, (output)->dy,  \
                             (output)->dx)

// Caso output convegir com input, aloca uma nova matriz com as mesmas dimensões
// de input e guarda na matrix_t alias.
#define GET_SAFE_OUTPUT(alias, output, input)                                  \
  GET_SAFE_OUTPUT_RULES(alias, output, input, MTX_OVERLAP((input), (output)))

// Caso matrix_t alias e matrix_t * output não sejam a mesma matriz, copia o que
// tem em alias para output. Usar somente depois de CREATE_OUTPUT_ALIAS e
// GET_SAFE_OUTPUT*.
#define COMMIT_OUTPUT(alias, output)                                           \
  do {                                                                         \
    if (!MTX_ARE_SAME(&alias, output)) {                                       \
      matrix_copy(output, &alias);                                             \
      matrix_free(&alias);                                                     \
    }                                                                          \
  } while (0)

void matrix_init(matrix_t *_M, int dy, int dx) {
  assert(dy <= MTX_MAX_ROWS && dx <= MTX_MAX_COLUMNS);
  assert(dy > 0 && dx > 0);

  _M->data = (matrix_data_t *)malloc(sizeof(matrix_data_t));
  _M->data->size1 = dy;
  _M->data->size2 = dx;

  _M->dy = _M->data->size1;
  _M->dx = _M->data->size2;

  _M->offY = 0;
  _M->offX = 0;

  _M->data->m = (double **)malloc(sizeof(double *) * dy);
  for (int i = 0; i < dy; ++i) {
    _M->data->m[i] = (double *)malloc(sizeof(double) * dx);
  }
}

void matrix_free(matrix_t *M) {
  assert(!MTX_IS_VIEW(M));
  if (M == NULL || M->data == NULL) {
    return;
  }

  for (int i = 0; i < M->dy; ++i) {
    free(M->data->m[i]);
    M->data->m[i] = NULL;
  }

  free(M->data->m);

  M->data->m = NULL;
  free(M->data);
  M->data = NULL;
}

void matrix_set_identity(matrix_t *_M) {
  for (int i = 0; i < _M->dy; ++i) {
    int j = 0;
    for (; j < _M->dx; ++j) {
      matrix_at(_M, i, j) = 0;
    }
    if (j > i) {
      matrix_at(_M, i, i) = 1;
    }
  }
}

void matrix_init_perm(matrix_perm_t *_M_PERM, int d) {
  assert(d <= MTX_MAX_COLUMNS && d <= MTX_MAX_COLUMNS);

  matrix_init(_M_PERM, d, d);
  matrix_set_identity(_M_PERM);
}

void matrix_fill_a(matrix_t *M, double *array) {
  MTX_ENSURE_INIT(M);
  for (int i = 0; i < M->dy; ++i) {
    memcpy(matrix_row(M, i), array, sizeof(double) * M->dx);
    array = &array[M->dx];
  }
}

void matrix_fill_m(matrix_t *M, double **matrix) {
  MTX_ENSURE_INIT(M);
  for (int j = 0; j < M->dy; ++j) {
    memcpy(matrix_row(M, j), matrix[j], sizeof(double) * M->dx);
  }
}

void matrix_clone(matrix_t *_M, const matrix_t *M) {
  assert(_M != M);
  MTX_ENSURE_INIT(M);

  matrix_init(_M, M->dy, M->dx);

  for (int j = 0; j < _M->dy; ++j) {
    memcpy(matrix_row(_M, j), matrix_row(M, j), sizeof(double) * _M->dx);
  }
}

int matrix_copy(matrix_t *M_TO, const matrix_t *M_FROM) {
  MTX_ENSURE_INIT(M_FROM);
  MTX_ENSURE_INIT(M_TO);

  if (MTX_OVERLAP_AFTER(M_FROM, M_TO)) {
    MTX_OVERLAP_ERR(M_FROM, M_TO);
  }
  if (!MTX_SAME_DIMENSIONS(M_TO, M_FROM)) {
    MTX_DIMEN_ERR(M_TO);
  }
  for (int i = 0; i < M_TO->dy; ++i) {
    memcpy(matrix_row(M_TO, i), matrix_row(M_FROM, i),
           sizeof(double) * M_TO->dx);
  }
  return 0;
}
matrix_view_t matrix_view_of(const matrix_t *M_OF, int init_i, int init_j,
                             int dy, int dx) {
  MTX_ENSURE_INIT(M_OF);
  assert(init_i >= 0 && init_j >= 0);
  assert(dy > 0 && dx > 0);

  if (init_i + dy > M_OF->dy || init_j + dx > M_OF->dx) {
    MTX_BOUNDS_ERR(M_OF);
  }

  matrix_t mtx;
  mtx.data = M_OF->data;
  mtx.offY = M_OF->offY + init_i;
  mtx.offX = M_OF->offX + init_j;
  mtx.dy = dy;
  mtx.dx = dx;

  return (matrix_view_t){.matrix = mtx};
}

#define matrix_column_of(M_OF, j) matrix_view_of(M_OF, 0, j, (M_OF)->dy, 1)
#define matrix_row_of(M_OF, i) matrix_view_of(M_OF, i, 0, 1, (M_OF)->dx)

int matrix_copy_from(matrix_t *M_TO, const matrix_t *M_FROM, int init_i,
                     int init_j) {
  MTX_ENSURE_INIT(M_FROM);
  MTX_ENSURE_INIT(M_TO);
  assert(init_i >= 0 && init_j >= 0);

  // if (init_i + M_TO->dy > M_FROM->dy || init_j + M_TO->dx > M_FROM->dx) {
  //   return 1;
  // }
  matrix_view_t part_from =
      matrix_view_of(M_FROM, init_i, init_j, M_TO->dy, M_TO->dx);

  matrix_copy(M_TO, &part_from.matrix);
}

void print_matrix(const matrix_t *M) {

  printf("matrix (%u, %u):\n", M->dy, M->dx);
  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      printf("%g ", matrix_at(M, i, j));
    }
    printf("\n");
  }
}
void matrix_fprintf(FILE *stream, const matrix_t *M) {

  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      fprintf(stream, "%g ", matrix_at(M, i, j));
    }
    fprintf(stream, "\n");
  }
}

void matrix_fread(FILE *stream, matrix_t *_M) {
  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      fscanf(stream, "%lf", &matrix_at(_M, i, j));
    }
  }
}

int matrix_mul(matrix_t *_C, const matrix_t *A, const matrix_t *B) {

  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);

  if (A->dx != B->dy) {
    MTX_DIMEN_ERR(B);
  }

  if (_C->data == NULL) {
    matrix_init(_C, A->dy, B->dx);
  } else {

    if (_C->dx != B->dx || _C->dy != A->dy) {
      MTX_DIMEN_ERR(_C);
    }
  }

  CREATE_OUTPUT_ALIAS(mul_res, _C);

  GET_SAFE_OUTPUT_RULES(mul_res, _C, MTX_OVERLAP(_C, A) || MTX_OVERLAP(_C, B));

  int bx = 0, ay = 0;

  // Garantir que A e B não sejam modificados durante os cálculos, ou
  // undefined behavior
  for (; ay < A->dy; ++ay) {
    bx = 0;
    for (; bx < B->dx; ++bx) {
      int i = 0;

      double sum = 0;
      for (; i < A->dx; ++i) {
        sum += matrix_at(A, ay, i) * matrix_at(B, i, bx);
      }
      matrix_at(&mul_res, ay, bx) = sum;
    }
  }

  COMMIT_OUTPUT(mul_res, _C);
  return 0;
}

int matrix_s_mul(matrix_t *_M, const matrix_t *M, double scalar) {
  MTX_ENSURE_INIT(M);

  if (_M->data == NULL) {
    matrix_init(_M, M->dy, M->dx);
  } else if (!MTX_SAME_DIMENSIONS(_M, M)) {
    MTX_DIMEN_ERR(_M);
  }

  CREATE_OUTPUT_ALIAS(m_res, _M);

  GET_SAFE_OUTPUT_RULES(m_res, _M, MTX_OVERLAP_AFTER(M, _M));
  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      matrix_at(_M, i, j) = matrix_at(M, i, j) * scalar;
    }
  }

  return 0;
}

int matrix_transpose(matrix_t *_M, const matrix_t *M) {
  MTX_ENSURE_INIT(M);
  if (_M->data == NULL) {
    matrix_init(_M, M->dx, M->dy);
  } else if (_M->dy != M->dx || _M->dx != M->dy) {
    MTX_DIMEN_ERR(_M);
  }

  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      matrix_at(_M, i, j) = matrix_at(M, j, i);
    }
  }
  return 0;
}

static int _mtx_row_pivot(double *row, int start, int end) {
  int r = -1;
  for (int i = start; i < end; ++i) {
    if (row[i] != 0) {
      r = i;
      break;
    }
  }

  return r;
}

// Swapa a row alocada toda de uma vez (swapando ponteiros)
static void _mtx_row_swap_restrict(matrix_t *_M, int r1, int r2) {
  assert(r1 != r2);
  assert(r1 >= 0 && r2 >= 0);
  double *tmp = _M->data->m[r1];
  _M->data->m[r1] = _M->data->m[r2];
  _M->data->m[r2] = tmp;
}

static void _mtx_row_swap(matrix_t *_M, int r1, int r2) {
  assert(r1 != r2);
  assert(r1 >= 0 && r2 >= 0);

  // Caso matriz não seja view ou seja uma view que contemple uma row alocada
  // inteira.
  if (_M->dx == _M->data->size2) {
    _mtx_row_swap_restrict(_M, r1, r2);
    return;
  }

  // Swapa elemento por elemento da linha.
  double tmp;
  double *row1 = matrix_row(_M, r1);
  double *row2 = matrix_row(_M, r2);
  for (int i = 0; i < _M->dx; ++i) {
    tmp = row1[i];
    row1[i] = row2[i];
    row2[i] = tmp;
  }
}

static inline void _mtx_row_copy(double *r_to, double *r_from, int count) {
  if (r_to != r_from) {
    memcpy(r_to, r_from, sizeof(double) * count);
  }
}

static inline void _mtx_row_mul(double *r, double mul, int count) {
  for (int i = 0; i < count; ++i) {
    r[i] *= mul;
  }
}
static inline void _mtx_sum_multiple(double *r, double *mul_r, double mul,
                                     int start, int end) {
  // r[init_pos] = 0;
  for (int i = start; i < end; ++i) {
    MTX_SET_DBL(r[i], r[i] + mul_r[i] * mul);
  }
}

int matrix_equals(const matrix_t *A, const matrix_t *B) {
  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (MTX_ARE_SAME(A, B)) {
    return 1;
  }
  if (!MTX_SAME_DIMENSIONS(A, B)) {
    return 0;
  }

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      if (!MTX_COMPARE_DBL(matrix_at(A, i, j), matrix_at(B, i, j))) {
        return 0;
      }
    }
  }

  return 1;
}

#define PIVOT(i) row_pivot[(i)]

#define ROW_ECHELON(r) ((r) > last_w_row || row_pivot[(r)] == (r))
#define SWAP(r1, r2)                                                           \
  _mtx_row_swap(_M_LU, (r1), (r2));                                            \
  if (permutate) {                                                             \
    _mtx_row_swap(__M_PERM, (r1), (r2));                                       \
  }                                                                            \
  odd_swaps = !odd_swaps;                                                      \
  int _pivot = PIVOT((r1));                                                    \
  row_pivot[(r1)] = PIVOT((r2));                                               \
  row_pivot[(r2)] = _pivot;

// Recalcular o valor do pivot. Retorna erro caso seja toda zerada ou o pivot
// não corresponder a uma coluna válida (caso dx > dy).
#define SET_PIVOT_perf(i, start)                                               \
  if ((row_pivot[(i)] =                                                        \
           _mtx_row_pivot(matrix_row(_M_LU, (i)), (start), _M_LU->dy)) < 0) {  \
    return -1;                                                                 \
  }

#define SET_PIVOT_INIT_perf(i) SET_PIVOT_perf(i, 0)

// Recalcular o valor do pivot, caso a linha seja toda zerada, swapar com a
// última linha não zerada. Retorna erro se o pivot não corresponder a uma
// coluna válida (caso dx > dy).
#define SET_PIVOT(i, start)                                                    \
  if ((row_pivot[(i)] =                                                        \
           _mtx_row_pivot(matrix_row(_M_LU, (i)), (start), _M_LU->dx)) < 0) {  \
    if ((i) == last_w_row) {                                                   \
      --last_w_row;                                                            \
      break;                                                                   \
    }                                                                          \
    SWAP(i, last_w_row);                                                       \
    --last_w_row;                                                              \
  } else if (PIVOT(i) > last_w_row) {                                          \
    return -1;                                                                 \
  }

#define SET_PIVOT_INIT(i)                                                      \
  while ((row_pivot[(i)] =                                                     \
              _mtx_row_pivot(_M_LU->data->m[(i)], 0, _M_LU->dx)) < 0) {        \
    row_pivot[last_w_row] =                                                    \
        _mtx_row_pivot(_M_LU->data->m[last_w_row], 0, _M_LU->dx);              \
    if (i == last_w_row) {                                                     \
      --last_w_row;                                                            \
      break;                                                                   \
    }                                                                          \
    SWAP(i, last_w_row);                                                       \
    --last_w_row;                                                              \
  }                                                                            \
  if (PIVOT(i) > last_w_row) {                                                 \
    return -1;                                                                 \
  }

int matrix_LU_decomposition(matrix_perm_t *__M_PERM, matrix_t *_M_LU,
                            const matrix_t *M, int perfect) {
  MTX_ENSURE_INIT(M);
  // caso perfect == true, linhas zeradas serão consideradas erro. Com dx <
  // dy, isso inevitavelmente ocorrerá.
  if (perfect && M->dx < M->dy) {
    return -1;
  }

  if (_M_LU->data == NULL) {
    matrix_clone(_M_LU, M);
  } else if (!MTX_ARE_SAME(_M_LU, M) && matrix_copy(_M_LU, M) != 0) {
    MTX_DIMEN_ERR(_M_LU);
  }

  int permutate = 0;
  if (__M_PERM != NULL) {
    permutate = 1;
    if (__M_PERM->data == NULL) {
      matrix_init_perm(__M_PERM, _M_LU->dy);
    } else if (!MTX_OVERLAP(__M_PERM, _M_LU) && MTX_IS_SQUARE(__M_PERM) &&
               __M_PERM->dy == _M_LU->dy) {
      matrix_set_identity(__M_PERM);
    } else {
      MTX_INVALID_ERR(__M_PERM);
    }
  }

  // 0 se número de swaps é par e 1 se for ímpar.
  int odd_swaps = 0;

  // O pivot de cada linha é armazenado para não ter que calcular toda vez.

  int row_pivot[MTX_MAX_ROWS];

  // A última linha que faz sentido processar com swaps ou com somas (depois
  // dela vem as linhas zeradas ou nada).
  int last_w_row = _M_LU->dy - 1;

  int pivot;

  if (!perfect) {
    for (int i = 0; i <= last_w_row; ++i) {
      SET_PIVOT_INIT(i);
    }
  } else {
    for (int i = 0; i <= last_w_row; ++i) {
      SET_PIVOT_INIT_perf(i);
    }
  }

  for (int p = 0; p <= last_w_row; ++p) {
    int pivot;
    while ((pivot = PIVOT(p)) > p) {

      // Swapa os números da linha atual com a linha que eles deveriam estar
      // (ideal), caso ela ja não esteja resolvida.
      if (!ROW_ECHELON(pivot)) {
        SWAP(p, pivot);
        continue;
      }

      // Swapa com qualquer outra linha que faça sentido (que vá resolver ou
      // mover o pivot para antes da diagonal)
      int pn;
      for (pn = p + 1; pn <= last_w_row; ++pn) {
        if (ROW_ECHELON(pn)) {
          continue;
        }

        if (PIVOT(pn) == p) {
          SWAP(p, pn);
          break;
        }
      }

      // Caso não seja possível fazer swap com nenhuma linha (significa sem
      // solução)
      if (pn > last_w_row) {
        return -1;
      }
    }

    // Garantir que o pivot seja o maior número em módulo da coluna para
    // aumentar a precisão dos cálculos.
    double pp = matrix_at(_M_LU, p, p);
    double max = _mod(pp);
    int i_max = p;

    for (int ic = p + 1; ic <= last_w_row; ++ic) {
      double mod_pivc = _mod(matrix_at(_M_LU, ic, p));
      if (mod_pivc > max) {
        i_max = ic;
        max = mod_pivc;
      }
    }
    if (i_max != p) {
      SWAP(p, i_max);
      pp = matrix_at(_M_LU, p, p);
    }

    // Reduz todas as linhas abaixo, substituindo todos os valores na mesma
    // coluna do pivot.
    for (int ic = p + 1; ic <= last_w_row; ++ic) {
      double ip = matrix_at(_M_LU, ic, p);

      if (ip == 0) {
        continue;
      }

      double mul = ip / pp; // abs(pp) >= abs(ip)
      matrix_at(_M_LU, ic, p) = mul;

      _mtx_sum_multiple(matrix_row(_M_LU, ic), matrix_row(_M_LU, p), -mul,
                        p + 1, _M_LU->dx);

      if (!perfect) {
        SET_PIVOT(ic, p + 1);
      } else {
        SET_PIVOT_perf(ic, p + 1);
      }
    }
  }

  return odd_swaps;
}

// Transforma uma matriz mxn na forma reduzida. O algoritmo vai tentar reduzir a
// matriz 100%, permitindo linhas zeradas e movendo-as para o fim da matriz.
// Retorna o signum >= 0 se sucesso e negativo caso haja falha.
#define matrix_LU_decomp(__M_PERM, _M, M)                                      \
  matrix_LU_decomposition((__M_PERM), (_M), (M), 0)

// Exatamente como matrix_echelon() porém otimizada para determinantes (se
// falhar, a submatriz mxm de uma matriz mxn tem determinante zero). Na prática,
// falha ao encontrar linhas zeradas e sempre falhará se n < m. Retorna o signum
// 0 ou 1 se sucesso e -1 caso haja falha.
#define matrix_LU_decomp_perf(__M_PERM, _M, M)                                 \
  matrix_LU_decomposition((__M_PERM), (_M), (M), 1)

#undef DEF_ECH
#undef PIVOT
#undef ROW_ECHELON
#undef SWAP
#undef SET_PIVOT
#undef SET_PIVOT_perf

// Permuta a matriz M de acordo com a matriz de permutação M_PERM. O resultado
// final é o mesmo de M_PERM x M, porém é mais otimizado para tal.
int matrix_permutate(matrix_t *_M, const matrix_t *M,
                     const matrix_perm_t *M_PERM) {
  MTX_ENSURE_INIT(M);
  MTX_ENSURE_INIT(M_PERM);

  assert(MTX_IS_SQUARE(M_PERM));

  if (M_PERM->dy != M->dy) {
    MTX_DIMEN_ERR(M);
  }

  if (_M->data == NULL) {
    matrix_init(_M, M->dy, M->dx);
  }
  if (!MTX_SAME_DIMENSIONS(_M, M)) {
    MTX_DIMEN_ERR(_M);
  }

  CREATE_OUTPUT_ALIAS(permutated, _M);

  GET_SAFE_OUTPUT_RULES(permutated, _M,
                        MTX_OVERLAP(_M, M) || MTX_OVERLAP(_M, M_PERM));

  int pivot;
  int lower_pivot = 0;
  int higher_pivot = M_PERM->dx - 1;
  for (int i = 0; i < M_PERM->dy; ++i) {
    pivot =
        _mtx_row_pivot(matrix_row(M_PERM, i), lower_pivot, higher_pivot + 1);
    if (pivot == lower_pivot) {
      ++lower_pivot;
    }
    if (pivot == higher_pivot) {
      --higher_pivot;
    }
    _mtx_row_copy(matrix_row(&permutated, i), matrix_row(M, pivot),
                  permutated.dx);
  }

  COMMIT_OUTPUT(permutated, _M);
  return 0;
}

// Calcula o determinante de uma dada matriz na forma decomposta. A
// matriz não necessariamente precisa ser quadrada: se a matriz for mxn, a
// submatriz mxm será considerada. Caso a n < m ou a matriz não estiver
// decomposta: undefined behavior. Recomendado usar somente combinado com
// matrix_LU_decomp_perf().
double matrix_det_LU(const matrix_t *M_LU, int signum) {

  MTX_ENSURE_INIT(M_LU);
  double det = 1;
  if (signum >= 0) {

    for (int p = 0; p < M_LU->dy; ++p) {
      det *= matrix_at(M_LU, p, p);
    }

    if (signum) {
      det *= -1;
    }
  } else {
    det = 0;
  }

  return det;
}

// Calcula o determinante de uma dada matriz quadrada M. Retorna zero se a
// matriz não for quadrada (para mxn e n > m, use matrix_det_LU()).
double matrix_det(const matrix_t *M) {
  MTX_ENSURE_INIT(M);
  if (!MTX_IS_SQUARE(M)) {
    MTX_DIMEN_ERR(M);
  }

  matrix_t lu = {0};

  // matrix_LU_decomp_perf() é a mais rápida pro calculo de determinante pois
  // evita cálculos desnecessários.
  int signum = matrix_LU_decomp_perf(NULL, &lu, M);
  double det = matrix_det_LU(&lu, signum);

  matrix_free(&lu);
  return det;
}

#define CHECK_SOLVER(read_mtx)                                                 \
  do {                                                                         \
    if (var_num < dy) {                                                        \
      return -1;                                                               \
    }                                                                          \
    if ((read_mtx)->data->m[var_num - 1][var_num - 1] == 0) {                  \
      return -1;                                                               \
    }                                                                          \
  } while (0)

// Realiza a back substitution (debaixo pra cima) do sistema Ux = B. A matriz
// U é uma matriz quadrada upper triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int matrix_back_subs(matrix_t *_X, const matrix_t *U, const matrix_t *B,
                     int jordan) {
  MTX_ENSURE_INIT(U);
  MTX_ENSURE_INIT(B);
  if (!MTX_IS_SQUARE(U) || U->dy != B->dy) {
    MTX_DIMEN_ERR(U);
  }

  int dx = U->dx;
  int dy = U->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (matrix_at(U, dy - 1, dx - 1) == 0) {
    return 1;
  }

  if (_X->data == NULL) {
    matrix_init(_X, var_num, B->dx);
  } else if (!MTX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  CREATE_OUTPUT_ALIAS(x, _X);

  GET_SAFE_OUTPUT_RULES(x, _X,
                        (MTX_OVERLAP(_X, B) && (_X->offY < B->offY)) ||
                            (MTX_OVERLAP(_X, U) && (_X->offY <= U->offY)));

  double *U_i;
  double *X_i;
  for (int i = var_num - 1; i >= 0; --i) {
    U_i = matrix_row(U, i);
    X_i = matrix_row(&x, i);

    _mtx_row_copy(X_i, matrix_row(B, i), x.dx);
    for (int j = i + 1; j < var_num; ++j) {
      _mtx_sum_multiple(X_i, matrix_row(&x, j), -U_i[j], 0, x.dx);
    }

    if (!jordan) {
      double pivot = U_i[i];
      _mtx_row_mul(matrix_row(&x, i), 1.0 / pivot, x.dx);
    }
  }

  COMMIT_OUTPUT(x, _X);

  return 0;
}

// Realiza a forward substitution (de cima para baixo) do sistema Lx = B. A
// matriz L é uma lower triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int matrix_forward_subs(matrix_t *_X, const matrix_t *L, const matrix_t *B,
                        int jordan) {

  MTX_ENSURE_INIT(L);
  MTX_ENSURE_INIT(B);
  if (B->dy != L->dy) {
    MTX_DIMEN_ERR(L);
  }
  if (_X->data == NULL) {
    matrix_init(_X, B->dy, B->dx);
  } else if (!MTX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  int dx = L->dx;
  int dy = L->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (var_num > dy || matrix_at(L, var_num - 1, var_num - 1) == 0) {
    return 1;
  }

  CREATE_OUTPUT_ALIAS(x, _X);

  GET_SAFE_OUTPUT_RULES(x, _X,
                        (MTX_OVERLAP(_X, B) && (_X->offY > B->offY)) ||
                            (MTX_OVERLAP(_X, L) && (_X->offY >= L->offY)));

  double *L_i;
  double *X_i;
  for (int i = 0; i < var_num; ++i) {
    L_i = matrix_row(L, i);
    X_i = matrix_row(&x, i);

    _mtx_row_copy(X_i, matrix_row(B, i), x.dx);
    for (int j = 0; j < i; ++j) {
      _mtx_sum_multiple(X_i, matrix_row(&x, j), -L_i[j], 0, x.dx);
    }

    if (!jordan) {
      double diagn = L_i[i];
      _mtx_row_mul(matrix_row(&x, i), 1.0 / diagn, x.dx);
    }
  }

  // Caso dy > dx, continua a substituir os elementos abaixo.
  for (int i = var_num; i < L->dy; ++i) {
    L_i = matrix_row(L, i);
    X_i = matrix_row(&x, i);

    _mtx_row_copy(X_i, matrix_row(B, i), x.dx);
    for (int j = 0; j < L->dx; ++j) {
      _mtx_sum_multiple(X_i, matrix_row(&x, j), -L_i[j], 0, x.dx);
    }
  }

  COMMIT_OUTPUT(x, _X);

  return 0;
}

// Resolve o sistema linear Ax = B, representado pela matriz aumentada A_LU. A
// matriz A_LU tem que estar previamente decomposta. Como as icógnitas podem
// representar vetores (nesse caso, o resultado X é uma matriz e não um
// vetor), B será separado de A nas ultimas colunas de A_LU de acordo com
// X->dx.
//
// Falha caso o sistema seja impossível, indeterminado ou caso não seja
// possível separar A de B (X-dx > A_LU->dx - 1).
int matrix_LU_AB_solve(matrix_t *X, const matrix_t *AB_LU) {

  MTX_ENSURE_INIT(AB_LU);
  MTX_ENSURE_INIT(X);
  int var_num = AB_LU->dx - X->dx;
  if (var_num < 1 || X->dy < var_num) {
    MTX_BOUNDS_ERR(X);
  }

  matrix_view_t B = matrix_view_of(AB_LU, 0, var_num, AB_LU->dy, X->dx);

  // Sistema impossível
  if (AB_LU->dy > var_num) {
    for (int bi = var_num; bi < B.matrix.dy; ++bi) {
      for (int bj = 0; bj < B.matrix.dx; ++bj) {
        if (matrix_at(&B.matrix, bi, bj) != 0) {
          return 1;
        }
      }
    }
  }

  matrix_view_t A = matrix_view_of(AB_LU, 0, 0, var_num, var_num);

  // B->dy = A->dx = A->dy
  B.matrix.dy = A.matrix.dx;

  return matrix_back_subs(X, &A.matrix, &B.matrix, 0);
}

// Resolve o sistema linear Ax = B, representado pela matriz decomposta de A
// em A_LU e pela matriz B. Tanto A_LU quanto B precisam ter o mesmo número de
// linhas.
//
// Falha caso o sistema seja indeterminado.
int matrix_LU_solve(matrix_t *_X, const matrix_perm_t *M_PERM,
                    const matrix_t *A_LU, const matrix_t *B) {
  MTX_ENSURE_INIT(A_LU);
  MTX_ENSURE_INIT(B);

  if (A_LU->dy != B->dy) {
    MTX_DIMEN_ERR(B);
  }

  if (_X->data == NULL) {
    matrix_init(_X, B->dy, B->dx);
  } else if (!MTX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  matrix_permutate(_X, B, M_PERM);
  if (matrix_forward_subs(_X, A_LU, _X, 1) != 0) {
    return 1;
  }
  if (matrix_back_subs(_X, A_LU, _X, 0) != 0) {
    return 1;
  }

  return 0;
}

// Calcula e retorna o grau de diferença entre as matrizes A e B. Retorna
// sempre um número positivo, exceto na falha na qual o número retornado é
// negativo.
double matrix_distance(const matrix_t *A, const matrix_t *B) {

  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (!MTX_SAME_DIMENSIONS(A, B)) {
    MTX_DIMEN_ERR(B);
  }
  if (MTX_ARE_SAME(A, B)) {
    return 0;
  }

  double dt = 0;

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      dt += _mod(matrix_at(A, i, j) - matrix_at(B, i, j));
    }
  }

  return dt;
}

// Subtrai a matriz A da matriz B com o resultado em _M_D e retorna o
// somátorio dos módulos de cada elemento de _M_D, que é sempre um número
// positivo. O valor retornado é exatamente a distancia entre as duas
// matrizes, sendo equivalente ao retornado por matrix_distance().
//
// Retorna zero se as matrizes forem idênticas e negativo caso ocorra erro. Em
// ambos os casos, _M_DU pode não conter a subtração de A e B, portanto, deve
// ser ignorado.
double matrix_distance_each(matrix_t *_M_D, const matrix_t *A,
                            const matrix_t *B) {

  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (!MTX_SAME_DIMENSIONS(A, B)) {
    MTX_DIMEN_ERR(A);
  }
  if (MTX_ARE_SAME(A, B)) {
    return 0;
  }

  if (_M_D->data == NULL) {
    matrix_init(_M_D, A->dy, A->dx);
  } else if (!MTX_SAME_DIMENSIONS(_M_D, A)) {
    MTX_DIMEN_ERR(_M_D);
    return -1;
  }

  CREATE_OUTPUT_ALIAS(m_d, _M_D);
  GET_SAFE_OUTPUT_RULES(
      m_d, _M_D, MTX_OVERLAP_AFTER(A, _M_D) || MTX_OVERLAP_AFTER(B, _M_D));

  double dt = 0;

  double diff;
  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      diff = matrix_at(A, i, j) - matrix_at(B, i, j);
      matrix_at(&m_d, i, j) = diff;
      dt += _mod(diff);
    }
  }

  COMMIT_OUTPUT(m_d, _M_D);

  return dt;
}
#define DEF_MTX_SIMPLE_OP(name, operation)                                     \
  int matrix_##name(matrix_t *_C, matrix_t *A, matrix_t *B) {                  \
                                                                               \
    MTX_ENSURE_INIT(A);                                                        \
    MTX_ENSURE_INIT(B);                                                        \
    if (!MTX_SAME_DIMENSIONS(A, B)) {                                          \
      MTX_DIMEN_ERR(A);                                                        \
    }                                                                          \
                                                                               \
    if (_C->data == NULL) {                                                    \
      matrix_init(_C, A->dy, A->dx);                                           \
    } else if (!MTX_SAME_DIMENSIONS(_C, A)) {                                  \
      MTX_DIMEN_ERR(_C);                                                       \
    }                                                                          \
                                                                               \
    for (int i = 0; i < A->dy; ++i) {                                          \
      for (int j = 0; j < A->dx; ++j) {                                        \
        matrix_at(_C, i, j) = matrix_at(A, i, j) operation matrix_at(B, i, j); \
      }                                                                        \
    }                                                                          \
    return 0;                                                                  \
  }

DEF_MTX_SIMPLE_OP(add, +);
DEF_MTX_SIMPLE_OP(sub, -);

// Refina a solução do sistema linear Ax = B. Recebe _M_WORK como matriz para
// cálculos intermediários, X a solução atual, M_PERM e A_LU como sendo a matriz
// de permutação e A decomposto e por fim a matriz A e B originais. No fim da
// execução, X será substituído por uma versão mais próxima da solução exata.
//
//
// Dado que a resolução de um sistema linear que gerou resíduos: Ax' = B + B', a
// função retorna a distancia entre B e B', indicando o grau de distancia que a
// solução exata x tem de x'.
double matrix_LU_refine(matrix_t *_M_WORK, matrix_t *X,
                        const matrix_perm_t *M_PERM, const matrix_t *A_LU,
                        const matrix_t *A, const matrix_t *B) {

  matrix_mul(_M_WORK, A, X);
  double dt;
  if ((dt = matrix_distance_each(_M_WORK, _M_WORK, B)) == 0) {
    return 0;
  }

  if (matrix_LU_solve(_M_WORK, M_PERM, A_LU, _M_WORK) != 0) {
    MTX_INVALID_ERR(X);
  }
  matrix_sub(X, X, _M_WORK);

  return dt;
}

#endif
