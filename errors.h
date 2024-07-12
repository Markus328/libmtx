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

typedef void (*mtx_error_handler_t)(const char *, const char *, int,
                                    mtx_error_t, ...);

void mtx_default_error_handler(const char *file, const char *fun, int line,
                               mtx_error_t error, ...);

extern mtx_error_handler_t mtx_cfg_error_handler;

void mtx_cfg_set_error_handler(mtx_error_handler_t handler);
#define MTX_ERROR(error, ...)                                                  \
  mtx_cfg_error_handler(__FILE__, __func__, __LINE__, error, __VA_ARGS__)

#define MTX_NULL_ERR_TEXT "matrix %s is unitialized."
#define MTX_DIMEN_ERR_TEXT "matrix (%d, %d) %s has invalid dimensions."
#define MTX_INVALID_ERR_TEXT "matrix %s is invalid."
#define MTX_BOUNDS_ERR_TEXT "matrix (%d, %d) %s is out of bounds."
#define MTX_OVERLAP_ERR_TEXT                                                   \
  "matrix (%d, %d, %d, %d) %s has a bad overlap with matrix "                  \
  "(%d, %d, %d, %d) %s."

#define MTX_NULL_ERR(M) MTX_ERROR(NULL_ERR, #M)
#define MTX_DIMEN_ERR(M) MTX_ERROR(DIMEN_ERR, (M)->dy, (M)->dx, #M)
#define MTX_INVALID_ERR(M) MTX_ERROR(INVALID_ERR, #M)
#define MTX_BOUNDS_ERR(M) MTX_ERROR(BOUNDS_ERR, (M)->dy, (M)->dx, #M)
#define MTX_OVERLAP_ERR(M1, M2)                                                \
  MTX_ERROR(OVERLAP_ERR, (M1)->offY, (M1)->offX, (M1)->dy, (M1)->dx, #M1,      \
            (M2)->offY, (M2)->offX, (M2)->dy, (M2)->dx, #M2)

#define MTX_ENSURE_INIT(M)                                                     \
  if ((M)->data == NULL) {                                                     \
    MTX_NULL_ERR(M);                                                           \
  }

#endif
