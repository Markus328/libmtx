#include <CppUTestExt/MockSupport_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc_mock(size_t size);
void *realloc_mock(void *ptr, size_t size);
void free_mock(void *p);
void *memcpy_mock(void *dest, const void *src, size_t size);
void *memmove_mock(void *dest, const void *src, size_t size);
int fprintf_mock(FILE *stream, const char *format, ...);
int fscanf_mock(FILE *stream, const char *format, ...);

#define malloc malloc_mock
#define realloc realloc_mock
#define free free_mock
#define memcpy memcpy_mock
#define memmove memmove_mock
#define fprintf fprintf_mock
#define fscanf fscanf_mock

#include "../errors.c"
#include "../linalg.c"
#include "../matrix.c"
#include "../matrix_operations.c"

#undef malloc
#undef realloc
#undef free
#undef memcpy
#undef memmove
#undef fprintf
#undef fscanf

void *malloc_mock(size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (act->hasReturnValue()) {
    return act->pointerReturnValue();
  }
  return malloc(size);
}
void *realloc_mock(void *ptr, size_t size) {
  MockActualCall_c *act = mock_c()->actualCall(__func__);
  if (act->hasReturnValue()) {
    return act->pointerReturnValue();
  }
  return realloc(ptr, size); 
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

int fprintf_mock(FILE *stream, const char *format, ...) {
  MockActualCall_c *act =
      mock_c()->actualCall(__func__)->withPointerParameters("stream", stream);
  if (act->hasReturnValue()) {
    return act->intReturnValue();
  }

  va_list args;
  va_start(args, format);

  int res = vfprintf(stream, format, args);
  va_end(args);

  return res;
}
int fscanf_mock(FILE *stream, const char *format, ...) {
  MockActualCall_c *act =
      mock_c()->actualCall(__func__)->withPointerParameters("stream", stream);
  if (act->hasReturnValue()) {
    return act->intReturnValue();
  }

  va_list args;
  va_start(args, format);

  int res = vfscanf(stream, format, args);
  va_end(args);

  return res;
}

#ifdef __cplusplus
}
#endif
