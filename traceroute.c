/**
 * traceroute host -- print the route ip packets take to `host`
 *
 * Trace the route to `host` by sending ip packets with short ttl
 * and using ICMP error advice messages to gain information about
 * intermediaries.
 */

#define _GNU_SOURCE // required for sys/time.h on GNU/Linux.

#include <arpa/inet.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "traceroute.h"
#include "utils.h"

static int assess_icmp_message4(const struct tr_opts *opts, u_short seq);
static int receive_icmp_message(struct timespec *timeout);
static int get_probe_response4(const struct tr_opts *opts, u_short seq);
static void send_probe4(int ttl, u_short seq, const struct tr_opts *opts);
static void recv_socket4();
static void send_socket4(u_short *sport, const struct tr_opts *opts);

static struct tr_send {
  int fd;
  struct addrinfo *ai;
} tr_send;

static struct tr_recv {
  int fd;
  int bytes;
  char buf[MAXDATASIZE4];
  struct addrinfo *ai;
} tr_recv;

// TODO: remove this once the message is dynamic
// and based on the number of bytes desired.
static char *message = "message";

/**
 * Asseses a received ICMP error response to determine
 * how the caller should proceed.
 *
 * Returns:
 *    -3 on an indeterminate result (caller should try to receive again)
 *	  -2 on ICMP time exceeded in transit (caller should continue)
 *	  -1 on ICMP port unreachable (caller is done)
 *  >= 0 return value is some other ICMP unreachable code
 */
static int assess_icmp_message4(const struct tr_opts *opts, u_short seq) {
  int iphlen;
  struct ip *ip;
  struct icmp *icmp;
  struct udphdr *udp;

  ip = (struct ip *)tr_recv.buf;
  // ip_hl is the length of the IP header in 32-bit words
  // so we multiply by 4 to convert to bytes.
  iphlen = ip->ip_hl << 2;

  if (tr_recv.bytes < iphlen + ICMP_MINLEN) {
    return -3;
  }

  icmp = (struct icmp *)(tr_recv.buf + iphlen);

  if (!((icmp->icmp_type == ICMP_TIMXCEED &&
         icmp->icmp_code == ICMP_TIMXCEED_INTRANS) ||
        (icmp->icmp_type == ICMP_UNREACH))) {
    return -3;
  }

  // Size of the ICMP error (advice) message (including IP options).
  if (tr_recv.bytes < iphlen + ICMP_ADVLEN(icmp)) {
    return -3;
  }

  // Cast to char* so that pointer arithmetic is in bytes.
  udp = (struct udphdr *)((char *)&icmp->icmp_ip + (icmp->icmp_ip.ip_hl << 2));

  // Ensure ICMP response is for this traceroute process.
  if (icmp->icmp_ip.ip_p == IPPROTO_UDP &&
      udp->uh_sport == htons(opts->sport) &&
      udp->uh_dport == htons(opts->dport + seq)) {
    if (icmp->icmp_code == ICMP_UNREACH_PORT) {
      return -1;
    } else {
      return -2;
    }
  }
  return -3;
}

/**
 * Waits `timeout` to receive an ICMP message.
 * Returns as soon as data is available.
 *
 * Returns -1 on timeout and 0 otherwise.
 */
static int receive_icmp_message(struct timespec *timeout) {
  struct timeval tv_timeout;

  // setsockopt() expects a timeval.
  TIMESPEC_TO_TIMEVAL(&tv_timeout, timeout);
  if (setsockopt(tr_recv.fd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout,
                 sizeof(tv_timeout)) == -1) {
    errorf("socket: failed to set recvfrom timeout\n");
  }

  if ((tr_recv.bytes =
           recvfrom(tr_recv.fd, tr_recv.buf, sizeof(tr_recv.buf), 0,
                    tr_recv.ai->ai_addr, &tr_recv.ai->ai_addrlen)) == -1) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      return -1;
    } else {
      errorf("recvfrom: failed to recvfrom ICMP message\n");
    }
  }

  return 0;
}

/**
 * Attempts to receive a response to the probe.
 *
 * Returns:
 *    -3 on timeout
 *	  -2 on ICMP time exceeded in transit (caller should continue)
 *	  -1 on ICMP port unreachable (caller is done)
 *  >= 0 return value is some other ICMP unreachable code
 */
static int get_probe_response4(const struct tr_opts *opts, u_short seq) {
  int result;
  struct timespec timeout, start, end, delta;

  timeout.tv_sec = opts->timeout;
  timeout.tv_nsec = 0;

  do {
    timespec_now(&start);

    if (receive_icmp_message(&timeout) == -1) {
      return -3;
    }
    if ((result = assess_icmp_message4(opts, seq)) != -3) {
      return result;
    }

    // Update the delta and check if we've exceeded the time limit.
    timespec_now(&end);
    assert(timespec_diff(&end, &start, &delta) == 1);
    if (timespec_ge(&delta, &timeout)) {
      return -3;
    }

    // Update the remaining time limit.
    assert(timespec_diff(&timeout, &delta, &timeout) == 1);
    assert(timeout.tv_sec != 0 || timeout.tv_nsec != 0);
  } while (1);
}

/**
 * Sends a probe with the provided TTL.
 */
static void send_probe4(int ttl, u_short seq, const struct tr_opts *opts) {
  // TODO: create message depending on opts->probe_size
  sock_set_port(tr_send.ai->ai_addr, htons(opts->dport + seq));
  if (setsockopt(tr_send.fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
    errorf("setsockopt: failed to set time-to-live");
  }
  if (sendto(tr_send.fd, message, sizeof(message), 0, tr_send.ai->ai_addr,
             tr_send.ai->ai_addrlen) == -1) {
    errorf("sendto: failed to send packet with TTL %d\n", ttl);
  }
}

/**
 * Initializes the global tr_recv struct used to receive ICMP messages.
 */
static void recv_socket4() {
  int rv;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;

  if ((rv = getaddrinfo("localhost", NULL, &hints, &tr_recv.ai)) != 0) {
    errorf("getaddrinfo: %s\n", gai_strerror(rv));
  }

  if ((tr_recv.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    errorf("socket: failed to create ICMP socket\n");
  }

  // Special permissions only required to open raw socket.
  setuid(getuid());
}

/**
 * Initializes the global tr_send struct used to send messages.
 */
static void send_socket4(u_short *sport, const struct tr_opts *opts) {
  int rv;
  struct addrinfo hints;
  struct sockaddr_in sabind;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(opts->hostname, NULL, &hints, &tr_send.ai)) != 0) {
    errorf("getaddrinfo: %s\n", gai_strerror(rv));
  }

  if ((tr_send.fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    errorf("socket: failed to create UDP socket\n");
  }

  // Bind to a particular local port in order to identify
  // responses meant for this traceroute process.
  *sport = (getpid() & 0xffff) | 0x8000;
  memset(&sabind, 0, sizeof(sabind));
  sabind.sin_family = AF_INET;
  sabind.sin_addr.s_addr = htonl(INADDR_ANY);
  sabind.sin_port = htons(*sport);
  if (bind(tr_send.fd, (struct sockaddr *)&sabind, sizeof(sabind)) == -1) {
    errorf("bind: failed to bind local port\n");
  }
}

void traceroute4(struct tr_opts *opts) {
  double rtt;
  u_short seq, ttl;
  int probe, done, response;
  struct timespec start, end, delta;
  char s[INET_ADDRSTRLEN];
  char h[NI_MAXHOST];

  // Setup raw socket for receiving ICMP responses.
  recv_socket4();

  // Setup socket for sending messages.
  send_socket4(&opts->sport, opts);

  inet_ntop(tr_send.ai->ai_family, get_in_addr(tr_send.ai->ai_addr), s,
            sizeof(s));
  printf("traceroute to %s (%s), %d hops max, %d byte packets\n",
         opts->hostname, s, opts->max_ttl, opts->probe_size);
  fflush(stdout);

  seq = 0;
  done = 0;
  for (ttl = 1; ttl <= opts->max_ttl && !done; ttl++) {
    printf("%2d  ", ttl);
    fflush(stdout);

    for (probe = 0; probe < opts->nprobes; probe++, seq++) {
      if (probe != 0) {
        printf("    ");
      }
      // TODO: group probe responses by IP
      timespec_now(&start);
      send_probe4(ttl, seq, opts);
      switch ((response = get_probe_response4(opts, seq))) {
      case -3:
        printf("* ");
        break;
      case -2:
      case -1:
        timespec_now(&end);
        assert(timespec_diff(&end, &start, &delta) == 1);
        rtt = delta.tv_sec * 1000.0 + (delta.tv_nsec / 1000.0 / 1000.0);
        get_host_name(tr_recv.ai->ai_addr, h, sizeof(h));
        inet_ntop(tr_recv.ai->ai_family, get_in_addr(tr_recv.ai->ai_addr), s,
                  sizeof(s));
        printf("%s (%s) %.3f ms", h, s, rtt);
        if (response == -1) {
          done = 1;
        }
        break;
      default:
        errorf("ICMP Unreachable code %d\n", response);
        break;
      }
      fflush(stdout);
      printf("\n");
    }
  }
}

int main(int argc, char *argv[]) {
  struct tr_opts opts;
  opts.nprobes = 3;
  opts.timeout = 5;
  opts.max_ttl = 64;
  opts.probe_size = sizeof(message);
  opts.dport = 33434;

  // TODO: getopts()

  if (argc != 2) {
    fprintf(stderr, "usage: hostname\n");
    exit(1);
  }

  opts.hostname = argv[1];

  traceroute4(&opts);
  return 0;
}

