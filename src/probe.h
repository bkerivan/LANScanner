#ifndef PROBE_H
#define PROBE_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>

/*
 * Probes remote host at specified address to determine if host is up
 * Attempts full TCP connection to specified port with specified timeout
 * Returns 1 if host is up, 0 if not, and -1 upon error
 * errno set upon error
 */
int
tcp_probe(struct sockaddr *target, uint16_t port, struct timeval *timeout);

#endif /* PROBE_H */

