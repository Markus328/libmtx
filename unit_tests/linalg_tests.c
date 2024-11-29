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

// TODO: To test a LU decomposition: since a matrix can have more than one LU
// decomposition, its better to check its vality using other functions which use
// a decomposition directly. One simpler method is just check L * U = P * A but
// this only works for square matrices. permutate, back/forward substutition,
// determinant and solving functions give unique outputs so its much better to
// test them first. (some through simple and generic routines).

// static MAKE_ROUTINE(lu_decomp, int success, int perf, const char *filename,
//                     int n) {
//   mtx_matrix_t A = next_matrix_of(__TEST_FILES, filename);

//   mtx_matrix_t _A_LU = RESERVE_MTX(0, 0, A.dy, A.dx);
//   mtx_matrix_t _P = RESERVE_MTX(_A_LU.dy, 0, _A_LU.dy, _A_LU.dy);

//   int ret = mtx_linalg_LU_decomposition(&_P, &_A_LU, &A, perf);

//   if (success && ret < 0) {
//     throw_error(
//         "LU decomposition failed when it had to be successful (%dth check).",
//         n);
//   }

//   if (!success && ret >= 0) {
//     throw_error(
//         "LU decomposition was successful when it had to fail (%dth check).",
//         n);
//   }

//   mtx_matrix_t PA = A;
//   mtx_linalg_permutate(&PA, &A, &_P);

//   mtx_matrix_t L = RESERVE_MTX(_A_LU.dy, _A_LU.dx, _A_LU.dy, _A_LU.dx);
//   mtx_matrix_t U = RESERVE_MTX(2 * _A_LU.dy, 0, _A_LU.dy, _A_LU.dx);

//   mtx_matrix_get_lower(&L, &_A_LU);
//   for (int dgn = 0; dgn < L.dy; ++dgn) {
//     mtx_matrix_at(&L, dgn, dgn) = 1;
//   }

//   mtx_matrix_get_upper(&U, &_A_LU);

//   mtx_matrix_t _PA = RESERVE_MTX(2 * _A_LU.dy, _A_LU.dx, PA.dy, PA.dy);

//   mtx_matrix_mul(&_PA, &L, &U);

//   if (mtx_matrix_distance(&_PA, &PA) > MAXIMUM_ERROR) {
//     throw_error("LU decomposition was sucessful but didn't give the correct "
//                 "answer (%dth check).",
//                 n);
//   }
// }

// MAKE_TEST(linalg, lu_decomp) {

//   for (int i = 1; i <= 5; ++i) {
//     CALL_ROUTINE(lu_decomp, 1, 0, "success", i);
//   }
//   // for (int i = 1; i <= 5; ++i) {
//   //   CALL_ROUTINE(lu_decomp, 0, 0, "failure", i);
//   // }
// }

#ifdef __cplusplus
}
#endif
