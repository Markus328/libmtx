#include <CppUTestExt/MockSupport_c.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc_mock(size_t size);
void free_mock(void *p);
void *memcpy_mock(void *dest, const void *src, size_t size);
void *memmove_mock(void *dest, const void *src, size_t size);

#define malloc malloc_mock
#define free free_mock
#define memcpy memcpy_mock
#define memmove memmove_mock
#include "../errors.c"
#include "../linalg.c"
#include "../matrix.c"
#undef malloc
#undef free
#undef memcpy
#undef memmove

void *malloc_mock(size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (act->hasReturnValue()) {
    return act->pointerReturnValue();
  }
  return malloc(size);
}
void free_mock(void *p) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);

  free(p);
}

void *memcpy_mock(void *dest, const void *src, size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (act->hasReturnValue()) {
    return act->pointerReturnValue();
  }

  return memcpy(dest, src, size);
}
void *memmove_mock(void *dest, const void *src, size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (act->hasReturnValue()) {
    return act->pointerReturnValue();
  }

  return memmove(dest, src, size);
}

#ifdef __cplusplus
}
#endif
