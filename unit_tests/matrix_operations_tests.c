#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../matrix.h"
#include "../matrix_operations.h"
#include "test_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

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
  MockValue_c v;
  CHECK_C(mtx_matrix_distance_each(&_result_de, &A, &_B) ==
          mtx_matrix_at(&_distance, 0, 0));
  CHECK_C(mtx_matrix_equals(&_expected_de, &_result_de));

  mtx_matrix_free(&A);
}

MAKE_TEST(matrix_arithmetic, element_wise) {
  mtx_matrix_t A = NEXT_TEST_MTX;

  mtx_matrix_t _expected = RESERVE_MTX(0, 0, A.dy, A.dx);
  mtx_matrix_t _result = RESERVE_MTX(0, A.dx, A.dy, A.dx);

  mtx_matrix_t _B = RESERVE_MTX(A.dy, 0, A.dy, A.dx);

  COPY_NEXT_TEST_MTX(&_B);

#define CHECK_OPERATION(op, op_name)                                           \
  COPY_NEXT_TEST_MTX(&_expected);                                              \
  mtx_matrix_##op(&_result, &A, &_B);                                          \
  CHECK_C_TEXT(mtx_matrix_distance(&_result, &_expected) < 1e-3,               \
               "Wrong " #op_name " of A and B");

  CHECK_OPERATION(add, addition);
  CHECK_OPERATION(sub, subtraction);
  CHECK_OPERATION(mul_elements, multiplication);
  CHECK_OPERATION(div_elements, division);
#undef CHECK_OPERATION

  mtx_matrix_free(&A);
}

#ifdef __cplusplus
}
#endif
