#include "matrix_operations.h"
#include "assert.h"
#include "atomic_operations.h"
#include "errors.h"

void mtx_matrix_set_identity(mtx_matrix_t *_M) {
  for (int i = 0; i < _M->dy; ++i) {
    int j = 0;
    for (; j < _M->dx; ++j) {
      mtx_matrix_at(_M, i, j) = 0;
    }
    if (j > i) {
      mtx_matrix_at(_M, i, i) = 1;
    }
  }
}

int mtx_matrix_mul(mtx_matrix_t *_C, const mtx_matrix_t *A,
                   const mtx_matrix_t *B) {

  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);

  if (A->dx != B->dy) {
    MTX_DIMEN_ERR(B);
  }

  if (_C->data == NULL) {
    mtx_matrix_init(_C, A->dy, B->dx);
  } else {

    if (_C->dx != B->dx || _C->dy != A->dy) {
      MTX_DIMEN_ERR(_C);
    }
  }

  MTX_MAKE_OUTPUT_ALIAS(mul_res, _C);

  MTX_ENSURE_SAFE_OUTPUT(mul_res, _C, A);
  MTX_ENSURE_SAFE_OUTPUT(mul_res, _C, B);

  int bx = 0, ay = 0;

  for (; ay < A->dy; ++ay) {
    bx = 0;
    for (; bx < B->dx; ++bx) {
      int i = 0;

      double sum = 0;
      for (; i < A->dx; ++i) {
        sum += mtx_matrix_at(A, ay, i) * mtx_matrix_at(B, i, bx);
      }
      mtx_matrix_at(&mul_res, ay, bx) = sum;
    }
  }

  MTX_COMMIT_OUTPUT(mul_res, _C);
  return 0;
}

int mtx_matrix_s_mul(mtx_matrix_t *_M, const mtx_matrix_t *M, double scalar) {
  MTX_ENSURE_INIT(M);

  if (_M->data == NULL) {
    mtx_matrix_init(_M, M->dy, M->dx);
  } else if (!MTX_MATRIX_SAME_DIMENSIONS(_M, M)) {
    MTX_DIMEN_ERR(_M);
  }

  MTX_MAKE_OUTPUT_ALIAS(m_res, _M);

  MTX_ENSURE_SAFE_OUTPUT_RULES(m_res, _M, M, MTX_MATRIX_OVERLAP_AFTER(M, _M));
  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      mtx_matrix_at(_M, i, j) = mtx_matrix_at(M, i, j) * scalar;
    }
  }

  return 0;
}

int mtx_matrix_transpose(mtx_matrix_t *_M, const mtx_matrix_t *M) {
  MTX_ENSURE_INIT(M);
  if (_M->data == NULL) {
    mtx_matrix_init(_M, M->dx, M->dy);
  } else if (_M->dy != M->dx || _M->dx != M->dy) {
    MTX_DIMEN_ERR(_M);
  }

  for (int i = 0; i < _M->dy; ++i) {
    for (int j = 0; j < _M->dx; ++j) {
      mtx_matrix_at(_M, i, j) = mtx_matrix_at(M, j, i);
    }
  }
  return 0;
}

double mtx_matrix_distance(const mtx_matrix_t *A, const mtx_matrix_t *B) {

  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (!MTX_MATRIX_SAME_DIMENSIONS(A, B)) {
    MTX_DIMEN_ERR(B);
  }
  if (MTX_MATRIX_ARE_SAME(A, B)) {
    return 0;
  }

  double dt = 0;

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      dt += _mod(mtx_matrix_at(A, i, j) - mtx_matrix_at(B, i, j));
    }
  }

  return dt;
}

double mtx_matrix_distance_each(mtx_matrix_t *_M_D, const mtx_matrix_t *A,
                                const mtx_matrix_t *B) {
  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (!MTX_MATRIX_SAME_DIMENSIONS(A, B)) {
    MTX_DIMEN_ERR(A);
  }
  if (MTX_MATRIX_ARE_SAME(A, B)) {
    return 0;
  }

  if (_M_D->data == NULL) {
    mtx_matrix_init(_M_D, A->dy, A->dx);
  } else if (!MTX_MATRIX_SAME_DIMENSIONS(_M_D, A)) {
    MTX_DIMEN_ERR(_M_D);
    return -1;
  }

  MTX_MAKE_OUTPUT_ALIAS(m_d, _M_D);
  MTX_ENSURE_SAFE_OUTPUT_RULES(m_d, _M_D, A, MTX_MATRIX_OVERLAP_AFTER(A, _M_D));
  MTX_ENSURE_SAFE_OUTPUT_RULES(m_d, _M_D, B, MTX_MATRIX_OVERLAP_AFTER(B, _M_D));

  double dt = 0;

  double diff;
  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      diff = mtx_matrix_at(A, i, j) - mtx_matrix_at(B, i, j);
      mtx_matrix_at(&m_d, i, j) = diff;
      dt += _mod(diff);
    }
  }

  MTX_COMMIT_OUTPUT(m_d, _M_D);

  return dt;
}
#define DEF_MTX_MATRIX_SIMPLE_OP(name, operation)                              \
  int mtx_matrix_##name(mtx_matrix_t *_C, const mtx_matrix_t *A,               \
                        const mtx_matrix_t *B) {                               \
                                                                               \
    MTX_ENSURE_INIT(A);                                                        \
    MTX_ENSURE_INIT(B);                                                        \
    if (!MTX_MATRIX_SAME_DIMENSIONS(A, B)) {                                   \
      MTX_DIMEN_ERR(A);                                                        \
    }                                                                          \
                                                                               \
    if (_C->data == NULL) {                                                    \
      mtx_matrix_init(_C, A->dy, A->dx);                                       \
    } else if (!MTX_MATRIX_SAME_DIMENSIONS(_C, A)) {                           \
      MTX_DIMEN_ERR(_C);                                                       \
    }                                                                          \
    MTX_MAKE_OUTPUT_ALIAS(c, _C);                                              \
    MTX_ENSURE_SAFE_OUTPUT_RULES(c, _C, A, MTX_MATRIX_OVERLAP_AFTER(A, _C));   \
    MTX_ENSURE_SAFE_OUTPUT_RULES(c, _C, B, MTX_MATRIX_OVERLAP_AFTER(B, _C));   \
                                                                               \
    for (int i = 0; i < A->dy; ++i) {                                          \
      for (int j = 0; j < A->dx; ++j) {                                        \
        mtx_matrix_at(_C, i, j) =                                              \
            mtx_matrix_at(A, i, j) operation mtx_matrix_at(B, i, j);           \
      }                                                                        \
    }                                                                          \
    return 0;                                                                  \
  }

DEF_MTX_MATRIX_SIMPLE_OP(add, +);
DEF_MTX_MATRIX_SIMPLE_OP(sub, -);
DEF_MTX_MATRIX_SIMPLE_OP(mul_elements, *);
DEF_MTX_MATRIX_SIMPLE_OP(div_elements, /);

#undef DEF_MTX_MATRIX_SIMPLE_OP
