#include "gsl/gsl_matrix_double.h"
#include "gsl/gsl_permutation.h"
#include "gsl/gsl_vector_double.h"
#include "matrix.h"
#include "tests/rnd.h"
#include "tests/workers.h"
#include <assert.h>
#include <float.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void print_gsl_matrix(gsl_matrix *gsl) {
  printf("matrix (%lu, %lu):\n", gsl->size1, gsl->size2);
  for (int j = 0; j < gsl->size1; ++j) {
    for (int i = 0; i < gsl->size2; ++i) {
      printf("%g ", gsl_matrix_get(gsl, j, i));
    }
    printf("\n");
  }
}
void copy_to_gsl_matrix(gsl_matrix *gsl, matrix_t *m) {
  assert(m->dx == gsl->size2 && m->dy == gsl->size1);

  for (int i = 0; i < m->dy; ++i) {
    for (int j = 0; j < m->dx; ++j) {
      gsl_matrix_set(gsl, i, j, matrix_at(m, i, j));
    }
  }
}

static double get_rnd_dbl(int min_exp, int max_exp, struct rnd_buffer *rnd) {
  assert(min_exp <= max_exp);

  double d = rnd_next_dbl(rnd);

  int xp;
  double _d = frexp(d, &xp);
  if (xp < min_exp || xp > max_exp) {
    d = 0;
  }

  return d;
}

// Variante de random_matrix para usar em testes pesados multithreading.
void t_random_matrix(matrix_t *_M, int dy, int dx, struct rnd_buffer *rnd) {
  rnd_prepare_chars(rnd, dy * dx);

  dy = dy > 0 ? dy : rnd_next_uchar(rnd) % 25;
  dx = dx > 0 ? dx : rnd_next_uchar(rnd) % 25;
  if (_M->data == NULL) {
    matrix_init(_M, dy, dx);
  } else if (_M->dy != dy || _M->dx != dx) {
    fprintf(stderr, "Error of dimensions in t_random_matrix()!\n");
    abort();
  }

  for (int i = 0; i < dy; ++i) {
    for (int j = 0; j < dx; ++j) {
      matrix_at(_M, i, j) = get_rnd_dbl(-512, 512, rnd);
    }
  }
}

// Máximo the threads a usar.
#define WORKERS 8

int mtx_solution_check(matrix_t *_B, const matrix_t *A, const matrix_t *B,
                       const matrix_t *X) {
  matrix_mul(_B, A, X);
  if (matrix_distance(_B, B) > 1e-6) {
    printf("Solution error: ");
    print_matrix(A);
    printf("X\n");
    print_matrix(X);
    printf("=\n");
    print_matrix(_B);
    printf("Which is different of \n");
    print_matrix(B);
    return 1;
  }

  return 0;
}

worker_data worker_solve_augmented(worker_data *args) {

  matrix_t m = {0};
  gsl_matrix *gsl_m;
  matrix_t lu = {0};

  unsigned long total = WORKER_NEXT_ARG(ul);
  int d = WORKER_NEXT_ARG(i);

  int dy = d;
  int dx = dy + 1;
  WORKER_PRINTF("total = %lu, d = %dx%d\n", total, dy, dx);

  struct rnd_buffer *rnd_buf = rnd_alloc_uchars(dy * dx * total);

  unsigned long zeros = 0;

  matrix_t x = {0};
  matrix_init(&x, dy, 1);
  gsl_m = gsl_matrix_alloc(dy, dx);

  matrix_init(&m, dy, dx);
  matrix_init(&lu, dy, dx);

  matrix_view_t A = matrix_view_of(&m, 0, 0, dy, dx - 1);
  matrix_view_t B = matrix_view_of(&m, 0, dx - 1, dy, 1);
  matrix_view_t B_LU = matrix_view_of(&lu, 0, dx - 1, dy, 1);

  for (unsigned long i = 0; i < total; ++i) {
    t_random_matrix(&m, dy, dx, rnd_buf);

    if (matrix_LU_decomp_perf(NULL, &lu, &m) < 0 ||
        matrix_LU_AB_solve(&x, &lu) != 0) {
      zeros++;
      WORKER_START_PRINT;
      printf("no solution for matrix(%d, %d):\n", dy, dx);
      print_matrix(&m);
      WORKER_END_PRINT;

      continue;
    }
    mtx_solution_check(&B_LU.matrix, &A.matrix, &B.matrix, &x);

    // WORKER_START_PRINT;
    // print_matrix(&m);
    // printf("solution:\n");
    // print_matrix(&x);
    // WORKER_END_PRINT;
  }

  matrix_free(&m);
  matrix_free(&lu);
  rnd_free(rnd_buf);

  return (worker_data){.ul = zeros};
}

worker_data test_decomposition(worker_data *args) {
  if (args == NULL) {
    return (worker_data)NULL;
  }

  unsigned long count = WORKER_NEXT_ARG(ul);
  unsigned long d = WORKER_NEXT_ARG(i);
  int dx = d, dy = dx + 1;
  unsigned long errors = 0;

  struct rnd_buffer *buf = rnd_alloc_uchars(dx * dy * count);

  matrix_t m = {0};
  matrix_t lu = {0};
  matrix_t perm = {0};
  matrix_t re_decomp = {0};
  for (unsigned long i = 0; i < count; ++i) {
    t_random_matrix(&m, dy, dx, buf);

    if (matrix_LU_decomp(&perm, &lu, &m) < 0) {
      continue;
    }

    matrix_permutate(&re_decomp, &m, &perm);
    matrix_forward_subs(&re_decomp, &lu, &re_decomp, 1);

    double dt = 0;
    for (int ui = 0; ui < lu.dy; ++ui) {
      for (int uj = ui; uj < lu.dx; ++uj) {
        dt += _mod(matrix_at(&re_decomp, ui, uj) - matrix_at(&lu, ui, uj));
      }
    }
    if (dt > 0) {
      WORKER_START_PRINT;

      errors++;
      printf("re-decomposition failed for ");
      print_matrix(&m);

      printf("Expected:\n");
      print_matrix(&lu);
      printf("Got:\n");
      print_matrix(&re_decomp);

      WORKER_END_PRINT;
    }
  }

  matrix_free(&m);
  matrix_free(&lu);
  matrix_free(&perm);
  matrix_free(&re_decomp);
  rnd_free(buf);

  return (worker_data){.ul = errors};
}

void matrix_random_compose() {
  int checks = 80000000;

  int each = checks / WORKERS;
  int d = 6;

  worker_t ws[WORKERS];

  unsigned long all_zeros = 0;

  worker_data args[] = {{.ul = each}, {.i = d}};

  printf("[compose] Starting solve augmented tests...");
  for (int i = 0; i < WORKERS; ++i) {
    ws[i] = worker_start(worker_solve_augmented, 1, args, 2);
  }
  for (int i = 0; i < WORKERS; ++i) {
    all_zeros += worker_join(&ws[i]).ul;
  }

  printf("[compose] ALL threads completed! Found %lu zeros in total.\n",
         all_zeros);

  printf("[compose] Starting decomposition_tests...");

  unsigned long all_errors = 0;
  for (int i = 0; i < WORKERS; ++i) {
    ws[i] = worker_start(test_decomposition, 1, args, 2);
  }
  for (int i = 0; i < WORKERS; ++i) {
    all_errors += worker_join(&ws[i]).ul;
  }

  printf("[compose] ALL threads completed! Found %lu errors in total.\n",
         all_errors);
}

static int test_refine_mtx(const matrix_t *M, double *distance) {
  matrix_perm_t perm = {0};
  matrix_t lu = {0};
  if (matrix_LU_decomp_perf(&perm, &lu, M) < 0) {
    fprintf(stderr, "No solution for matrix (%d, %d):\n", M->dy, M->dx);
    matrix_fprintf(stderr, M);
    fputc('\n', stderr);
    abort();
  }

  matrix_view_t A = matrix_view_of(M, 0, 0, M->dy, M->dx - 1);
  matrix_view_t B = matrix_column_of(M, M->dx - 1);
  matrix_view_t A_LU = matrix_view_of(&lu, 0, 0, M->dy, M->dx - 1);
  matrix_view_t X = matrix_column_of(&lu, M->dx - 1);

  matrix_LU_AB_solve(&X.matrix, &lu);

  matrix_t work = {0};

  double dt = -1, dn;
  matrix_t new_X = {0};
  matrix_clone(&new_X, &X.matrix);
  while (1) {
    printf("X = ");
    print_matrix(&X.matrix);

    double dn = matrix_LU_refine(&work, &new_X, &perm, &A_LU.matrix, &A.matrix,
                                 &B.matrix);
    if (dt == dn && matrix_equals(&X.matrix, &new_X)) {
      break;
    }
    dt = dn;
    matrix_copy(&X.matrix, &new_X);
    printf("distance = %g\n", dt);
    print_matrix(M);
    if (dt < 1) {
      break;
    }
  }

  matrix_free(&lu);
  matrix_free(&perm);
  matrix_free(&work);

  return 0;
}

static int test_refine_gsl(gsl_matrix *gm) {
  gsl_permutation *perm = gsl_permutation_alloc(gm->size1);

  int signum;
  gsl_matrix_view A = gsl_matrix_submatrix(gm, 0, 0, gm->size1, gm->size2 - 1);
  gsl_matrix *LU = gsl_matrix_alloc_from_matrix(&A.matrix, 0, 0, A.matrix.size1,
                                                A.matrix.size2);
  gsl_linalg_LU_decomp(LU, perm, &signum);

  gsl_vector_view B = gsl_matrix_column(gm, gm->size2 - 1);

  gsl_vector *x = gsl_vector_alloc(A.matrix.size1);
  gsl_linalg_LU_solve(LU, perm, &B.vector, x);
  gsl_vector *work = gsl_vector_alloc(A.matrix.size1);
  gsl_linalg_LU_refine(&A.matrix, LU, perm, &B.vector, x, work);
  printf("GSL X = vector(%ld):\n", x->size);
  gsl_vector_fprintf(stdout, x, "%g");

  return 0;
}

int main(void) {

  // matrix_random_compose();
  matrix_t m;
  matrix_init(&m, 5, 6);

  FILE *fd = fopen("./test-refine.txt", "r");
  if (fd == NULL) {
    struct rnd_buffer *buf = rnd_alloc_dbls(5 * 6);
    t_random_matrix(&m, 5, 6, buf);
    rnd_free(buf);
  } else {
    matrix_fread(fd, &m);
    fclose(fd);
  }
  printf("just read ");
  print_matrix(&m);
  gsl_matrix *gm = gsl_matrix_alloc(5, 6);
  copy_to_gsl_matrix(gm, &m);

  double dt_mtx = 0;
  test_refine_mtx(&m, &dt_mtx);
  test_refine_gsl(gm);

  matrix_free(&m);
  gsl_matrix_free(gm);
}
