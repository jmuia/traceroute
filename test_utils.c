#include <stdio.h>
#include <time.h>

#include "minunit.h"
#include "utils.h"

MU_TEST(test_timespec_cmp) {
  struct timespec x, y;
  x.tv_sec = 1;
  x.tv_nsec = 9;
  y.tv_sec = 1;
  y.tv_nsec = 9;
  mu_assert_int_eq(0, timespec_cmp(&x, &y));
  mu_assert_int_eq(0, timespec_cmp(&y, &x));

  x.tv_sec = 9;
  x.tv_nsec = 0;
  y.tv_sec = 1;
  y.tv_nsec = 9;
  mu_assert_int_eq(1, timespec_cmp(&x, &y));
  mu_assert_int_eq(-1, timespec_cmp(&y, &x));

  x.tv_sec = 0;
  x.tv_nsec = 330;
  y.tv_sec = 0;
  y.tv_nsec = 9;
  mu_assert_int_eq(1, timespec_cmp(&x, &y));
  mu_assert_int_eq(-1, timespec_cmp(&y, &x));
}

MU_TEST(test_timespec_ge) {
  struct timespec x, y;
  x.tv_sec = 1;
  x.tv_nsec = 9;
  y.tv_sec = 1;
  y.tv_nsec = 9;
  mu_assert_int_eq(1, timespec_ge(&x, &y));
  mu_assert_int_eq(1, timespec_ge(&y, &x));

  x.tv_sec = 9;
  x.tv_nsec = 0;
  y.tv_sec = 1;
  y.tv_nsec = 9;
  mu_assert_int_eq(1, timespec_ge(&x, &y));
  mu_assert_int_eq(0, timespec_ge(&y, &x));

  x.tv_sec = 0;
  x.tv_nsec = 330;
  y.tv_sec = 0;
  y.tv_nsec = 9;
  mu_assert_int_eq(1, timespec_ge(&x, &y));
  mu_assert_int_eq(0, timespec_ge(&y, &x));
}

MU_TEST(test_timespec_diff) {
  int sign;
  struct timespec x, y, res;
  x.tv_sec = 5;
  x.tv_nsec = 5000;
  y.tv_sec = 4;
  y.tv_nsec = 2000;

  sign = timespec_diff(&x, &y, &res);

  mu_assert_int_eq(1, sign);
  mu_assert_int_eq(1, res.tv_sec);
  mu_assert_int_eq(3000, res.tv_nsec);
}

MU_TEST(test_timespec_diff_eq) {
  int sign;
  struct timespec x, y, res;
  x.tv_sec = 5;
  x.tv_nsec = 5000;
  y.tv_sec = 5;
  y.tv_nsec = 5000;

  sign = timespec_diff(&x, &y, &res);

  mu_assert_int_eq(1, sign);
  mu_assert_int_eq(0, res.tv_sec);
  mu_assert_int_eq(0, res.tv_nsec);
}

MU_TEST(test_timespec_diff_carry) {
  int sign;
  struct timespec x, y, res;
  x.tv_sec = 5;
  x.tv_nsec = 100;
  y.tv_sec = 4;
  y.tv_nsec = 2000;

  sign = timespec_diff(&x, &y, &res);

  mu_assert_int_eq(1, sign);
  mu_assert_int_eq(0, res.tv_sec);
  mu_assert_int_eq(999998100, res.tv_nsec);
}

MU_TEST(test_timespec_diff_negative) {
  int sign;
  struct timespec x, y, res;
  x.tv_sec = 2;
  x.tv_nsec = 2000;
  y.tv_sec = 4;
  y.tv_nsec = 6000;

  sign = timespec_diff(&x, &y, &res);

  mu_assert_int_eq(-1, sign);
  mu_assert_int_eq(2, res.tv_sec);
  mu_assert_int_eq(4000, res.tv_nsec);
}

MU_TEST(test_timespec_diff_negative_carry) {
  int sign;
  struct timespec x, y, res;
  x.tv_sec = 2;
  x.tv_nsec = 6000;
  y.tv_sec = 4;
  y.tv_nsec = 2000;

  sign = timespec_diff(&x, &y, &res);

  mu_assert_int_eq(-1, sign);
  mu_assert_int_eq(1, res.tv_sec);
  mu_assert_int_eq(999996000, res.tv_nsec);
}

MU_TEST(test_timespec_diff_safe_update_minuend) {
  int sign;
  struct timespec x, y;
  x.tv_sec = 5;
  x.tv_nsec = 100;
  y.tv_sec = 4;
  y.tv_nsec = 2000;

  sign = timespec_diff(&x, &y, &x);
  mu_assert_int_eq(1, sign);
  mu_assert_int_eq(0, x.tv_sec);
  mu_assert_int_eq(999998100, x.tv_nsec);
}

MU_TEST(test_timespec_diff_safe_update_subtrahend) {
  int sign;
  struct timespec x, y;
  x.tv_sec = 5;
  x.tv_nsec = 100;
  y.tv_sec = 4;
  y.tv_nsec = 2000;

  sign = timespec_diff(&x, &y, &y);
  mu_assert_int_eq(1, sign);
  mu_assert_int_eq(0, y.tv_sec);
  mu_assert_int_eq(999998100, y.tv_nsec);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_timespec_cmp);
  MU_RUN_TEST(test_timespec_ge);
  MU_RUN_TEST(test_timespec_diff);
  MU_RUN_TEST(test_timespec_diff_eq);
  MU_RUN_TEST(test_timespec_diff_carry);
  MU_RUN_TEST(test_timespec_diff_negative);
  MU_RUN_TEST(test_timespec_diff_negative_carry);
  MU_RUN_TEST(test_timespec_diff_safe_update_minuend);
  MU_RUN_TEST(test_timespec_diff_safe_update_subtrahend);
}

int main() {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_status;
}
