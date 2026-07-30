#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <math.h>

extern int tests_failed;
extern int tests_run;

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define ASSERT_NEAR(a, b, epsilon) \
  ASSERT_TRUE(fabsf((float)(a) - (float)(b)) <= (epsilon))

#define ASSERT_TRUE(cond) \
  do { \
    tests_run++; \
    if (!(cond)) { \
      tests_failed++; \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
  } while (0)

#endif
