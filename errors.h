typedef enum error_codes {
  SUCCESS = 0,
  NULL_ERR,
  DIMEN_ERR,
  INVALID_ERR,
  BOUNDS_ERR,
  OVERLAP_ERR,
} mtx_error_t;

#define MTX_ERROR(text, error, ...)                                            \
  fprintf(stderr, "MTX ERROR in %s(): " text "\n", __func__, __VA_ARGS__);     \
  abort()

#define MTX_NULL_ERR(M) MTX_ERROR("matrix %s is unitialized.", NULL_ERR, #M)
#define MTX_DIMEN_ERR(M)                                                       \
  MTX_ERROR("matrix (%d, %d) %s has invalid dimensions.", DIMEN_ERR, (M)->dy,  \
            (M)->dx, #M)
#define MTX_INVALID_ERR(M) MTX_ERROR("matrix %s is invalid.", INVALID_ERR, #M)
#define MTX_BOUNDS_ERR(M)                                                      \
  MTX_ERROR("matrix (%d, %d) %s is out of bounds.", BOUNDS_ERR, (M)->dy,       \
            (M)->dx, #M)
#define MTX_OVERLAP_ERR(M1, M2)                                                \
  MTX_ERROR("matrix (%d, %d, %d, %d) %s has a bad overlap with matrix "        \
            "(%d, %d, %d, %d) %s.",                                            \
            OVERLAP_ERR, (M1)->offY, (M1)->offX, (M1)->dy, (M1)->dx, #M1,      \
            (M2)->offY, (M2)->offX, (M2)->dy, (M2)->dx, #M2)

#define MTX_ENSURE_INIT(M)                                                     \
  if ((M)->data == NULL) {                                                     \
    MTX_NULL_ERR(M);                                                           \
  }
