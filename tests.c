#include "gsl/gsl_matrix_double.h"
#include "gsl/gsl_permutation.h"
#include "gsl/gsl_vector_double.h"
#include "matrix.h"
#include "tests/rnd.h"
#include "tests/workers.h"
#include <assert.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
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

/* "template" para função de matriz aleatória */
#define DEF_RND_MTX(RND)                                                       \
  dx = dx > 0 ? dx : RND % 25 + 1;                                             \
  dy = dy > 0 ? dy : RND % 25 + 1;                                             \
                                                                               \
  if (_M->data == NULL) {                                                      \
    matrix_init(_M, dy, dx);                                                   \
  } else if (_M->dx != dx || _M->dy != dy) {                                   \
    printf("Invalid matrix in random_matrix()!\n");                            \
    return;                                                                    \
  }                                                                            \
                                                                               \
  for (int i = 0; i < dy; ++i) {                                               \
    for (int j = 0; j < dx; ++j) {                                             \
      matrix_at(_M, i, j) = RND % 10;                                          \
    }                                                                          \
  }

// Usar somente em single thread.
void random_matrix(matrix_t *_M, int dy, int dx) { DEF_RND_MTX(rand()) }

// Variante de random_matrix para usar em testes pesados multithreading.
void t_random_matrix(matrix_t *_M, int dy, int dx, struct rnd_buffer *rnd) {
  rnd_prepare_chars(rnd, dy * dx);
  DEF_RND_MTX(rnd_next_char(rnd))
}

#undef DEF_RND_MTX

// Máximo the threads a usar.
#define WORKERS 1

worker_data worker_solve_augmented(worker_data *args) {

  matrix_t m = {0};
  matrix_t ech = {0};

  unsigned long total = WORKER_NEXT_ARG(ul);
  int d = WORKER_NEXT_ARG(i);

  int dy = d;
  int dx = dy + 1;
  WORKER_PRINTF("total = %lu, d = %dx%d\n", total, dy, dx);

  struct rnd_buffer *rnd_buf = rnd_alloc_uchars(dy * dx * total);

  unsigned long zeros = 0;

  vector_t x = {0};

  matrix_t x_m = {0};
  matrix_t b_m = {0};
  matrix_t a_m = {0};
  matrix_init(&x_m, dy, 1);
  matrix_init(&b_m, dy, 1);
  matrix_init(&a_m, dy, dx - 1);

  for (unsigned long i = 0; i < total; ++i) {
  }

  matrix_free(&m);
  matrix_free(&ech);
  vector_free(&x);
  rnd_free(rnd_buf);

  return (worker_data){.ul = zeros};
}

void matrix_random_compose() {
  int checks = 8000000;

  int each = checks / WORKERS;
  int d = 5;

  worker_t ws[WORKERS];

  unsigned long all_zeros = 0;

  worker_data args[] = {{.ul = each}, {.i = d}};

  // pthread_mutex_lock(&printf_lock);
  // for (int i = 0; i < WORKERS; ++i) {
  //   printf("[compose] starting a new worker (%d/%d)...\n", i + 1, WORKERS);
  //   pthread_create(&ids[i], NULL, _worker, args);
  // }
  // pthread_mutex_unlock(&printf_lock);
  // for (int i = 0; i < WORKERS; ++i) {
  //   unsigned long zeros;
  //   pthread_join(ids[i], (void **)(&zeros));
  //   all_zeros += zeros;
  // }

  for (int i = 0; i < WORKERS; ++i) {
    ws[i] = worker_start(worker_solve_augmented, 1, args, 2);
  }
  for (int i = 0; i < WORKERS; ++i) {
    all_zeros += worker_join(&ws[i]).ul;
  }

  printf("[compose] ALL threads completed! Found %lu zeros in total.\n",
         all_zeros);
}

int main(void) {

  srand(time(NULL));
  matrix_t m = {0};
  matrix_t lu = {0};
  int dx = 5, dy = dx - 1;

  random_matrix(&m, dy, dx);
  print_matrix(&m);

  matrix_view_t B = matrix_view_of(&m, 0, dx - 1, dy, 1);

  matrix_perm_t perm = {0};
  int sig;
  if ((sig = matrix_LU_decomp(&perm, &lu, &m) < 0)){
    return 1;
  }

  print_matrix(&lu);

  printf("permutation: ");
  print_matrix(&perm);
  matrix_mul(&m, &perm, &m);
  print_matrix(&m);


  matrix_view_t Alu = matrix_view_of(&lu, 0, 0, dy, dx - 1);
  if(matrix_forward_subs(&m, &Alu.matrix, &m, 1) != 0){
    printf("Error!\n");
    return 1;
  }



  matrix_view_t Blu = matrix_view_of(&lu, 0, dx - 1, dy, 1);
  if(matrix_distance(&Blu.matrix, &B.matrix) > 1){
    printf("Method fail!\n");
  }

  printf("det lu = %g\n", matrix_det_LU(&lu, sig)) ;
  printf("det m = %g\n", matrix_det_LU(&m, sig)) ;

  print_matrix(&m);

}
