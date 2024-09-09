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
TEST_ORDERED_C_WRAPPER(matrix_io, finit, 20);
TEST_ORDERED_C_WRAPPER(matrix_io, finit_fail, 20);

int main(int argc, char **argv) {

  CommandLineTestRunner::RunAllTests(argc, argv);
}
