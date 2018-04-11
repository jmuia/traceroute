#include <time.h>

int timespec_ge(const struct timespec* x, const struct timespec* y);
int timespec_cmp(const struct timespec* x, const struct timespec* y);
int timespec_diff(const struct timespec* x, const struct timespec* y, struct timespec* res);
int timespec_now(struct timespec* res);
void errorf(char *fmt, ...);

