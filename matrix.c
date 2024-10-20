#include "matrix.h"
#include "atomic_operations.h"
#include "errors.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define FSCANF_ONE(stream, fmt, pointer)                                       \
  do {                                                                         \
    if (fscanf(stream, fmt, pointer) != 1) {                                   \
      MTX_SYSTEM_ERR("fscanf");                                                \
    }                                                                          \
  } while (0)

#define FPRINTF_ONE(stream, fmt, data)                                         \
  do {                                                                         \
    if (fprintf(stream, fmt, data) < 0) {                                      \
      MTX_SYSTEM_ERR("fprintf");                                               \
    }                                                                          \
  } while (0)

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

static void __configure_matrix(mtx_matrix_t *_M, int dy, int dx) {
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
}

static inline void __reference_arr(mtx_matrix_t *_M, double *arr, int dy,
                                   int dx) {
  for (int i = 0; i < dy; ++i) {
    _M->data->m[i] = &arr[i * dx];
  }
}

void mtx_matrix_init(mtx_matrix_t *_M, int dy, int dx) {
  __configure_matrix(_M, dy, dx);

  double *m = (double *)mtx_mem_alloc(dy * dx * sizeof(double));
  __reference_arr(_M, m, dy, dx);
}

void mtx_matrix_ref_a(mtx_matrix_t *_M, double *arr, int dy, int dx) {
  __configure_matrix(_M, dy, dx);
  __reference_arr(_M, arr, dy, dx);
}

double *mtx_matrix_raw_a(mtx_matrix_t *M) {
  MTX_ENSURE_INIT(M);

  double *head_mem = M->data->m[0]; // head mem is the pointer which can be
                                    // freed.
  for (int i = 0; i < M->dy; ++i) {
    // rows swaps can be done, then the head mem might not be the first. Since
    // its a contiguous array, the pointer which the lowest value is the head.
    if (M->data->m[i] < head_mem) {
      head_mem = M->data->m[i];
    }
  }

  return head_mem;
}

void mtx_matrix_unref(mtx_matrix_t *__M) {

  if (__M == NULL || __M->data == NULL) {
    return;
  }

  free(__M->data->m);
  __M->data->m = NULL;
  free(__M->data);
  __M->data = NULL;
}

void mtx_matrix_swap(mtx_matrix_t *M1, mtx_matrix_t *M2) {
  MTX_ENSURE_INIT(M1);
  MTX_ENSURE_INIT(M2);
  if (!MTX_MATRIX_SAME_DIMENSIONS(M1, M2) || MTX_MATRIX_IS_VIEW(M1) ||
      MTX_MATRIX_IS_VIEW(M2)) {
    MTX_INVALID_ERR(M1);
  }

  double **tmp = M1->data->m;
  M1->data->m = M2->data->m;
  M2->data->m = tmp;
}

void mtx_matrix_free(mtx_matrix_t *__M) {
  if (__M == NULL || __M->data == NULL) {
    return;
  }

  if (MTX_MATRIX_IS_VIEW(__M)) {
    MTX_INVALID_ERR(__M);
  }

  double *head_mem = mtx_matrix_raw_a(__M);
  // This only works because all rows are a single CONTIGUOUS and splited
  // array starting at head_mem.
  free(head_mem);

  mtx_matrix_unref(__M);
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

void mtx_matrix_copy(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM) {
  MTX_ENSURE_INIT(M_FROM);
  MTX_ENSURE_INIT(M_TO);

  if (!MTX_MATRIX_SAME_DIMENSIONS(M_TO, M_FROM)) {
    MTX_DIMEN_ERR(M_TO);
  }

  void *(*copy)(void *, const void *, size_t) = memcpy;
  if (MTX_MATRIX_OVERLAP_AFTER(M_FROM, M_TO)) {
    copy = memmove;
  }
  for (int i = 0; i < M_TO->dy; ++i) {
    copy(mtx_matrix_row(M_TO, i), mtx_matrix_row(M_FROM, i),
         sizeof(double) * M_TO->dx);
  }
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
  mtx_matrix_fprint(stdout, M);
}
void mtx_matrix_fprint(FILE *stream, const mtx_matrix_t *M) {

  if (stream == NULL) {
    MTX_SYSTEM_ERR("fprintf");
  }
  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      FPRINTF_ONE(stream, "%g ", mtx_matrix_at(M, i, j));
    }
    fprintf(stream, "\n");
  }
}

void mtx_matrix_fread(FILE *stream, mtx_matrix_t *M) {
  if (stream == NULL) {
    MTX_SYSTEM_ERR("fscanf");
  }
  for (int i = 0; i < M->dy; ++i) {
    for (int j = 0; j < M->dx; ++j) {
      FSCANF_ONE(stream, "%lf", &mtx_matrix_at(M, i, j));
    }
  }
}

double *mtx_matrix_fread_raw(FILE *stream, int *dy, int *dx) {

  if (stream == NULL) {
    MTX_SYSTEM_ERR("fscanf");
  }

  double first_dbl;

  // _dx is the matrix's columns number determined by the number of elements in
  // the first line. It must be at least 1 . _dy can be of any size, depending
  // of number of valid lines read. It must be at least 1
  int _dx, _dy;

  int c = ' ';

#define READ_FAIL                                                              \
  free(mtx_m);                                                                 \
  MTX_SYSTEM_ERR("fscanf")

#define READ_DBL                                                               \
  if (c != ' ') {                                                              \
    READ_FAIL;                                                                 \
  }                                                                            \
  fscanf(stream, "%lf", &mtx_m[num_index++]);                                  \
  c = fgetc(stream);

  // Check if stream has at least one valid number then scan it, throw error
  // otherwise.
  FSCANF_ONE(stream, "%lf", &first_dbl);
  c = fgetc(stream);

  // TODO: Decrease the buffer size to avoid memory waste. Maybe
  // use realloc() when necessary.
  double *mtx_m = (double *)mtx_mem_alloc(MTX_MATRIX_MAX_COLUMNS *
                                          MTX_MATRIX_MAX_ROWS * sizeof(double));
  int num_index = 0;

  mtx_m[num_index++] = first_dbl;

  // Read first only row 1
  for (_dx = 1; c != EOF && c != '\n'; ++_dx) {
    if (_dx > MTX_MATRIX_MAX_COLUMNS) {
      READ_FAIL;
    }
    READ_DBL;
  }
  for (_dy = 1; _dy < MTX_MATRIX_MAX_ROWS && c != EOF; ++_dy) {

    assert(c == '\n');
    if (fscanf(stream, "%lf", &mtx_m[num_index++]) != 1) {
      break;
    }

    c = fgetc(stream);
    int dxi = 1;
    for (; dxi <= _dx && c != EOF && c != '\n'; ++dxi) {
      READ_DBL;
    }
    if (dxi < _dx) {
      READ_FAIL;
    } else if (dxi > _dx) {
      READ_FAIL;
    }
  }

#undef READ_DBL
#undef READ_FAIL

  double *tmp_ptr = (double *)realloc(mtx_m, _dy * _dx * sizeof(double));
  if (tmp_ptr == NULL) {
    free(mtx_m);
    MTX_SYSTEM_ERR("realloc");
  }

  mtx_m = tmp_ptr;

  *dx = _dx;
  *dy = _dy;
  return mtx_m;
}

void mtx_matrix_finit(FILE *stream, mtx_matrix_t *_M) {

  int dx, dy;
  double *mtx_m = mtx_matrix_fread_raw(stream, &dy, &dx);
  mtx_matrix_ref_a(_M, mtx_m, dy, dx);
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
