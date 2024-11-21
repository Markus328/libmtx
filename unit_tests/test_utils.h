#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../errors.h"
#include "../matrix.h"
#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct test_mtx_file {
  const char *filename;
  FILE *stream;
} mtx_file;

#define MAX_TEST_FILES 32

struct test_mtx_files {
  const char *groupname, *testname;
  mtx_file fds[MAX_TEST_FILES];
  int size;
};

typedef struct exception_data {
  const char *file, *fun;
  int line;

  jmp_buf buffer;
  int state;

  MockValue_c object;
} exp_data;

struct exception_stack {
  exp_data m[32];
  int size;
};

extern struct exception_stack __EXP_STACK;

#define IF_CAN_THROW(ID) if (__EXP_STACK.size > 0)

#define EXCEPTION_DATA (__EXP_STACK.m[__EXP_STACK.size - 1])
#define CATCH_EXCEPTION_DATA (__EXP_STACK.m[__EXP_STACK.size])

#define TRY                                                                    \
  if (++__EXP_STACK.size > sizeof(__EXP_STACK.m) / sizeof(exp_data)) {         \
    throw_error(                                                               \
        "The try statment in %s on %s() at %d is too deep, too many nesting.", \
        __FILE__, __func__, __LINE__);                                         \
  }                                                                            \
  EXCEPTION_DATA.file = __FILE__;                                              \
  EXCEPTION_DATA.fun == __func__;                                              \
  EXCEPTION_DATA.line = __LINE__;                                              \
  EXCEPTION_DATA.state = setjmp(EXCEPTION_DATA.buffer);                        \
  if (EXCEPTION_DATA.state != 0) {                                             \
    __EXP_STACK.size--;                                                        \
  } else {

#define CATCH(data_type, var_name)                                             \
  __EXP_STACK.size--;                                                          \
  }                                                                            \
  MockValue_c *var_name = &(CATCH_EXCEPTION_DATA.object);                      \
  if (CATCH_EXCEPTION_DATA.state != 0 &&                                       \
      CATCH_EXCEPTION_DATA.object.type == MOCKVALUETYPE_##data_type)

#define PREP_THROW                                                             \
  do {                                                                         \
    if (__EXP_STACK.size <= 0) {                                               \
      throw_error("Unwaited Exception in %s on %s() at line %d\n", __FILE__,   \
                  __func__, __LINE__);                                         \
    }                                                                          \
  } while (0)

#define THROW(data_type, data)                                                 \
  PREP_THROW;                                                                  \
  EXCEPTION_DATA.object.value.pointerValue = (void *)data;                     \
  EXCEPTION_DATA.object.type = MOCKVALUETYPE_##data_type;                      \
  longjmp(EXCEPTION_DATA.buffer, 1)

#define MAKE_TEST(groupname, testname)                                         \
  extern void test_##groupname##_##testname##_wrapper_c();                     \
  static void __test_##groupname##_##testname(                                 \
      const char *G_NAME, const char *T_NAME, const char *DEFAULT_MTX,         \
      struct test_mtx_files *__TEST_FILES);                                    \
  void test_##groupname##_##testname##_wrapper_c() {                           \
    mock_c()->setStringData("G_NAME", #groupname);                             \
    mock_c()->setStringData("T_NAME", #testname);                              \
    struct test_mtx_files __TEST_FILES = {#groupname, #testname, {}, 0};       \
    __test_##groupname##_##testname(#groupname, #testname,                     \
                                    #groupname "/" #testname "/default.txt",   \
                                    &__TEST_FILES);                            \
    close_mtx_fds(&__TEST_FILES);                                              \
  }                                                                            \
  static void __test_##groupname##_##testname(                                 \
      const char *G_NAME, const char *T_NAME, const char *DEFAULT_MTX,         \
      struct test_mtx_files *__TEST_FILES)


void test_fail(const char *file, const char *fun, int line, mtx_error_t error,
               ...);

void throw_error(const char *msg_fmt, ...);

// Its recommended a string literal to be passed as filename. If using a non
// literal string (stored as lvalue) the same pointer to string per file should
// be used to avoid opening more than one file descriptor for the same file.
mtx_file *get_mtx_fd(struct test_mtx_files *files, const char *filename);
void close_mtx_fds(struct test_mtx_files *files);
mtx_matrix_t get_mtx_from(const char *path);
mtx_matrix_t test_matrix_from(struct test_mtx_files *files,
                              const char *filename);
void copy_test_matrix_from(mtx_matrix_t *M, struct test_mtx_files *files,
                           const char *filename);

mtx_matrix_t next_matrix_of(struct test_mtx_files *files, const char *filename);
void copy_next_matrix_of(mtx_matrix_t *M, struct test_mtx_files *files,
                         const char *filename);

#define TEST_MTX_PATH(PATH_) get_mtx_from("files/" #PATH_)
#define TEST_MTX test_matrix_from(__TEST_FILES, "default")
#define COPY_TEST_MTX(MTX) test_matrix_from((MTX), __TEST_FILES, "default")
#define TEST_MTX_NAME(NAME_) test_matrix_from(__TEST_FILES, #NAME_)

#define COPY_TEST_MTX_NAME(MTX, NAME_)                                         \
  copy_test_matrix_from((MTX), __TEST_FILES, #NAME_)

#define NEXT_TEST_MTX next_matrix_of(__TEST_FILES, "default")
#define COPY_NEXT_TEST_MTX(MTX)                                                \
  copy_next_matrix_of((MTX), __TEST_FILES, "default")

#define NEXT_TEST_MTX_NAME(NAME_) next_matrix_of(__TEST_FILES, #NAME_)
#define COPY_NEXT_TEST_MTX_NAME(MTX, NAME_)                                    \
  copy_next_matrix_of((MTX), __TEST_FILES, #NAME_)

extern mtx_matrix_t M;
#define M_DX 25
#define M_DY 25

#define RESERVE_MTX(init_i, init_j, dy, dx)                                    \
  mtx_matrix_view_of(&M, init_i, init_j, dy, dx).matrix

#ifdef __cplusplus
}
#endif

#endif
