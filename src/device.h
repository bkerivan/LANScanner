#ifndef DEVICE_H
#define DEVICE_H

#include <net/if.h>
#include <netinet/in.h>

/*
 * Stores the necessary information from an IPv4 network interface.
 *
 * bcast will be NULL if the interface does not have a broadcast address.
 */
struct device
{
    char name[IFNAMSIZ];
    struct sockaddr_in local;
    struct sockaddr_in netmask;
    struct sockaddr_in *bcast;
};

/*
 * Finds and returns a running, non-loopback IPv4 network interface with
 * resources allocated.
 *
 * If dev_name is not NULL, attempts to find a device with the provided name
 * that meets the above specifications.
 *
 * On success, returns a pointer to a dynamically allocated struct device;
 * upon error returns NULL and sets errno.
 *
 * If no suitable device can be found, returns NULL and does not set errno.
 */
struct device *
get_live_device(const char *dev_name);

/*
 * Frees all resources associated with a struct device obtained from
 * get_live_device.
 */
void
free_device(struct device *dev);

#endif /* DEVICE_H */

