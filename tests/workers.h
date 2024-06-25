#ifndef WORKERS_H
#define WORKERS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define worker_self pthread_self

typedef pthread_t worker_id;

typedef union anytype {
  int i;
  char c;
  float f;
  double d;
  long l;
  void *p;

  unsigned int ui;
  unsigned char uc;
  unsigned long ul;
} worker_data;

typedef struct worker {
  worker_id id;
  worker_data *data;
  worker_data *args;
} worker_t;

#define HAS_DATA (data != NULL)
#define WORKER_NEXT_ARG(t)                                                     \
  args->t;                                                                     \
  args = &args[1];

typedef pthread_mutex_t worker_mutex;
#define WORKER_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER

// Mutex para controlar os printfs nos workers de teste
static worker_mutex printf_lock = WORKER_MUTEX_INIT;

#define worker_mutex_unlock pthread_mutex_unlock
#define worker_mutex_lock pthread_mutex_lock

#define WORKER_PRINTF(...)                                                     \
  worker_mutex_lock(&printf_lock);                                             \
  printf("[worker %lu] ", worker_self());                                      \
  printf(__VA_ARGS__);                                                         \
  worker_mutex_unlock(&printf_lock)

#define WORKER_START_PRINT                                                     \
  worker_mutex_lock(&printf_lock);                                             \
  printf("[worker %lu] ", worker_self())

#define WORKER_END_PRINT worker_mutex_unlock(&printf_lock)

// Worker genérico.
void *_worker(void *data) {
  if (!HAS_DATA) {
    return NULL;
  }

  worker_data *args = (worker_data *)data;
  worker_data (*work)(worker_data *) = (worker_data(*)(worker_data *))args[0].p;

  int verbose = args[1].i;

  worker_data ret;

  if (verbose) {
    WORKER_PRINTF("Iniciado!\n");

    time_t t1 = time(NULL);
    ret = work(&args[2]);
    WORKER_PRINTF("Completado %lu em segundos.\n", time(NULL) - t1);
  } else {
    ret = work(&args[2]);
  }

  return ret.p;
}

worker_t worker_start(worker_data (*work)(worker_data *), int verbose,
                      worker_data *args, int argc) {
  worker_t wt;
  wt.data = (worker_data *)malloc(sizeof(worker_data) * (3 + argc));
  wt.args = &wt.data[2];
  wt.data[0] = (worker_data){.p = (void *)work};
  wt.data[1] = (worker_data){.i = verbose};
  memcpy(wt.args, args, sizeof(worker_data) * argc);
  pthread_create(&wt.id, NULL, _worker, wt.data);

  return wt;
}

worker_data worker_join(worker_t *wt) {
  void *ret;
  pthread_join(wt->id, &ret);

  free(wt->data);
  wt->data = NULL;
  wt->args = NULL;
  return (worker_data){.p = ret};
}

#endif
