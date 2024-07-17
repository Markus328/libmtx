#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../matrix.h"
#include "test_utilts.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mock_functions.c"

void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...) {
  mock_c()->actualCall(__func__)->withIntParameters("error", error);

  // jump to test's stack.
  RAISE_TEST;
}

TEST_GROUP_C_SETUP(matrix_lifecycle) {
  mock_c()->setBoolData("reportParameters", 0);
  mock_c()->enable();
}

TEST_GROUP_C_TEARDOWN(matrix_lifecycle) {
  mock_c()->checkExpectations();
  mock_c()->clear();
}

TEST_C(matrix_lifecycle, init_and_free) {
  mtx_matrix_t m = {0};

  int d = 5;
  mock_c()->expectNCalls(
      2 + d, "malloc_mock"); // One extra call of malloc() for each row
  mock_c()->expectNCalls(2 + d,
                         "free_mock"); // One extra call of free() for each row

  mtx_matrix_init(&m, d, d);
  CHECK_C(m.data != NULL);
  CHECK_C_TEXT(m.offX == 0 && m.offY == 0,
               "Wrong offsets on matrix_initialization!");
  CHECK_C_TEXT(m.dx == d && m.dy == d, "Wrong dimensions on initialization!");
  CHECK_C_TEXT(m.data->size1 == d && m.data->size2 == d,
               "Wrong matrix_data dimensions on initialization!");

  mtx_matrix_free(&m);
}

TEST_C(matrix_lifecycle, init_fail) {
  mtx_matrix_t m = {0};

  int d = 5;

  // mtx_default_mem_alloc being tested
  mtx_cfg_set_mem_alloc(mtx_default_mem_alloc);
  // Emulate malloc() error
  mock_c()->expectOneCall("malloc_mock")->andReturnPointerValue(NULL);
  mock_c()
      ->expectOneCall("test_fail")
      ->withIntParameters("error", MTX_SYSTEM_ERR);

  WAIT_RAISE(malloc_err) { mtx_matrix_init(&m, d, d); }
}

TEST_C(matrix_lifecycle, free_fail) {
  mtx_matrix_t m = {0};
  int d = 5;

  mock_c()->disable();
  mtx_matrix_init(&m, d, d);

  mtx_matrix_view_t view = mtx_matrix_row_of(&m, 0);

  mock_c()->enable();
  // All free() should be avoided.
  mock_c()->expectNoCall("free_mock");
  // Check error in passing view to mtx_matrix_free()
  mock_c()
      ->expectOneCall("test_fail")
      ->withIntParameters("error", MTX_INVALID_ERR);

  WAIT_RAISE(free_view) { mtx_matrix_free(&view.matrix); }

  mtx_matrix_free(NULL);
  mtx_matrix_t m_null = {0};
  mtx_matrix_free(&m_null);
  mock_c()->checkExpectations();

  mock_c()->disable();
  mtx_matrix_free(&m);
}

mtx_matrix_t M1 = {0};
mtx_matrix_t M2 = {0};

TEST_GROUP_C_SETUP(matrix_basic) {}
TEST_GROUP_C_TEARDOWN(matrix_basic) {
  mock_c()->checkExpectations();
  mock_c()->clear();
}

TEST_C(matrix_basic, clone) {
  mock_c()->disable();

  mtx_matrix_t m = {0};
  int d = 5;
  mtx_matrix_init(&m, d, d);

  mock_c()->enable();

  mock_c()->expectNCalls(d, "memcpy_mock");
  mock_c()->ignoreOtherCalls();
  mtx_matrix_t n;
  mtx_matrix_clone(&n, &m);

  CHECK_C(!MTX_MATRIX_ARE_SAME(&n, &m));
  CHECK_C(mtx_matrix_equals(&n, &m));

  mtx_matrix_free(&n);
}

TEST_C(matrix_basic, copy) {
  mock_c()->disable();

  mtx_matrix_t m = {0};
  mtx_matrix_t n = {0};
  int d = 5;
  mtx_matrix_init(&m, d, d);
  mtx_matrix_init(&n, d, d);

  for (int i = 0; i < m.dy; ++i) {
    for (int j = 0; j < m.dx; ++j) {
      mtx_matrix_at(&m, i, j) = rand();
    }
  }

  mock_c()->enable();
  mock_c()->expectNCalls(d, "memcpy_mock");
  mtx_matrix_copy(&n, &m);

  mock_c()->ignoreOtherCalls();

  CHECK_C(mtx_matrix_equals(&m, &n));

  mtx_matrix_free(&m);
  mtx_matrix_free(&n);
}

TEST_C(matrix_basic, copy_overlap) {
  mock_c()->disable();

  mtx_matrix_t m = {0};
  int d = 5;
  mtx_matrix_init(&m, d, d);
  mtx_matrix_view_t view_before = mtx_matrix_view_of(&m, 0, 0, d - 1, d - 1);
  mtx_matrix_view_t view_after = mtx_matrix_view_of(&m, 1, 1, d - 1, d - 1);

  mock_c()->enable();

  // memmove since overlaps breaks restrict contract.
  mock_c()->expectNCalls(view_after.matrix.dy, "memmove_mock");

  // memcpy in overlapping memory should be avoided.
  mock_c()->expectNoCall("memcpy_mock");
  mock_c()->ignoreOtherCalls();

  WAIT_RAISE(clone) {
    mtx_matrix_copy(&view_after.matrix, &view_before.matrix);
  }

  CHECK_C(mtx_matrix_equals(&view_before.matrix, &view_after.matrix));

  mtx_matrix_free(&m);
}

#ifdef __cplusplus
}
#endif
