#include "../errors.h"
#include "test_utilts.h"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness_c.h>

TEST_GROUP_C_WRAPPER(matrix_lifecycle) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_lifecycle);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_lifecycle);
};

TEST_C_WRAPPER(matrix_lifecycle, init_and_free);
TEST_C_WRAPPER(matrix_lifecycle, init_fail);
TEST_C_WRAPPER(matrix_lifecycle, free_fail);

TEST_GROUP_C_WRAPPER(matrix_basic) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_basic);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_basic);
};

TEST_C_WRAPPER(matrix_basic, clone);
TEST_C_WRAPPER(matrix_basic, copy_overlap);
TEST_C_WRAPPER(matrix_basic, copy);

int main(int argc, char **argv) {

  mtx_cfg_set_error_handler(test_fail);
  CommandLineTestRunner::RunAllTests(argc, argv);
}
