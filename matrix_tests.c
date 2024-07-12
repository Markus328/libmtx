#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "matrix.h"
#include <stdlib.h>
void *malloc_mock(size_t size);
void free_mock(void *p);
#define malloc malloc_mock
#define free free_mock
#include "matrix.c"
#undef malloc
#undef free

void *malloc_mock(size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (mock_c()->getData("reportParameters").value.boolValue) {
    act->withUnsignedLongIntParameters("size", size);
  }
  return mock_c()->returnPointerValueOrDefault(malloc(size));
}
void free_mock(void *p) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (mock_c()->getData("reportParameters").value.boolValue) {
    act->withPointerParameters("p", p);
  }

  free(p);
}

TEST_GROUP_C_SETUP(matrix_lifecycle) {
  mock_c()->setBoolData("reportParameters", 0);
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

TEST_C(matrix_lifecycle, free_fail) {
  mtx_matrix_t m = {0};
  int d = 5;
  mock_c()->ignoreOtherCalls();
  mtx_matrix_init(&m, d, d);
  mtx_matrix_view_t view = mtx_matrix_row_of(&m, 0);

  mock_c()->expectNoCall("free_mock");
  mtx_matrix_free(&view.matrix);
  mtx_matrix_free(NULL);
  mtx_matrix_t m_null = {0};
  mtx_matrix_free(&m_null);
  mock_c()->checkExpectations();

  mock_c()->ignoreOtherCalls();
  mtx_matrix_free(&m);

#ifdef __cplusplus
}
#endif
}
