#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../linalg.h"
#include "../matrix.h"
#include "routines.h"
#include "test_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_ERROR 1e-3

TEST_GROUP_C_SETUP(linalg) { mock_c()->disable(); }

TEST_GROUP_C_TEARDOWN(linalg) { mock_c()->clear(); }

MAKE_TEST(linalg, permutate) {
  CALL_ROUTINE(check_3m_i, mtx_linalg_permutate, 0, 1);
  CALL_ROUTINE(check_3m_i, mtx_linalg_permutate, 0, 2);
  CALL_ROUTINE(check_3m_i, mtx_linalg_permutate, 0, 3);
}

// Only for square matrices
MAKE_TEST(linalg, lu_decomp_square) {

  mtx_matrix_t A = NEXT_TEST_MTX_NAME(successful);

  mtx_matrix_t A_LU = RESERVE_MTX(0, 0, A.dy, A.dx);
  mtx_matrix_t _A_LU = RESERVE_MTX(0, A_LU.dx, A_LU.dy, A_LU.dx);
  mtx_matrix_t P = RESERVE_MTX(A_LU.dy, 0, A_LU.dy, A_LU.dx);
  mtx_matrix_t _P = RESERVE_MTX(A_LU.dy, A_LU.dx, A_LU.dy, A_LU.dx);

  COPY_NEXT_TEST_MTX_NAME(&A_LU, successful);
  COPY_NEXT_TEST_MTX_NAME(&P, successful);

  mtx_linalg_LU_decomp(&_P, &_A_LU, &A);
}

#ifdef __cplusplus
}
#endif
