#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../errors.h"
#include "../matrix.h"
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
  }                                                                            \
  if (ID##_r == 0)

#define RAISE_TEST                                                             \
  if (mock_c()->getData("jmp_buf").value.pointerValue == NULL) {               \
    fprintf(stderr, "Unwaited Exception in %s on %s() at line %d\n", __FILE__, \
            __func__, __LINE__);                                               \
    abort();                                                                   \
  }                                                                            \
  longjmp(*(jmp_buf *)mock_c()->getData("jmp_buf").value.pointerValue, 1)

void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...);

void *malloc_mock(size_t size);
void free_mock(void *p);

extern mtx_matrix_t M1;

#ifdef __cplusplus
}
#endif

#endif
