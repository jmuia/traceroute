#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>
#include <sys/types.h>

#define IP4_IHL_MAX 60 // IPv4 IHL is 4 bits to measure size of header in 32-bit words.

// I wanted to be precise in the buffer size I would need,
// rather than choosing an arbitrarily large buffer.
#define MAXDATASIZE4 IP4_IHL_MAX + ICMP_ADVLENMIN + IP4_IHL_MAX + sizeof(struct udphdr)

struct tr_opts {
  char *hostname;
  int nprobes;
  int timeout;
  int max_ttl;
  int probe_size;
  u_short dport;
  u_short sport;
};

void traceroute4(struct tr_opts *opts);

