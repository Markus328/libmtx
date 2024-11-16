#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../matrix.h"
#include "../matrix_operations.h"
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
  CHECK_C_TEXT(mtx_matrix_distance(&_result, &_expected) < MAXIMUM_ERROR,      \
               "Wrong " #op_name " of A and B");

  CHECK_OPERATION(add, addition);
  CHECK_OPERATION(sub, subtraction);
  CHECK_OPERATION(mul_elements, multiplication);
  CHECK_OPERATION(div_elements, division);
#undef CHECK_OPERATION

  mtx_matrix_free(&A);
}

MAKE_TEST(matrix_arithmetic, mul) {
  mtx_matrix_t A = NEXT_TEST_MTX;
  mtx_matrix_t B = NEXT_TEST_MTX;
  mtx_matrix_t C = RESERVE_MTX(0, 0, A.dy, B.dx);
  mtx_matrix_t _C = {0};

  COPY_NEXT_TEST_MTX(&C);

  mtx_matrix_mul(&_C, &A, &B);

  CHECK_C_TEXT(MTX_MATRIX_SAME_DIMENSIONS(&_C, &C),
               "mtx_matrix_mul() didn't want to to allocate a suitable matrix "
               "for the result");
  CHECK_C_TEXT(mtx_matrix_distance(&_C, &C) < MAXIMUM_ERROR,
               "mtx_matrix_mul() forgot how to multiply two matrices.");

  mtx_matrix_free(&A);
  mtx_matrix_free(&B);
  mtx_matrix_free(&_C);
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
MAKE_TEST(matrix_arithmetic, set_identity) {

#define CHECK_IDENTITY(type_matrix)                                            \
  do {                                                                         \
    mtx_matrix_t A = NEXT_TEST_MTX;                                            \
    mtx_matrix_t I = RESERVE_MTX(0, 0, A.dy, A.dx);                            \
    COPY_NEXT_TEST_MTX(&I);                                                    \
                                                                               \
    mtx_matrix_set_identity(&A);                                               \
                                                                               \
    CHECK_C_TEXT(                                                              \
        mtx_matrix_equals(&A, &I),                                             \
        "mtx_matrix_set_identity() is afraid of going down stairs in "         \
        "a " type_matrix " matrix.");                                          \
                                                                               \
    mtx_matrix_free(&A);                                                       \
  } while (0);

  CHECK_IDENTITY("square");
  CHECK_IDENTITY("dx < dy");
  CHECK_IDENTITY("dx > dy");
}

#undef MAXIMUM_ERROR

#ifdef __cplusplus
}
#endif
