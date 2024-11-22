
#ifdef __cplusplus
extern "C" {
#endif

#include "routines.h"
#include "../matrix.h"
#include "../matrix_operations.h"

// TODO: Try to create a custom mtx_mem_alloc() (and implement cfg for
// 'mtx_mem_free()') to decrease overhead in NEXT_TEST_MTX.
MAKE_ROUTINE(check_3m_i,
             int (*fun)(mtx_matrix_t *, const mtx_matrix_t *,
                        const mtx_matrix_t *),
             double tol, int n) {

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
    FAIL_C();
  }

  mtx_matrix_free(&C);
}

#ifdef __cplusplus
}
#endif
