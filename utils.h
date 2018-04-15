#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

// timespec utils
int timespec_ge(const struct timespec* x, const struct timespec* y);
int timespec_cmp(const struct timespec* x, const struct timespec* y);
int timespec_diff(const struct timespec* x, const struct timespec* y, struct timespec* res);
int timespec_now(struct timespec* res);

// Error utils
void errorf(char *fmt, ...);

// Socket utils
socklen_t socklen(const struct sockaddr *sa);
void get_host_name(const struct sockaddr *sa, char* s, int slen);
void* get_in_addr(struct sockaddr *sa);
void sock_set_port(struct sockaddr *sa, u_short port);

