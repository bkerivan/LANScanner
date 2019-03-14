#ifndef PACKETS_H
#define PACKETS_H

#include <stddef.h>
#include <stdint.h>

/*
 * ICMP echo request header
 */
struct icmp_echo_header
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
};

/*
 * Calculates and returns the host byte order IP checksum (RFC 791) of the
 * provided data of length dlen.
 */
uint16_t
ip_checksum(const void *data, size_t dlen);

/*
 * Builds and returns an ICMP echo request, containing the ICMP echo request
 * header and the provided data of length dlen (if any).
 *
 * Returns a pointer to the dynamically allocated datagram upon success;
 * returns NULL and sets errno upon error.
 */
void *
build_icmp_echo(const void *data, size_t dlen);

#endif /* PACKETS_H */

