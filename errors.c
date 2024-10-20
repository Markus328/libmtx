#include "errors.h"
#include "matrix.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_AND_ABORT(text, ...)                                             \
  fprintf(stderr, "MTX ERROR in %s on %s() at line %d: ", file, fun, line);    \
  fprintf(stderr, text "\n", __VA_ARGS__);                                     \
  va_end(list);                                                                \
  abort()

#define GET_MTX va_arg(list, mtx_matrix_t *)
#define GET_STRING va_arg(list, const char *)

#define GET_MTX_AND_NAME                                                       \
  M = GET_MTX;                                                                 \
  M_NAME = va_arg(list, const char *)

#define GET_TWO_MTX_AND_NAME                                                   \
  M1 = GET_MTX;                                                                \
  M1_NAME = va_arg(list, const char *);                                        \
  M2 = GET_MTX;                                                                \
  M2_NAME = va_arg(list, const char *);

#define PASS_NAME(M) M##_NAME
#define PASS_DIMEN_NAME(M) (M)->dy, (M)->dx, PASS_NAME(M)
#define PASS_BOUNDS_NAME(M) (M)->offY, (M)->offX, PASS_DIMEN_NAME(M)

void mtx_default_error_handler(const char *file, const char *fun, int line,
                               mtx_error_t error, ...) {

  va_list list;
  va_start(list, error);

  mtx_matrix_t *M, *M1, *M2;
  const char *M_NAME;
  const char *M1_NAME;
  const char *M2_NAME;
  switch (error) {
  case MTX_SUCCESS:
    fprintf(
        stderr,
        "MTX ABORT: suddenly stopped with no errors reported in %s on %s() at "
        "line %d.\n",
        file, fun, line);
    va_end(list);
    abort();
    break;
  case MTX_NULL_ERR:
    GET_MTX_AND_NAME;
    PRINT_AND_ABORT(MTX_NULL_ERR_TEXT, PASS_NAME(M));
    break;
  case MTX_DIMEN_ERR:
    GET_MTX_AND_NAME;
    PRINT_AND_ABORT(MTX_DIMEN_ERR_TEXT, PASS_DIMEN_NAME(M));
    break;
  case MTX_INVALID_ERR:
    GET_MTX_AND_NAME;
    PRINT_AND_ABORT(MTX_INVALID_ERR_TEXT, PASS_NAME(M));
    break;
  case MTX_BOUNDS_ERR:
    GET_MTX_AND_NAME;
    PRINT_AND_ABORT(MTX_BOUNDS_ERR_TEXT, PASS_BOUNDS_NAME(M));
    break;
  case MTX_OVERLAP_ERR:
    GET_TWO_MTX_AND_NAME;

    if (MTX_MATRIX_ARE_SAME(M1, M2)) {
      PRINT_AND_ABORT(MTX_OVERLAP_ERR_TEXT_2, PASS_DIMEN_NAME(M1),
                      PASS_NAME(M2));
    } else {
      PRINT_AND_ABORT(MTX_OVERLAP_ERR_TEXT_1, PASS_BOUNDS_NAME(M1),
                      PASS_BOUNDS_NAME(M2));
    }
    break;
  case MTX_SYSTEM_ERR:
    PRINT_AND_ABORT(MTX_SYSTEM_ERR_TEXT, GET_STRING);
    break;
  default:
    PRINT_AND_ABORT("Unknown error %d.", error);
  }
}

#undef PRINT_AND_ABORT
#undef PASS_NAME
#undef PASS_DIMEN_NAME
#undef PASS_BOUNDS_NAME
#undef GET_TWO_MTX_AND_NAME
#undef GET_MTX_AND_NAME
#undef GET_MTX
#undef GET_STRING

mtx_error_handler_t __mtx_cfg_error_handler = mtx_default_error_handler;

void mtx_cfg_set_error_handler(mtx_error_handler_t handler) {
  // TODO: Make thread safe.
  __mtx_cfg_error_handler = handler;
}
