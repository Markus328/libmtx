#ifndef RND_H
#define RND_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

// Buffer de chars aleatórios para minimizar as chamadas de getrandom()
struct rnd_buffer {
  unsigned char *rnds;
  size_t c;
  size_t count;
};

// O Máximo de memória em bytes de entropia armazenada por buffer.
#define RND_MAX_MEM 1024 * 1024 // 1M

#define RND_FILL_BUF(rnd, rnd_type)                                            \
  if (getrandom((rnd)->rnds, sizeof(rnd_type) * (rnd)->count, 0) < 0) {        \
    fprintf(stderr, "Error in getrandom().\n");                                \
  }                                                                            \
  (rnd)->c = 0

// Macro para definir as funções de random para vários tipos.
#define DEF_RND_FUNCS(rnd_type, fun_name)                                       \
  struct rnd_buffer *rnd_alloc_##fun_name##s(size_t count) {                    \
    assert(count > 0);                                                          \
    struct rnd_buffer *buf =                                                    \
        (struct rnd_buffer *)malloc(sizeof(struct rnd_buffer));                 \
                                                                                \
    count = count * sizeof(rnd_type) > RND_MAX_MEM                              \
                ? RND_MAX_MEM / sizeof(rnd_type)                                \
                : count;                                                        \
    buf->rnds = (unsigned char *)malloc(sizeof(rnd_type) * count);              \
    buf->count = count;                                                         \
                                                                                \
    RND_FILL_BUF(buf, rnd_type);                                                \
    return buf;                                                                 \
  }                                                                             \
                                                                                \
  /* Retorna um char aleatório do buffer e atualiza o buffer com entropia se   \
   */                                                                           \
  /* necessário. */                                                            \
  rnd_type rnd_get_##fun_name(struct rnd_buffer *rnd) {                         \
    if (rnd->c == rnd->count) {                                                 \
      RND_FILL_BUF(rnd, rnd_type);                                              \
    }                                                                           \
    return ((rnd_type *)rnd->rnds)[rnd->c++];                                   \
  }                                                                             \
                                                                                \
  /* Retorna o próximo char aleatório do buffer quando se tem certeza que ele \
   */                                                                           \
  /* existe. */                                                                 \
  rnd_type rnd_next_##fun_name(struct rnd_buffer *rnd) {                        \
    return ((rnd_type *)rnd->rnds)[rnd->c++];                                   \
  }                                                                             \
                                                                                \
  /* Checa se existem n chars restantes no buffer e atualiza o buffer caso não \
   */                                                                           \
  /* exista. */                                                                 \
  void rnd_prepare_##fun_name##s(struct rnd_buffer *rnd, size_t n) {            \
    assert(n <= rnd->count);                                                    \
    if (rnd->c + n >= rnd->count) {                                             \
      RND_FILL_BUF(rnd, rnd_type);                                              \
    }                                                                           \
  }

#define DEF_SIG_UNSIG_RND(rnd_type)                                            \
  DEF_RND_FUNCS(rnd_type, rnd_type)                                            \
  DEF_RND_FUNCS(unsigned rnd_type, u##rnd_type)

DEF_SIG_UNSIG_RND(char);
DEF_SIG_UNSIG_RND(int);
DEF_SIG_UNSIG_RND(short);
DEF_SIG_UNSIG_RND(long);

#undef DEF_SIG_UNSIG_RND

DEF_RND_FUNCS(long long, llong)
DEF_RND_FUNCS(unsigned long long, ullong)

DEF_RND_FUNCS(float, flt)
DEF_RND_FUNCS(double, dbl)
DEF_RND_FUNCS(long double, ldbl)

#undef DEF_RND_FUNCS
#undef RND_FILL_BUF

void rnd_free(struct rnd_buffer *rnd) {
  if (rnd != NULL) {
    free(rnd->rnds);
    free(rnd);
  }
}

#endif
