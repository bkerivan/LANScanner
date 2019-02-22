#ifndef PROBE_H
#define PROBE_H

#include <netinet/in.h>
#include <stdint.h>
#include <sys/time.h>

/*
 * Probes remote host at specified address to determine if host is up.
 * Attempts full TCP connection to specified port with specified timeout.
 *
 * Returns 1 if host is up, 0 if not.
 * Upon error, returns -1 and sets errno.
 */
int
tcp_probe(struct sockaddr_in *target, uint16_t port, struct timeval *timeout);

#endif /* PROBE_H */

