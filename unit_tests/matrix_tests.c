#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../matrix.h"
#include "test_utilts.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mock_functions.c"

TEST_GROUP_C_SETUP(matrix_lifecycle) {
  mock_c()->setBoolData("reportParameters", 0);
  mock_c()->enable();
}

TEST_GROUP_C_TEARDOWN(matrix_lifecycle) {
  mock_c()->checkExpectations();
  mock_c()->clear();
}

MAKE_TEST(matrix_lifecycle, init) {

  // mtx_default_mem_alloc being tested
  mtx_cfg_set_mem_alloc(mtx_default_mem_alloc);

  mock_c()->expectNCalls(3, "malloc_mock");

  mtx_matrix_init(&M, M_DY, M_DX);
  CHECK_C(M.data != NULL);
  CHECK_C_TEXT(M.offX == 0 && M.offY == 0,
               "Wrong offsets on matrix_initialization!");
  CHECK_C_TEXT(M.dy == M_DY && M.dx == M_DX,
               "Wrong dimensions on initialization!");
  CHECK_C_TEXT(M.data->size1 == M_DY && M.data->size2 == M_DX,
               "Wrong matrix_data dimensions on initialization!");
}

MAKE_TEST(matrix_lifecycle, init_fail) {
  mtx_matrix_t m = {0};

  int d = 5;

  mtx_cfg_set_error_handler(test_fail);

  // Emulate malloc() error
  mock_c()->expectOneCall("malloc_mock")->andReturnPointerValue(NULL);
  mock_c()
      ->expectOneCall("test_fail")
      ->withIntParameters("error", MTX_SYSTEM_ERR);

  WAIT_RAISE(malloc_err) { mtx_matrix_init(&m, d, d); }
}

// Last test
MAKE_TEST(matrix_lifecycle, free) {

  mock_c()->expectNCalls(3, "free_mock");

  mtx_matrix_free(&M);
}

MAKE_TEST(matrix_lifecycle, free_fail) {

  // All free() should be avoided.
  mock_c()->expectNoCall("free_mock");

  mtx_matrix_free(NULL);
  mtx_matrix_t m_null = {0};
  mtx_matrix_free(&m_null);
}

TEST_GROUP_C_SETUP(matrix_basic) {}
TEST_GROUP_C_TEARDOWN(matrix_basic) {
  mock_c()->checkExpectations();
  mock_c()->clear();
}

MAKE_TEST(matrix_lifecycle, ref_a) {
  double arr[6];

  mtx_matrix_t m;

  mock_c()->expectNCalls(2, "malloc_mock");
  mtx_matrix_ref_a(&m, arr, 3, 2);

  for (int i = 0; i < m.dy; ++i) {
    CHECK_C(mtx_matrix_row(&m, i) == &arr[i * m.dx]);
  }

  mock_c()->disable();

  mtx_matrix_unref(&m);
}

MAKE_TEST(matrix_lifecycle, unref) {
  mock_c()->disable();
  double arr[6];
  mtx_matrix_t m;
  mtx_matrix_ref_a(&m, arr, 3, 2);

  mock_c()->enable();

  mock_c()->expectNCalls(2, "free_mock");
  mtx_matrix_unref(&m);
}

MAKE_TEST(matrix_lifecycle, raw_a) {
  mock_c()->disable();
  double arr[6];
  mtx_matrix_t m;
  mtx_matrix_ref_a(&m, arr, 3, 2);

  // swaps row 1 and row 3
  double *tmp = m.data->m[0];
  m.data->m[0] = m.data->m[2];
  m.data->m[2] = tmp;

  CHECK_C(mtx_matrix_raw_a(&m) == arr);

  mtx_matrix_unref(&m);
}

MAKE_TEST(matrix_basic, clone) {

  mock_c()->expectNCalls(M_DY, "memcpy_mock");
  mock_c()->ignoreOtherCalls();
  mtx_matrix_t n;
  mtx_matrix_clone(&n, &M);

  CHECK_C(!MTX_MATRIX_ARE_SAME(&n, &M));
  CHECK_C(mtx_matrix_equals(&n, &M));

  mtx_matrix_free(&n);
}

MAKE_TEST(matrix_basic, copy) {

  mtx_matrix_view_t m = mtx_matrix_view_of(&M, 0, 0, M_DY / 2, M_DX);
  mtx_matrix_view_t n = mtx_matrix_view_of(&M, m.matrix.dy, 0, M_DY / 2, M_DX);

  for (int i = 0; i < m.matrix.dy; ++i) {
    for (int j = 0; j < m.matrix.dx; ++j) {
      mtx_matrix_at(&m.matrix, i, j) = rand();
    }
  }

  mock_c()->expectNCalls(m.matrix.dy, "memcpy_mock");
  mtx_matrix_copy(&n.matrix, &m.matrix);

  mock_c()->ignoreOtherCalls();

  CHECK_C(mtx_matrix_equals(&m.matrix, &n.matrix));
}

MAKE_TEST(matrix_basic, copy_overlap) {
  mtx_matrix_view_t view_before =
      mtx_matrix_view_of(&M, 0, 0, M_DY - 1, M_DX - 1);
  mtx_matrix_view_t view_after =
      mtx_matrix_view_of(&M, 1, 1, M_DY - 1, M_DX - 1);

  // memmove since overlaps breaks restrict contract.
  mock_c()->expectNCalls(view_after.matrix.dy, "memmove_mock");

  // memcpy in overlapping memory should be avoided.
  mock_c()->expectNoCall("memcpy_mock");

  WAIT_RAISE(clone) {
    mtx_matrix_copy(&view_after.matrix, &view_before.matrix);
  }

  mock_c()->ignoreOtherCalls();

  CHECK_C(mtx_matrix_equals(&view_before.matrix, &view_after.matrix));
}

MAKE_TEST(matrix_basic, fill) {
  mock_c()->disable();
  double arr[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  mtx_matrix_view_t m = mtx_matrix_view_of(&M, 1, 1, 3, 3);

  mock_c()->enable();
  mock_c()->expectNCalls(3, "memcpy_mock");
  mtx_matrix_fill_a(&m.matrix, arr);

  int count = 0;
  for (int i = 0; i < m.matrix.dy; ++i) {
    for (int j = 0; j < m.matrix.dx; ++j) {
      CHECK_C(arr[count++] == mtx_matrix_at(&m.matrix, i, j));
    }
  }
}

MAKE_TEST(matrix_basic, view) {

#define CHECK_VIEW_BASIC(view, y, x, d1, d2)                                   \
  CHECK_C_TEXT(MTX_MATRIX_ARE_SHARED(&view.matrix, &M),                        \
               "for some reason MTX_MATRIX_ARE_SHARED() thinks " #view         \
               " and M have nothing to do with each other");                   \
  CHECK_C_TEXT(MTX_MATRIX_IS_VIEW(&(view).matrix), #view                       \
               " is a fakey one but MTX_MATRIX_IS_VIEW() doesn't believe it"); \
  CHECK_C_TEXT((view).matrix.offY == y && (view).matrix.offX == x,             \
               "wrong offsets at created view " #view);                        \
  CHECK_C_TEXT((view).matrix.dy == d1 && (view).matrix.dx == d2,               \
               "wrong dimensions at created view " #view);                     \
  CHECK_C_TEXT(&mtx_matrix_at(&M, y, x) ==                                     \
                   &mtx_matrix_at(&(view).matrix, 0, 0),                       \
               "wrong mtx_matrix_at access in view" #view)

  mtx_matrix_view_t v1 = mtx_matrix_view_of(&M, 3, 1, 5, 5);

  CHECK_VIEW_BASIC(v1, 3, 1, 5, 5);

  // Will overlap with v1 and come after.
  mtx_matrix_view_t v2 = mtx_matrix_view_of(&M, 3, 3, 5, 5);
  CHECK_VIEW_BASIC(v2, 3, 3, 5, 5);

  CHECK_C_TEXT(MTX_MATRIX_OVERLAP(&v1.matrix, &v2.matrix),
               "v1 and v2 overlaps but MTX_MATRIX_OVERLAP() doesn't think so.");
  CHECK_C_TEXT(
      MTX_MATRIX_OVERLAP_AFTER(&v1.matrix, &v2.matrix),
      "v2 comes \"after\" v1 but MTX_MATRIX_OVERLAP_AFTER() says it's false.");
  CHECK_C_TEXT(
      !MTX_MATRIX_OVERLAP_AFTER(&v2.matrix, &v1.matrix),
      "v1 doesn't come \"after\" v2 but MTX_MATRIX_OVERLAP_AFTER() is lying.");

  mtx_matrix_view_t v3 = mtx_matrix_view_of(&v1.matrix, 0, 2, 5, 3);
  CHECK_VIEW_BASIC(v3, 3, 3, 5, 3);

  mtx_matrix_view_t v4 = mtx_matrix_view_of(&v2.matrix, 0, 0, 5, 3);

  CHECK_C_TEXT(
      MTX_MATRIX_ARE_SAME(&v3.matrix, &v4.matrix),
      "v3 and v4 are meant to be only one but MTX_MATRIX_ARE_SAME() sees two "
      "different ones.");

#undef CHECK_VIEW_BASIC
}

MAKE_TEST(matrix_basic, view_fail) {
  mock_c()
      ->expectNCalls(3, "test_fail")
      ->withIntParameters("error", MTX_BOUNDS_ERR);

  mtx_matrix_view_t vf;
  WAIT_RAISE(init_erange) { vf = mtx_matrix_view_of(&M, M.dy, 0, 2, 2); }
  WAIT_RAISE(dimen_erange1) { vf = mtx_matrix_view_of(&M, M.dy - 1, 0, 2, 1); }
  WAIT_RAISE(dimen_erange2) {
    vf = mtx_matrix_view_of(&M, M.dy - 1, 0, 1, M.dx + 1);
  }
}
MAKE_TEST(matrix_basic, view_free) {
  mock_c()->expectNoCall("free_mock");

  mtx_matrix_view_t view = mtx_matrix_row_of(&M, 0);
  mock_c()
      ->expectOneCall("test_fail")
      ->withIntParameters("error", MTX_INVALID_ERR);

  WAIT_RAISE(free_view) { mtx_matrix_free(&view.matrix); }
}

TEST_GROUP_C_SETUP(matrix_io) {}
TEST_GROUP_C_TEARDOWN(matrix_io) {
  mock_c()->checkExpectations();
  mock_c()->clear();
}

#define FIOTEST(variation, sys_fun, ncalls)                                    \
  mock_c()->disable();                                                         \
  mtx_matrix_view_t m = mtx_matrix_view_of(&M, 3, 3, 2, 2);                    \
                                                                               \
  mock_c()->enable();                                                          \
                                                                               \
  FILE *fake_fd = (FILE *)234828148;                                           \
  mock_c()                                                                     \
      ->expectNCalls((ncalls), #sys_fun "_mock")                               \
      ->withPointerParameters("stream", fake_fd)                               \
      ->andReturnIntValue(1);                                                  \
  mtx_matrix_##variation(fake_fd, &m.matrix)

MAKE_TEST(matrix_io, fread) {
  FIOTEST(fread, fscanf, m.matrix.dx * m.matrix.dy);
}
MAKE_TEST(matrix_io, fprint) {
  FIOTEST(fprint, fprintf, (m.matrix.dx + 1) * m.matrix.dy);
}

#define FIOTEST_FAIL(variation, sys_fun)                                       \
  mock_c()->disable();                                                         \
  mtx_matrix_view_t m = mtx_matrix_view_of(&M, 3, 3, 2, 2);                    \
  mock_c()->enable();                                                          \
                                                                               \
  mock_c()                                                                     \
      ->expectNCalls(2, "test_fail")                                           \
      ->withIntParameters("error", MTX_SYSTEM_ERR);                            \
                                                                               \
  FILE *fake_fd = NULL;                                                        \
  mock_c()->expectNoCall(#sys_fun "_mock");                                    \
  WAIT_RAISE(null_fd) { mtx_matrix_##variation(fake_fd, &m.matrix); }          \
                                                                               \
  fake_fd = (FILE *)83841;                                                     \
                                                                               \
  mock_c()                                                                     \
      ->expectOneCall(#sys_fun "_mock")                                        \
      ->withPointerParameters("stream", fake_fd)                               \
      ->andReturnIntValue(-1);                                                 \
  mock_c()->ignoreOtherCalls();                                                \
                                                                               \
  WAIT_RAISE(fscanf_err) { mtx_matrix_##variation(fake_fd, &m.matrix); }

MAKE_TEST(matrix_io, fread_fail) { FIOTEST_FAIL(fread, fscanf); }
MAKE_TEST(matrix_io, fprint_fail) { FIOTEST_FAIL(fprint, fprintf); }

MAKE_TEST(matrix_io, finit) {
  mtx_matrix_t m;
  FILE *fd = test_matrix_from(G_NAME, T_NAME, "default.txt");

  mock_c()->expectNCalls(1 + 2, "malloc_mock"); // elements + metadata malloc()
  mock_c()
      ->expectNCalls(10, "fscanf_mock")
      ->withPointerParameters("stream", fd);

  mtx_matrix_finit(fd, &m);
  fclose(fd);

  mock_c()->disable();

  double arr[9] = {2, 3, 1, 48, -24.3, 1e-242, -3.342e+119, 0, -0.00044342};

  mtx_matrix_view_t A = mtx_matrix_view_of(&M, 0, 0, 3, 3);
  mtx_matrix_fill_a(&A.matrix, arr);
  CHECK_C(mtx_matrix_equals(&A.matrix, &m));

  mtx_matrix_free(&m);
}

MAKE_TEST(matrix_io, finit_fail) { FIOTEST_FAIL(finit, fscanf); }

#ifdef __cplusplus
}
#endif
