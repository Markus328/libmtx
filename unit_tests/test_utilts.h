#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../errors.h"
#include "../matrix.h"
#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#define WAIT_RAISE(ID)                                                         \
  jmp_buf ID##_buf;                                                            \
  int ID##_r = setjmp(ID##_buf);                                               \
  if (ID##_r == 0) {                                                           \
    mock_c()->setPointerData("jmp_buf", &(ID##_buf));                          \
  } else {                                                                     \
    mock_c()->setPointerData("jmp_buf", NULL);                                 \
  }                                                                            \
  if (ID##_r == 0)

#define RAISE_TEST                                                             \
  if (mock_c()->getData("jmp_buf").value.pointerValue == NULL) {               \
    fprintf(stderr, "Unwaited Exception in %s on %s() at line %d\n", __FILE__, \
            __func__, __LINE__);                                               \
    abort();                                                                   \
  }                                                                            \
  longjmp(*(jmp_buf *)mock_c()->getData("jmp_buf").value.pointerValue, 1)

#define MAKE_TEST(groupname, testname)                                         \
  extern void test_##groupname##_##testname##_wrapper_c();                     \
  static void __test_##groupname##_##testname(                                 \
      const char *G_NAME, const char *T_NAME, const char *DEFAULT_MTX);        \
  void test_##groupname##_##testname##_wrapper_c() {                           \
    __test_##groupname##_##testname(#groupname, #testname,                     \
                                    #groupname "/" #testname "/default.txt");  \
  }                                                                            \
  static void __test_##groupname##_##testname(                                 \
      const char *G_NAME, const char *T_NAME, const char *DEFAULT_MTX)

void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...);

FILE *test_matrix_from(const char *group_name, const char *test_name,
                       const char *matrix_name);
mtx_matrix_t test_read_matrix_from(FILE *fd);

#define TEST_GET_PATH(group, test) "files/" #group "/" #test ".txt"

mtx_matrix_t get_mtx_from(const char *path);

extern mtx_matrix_t M;
#define M_DX 25
#define M_DY 25

#ifdef __cplusplus
}
#endif

#endif
