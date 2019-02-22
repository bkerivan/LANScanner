#ifndef PROBE_H
#define PROBE_H

#include "scanner.h"

/*
 * Probes remote host at specified address to determine if host is up.
 * Attempts full TCP connection to specified port with specified timeout.
 *
 * Returns 1 if host is up, 0 if not.
 * Upon error, returns -1 and sets errno.
 */
int
connect_probe(struct scanner *sc);

#endif /* PROBE_H */

