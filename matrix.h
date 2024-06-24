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

typedef struct vector {
  double *m;
  int d;
} vector_t;

#define MTX_MAX_ROWS 1024
#define MTX_MAX_COLUMNS 1024
#define _mod(x) ((x) < 0 ? -(x) : (x))

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
  ((M)->offY + (M)->dy >= (M_OF)->offY && (M)->offX + (M)->dx >= (M_OF)->offX)
#define MTX_OVERLAP(M1, M2)                                                    \
  (MTX_ARE_SHARED(M1, M2) && (MTX_OVERLAP_OF(M1, M2) && MTX_OVERLAP_OF(M2, M1)))

// void matrix_init0(matrix_t *M, unsigned int dy, unsigned int dx) {
//   _m_init_dy(M, dy);
//   M->dx = dx;

//   for (int i = 0; i < dy; ++i) {
//     M->m[i] = (double *)calloc(dx, sizeof(double));
//   }
// }
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
void vector_init(vector_t *V, int d) {
  assert(d <= MTX_MAX_COLUMNS);

  V->m = (double *)malloc(sizeof(double) * d);
  V->d = d;
}

void vector_free(vector_t *V) {
  if (V == NULL || V->m == NULL) {
    return;
  }
  free(V->m);
}

void matrix_fill_a(matrix_t *_M, double *array) {
  assert(_M->data->m != NULL);
  for (int i = 0; i < _M->dy; ++i) {
    memcpy(matrix_row(_M, i), array, sizeof(double) * _M->dx);
    array = &array[_M->dx];
  }
}

void vector_fill_a(vector_t *_V, double *array) { memcpy(_V->m, array, _V->d); }

void matrix_fill_m(matrix_t *_M, double **matrix) {
  assert(_M->data->m != NULL && matrix != NULL);
  assert(_M->offX == 0);
  for (int j = 0; j < _M->dy; ++j) {
    memcpy(_M->data->m[j], matrix[j], sizeof(double) * _M->dx);
  }
}

void vector_fill_m(vector_t *_V, double **matrix, int dy) {
  assert(dy <= _V->d);
  int amt = _V->d / dy;
  for (int i = 0; i < dy; ++i) {
    memcpy(&_V->m[amt * i], matrix[i], amt);
  }
}

void matrix_clone(matrix_t *_M, const matrix_t *M) {
  assert(_M != M && M->data->m != NULL);

  matrix_init(_M, M->dy, M->dx);
  matrix_fill_m(_M, M->data->m);
}

int matrix_copy(matrix_t *M_TO, const matrix_t *M_FROM) {
  assert(M_FROM->data->m != NULL && M_TO->data->m != NULL);

  if (MTX_OVERLAP(M_TO, M_FROM)) {
    return 1;
  }
  if (!MTX_SAME_DIMENSIONS(M_TO, M_FROM)) {
    return 1;
  }
  for (int i = 0; i < M_TO->dy; ++i) {
    memcpy(matrix_row(M_TO, i), matrix_row(M_FROM, i),
           sizeof(double) * M_TO->dx);
  }
  return 0;
}
matrix_view_t matrix_view_of(const matrix_t *M_OF, int init_i, int init_j,
                             int dy, int dx) {
  assert(M_OF->data->m != NULL);
  assert(init_i >= 0 && init_j >= 0);
  assert(dy >= 0 && dy >= 0);

  if (init_i + dy > M_OF->dy || init_j + dx > M_OF->dx) {
    return (matrix_view_t){{0}};
  }

  matrix_t mtx;
  mtx.data = M_OF->data;
  mtx.offY = M_OF->offY + init_i;
  mtx.offX = M_OF->offX + init_j;
  mtx.dy = dy;
  mtx.dx = dx;

  return (matrix_view_t){.matrix = mtx};
}

int matrix_copy_from(matrix_t *M_TO, const matrix_t *M_FROM, int init_i,
                     int init_j) {
  assert(M_FROM->data->m != NULL && M_TO->data->m != NULL);
  assert(init_i >= 0 && init_j >= 0);

  // if (init_i + M_TO->dy > M_FROM->dy || init_j + M_TO->dx > M_FROM->dx) {
  //   return 1;
  // }
  matrix_view_t part_from =
      matrix_view_of(M_FROM, init_i, init_j, M_TO->dy, M_TO->dx);
  if (part_from.matrix.data == NULL) {
    return 1;
  }

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

int matrix_mul(matrix_t *_C, const matrix_t *A, const matrix_t *B) {

  assert(A->data->m != NULL && B->data->m != NULL);
  if (A->dx != B->dy) {
    return 1;
  }

  matrix_t mul_res;

  if (_C->data == NULL) {
    matrix_init(_C, A->dy, B->dx);
    mul_res = *_C;
  } else {

    if (_C->dx != B->dx || _C->dy != A->dy) {
      return 1;
    }

    // Se a matriz _C convegir com a matrix A ou B, realizar cálculos em uma
    // nova matriz temporária e depois copiar o resultado para _C
    if (MTX_OVERLAP(_C, A) || MTX_OVERLAP(_C, B)) {
      matrix_init(&mul_res, _C->dy, _C->dx);
    } else {
      mul_res = *_C;
    }
  }

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

  if (!MTX_ARE_SAME(_C, &mul_res)) {
    // Copiar o que está na matriz temporária para _C
    matrix_copy(_C, &mul_res);
    matrix_free(&mul_res);
  }

  return 0;
}

int matrix_s_mul(matrix_t *_M, const matrix_t *M, double scalar) {
  assert(M != NULL && M->data->m != NULL);

  if (_M->data->m == NULL) {
    matrix_init(_M, M->dy, M->dx);
  } else if (!MTX_SAME_DIMENSIONS(_M, M)) {
    return 1;
  }
  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      matrix_at(_M, i, j) = matrix_at(M, i, j) * scalar;
    }
  }

  return 0;
}

int matrix_transpose(matrix_t *_M, const matrix_t *M) {
  assert(M != NULL && M->data->m != NULL);

  if (_M->data == NULL) {
    matrix_init(_M, M->dx, M->dy);
  } else if (_M->dy != M->dx || _M->dx != M->dy) {
    return 1;
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
    r[i] += mul_r[i] * mul;
  }
}

int matrix_equals(const matrix_t *A, const matrix_t *B) {
  assert(A->data->m != NULL && B->data->m != NULL);
  if (MTX_ARE_SAME(A, B)) {
    return 1;
  }
  if (!MTX_SAME_DIMENSIONS(A, B)) {
    return 0;
  }

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      if (matrix_at(A, i, j) != matrix_at(B, i, j)) {
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
  assert(M->data->m != NULL);

  // caso perfect == true, linhas zeradas serão consideradas erro. Com dx < dy,
  // isso inevitavelmente ocorrerá.
  if (perfect && M->dx < M->dy) {
    return -1;
  }

  if (_M_LU->data == NULL) {
    matrix_clone(_M_LU, M);
  } else if (!MTX_ARE_SAME(_M_LU, M) && matrix_copy(_M_LU, M) != 0) {
    return -1;
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
      return -1;
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

  for (int i = 0; i <= last_w_row; ++i) {
    while (!ROW_ECHELON(i)) {
      pivot = PIVOT(i);

      // Apenas se tiver mais zeros passando a diagonal.
      if (pivot > i) {
        // Swapa os números da linha atual com a linha que eles deveriam estar
        // (ideal), caso ela ja não esteja resolvida.
        if (!ROW_ECHELON(pivot)) {
          SWAP(i, pivot);
          continue;
        }

        // Swapa com qualquer outra linha que faça sentido (que vá resolver ou
        // mover o pivot para antes da diagonal)
        int p;
        for (p = i + 1; p <= last_w_row; ++p) {
          if (ROW_ECHELON(p)) {
            continue;
          }

          if (PIVOT(p) <= i) {
            SWAP(i, p);
            break;
          }
        }

        // Caso não seja possível fazer swap com nenhuma linha (significa sem
        // solução)
        if (p > last_w_row) {
          return -1;
        }

        // Caso o pivot esteja antes da diagonal, realizar as somas com outra
        // linha já resolvida para reduzir.
      } else {
        // Pega a linha que tem o pivot na mesma coluna do pivot da linha
        // atual.
        int prev_i = pivot;

        // Caso a eliminação precise dividir um número maior que o menor, troca
        // as linhas para evitar erros de precisão.
        // if (_mod(matrix_at(_M_LU, prev_i, prev_i)) <
        //     _mod(matrix_at(_M_LU, i, pivot))) {
        //   SWAP(i, prev_i);
        // }

        double mul =
            matrix_at(_M_LU, i, pivot) / matrix_at(_M_LU, prev_i, prev_i);

        matrix_at(_M_LU, i, pivot) = mul;
        _mtx_sum_multiple(_M_LU->data->m[i], _M_LU->data->m[prev_i], -mul,
                          pivot + 1, _M_LU->dx);

        if (!perfect) {
          SET_PIVOT(i, pivot + 1);
        } else {
          SET_PIVOT_perf(i, pivot + 1);
        }
      }
    }
  }

  return odd_swaps;
}

// Transforma uma matriz mxn na forma reduzida. O algoritmo vai tentar reduzir a
// matriz 100%, permitindo linhas zeradas e movendo-as para o fim da matriz.
// Retorna o signum 0 ou 1 se sucesso e -1 caso haja falha.
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

// Calcula o determinante de uma dada matriz na forma decomposta. A
// matriz não necessariamente precisa ser quadrada: se a matriz for mxn, a
// submatriz mxm será considerada. Caso a n < m ou a matriz não estiver
// decomposta: undefined behavior. Recomendado usar somente combinado com
// matrix_LU_decomp_perf().
double matrix_det_LU(const matrix_t *M_LU, int signum) {
  assert(M_LU->data->m != NULL);

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
  assert(M->data->m != NULL);

  if (!MTX_IS_SQUARE(M)) {
    return 0;
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

// Realiza a back substituition (debaixo pra cima) do sistema Ux = B. A matriz U
// é uma upper triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int matrix_back_subs(matrix_t *X, const matrix_t *U, const matrix_t *B,
                     int jordan) {
  assert(U->data != NULL && B->data != NULL && X->data != NULL);

  if (!MTX_SAME_DIMENSIONS(X, B)) {
    return 1;
  }

  // Caso escrever em X vai alterar alterar alguma linha não processada (linha
  // atual ou acima) de U.
  if (MTX_OVERLAP(X, U) && (X->offY <= U->offY)) {
    return 1; // TODO: Impl operação atômica.
  }

  // Caso escrever em X vai alterar alguma linha não processada (linha acima) de
  // B.
  if (MTX_OVERLAP(X, B) && (X->offY < B->offY)) {
    return 1; // TODO: Impl operação atômica
  }

  int dx = U->dx;
  int dy = U->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (var_num > dy || matrix_at(U, var_num - 1, var_num - 1) == 0) {
    return 1;
  }

  double *U_i;
  double *X_i;
  for (int i = var_num - 1; i >= 0; --i) {
    U_i = matrix_row(U, i);
    X_i = matrix_row(X, i);

    _mtx_row_copy(X_i, matrix_row(B, i), X->dx);
    for (int j = i + 1; j < var_num; ++j) {
      _mtx_sum_multiple(X_i, matrix_row(X, j), -U_i[j], 0, X->dx);
    }

    if (!jordan) {
      double pivot = U_i[i];
      _mtx_row_mul(matrix_row(X, i), 1.0 / pivot, X->dx);
    }
  }

  return 0;
}

// Realiza a forward substituition (de cima para baixo) do sistema Lx = B. A
// matriz L é uma lower triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int matrix_forward_subs(matrix_t *X, const matrix_t *L, const matrix_t *B,
                        int jordan) {

  assert(L->data != NULL && B->data != NULL && X->data != NULL);

  if (!MTX_SAME_DIMENSIONS(X, B)) {
    return 1;
  }
  // Caso escrever em X vai alterar alterar alguma linha não processada (linha
  // atual ou abaixo) de L.
  if (MTX_OVERLAP(X, L) && (X->offY >= L->offY)) {
    return 1; // TODO: Impl operação atômica.
  }

  // Caso escrever em X vai alterar alguma linha não processada (linha abaixo)
  // de B.
  if (MTX_OVERLAP(X, B) && (X->offY > B->offY)) {
    return 1; // TODO: Impl operação atômica
  }

  int dx = L->dx;
  int dy = L->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (var_num > dy || matrix_at(L, var_num - 1, var_num - 1) == 0) {
    return 1;
  }

  double *L_i;
  double *X_i;
  for (int i = 0; i < var_num; ++i) {
    L_i = matrix_row(L, i);
    X_i = matrix_row(X, i);

    _mtx_row_copy(X_i, matrix_row(B, i), X->dx);
    for (int j = 0; j < i; ++j) {
      _mtx_sum_multiple(X_i, matrix_row(X, j), -L_i[j], 0, X->dx);
    }

    if (!jordan) {
      double diagn = L_i[i];
      _mtx_row_mul(matrix_row(X, i), 1.0 / diagn, X->dx);
    }
  }

  return 0;
}

// Resolve o sistema linear Ax = B, representado pela matriz aumentada M_LU. A
// matriz M_LU tem que estar previamente decomposta. Como as icógnitas podem
// representar vetores (nesse caso, o resultado X é uma matriz e não um vetor),
// B será separado de A nas ultimas colunas de M_LU de acordo com X->dx.
//
// Retorna 1 caso o sistema seja indeterminado ou caso não seja possível separar
// A de B (X-dx > M_LU->dx - 1).
int matrix_LU_AB_solve(matrix_t *X, const matrix_t *M_LU) {
  assert(M_LU->data != NULL);
  assert(X->data != NULL);

  if (X->dx > M_LU->dx - 1 || X->dy != M_LU->dy) {
    return 1;
  }
  int var_num = M_LU->dx - X->dx;
  matrix_view_t A = matrix_view_of(M_LU, 0, 0, M_LU->dy, var_num);
  matrix_view_t B = matrix_view_of(M_LU, 0, var_num, M_LU->dy, X->dx);

  return matrix_back_subs(X, &A.matrix, &B.matrix, 0);
}

double matrix_distance(const matrix_t *A, const matrix_t *B) {
  assert(A->data != NULL && B->data != NULL);

  if (MTX_ARE_SAME(A, B)) {
    return 0;
  }
  if (!MTX_SAME_DIMENSIONS(A, B)) {
    return -1;
  }

  double dt = 0;

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      dt += _mod(matrix_at(A, i, j) - matrix_at(B, i, j));
    }
  }

  return dt;
}

#define DEF_MTX_SIMPLE_OP(name, operation)                                     \
  int matrix_##name(matrix_t *_C, matrix_t *A, matrix_t *B) {                  \
    assert(A->data->m != NULL && B->data->m != NULL);                          \
                                                                               \
    if (!MTX_SAME_DIMENSIONS(A, B)) {                                          \
      return 1;                                                                \
    }                                                                          \
                                                                               \
    if (_C->data == NULL) {                                                    \
      matrix_init(_C, A->dy, A->dx);                                           \
    } else if (!MTX_SAME_DIMENSIONS(_C, A)) {                                  \
      return 1;                                                                \
    }                                                                          \
                                                                               \
    for (int i = 0; i < A->dy; ++i) {                                          \
      for (int j = 0; j < A->dx; ++j) {                                        \
        matrix_at(_C, i, j) = matrix_at(A, i, j) operation matrix_at(B, i, j); \
      }                                                                        \
    }                                                                          \
    return 0;                                                                  \
  }

DEF_MTX_SIMPLE_OP(add, +)
DEF_MTX_SIMPLE_OP(sub, -)

#endif
