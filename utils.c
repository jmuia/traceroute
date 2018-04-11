#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
 * It is safe to use one of the diff arguments as the result. eg. x is also res.
 */
int timespec_diff(const struct timespec* x, const struct timespec* y, struct timespec* res) {
  long BILLION = 1000000000;
  const struct timespec *big, *small;
  int sign, carry_secs = 0, carry_nsecs = 0;

  assert(x->tv_nsec < BILLION && y->tv_nsec < BILLION);

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
    carry_secs = -1;
    carry_nsecs = BILLION;
  }
  res->tv_sec = big->tv_sec - small->tv_sec + carry_secs;
  res->tv_nsec = big->tv_nsec - small->tv_nsec + carry_nsecs;
  return sign;
}

/**
 * Prints a formatted message to stderr, prints a friendly version of errno, and then exits with error code 1.
 */
void errorf(char *fmt, ...) {
  int errnobak = errno;
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  errno = errnobak;
  perror(NULL);
  exit(1);
}

