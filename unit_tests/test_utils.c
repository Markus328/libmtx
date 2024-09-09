#include "test_utilts.h"

#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...) {
  mock_c()->actualCall(__func__)->withIntParameters("error", error);

  // jump to test's stack.
  RAISE_TEST;
}


FILE *test_matrix_from(const char *group_name, const char *test_name,
                       const char *matrix_name) {

  mock_c()->disable();

  char line_buf[MTX_MATRIX_MAX_COLUMNS * 20];

  char filename[128];
  if (snprintf(filename, sizeof(filename), "files/%s/%s/%s", group_name,
               test_name, matrix_name) < 0) {
    fprintf(stderr,
            "Error reading test matrix: invalid group_name, test_name or "
            "matrix_name: %s, %s, %s\n",
            group_name, test_name, matrix_name);
    abort();
  }

  FILE *tmtx_file = fopen(filename, "r");
  if (tmtx_file == NULL) {
    fprintf(stderr,
            "Error reading test matrix: cannot open matrix's file: %s\n",
            filename);
    abort();
  }

  mock_c()->enable();

  return tmtx_file;
}

mtx_matrix_t get_mtx_from(const char *path) {
  mtx_matrix_t m;
  FILE *fd = fopen(path, "r");
  mtx_matrix_finit(fd, &m);
  fclose(fd);

  return m;
}

mtx_matrix_t M = {0};
