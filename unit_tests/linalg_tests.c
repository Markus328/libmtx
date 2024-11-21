#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../linalg.h"
#include "../matrix.h"
#include "../matrix_operations.h"
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

// MAKE_ROUTINE(lu_decomp, int success, int perf, const char *filename) {
//   mtx_matrix_t A = next_matrix_of(__TEST_FILES, filename);

//   mtx_matrix_t _A_LU = RESERVE_MTX(0, 0, A.dy, A.dx);
//   mtx_matrix_t _P = RESERVE_MTX(_A_LU.dy, 0, _A_LU.dy, _A_LU.dy);

//   int ret = mtx_linalg_LU_decomposition(&_P, &_A_LU, &A, perf);

//   if (success && ret < 0) {
//     FAIL_TEXT_C("LU decomposition failed when it had to be successful.");
//   }

//   if (!success && ret >= 0) {
//     FAIL_TEXT_C("LU decomposition was successful when it had to fail.");
//   }
// }

#ifdef __cplusplus
}
#endif
