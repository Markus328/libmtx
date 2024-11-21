#ifndef ROUT_H
#define ROUT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../matrix.h"
#include "test_utils.h"

#define MAKE_ROUTINE(name, ...)                                                \
  void __rout_##name(const char *G_NAME, const char *T_NAME,                   \
                     const char *DEFAULT_MTX,                                  \
                     struct test_mtx_files *__TEST_FILES, __VA_ARGS__)

#define CALL_ROUTINE(name, ...)                                                \
  __rout_##name(G_NAME, T_NAME, DEFAULT_MTX, __TEST_FILES, __VA_ARGS__)

#define MTX_3M_SIG mtx_matrix_t *, const mtx_matrix_t *, const mtx_matrix_t *

#define DEF_CHECK_3M(return_type, r)                                           \
  MAKE_ROUTINE(check_3m_##r, return_type (*fun)(MTX_3M_SIG), double tol, int n)

DEF_CHECK_3M(int, i);

#undef DEF_CHECK_3M

#ifdef __cplusplus
}
#endif
#endif
