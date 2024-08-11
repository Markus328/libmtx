#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/OrderedTest.h>

TEST_GROUP_C_WRAPPER(matrix_lifecycle) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_lifecycle);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_lifecycle);
};

TEST_ORDERED_C_WRAPPER(matrix_lifecycle, init, 0);
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, init_fail, 0);

TEST_ORDERED_C_WRAPPER(matrix_lifecycle, free, 999); // Must be the last
TEST_ORDERED_C_WRAPPER(matrix_lifecycle, free_fail, 0);

TEST_GROUP_C_WRAPPER(matrix_basic) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_basic);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_basic);
};

TEST_ORDERED_C_WRAPPER(matrix_basic, view, 1);
TEST_ORDERED_C_WRAPPER(matrix_basic, view_fail, 1);
TEST_ORDERED_C_WRAPPER(matrix_basic, view_free, 1);
TEST_ORDERED_C_WRAPPER(matrix_basic, clone, 2);
TEST_ORDERED_C_WRAPPER(matrix_basic, copy, 2);
TEST_ORDERED_C_WRAPPER(matrix_basic, copy_overlap, 2);

TEST_GROUP_C_WRAPPER(matrix_io) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_io);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_io);
};

TEST_ORDERED_C_WRAPPER(matrix_io, fread, 2);
TEST_ORDERED_C_WRAPPER(matrix_io, fprint, 2);
TEST_ORDERED_C_WRAPPER(matrix_io, fread_fail, 2);
TEST_ORDERED_C_WRAPPER(matrix_io, fprint_fail, 2);
TEST_ORDERED_C_WRAPPER(matrix_io, finit, 2);
TEST_ORDERED_C_WRAPPER(matrix_io, finit_fail, 2);

int main(int argc, char **argv) {

  CommandLineTestRunner::RunAllTests(argc, argv);
}
