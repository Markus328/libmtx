#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../matrix.h"
#include "../matrix_operations.h"
#include "routines.h"
#include "test_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_ERROR 1e-3

TEST_GROUP_C_SETUP(matrix_arithmetic) { mock_c()->disable(); }

TEST_GROUP_C_TEARDOWN(matrix_arithmetic) { mock_c()->clear(); }

MAKE_TEST(matrix_arithmetic, distance_each) {
  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t _B = RESERVE_MTX(0, 0, A.dy, A.dx);
  mtx_matrix_t _result_de = RESERVE_MTX(0, A.dx, A.dy, A.dx);
  mtx_matrix_t _expected_de = RESERVE_MTX(A.dy, 0, A.dy, A.dx);
  mtx_matrix_t _distance = RESERVE_MTX(A.dy, A.dx, 1, 1);

  COPY_NEXT_TEST_MTX(&_B);
  COPY_NEXT_TEST_MTX(&_expected_de);
  COPY_NEXT_TEST_MTX(&_distance);

  CHECK_C(mtx_matrix_distance(&A, &A) == 0);
  CHECK_C(mtx_matrix_distance_each(&_result_de, &A, &_B) ==
          mtx_matrix_at(&_distance, 0, 0));
  CHECK_C(mtx_matrix_equals(&_expected_de, &_result_de));

  mtx_matrix_free(&A);
}

static MAKE_ROUTINE(element_wise, const char *name, int (*fun_wise)(MTX_3M_SIG),
                    mtx_matrix_t *A, mtx_matrix_t *B) {
  mtx_matrix_t C = RESERVE_MTX(0, A->dx, A->dy, A->dx);
  COPY_NEXT_TEST_MTX(&C);

  mtx_matrix_t _C = RESERVE_MTX(A->dy, 0, A->dy, A->dx);

  fun_wise(&_C, A, B);
  if (mtx_matrix_distance(&_C, &C) > MAXIMUM_ERROR) {
    fprintf(stderr, "Wrong %s of elements in matrices A and B.\n", name);
    FAIL_C();
  }
}

MAKE_TEST(matrix_arithmetic, element_wise) {
  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t B = RESERVE_MTX(0, 0, A.dy, A.dx);

  COPY_NEXT_TEST_MTX(&B);

  CALL_ROUTINE(element_wise, "addition", mtx_matrix_add, &A, &B);
  CALL_ROUTINE(element_wise, "subtraction", mtx_matrix_sub, &A, &B);
  CALL_ROUTINE(element_wise, "multiplication", mtx_matrix_mul_elements, &A, &B);
  CALL_ROUTINE(element_wise, "division", mtx_matrix_div_elements, &A, &B);

  mtx_matrix_free(&A);
}

MAKE_TEST(matrix_arithmetic, mul) {
  // TODO: can lead to Unwaited Exception depending on the file input.
  // Improving test_fail() to display MTX_ERROR strings in CppuTest mode (using
  // both FAIL_C or CHECK_C TEXTs) may be necessary.
  CALL_ROUTINE(check_3m_i, mtx_matrix_mul, MAXIMUM_ERROR, 1);
}

MAKE_TEST(matrix_arithmetic, s_mul) {
  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t scalar = RESERVE_MTX(0, 0, 1, 1);
  mtx_matrix_t B = RESERVE_MTX(0, 1, A.dy, A.dx);
  mtx_matrix_t _B = RESERVE_MTX(B.dy, 0, B.dy, B.dx);

  COPY_NEXT_TEST_MTX(&scalar);
  COPY_NEXT_TEST_MTX(&B);

  mtx_matrix_s_mul(&_B, &A, mtx_matrix_at(&scalar, 0, 0));
  CHECK_C_TEXT(
      mtx_matrix_distance(&B, &_B) < MAXIMUM_ERROR,
      "mtx_matrix_s_mul() doesn't know how to work with scalars (or maybe "
      "matrices).");

  mtx_matrix_free(&A);
}

MAKE_TEST(matrix_arithmetic, transpose) {
  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t At = RESERVE_MTX(0, 0, A.dx, A.dy);
  mtx_matrix_t _At = {0};

  COPY_NEXT_TEST_MTX(&At);

  mtx_matrix_transpose(&_At, &A);

  CHECK_C_TEXT(MTX_MATRIX_SAME_DIMENSIONS(&_At, &At),
               "mtx_matrix_transpose() is confused about the matrix size.");
  CHECK_C_TEXT(mtx_matrix_equals(&_At, &At),
               "mtx_matrix_transpose() did its role wrong.");

  mtx_matrix_free(&A);
  mtx_matrix_free(&_At);
}
static MAKE_ROUTINE(check_identity, const char *m_class) {

  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t I = RESERVE_MTX(0, 0, A.dy, A.dx);
  COPY_NEXT_TEST_MTX(&I);

  mtx_matrix_set_identity(&A);

  if (!mtx_matrix_equals(&A, &I)) {
    throw_error("mtx_matrix_set_identity() is afraid of going down stairs in "
                "a %s matrix.",
                m_class);
  }

  mtx_matrix_free(&A);
}
MAKE_TEST(matrix_arithmetic, set_identity) {
  CALL_ROUTINE(check_identity, "square");
  CALL_ROUTINE(check_identity, "dx < dy");
  CALL_ROUTINE(check_identity, "dx > dy");
}

MAKE_TEST(matrix_arithmetic, get_upper) {
  CALL_ROUTINE(check_2m_i, mtx_matrix_get_upper, 1);
  CALL_ROUTINE(check_2m_i, mtx_matrix_get_upper, 2);
  CALL_ROUTINE(check_2m_i, mtx_matrix_get_upper, 3);
}

// MAKE_TEST(matrix_arithmetic, get_lower) {
//   CALL_ROUTINE(check_2m_i, mtx_matrix_get_lower, 1);
//   CALL_ROUTINE(check_2m_i, mtx_matrix_get_lower, 2);
// }

#undef MAXIMUM_ERROR

#ifdef __cplusplus
}
#endif
