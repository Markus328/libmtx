#include <CppUTest/TestHarness_c.h>
#include <CppUTestExt/MockSupport_c.h>

#include "../linalg.h"
#include "../matrix.h"
#include "test_utils.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_ERROR 1e-3

MAKE_TEST(linalg, lu_decomp){

}

#ifdef __cplusplus
}
#endif
