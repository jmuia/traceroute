#include <time.h>
#include <assert.h>

#include "utils.h"

/**
 * Compares two timespec values.
 * Returns:
 *   x == y  :  0
 *   x > y   :  1
 *   x < y   : -1
 */
int timespec_cmp(const struct timespec* x, const struct timespec* y) {
  if (x->tv_sec > y->tv_sec) {
    return 1;
  } else if (x->tv_sec < y->tv_sec) {
    return -1;
  } else if (x->tv_nsec > y->tv_nsec) {
    return 1;
  } else if (x->tv_nsec < y->tv_nsec) {
    return -1;
  } else {
    return 0;
  }
}

int timespec_ge(const struct timespec* x, const struct timespec* y) {
  return timespec_cmp(x, y) >= 0;
}

/**
 * Computes the difference between x and y timespecs.
 * The absolute difference is returned in res.
 * The sign of the difference between x and y is the return value.
 * In particular, if the difference is negative the return value is -1
 * otherwise it is 1 (including for 0).
 */
int timespec_diff(const struct timespec* x, const struct timespec* y, struct timespec* res) {
  long BILLION = 1000000000;
  const struct timespec *big, *small;
  int sign;

  assert(x->tv_nsec < BILLION && y->tv_nsec < BILLION);
  res->tv_sec = 0;
  res->tv_nsec = 0;

  if (timespec_ge(x, y)) {
    big = x;
    small = y;
    sign = 1;
  } else {
    big = y;
    small = x;
    sign = -1;
  }

  if (big->tv_nsec < small->tv_nsec) {
    res->tv_sec = -1;
    res->tv_nsec = BILLION;
  }
  res->tv_sec += big->tv_sec - small->tv_sec;
  res->tv_nsec += big->tv_nsec - small->tv_nsec;
  return sign;
}
