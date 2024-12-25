#include "test_utils.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdio.h>

#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...) {
  mock_c()->actualCall(__func__)->withIntParameters("error", error);
  // jump to test's stack.
  THROW(INTEGER, error);
}

struct exception_stack __EXP_STACK = {{0}, 0};
void throw_error(const char *msg_fmt, ...) {

  va_list args;
  va_start(args, msg_fmt);

  char msg[MAX_ERROR_LENGHT + 1];

  vsnprintf(msg, MAX_ERROR_LENGHT + 1, msg_fmt, args);

  va_end(args);

  FAIL_TEXT_C(msg);
}

static inline FILE *__open_fd(struct test_mtx_files *files,
                              const char *filename) {
  char filepath[128];
  if (snprintf(filepath, sizeof(filepath), "files/%s/%s/%s.txt",
               files->groupname, files->testname, filename) < 0) {
    throw_error("Error reading test matrix: path for matrix file is too big\n");
  }

  FILE *tmtx_file = fopen(filepath, "r");
  if (tmtx_file == NULL) {
    throw_error("Error reading test matrix: cannot open matrix's file: %s\n",
                filepath);
  }
  return tmtx_file;
}

static inline void __close_fd(mtx_file *file) {
  if (file->stream != NULL) {
    fclose(file->stream);
    file->stream = NULL;
  }
}

mtx_file *get_mtx_fd(struct test_mtx_files *files, const char *filename) {

  mtx_file *fds = files->fds;
  for (int i = 0; i < files->size; ++i) {
    if (fds[i].filename == filename) {
      if (fds[i].stream == NULL) {
        fds[i].stream = __open_fd(files, filename);
      }
      return &fds[i];
    }
  }

  if (files->size >= MAX_TEST_FILES) {
    throw_error("Too many test files for test %s of group %s.", files->testname,
                files->groupname);
  }

  fds[files->size].filename = filename;

  fds[files->size].stream = __open_fd(files, filename);
  return &fds[files->size++];
}

void close_mtx_fds(struct test_mtx_files *files) {
  for (int i = 0; i < files->size; ++i) {
    __close_fd(&files->fds[i]);
  }
}

void copy_test_matrix_from(mtx_matrix_t *M, struct test_mtx_files *files,
                           const char *filename) {

  mtx_file *fd = get_mtx_fd(files, filename);
  mtx_matrix_fread(fd->stream, M);
  __close_fd(fd);
}
mtx_matrix_t test_matrix_from(struct test_mtx_files *files,
                              const char *filename) {

  mtx_file *fd = get_mtx_fd(files, filename);

  mtx_matrix_t m;
  mtx_matrix_finit(fd->stream, &m);
  __close_fd(fd);

  return m;
}

mtx_matrix_t next_matrix_of(struct test_mtx_files *files,
                            const char *filename) {
  mtx_matrix_t m = {0};
  FILE *stream = get_mtx_fd(files, filename)->stream;

  while (!feof(stream)) {
    mtx_matrix_finit(stream, &m);
    if (m.data == NULL) {
      int c;
      while ((c = fgetc(stream)) != '\n' && c != EOF)
        ;
      continue;
    }
    return m;
  }

  throw_error("Cannot read any matrix in %s/%s/%s.txt", files->groupname,
              files->testname, filename);

  return m;
}
void copy_next_matrix_of(mtx_matrix_t *M, struct test_mtx_files *files,
                         const char *filename) {
  FILE *stream = get_mtx_fd(files, filename)->stream;
  mtx_matrix_t m = *M;

  int c = 0;
  while (!feof(stream)) {
    TRY mtx_matrix_fread(stream, &m);
    CATCH(INTEGER, error) {
      while ((c = fgetc(stream)) != '\n' && c != EOF)
        ;
      continue;
    }
    break;
  }

  if (c == EOF) {
    throw_error("Cannot read any %dx%d matrix in %s/%s/%s.txt", m.dy, m.dx,
                files->groupname, files->testname, filename);
  }
}

mtx_matrix_t get_mtx_from(const char *path) {
  mtx_matrix_t m;
  FILE *fd = fopen(path, "r");
  mtx_matrix_finit(fd, &m);
  fclose(fd);

  return m;
}

mtx_matrix_t __M = {0};
