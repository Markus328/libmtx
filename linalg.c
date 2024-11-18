#include "linalg.h"
#include "atomic_operations.h"
#include "errors.h"
#include "matrix.h"
#include "matrix_operations.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void mtx_matrix_init_perm(mtx_matrix_perm_t *_M_PERM, int d) {
  assert(d <= MTX_MATRIX_MAX_COLUMNS && d <= MTX_MATRIX_MAX_COLUMNS);

  mtx_matrix_init(_M_PERM, d, d);
  mtx_matrix_set_identity(_M_PERM);
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
static void _mtx_row_swap_restrict(mtx_matrix_t *_M, int r1, int r2) {
  assert(r1 != r2);
  assert(r1 >= 0 && r2 >= 0);
  double *tmp = _M->data->m[r1];
  _M->data->m[r1] = _M->data->m[r2];
  _M->data->m[r2] = tmp;
}

static void _mtx_row_swap(mtx_matrix_t *_M, int r1, int r2) {
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
  double *row1 = mtx_matrix_row(_M, r1);
  double *row2 = mtx_matrix_row(_M, r2);
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
  if ((row_pivot[(i)] = _mtx_row_pivot(mtx_matrix_row(_M_LU, (i)), (start),    \
                                       _M_LU->dy)) < 0) {                      \
    return -1;                                                                 \
  }

#define SET_PIVOT_INIT_perf(i) SET_PIVOT_perf(i, 0)

// Recalcular o valor do pivot, caso a linha seja toda zerada, swapar com a
// última linha não zerada. Retorna erro se o pivot não corresponder a uma
// coluna válida (caso dx > dy).
#define SET_PIVOT(i, start)                                                    \
  if ((row_pivot[(i)] = _mtx_row_pivot(mtx_matrix_row(_M_LU, (i)), (start),    \
                                       _M_LU->dx)) < 0) {                      \
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

int mtx_linalg_LU_decomposition(mtx_matrix_perm_t *__M_PERM,
                                mtx_matrix_t *_M_LU, const mtx_matrix_t *M,
                                int perfect) {
  MTX_ENSURE_INIT(M);
  // caso perfect == true, linhas zeradas serão consideradas erro. Com dx <
  // dy, isso inevitavelmente ocorrerá.
  if (perfect && M->dx < M->dy) {
    return -1;
  }

  if (_M_LU->data == NULL) {
    mtx_matrix_clone(_M_LU, M);
  } else if (!MTX_MATRIX_ARE_SAME(_M_LU, M)) {
    mtx_matrix_copy(_M_LU, M);
  }

  int permutate = 0;
  if (__M_PERM != NULL) {
    permutate = 1;
    if (__M_PERM->data == NULL) {
      mtx_matrix_init_perm(__M_PERM, _M_LU->dy);
    } else if (!MTX_MATRIX_OVERLAP(__M_PERM, _M_LU) &&
               MTX_MATRIX_IS_SQUARE(__M_PERM) && __M_PERM->dy == _M_LU->dy) {
      mtx_matrix_set_identity(__M_PERM);
    } else {
      MTX_INVALID_ERR(__M_PERM);
    }
  }

  // 0 se número de swaps é par e 1 se for ímpar.
  int odd_swaps = 0;

  // O pivot de cada linha é armazenado para não ter que calcular toda vez.

  int row_pivot[MTX_MATRIX_MAX_ROWS];

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
    double pp = mtx_matrix_at(_M_LU, p, p);
    double max = _mod(pp);
    int i_max = p;

    for (int ic = p + 1; ic <= last_w_row; ++ic) {
      double mod_pivc = _mod(mtx_matrix_at(_M_LU, ic, p));
      if (mod_pivc > max) {
        i_max = ic;
        max = mod_pivc;
      }
    }
    if (i_max != p) {
      SWAP(p, i_max);
      pp = mtx_matrix_at(_M_LU, p, p);
    }

    // Reduz todas as linhas abaixo, substituindo todos os valores na mesma
    // coluna do pivot.
    for (int ic = p + 1; ic <= last_w_row; ++ic) {
      double ip = mtx_matrix_at(_M_LU, ic, p);

      if (ip == 0) {
        continue;
      }

      double mul = ip / pp; // abs(pp) >= abs(ip)
      mtx_matrix_at(_M_LU, ic, p) = mul;

      _mtx_sum_multiple(mtx_matrix_row(_M_LU, ic), mtx_matrix_row(_M_LU, p),
                        -mul, p + 1, _M_LU->dx);

      if (!perfect) {
        SET_PIVOT(ic, p + 1);
      } else {
        SET_PIVOT_perf(ic, p + 1);
      }
    }
  }

  return odd_swaps;
}

#undef DEF_ECH
#undef PIVOT
#undef ROW_ECHELON
#undef SWAP
#undef SET_PIVOT
#undef SET_PIVOT_perf

int mtx_linalg_permutate(mtx_matrix_t *_M, const mtx_matrix_t *M,
                         const mtx_matrix_perm_t *M_PERM) {
  MTX_ENSURE_INIT(M);
  MTX_ENSURE_INIT(M_PERM);

  assert(MTX_MATRIX_IS_SQUARE(M_PERM));

  if (M_PERM->dy != M->dy) {
    MTX_DIMEN_ERR(M);
  }

  if (_M->data == NULL) {
    mtx_matrix_init(_M, M->dy, M->dx);
  }
  if (!MTX_MATRIX_SAME_DIMENSIONS(_M, M)) {
    MTX_DIMEN_ERR(_M);
  }

  MTX_MAKE_OUTPUT_ALIAS(permutated, _M);

  MTX_ENSURE_SAFE_OUTPUT(permutated, _M, M);
  MTX_ENSURE_SAFE_OUTPUT(permutated, _M, M_PERM);

  int pivot;
  int lower_pivot = 0;
  int higher_pivot = M_PERM->dx - 1;
  for (int i = 0; i < M_PERM->dy; ++i) {
    pivot = _mtx_row_pivot(mtx_matrix_row(M_PERM, i), lower_pivot,
                           higher_pivot + 1);
    if (pivot == lower_pivot) {
      ++lower_pivot;
    }
    if (pivot == higher_pivot) {
      --higher_pivot;
    }
    _mtx_row_copy(mtx_matrix_row(&permutated, i), mtx_matrix_row(M, pivot),
                  permutated.dx);
  }

  MTX_COMMIT_OUTPUT(permutated, _M);
  return 0;
}

double mtx_linalg_det_LU(const mtx_matrix_t *M_LU, int signum) {

  MTX_ENSURE_INIT(M_LU);
  double det = 1;
  if (signum >= 0) {

    for (int p = 0; p < M_LU->dy; ++p) {
      det *= mtx_matrix_at(M_LU, p, p);
    }

    if (signum) {
      det *= -1;
    }
  } else {
    det = 0;
  }

  return det;
}

double mtx_matrix_det(const mtx_matrix_t *M) {
  MTX_ENSURE_INIT(M);
  if (!MTX_MATRIX_IS_SQUARE(M)) {
    MTX_DIMEN_ERR(M);
  }

  mtx_matrix_t lu = {0};

  int signum = mtx_linalg_LU_decomp_perf(NULL, &lu, M);
  double det = mtx_linalg_det_LU(&lu, signum);

  mtx_matrix_free(&lu);
  return det;
}

int mtx_linalg_back_subs(mtx_matrix_t *_X, const mtx_matrix_t *U,
                         const mtx_matrix_t *B, int jordan) {
  MTX_ENSURE_INIT(U);
  MTX_ENSURE_INIT(B);
  if (!MTX_MATRIX_IS_SQUARE(U) || U->dy != B->dy) {
    MTX_DIMEN_ERR(U);
  }

  int dx = U->dx;
  int dy = U->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (mtx_matrix_at(U, dy - 1, dx - 1) == 0) {
    return 1;
  }

  if (_X->data == NULL) {
    mtx_matrix_init(_X, var_num, B->dx);
  } else if (!MTX_MATRIX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  MTX_MAKE_OUTPUT_ALIAS(x, _X);

  MTX_ENSURE_SAFE_OUTPUT_RULES(
      x, _X, B, (MTX_MATRIX_OVERLAP(_X, B) && (_X->offY < B->offY)));
  MTX_ENSURE_SAFE_OUTPUT_RULES(
      x, _X, U, (MTX_MATRIX_OVERLAP(_X, U) && (_X->offY <= U->offY)));

  double *U_i;
  double *X_i;
  for (int i = var_num - 1; i >= 0; --i) {
    U_i = mtx_matrix_row(U, i);
    X_i = mtx_matrix_row(&x, i);

    _mtx_row_copy(X_i, mtx_matrix_row(B, i), x.dx);
    for (int j = i + 1; j < var_num; ++j) {
      _mtx_sum_multiple(X_i, mtx_matrix_row(&x, j), -U_i[j], 0, x.dx);
    }

    if (!jordan) {
      double pivot = U_i[i];
      _mtx_row_mul(mtx_matrix_row(&x, i), 1.0 / pivot, x.dx);
    }
  }

  MTX_COMMIT_OUTPUT(x, _X);

  return 0;
}

int mtx_linalg_forward_subs(mtx_matrix_t *_X, const mtx_matrix_t *L,
                            const mtx_matrix_t *B, int jordan) {

  MTX_ENSURE_INIT(L);
  MTX_ENSURE_INIT(B);
  if (B->dy != L->dy) {
    MTX_DIMEN_ERR(L);
  }
  if (_X->data == NULL) {
    mtx_matrix_init(_X, B->dy, B->dx);
  } else if (!MTX_MATRIX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  int dx = L->dx;
  int dy = L->dy;
  int var_num = dx;

  // Sistema indeterminado
  if (var_num > dy || mtx_matrix_at(L, var_num - 1, var_num - 1) == 0) {
    return 1;
  }

  MTX_MAKE_OUTPUT_ALIAS(x, _X);

  MTX_ENSURE_SAFE_OUTPUT_RULES(
      x, _X, B, (MTX_MATRIX_OVERLAP(_X, B) && (_X->offY > B->offY)));

  MTX_ENSURE_SAFE_OUTPUT_RULES(
      x, _X, L, (MTX_MATRIX_OVERLAP(_X, L) && (_X->offY >= L->offY)));

  double *L_i;
  double *X_i;
  for (int i = 0; i < var_num; ++i) {
    L_i = mtx_matrix_row(L, i);
    X_i = mtx_matrix_row(&x, i);

    _mtx_row_copy(X_i, mtx_matrix_row(B, i), x.dx);
    for (int j = 0; j < i; ++j) {
      _mtx_sum_multiple(X_i, mtx_matrix_row(&x, j), -L_i[j], 0, x.dx);
    }

    if (!jordan) {
      double diagn = L_i[i];
      _mtx_row_mul(mtx_matrix_row(&x, i), 1.0 / diagn, x.dx);
    }
  }

  // Caso dy > dx, continua a substituir os elementos abaixo.
  for (int i = var_num; i < L->dy; ++i) {
    L_i = mtx_matrix_row(L, i);
    X_i = mtx_matrix_row(&x, i);

    _mtx_row_copy(X_i, mtx_matrix_row(B, i), x.dx);
    for (int j = 0; j < L->dx; ++j) {
      _mtx_sum_multiple(X_i, mtx_matrix_row(&x, j), -L_i[j], 0, x.dx);
    }
  }

  MTX_COMMIT_OUTPUT(x, _X);

  return 0;
}

int mtx_linalg_LU_AB_solve(mtx_matrix_t *X, const mtx_matrix_t *AB_LU) {

  MTX_ENSURE_INIT(AB_LU);
  MTX_ENSURE_INIT(X);
  int var_num = AB_LU->dx - X->dx;
  if (var_num < 1 || X->dy < var_num) {
    MTX_BOUNDS_ERR(X);
  }

  mtx_matrix_view_t B = mtx_matrix_view_of(AB_LU, 0, var_num, AB_LU->dy, X->dx);

  // Sistema impossível
  if (AB_LU->dy > var_num) {
    for (int bi = var_num; bi < B.matrix.dy; ++bi) {
      for (int bj = 0; bj < B.matrix.dx; ++bj) {
        if (mtx_matrix_at(&B.matrix, bi, bj) != 0) {
          return 1;
        }
      }
    }
  }

  mtx_matrix_view_t A = mtx_matrix_view_of(AB_LU, 0, 0, var_num, var_num);

  // B->dy = A->dx = A->dy
  B.matrix.dy = A.matrix.dx;

  return mtx_linalg_back_subs(X, &A.matrix, &B.matrix, 0);
}

int mtx_linalg_LU_solve(mtx_matrix_t *_X, const mtx_matrix_perm_t *M_PERM,
                        const mtx_matrix_t *A_LU, const mtx_matrix_t *B) {
  MTX_ENSURE_INIT(A_LU);
  MTX_ENSURE_INIT(B);

  if (A_LU->dy != B->dy) {
    MTX_DIMEN_ERR(B);
  }

  if (_X->data == NULL) {
    mtx_matrix_init(_X, B->dy, B->dx);
  } else if (!MTX_MATRIX_SAME_DIMENSIONS(_X, B)) {
    MTX_DIMEN_ERR(_X);
  }

  mtx_linalg_permutate(_X, B, M_PERM);
  if (mtx_linalg_forward_subs(_X, A_LU, _X, 1) != 0) {
    return 1;
  }
  if (mtx_linalg_back_subs(_X, A_LU, _X, 0) != 0) {
    return 1;
  }

  return 0;
}

double mtx_linalg_LU_refine(mtx_matrix_t *_M_WORK, mtx_matrix_t *X,
                            const mtx_matrix_perm_t *M_PERM,
                            const mtx_matrix_t *A_LU, const mtx_matrix_t *A,
                            const mtx_matrix_t *B) {

  mtx_matrix_mul(_M_WORK, A, X);
  double dt;
  if ((dt = mtx_matrix_distance_each(_M_WORK, _M_WORK, B)) == 0) {
    return 0;
  }

  if (mtx_linalg_LU_solve(_M_WORK, M_PERM, A_LU, _M_WORK) != 0) {
    MTX_INVALID_ERR(X);
  }
  mtx_matrix_sub(X, X, _M_WORK);

  return dt;
}
