
#ifdef __cplusplus
extern "C" {
#endif

#include "routines.h"
#include "../matrix.h"
#include "../matrix_operations.h"

// TODO: Try to create a custom mtx_mem_alloc() (and implement cfg for
// 'mtx_mem_free()') to decrease overhead in NEXT_TEST_MTX.
MAKE_ROUTINE(check_3m_i, int (*fun)(MTX_3M_SIG), double tol, int n) {

  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t B = NEXT_TEST_MTX;
  mtx_matrix_t C = NEXT_TEST_MTX;

  mtx_matrix_t _C = RESERVE_MTX(0, 0, C.dy, C.dx);

  fun(&_C, &A, &B);

  mtx_matrix_free(&A);
  mtx_matrix_free(&B);

  if (mtx_matrix_distance(&_C, &C) > tol) {
    mtx_matrix_free(&C);

    throw_error("check_3m %s/%s: output of function doesn't match the actual "
                "result matrix (%dth CHECK).\n",
                G_NAME, T_NAME, n);
  }

  mtx_matrix_free(&C);
}
MAKE_ROUTINE(check_2m_i, int (*fun)(MTX_2M_SIG), int n) {

  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t B = NEXT_TEST_MTX;

  mtx_matrix_t _B = RESERVE_MTX(0, 0, B.dy, B.dx);

  fun(&_B, &A);

  mtx_matrix_free(&A);

  if (!mtx_matrix_equals(&_B, &B)) {
    mtx_matrix_free(&B);

    throw_error("check_2m %s/%s: output of function doesn't match the actual "
                "result matrix (%dth CHECK).\n",
                G_NAME, T_NAME, n);
  }

  mtx_matrix_free(&B);
}

#ifdef __cplusplus
}
#endif
