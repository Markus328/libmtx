#include "matrix.h"
#include "atomic_operations.h"
#include "errors.h"

int __mtx_cfg_fix_unsafe_overlappings = 1;

void mtx_cfg_fix_unsafe_overlappings(int enable) {
  // TODO: Make Thread safe
  __mtx_cfg_fix_unsafe_overlappings = enable;
}

int mtx_cfg_are_unsafe_overlappings_fixed() {
  return __mtx_cfg_fix_unsafe_overlappings;
}

mtx_mem_allocator_t __mtx_cfg_mem_allocator = mtx_default_mem_alloc;

void *mtx_default_mem_alloc(size_t size) {
  void *p = NULL;

  if (size > 0) {
    p = malloc(size);
  }
  if (p == NULL) {
    MTX_SYSTEM_ERR("malloc");
  }

  return p;
}

void mtx_cfg_set_mem_alloc(mtx_mem_allocator_t allocator) {
  __mtx_cfg_mem_allocator = allocator; // TODO: Make Thread safe.
}

void mtx_matrix_init(mtx_matrix_t *_M, int dy, int dx) {
  assert(dy <= MTX_MATRIX_MAX_ROWS && dx <= MTX_MATRIX_MAX_COLUMNS);
  assert(dy > 0 && dx > 0);

  _M->data = (mtx_matrix_data_t *)mtx_mem_alloc(sizeof(mtx_matrix_data_t));
  _M->data->size1 = dy;
  _M->data->size2 = dx;

  _M->dy = _M->data->size1;
  _M->dx = _M->data->size2;

  _M->offY = 0;
  _M->offX = 0;

  _M->data->m = (double **)mtx_mem_alloc(sizeof(double *) * dy);
  for (int i = 0; i < dy; ++i) {
    _M->data->m[i] = (double *)mtx_mem_alloc(sizeof(double) * dx);
  }
}

void mtx_matrix_free(mtx_matrix_t *__M) {
  if (__M == NULL || __M->data == NULL) {
    return;
  }

  if (MTX_MATRIX_IS_VIEW(__M)) {
    MTX_INVALID_ERR(__M);
  }

  for (int i = 0; i < __M->dy; ++i) {
    free(__M->data->m[i]);
    __M->data->m[i] = NULL;
  }

  free(__M->data->m);

  __M->data->m = NULL;
  free(__M->data);
  __M->data = NULL;
}

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

void mtx_matrix_init_perm(mtx_matrix_perm_t *_M_PERM, int d) {
  assert(d <= MTX_MATRIX_MAX_COLUMNS && d <= MTX_MATRIX_MAX_COLUMNS);

  mtx_matrix_init(_M_PERM, d, d);
  mtx_matrix_set_identity(_M_PERM);
}

void mtx_matrix_fill_a(mtx_matrix_t *M, double *array) {
  MTX_ENSURE_INIT(M);
  for (int i = 0; i < M->dy; ++i) {
    memcpy(mtx_matrix_row(M, i), array, sizeof(double) * M->dx);
    array = &array[M->dx];
  }
}

void mtx_matrix_fill_m(mtx_matrix_t *M, double **matrix) {
  MTX_ENSURE_INIT(M);
  for (int j = 0; j < M->dy; ++j) {
    memcpy(mtx_matrix_row(M, j), matrix[j], sizeof(double) * M->dx);
  }
}

void mtx_matrix_clone(mtx_matrix_t *_M, const mtx_matrix_t *M) {
  assert(_M != M);
  MTX_ENSURE_INIT(M);

  mtx_matrix_init(_M, M->dy, M->dx);

  for (int j = 0; j < _M->dy; ++j) {
    memcpy(mtx_matrix_row(_M, j), mtx_matrix_row(M, j),
           sizeof(double) * _M->dx);
  }
}

int mtx_matrix_copy(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM) {
  MTX_ENSURE_INIT(M_FROM);
  MTX_ENSURE_INIT(M_TO);

  if (MTX_MATRIX_OVERLAP_AFTER(M_FROM, M_TO)) {
    MTX_OVERLAP_ERR(M_FROM, M_TO);
  }
  if (!MTX_MATRIX_SAME_DIMENSIONS(M_TO, M_FROM)) {
    MTX_DIMEN_ERR(M_TO);
  }
  for (int i = 0; i < M_TO->dy; ++i) {
    memcpy(mtx_matrix_row(M_TO, i), mtx_matrix_row(M_FROM, i),
           sizeof(double) * M_TO->dx);
  }
  return 0;
}
mtx_matrix_view_t mtx_matrix_view_of(const mtx_matrix_t *M_OF, int init_i,
                                     int init_j, int dy, int dx) {
  MTX_ENSURE_INIT(M_OF);
  assert(init_i >= 0 && init_j >= 0);
  assert(dy > 0 && dx > 0);

  if (init_i + dy > M_OF->dy || init_j + dx > M_OF->dx) {
    MTX_BOUNDS_ERR(M_OF);
  }

  mtx_matrix_t mtx;
  mtx.data = M_OF->data;
  mtx.offY = M_OF->offY + init_i;
  mtx.offX = M_OF->offX + init_j;
  mtx.dy = dy;
  mtx.dx = dx;

  return (mtx_matrix_view_t){.matrix = mtx};
}

#define mtx_matrix_column_of(M_OF, j)                                          \
  mtx_matrix_view_of(M_OF, 0, j, (M_OF)->dy, 1)
#define mtx_matrix_row_of(M_OF, i) mtx_matrix_view_of(M_OF, i, 0, 1, (M_OF)->dx)

int mtx_matrix_copy_from(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM,
                         int init_i, int init_j) {
  MTX_ENSURE_INIT(M_FROM);
  MTX_ENSURE_INIT(M_TO);
  assert(init_i >= 0 && init_j >= 0);

  // if (init_i + M_TO->dy > M_FROM->dy || init_j + M_TO->dx > M_FROM->dx) {
  //   return 1;
  // }
  mtx_matrix_view_t part_from =
      mtx_matrix_view_of(M_FROM, init_i, init_j, M_TO->dy, M_TO->dx);

  mtx_matrix_copy(M_TO, &part_from.matrix);

  return 0;
}

void mtx_matrix_print(const mtx_matrix_t *M) {

  printf("matrix (%u, %u):\n", M->dy, M->dx);
  mtx_matrix_fprintf(stdout, M);
}
void mtx_matrix_fprintf(FILE *stream, const mtx_matrix_t *M) {

  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      fprintf(stream, "%g ", mtx_matrix_at(M, i, j));
    }
    fprintf(stream, "\n");
  }
}

void mtx_matrix_fread(FILE *stream, mtx_matrix_t *M) {
  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      fscanf(stream, "%lf", &mtx_matrix_at(M, i, j));
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

int mtx_matrix_equals(const mtx_matrix_t *A, const mtx_matrix_t *B) {
  MTX_ENSURE_INIT(A);
  MTX_ENSURE_INIT(B);
  if (MTX_MATRIX_ARE_SAME(A, B)) {
    return 1;
  }
  if (!MTX_MATRIX_SAME_DIMENSIONS(A, B)) {
    return 0;
  }

  for (int i = 0; i < A->dy; ++i) {
    for (int j = 0; j < A->dx; ++j) {
      if (mtx_matrix_at(A, i, j) != mtx_matrix_at(B, i, j)) {
        return 0;
      }
    }
  }

  return 1;
}

// Calcula e retorna o grau de diferença entre as matrizes A e B. Retorna
// sempre um número positivo, exceto na falha na qual o número retornado é
// negativo.
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

// Subtrai a matriz A da matriz B com o resultado em _M_D e retorna o
// somátorio dos módulos de cada elemento de _M_D, que é sempre um número
// positivo. O valor retornado é exatamente a distancia entre as duas
// matrizes, sendo equivalente ao retornado por mtx_matrix_distance().
//
// Retorna zero se as matrizes forem idênticas e negativo caso ocorra erro. Em
// ambos os casos, _M_DU pode não conter a subtração de A e B, portanto, deve
// ser ignorado.
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
