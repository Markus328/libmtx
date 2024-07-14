#ifndef MTX_ERRORS_H
#define MTX_ERRORS_H

typedef enum error_codes {
  SUCCESS = 0,
  NULL_ERR,
  DIMEN_ERR,
  INVALID_ERR,
  BOUNDS_ERR,
  OVERLAP_ERR,
} mtx_error_t;

// filename, function name, line, error code and extra details.
typedef void (*mtx_error_handler_t)(const char *, const char *, int,
                                    mtx_error_t, ...);

extern mtx_error_handler_t __mtx_cfg_error_handler;

// Seta a função que atuará como o handler de erros.
void mtx_cfg_set_error_handler(mtx_error_handler_t handler);

// Retorna o handler de erros.
mtx_error_handler_t mtx_cfg_get_error_handler();

// Handler padrão de erros.
void mtx_default_error_handler(const char *file, const char *fun, int line,
                               mtx_error_t error, ...);

#define MTX_ERROR(error, ...)                                                  \
  mtx_cfg_get_error_handler()(__FILE__, __func__, __LINE__, error, __VA_ARGS__)

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
#define MTX_UNSAFE_ERR_TEXT                                                    \
  "attempt to write on output matrix (%d, %d, %d, %d) %s which is "            \
  "overlapping unsafely one of the input matrixes (enable "                    \
  "mtx_cfg_unsafe_overlappings to handle this)."

#define MTX_NULL_ERR(M) MTX_ERROR(NULL_ERR, M, #M)

#define MTX_DIMEN_ERR(M) MTX_ERROR(DIMEN_ERR, M, #M)

#define MTX_INVALID_ERR(M) MTX_ERROR(INVALID_ERR, M, #M)

#define MTX_BOUNDS_ERR(M) MTX_ERROR(BOUNDS_ERR, M, #M)

#define MTX_OVERLAP_ERR(M1, M2) MTX_ERROR(OVERLAP_ERR, M1, #M1, M2, #M2)

#define MTX_ENSURE_INIT(M)                                                     \
  if ((M)->data == NULL) {                                                     \
    MTX_NULL_ERR(M);                                                           \
  }

#endif
