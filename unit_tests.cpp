#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness_c.h>

TEST_GROUP_C_WRAPPER(matrix_lifecycle) {
  TEST_GROUP_C_SETUP_WRAPPER(matrix_lifecycle);
  TEST_GROUP_C_TEARDOWN_WRAPPER(matrix_lifecycle);
};

TEST_C_WRAPPER(matrix_lifecycle, init_and_free);
TEST_C_WRAPPER(matrix_lifecycle, free_fail);

int main(int argc, char **argv) {
  CommandLineTestRunner::RunAllTests(argc, argv);
}
