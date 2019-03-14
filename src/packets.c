#include <errno.h>
#include <string.h>

#include "packets.h"
#include "util.h"

uint16_t
ip_checksum(const void *data, size_t dlen)
{
    unsigned long sum = 0;
    uint16_t *ptr = (uint16_t *) data;

    /*
     * Just return 0 for the checksum if no data was provided or the data
     * length was not specified
     */
    if (!ptr || !dlen)
    {
        return 0;
    }

    while (dlen > 1)
    {
        sum += *ptr++;
        dlen -= 2;
    }

    if (dlen > 0)
    {
        sum += *ptr;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~sum;
}

void *
build_icmp_echo(const void *data, size_t dlen)
{
    unsigned char *dgram = NULL;
    struct icmp_echo_header header = {0}; 

    /*
     * If packet has no data, make sure dlen matches
     */
    if ((data && !dlen) || (!data && dlen))
    {
        errno = EINVAL;
        return NULL;
    }

    /*
     * ICMP Echo type
     */
    header.type = 8; 

    dgram = zmalloc(sizeof(header) + dlen);
    
    if (!dgram)
    {
        return NULL;
    }

    memcpy(dgram, &header, sizeof(header));

    if (data)
    {
        memcpy(dgram + sizeof(header), data, dlen);
    }

    header.checksum = ip_checksum(dgram, sizeof(header) + dlen);

    /*
     * Perhaps not the most efficient approach, but have to clear datagram and
     * re-copy header + data now that checksum has been calculated
     */
    memset(dgram, 0, sizeof(header) + dlen);
    memcpy(dgram, &header, sizeof(header));

    if (data)
    {
        memcpy(dgram + sizeof(header), data, dlen);
    }

    return dgram;
}

