#ifndef MTX_ERRORS_H
#define MTX_ERRORS_H

typedef enum error_codes {
  MTX_SUCCESS = 0,
  MTX_NULL_ERR,
  MTX_DIMEN_ERR,
  MTX_INVALID_ERR,
  MTX_BOUNDS_ERR,
  MTX_OVERLAP_ERR,
  MTX_SYSTEM_ERR,
} mtx_error_t;

// filename, function name, line, error code and extra details.
typedef void (*mtx_error_handler_t)(const char *, const char *, int,
                                    mtx_error_t, ...);

extern mtx_error_handler_t __mtx_cfg_error_handler;

// Seta a função que atuará como o handler de erros.
void mtx_cfg_set_error_handler(mtx_error_handler_t handler);

#define mtx_error_handler(file, fun, line, error, ...)                         \
  __mtx_cfg_error_handler(file, fun, line, error, __VA_ARGS__)

// Handler padrão de erros.
void mtx_default_error_handler(const char *file, const char *fun, int line,
                               mtx_error_t error, ...);

#define MTX_ERROR(error, ...)                                                  \
  mtx_error_handler(__FILE__, __func__, __LINE__, error, __VA_ARGS__)

#define MTX_NULL_ERR_TEXT "matrix %s is unitialized."
#define MTX_DIMEN_ERR_TEXT "matrix (%d, %d) %s has invalid dimensions."
#define MTX_INVALID_ERR_TEXT "matrix %s is invalid."
#define MTX_BOUNDS_ERR_TEXT "matrix (%d, %d,%d, %d) %s is out of bounds."
#define MTX_OVERLAP_ERR_TEXT_1                                                 \
  "matrix (%d, %d, %d, %d) %s has an unsafe overlap with matrix "              \
  "(%d, %d, %d, %d) %s."
#define MTX_OVERLAP_ERR_TEXT_2                                                 \
  "matrix (%d, %d) %s and %s are the same and it isn't allowed in "            \
  "this function."

#define MTX_SYSTEM_ERR_TEXT "Got a system error after a call to %s()"

#define MTX_NULL_ERR(M) MTX_ERROR(MTX_NULL_ERR, M, #M)

#define MTX_DIMEN_ERR(M) MTX_ERROR(MTX_DIMEN_ERR, M, #M)

#define MTX_INVALID_ERR(M) MTX_ERROR(MTX_INVALID_ERR, M, #M)

#define MTX_BOUNDS_ERR(M) MTX_ERROR(MTX_BOUNDS_ERR, M, #M)

#define MTX_OVERLAP_ERR(M1, M2) MTX_ERROR(MTX_OVERLAP_ERR, M1, #M1, M2, #M2)

#define MTX_SYSTEM_ERR(fun_error) MTX_ERROR(MTX_SYSTEM_ERR, fun_error)

#define MTX_ENSURE_INIT(M)                                                     \
  if ((M)->data == NULL) {                                                     \
    MTX_NULL_ERR(M);                                                           \
  }

#endif
