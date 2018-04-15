#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>

#if defined(__MACH__) && !defined(CLOCK_MONOTONIC)
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/kern_return.h>
#endif

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
 * Provides a timespec of the current time.
 * Prefers clock_gettime() but falls back to mach/clock_get_time()
 * on Mac OS X where CLOCK_MONOTONIC is not defined
 * since clock_gettime() is not implemented until Sierra.
 *
 * Returns 0 on success -1 on failure.
 */
int timespec_now(struct timespec* res) {
#if defined(__MACH__) && !defined(CLOCK_MONOTONIC)
  clock_serv_t cclock;
  mach_timespec_t mts;
  if (host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock) != KERN_SUCCESS) {
    return -1;
  }
  if (clock_get_time(cclock, &mts) != KERN_SUCCESS) {
    int errnobak = errno;
    mach_port_deallocate(mach_task_self(), cclock);
    errno = errnobak;
    return -1;
  } else {
    mach_port_deallocate(mach_task_self(), cclock);
    res->tv_sec = mts.tv_sec;
    res->tv_nsec = mts.tv_nsec;
    return 0;
  }
#else
  return clock_gettime(CLOCK_MONOTONIC, &res);
#endif
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

/**
 * Returns the appropriate socklen_t depending on IP version.
 */
socklen_t socklen(const struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return sizeof(struct sockaddr_in);
  } else {
    return sizeof(struct sockaddr_in6);
  }
}

/**
 * Helper to get the host name for the given sockaddr.
 */
void get_host_name(const struct sockaddr *sa, char* s, int slen) {
  int rv;
  if ((rv = getnameinfo(sa, socklen(sa), s, slen, NULL, 0, 0)) != 0) {
    errorf("getnameinfo: %s\n", gai_strerror(rv));
  }
}

/**
 * Helper to get appropriate sockaddr depending on IP version.
 */
void* get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
}

/**
 * Helper to set the port of a sockaddr for either IPv4 or IPv6.
 * `port` argument should already be in network byte order.
 */
void sock_set_port(struct sockaddr *sa, u_short port) {
  if (sa->sa_family == AF_INET) {
    ((struct sockaddr_in*)sa)->sin_port = port;
  } else {
    ((struct sockaddr_in6*)sa)->sin6_port = port;
  }
}

