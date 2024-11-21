#include "test_utils.h"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/OrderedTest.h>

// MATRIX TESTS

TEST_GROUP_C_WRAPPER(matrix_lifecycle) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_lifecycle);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_lifecycle);
};

TEST_ORDERED_C_WRAPPER(matrix_lifecycle, init, 0);
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, init_fail, 0);

TEST_ORDERED_C_WRAPPER(matrix_lifecycle, free, 999); // Must be the last
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, free_fail, 0);

TEST_ORDERED_C_WRAPPER(matrix_lifecycle, ref_a, 0);
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, unref, 1);
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, raw_a, 2);

TEST_GROUP_C_WRAPPER(matrix_basic) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_basic);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_basic);
};

TEST_ORDERED_C_WRAPPER(matrix_basic, view, 10);
TEST_ORDERED_C_WRAPPER(matrix_basic, view_fail, 10);
TEST_ORDERED_C_WRAPPER(matrix_basic, view_free, 10);
TEST_ORDERED_C_WRAPPER(matrix_basic, fill, 11);
TEST_ORDERED_C_WRAPPER(matrix_basic, equals, 12);
TEST_ORDERED_C_WRAPPER(matrix_basic, clone, 13);
TEST_ORDERED_C_WRAPPER(matrix_basic, copy, 13);
TEST_ORDERED_C_WRAPPER(matrix_basic, copy_overlap, 13);

TEST_GROUP_C_WRAPPER(matrix_io) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_io);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_io);
};

TEST_ORDERED_C_WRAPPER(matrix_io, fread, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, fprint, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, fread_fail, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, fprint_fail, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, fread_raw, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, fread_raw_fail, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, finit_fail, 20);

// MATRIX OPERATIONS

TEST_GROUP_C_WRAPPER(matrix_arithmetic) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_arithmetic);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_arithmetic);
};

TEST_ORDERED_C_WRAPPER(matrix_arithmetic, distance_each, 30);
TEST_ORDERED_C_WRAPPER(matrix_arithmetic, element_wise, 31);
TEST_ORDERED_C_WRAPPER(matrix_arithmetic, mul, 31);
TEST_ORDERED_C_WRAPPER(matrix_arithmetic, s_mul, 31);
TEST_ORDERED_C_WRAPPER(matrix_arithmetic, transpose, 31);
TEST_ORDERED_C_WRAPPER(matrix_arithmetic, set_identity, 31);


// LINEAR ALGEBRA

TEST_GROUP_C_WRAPPER(linalg) {
  TEST_GROUP_C_SETUP_WRAPPER(linalg);
  TEST_GROUP_C_TEARDOWN_WRAPPER(linalg);
};

TEST_ORDERED_C_WRAPPER(linalg, permutate, 40);





int main(int argc, char **argv) {
  mtx_cfg_set_mem_alloc(mtx_default_mem_alloc);
  mtx_cfg_set_error_handler(test_fail);
  CommandLineTestRunner::RunAllTests(argc, argv);
}
