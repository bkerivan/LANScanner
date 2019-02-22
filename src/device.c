#include <errno.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "device.h"

struct device *
get_live_device(const char *dev_name)
{
    struct ifaddrs *ifa = NULL, *ifap = NULL;
    struct device *dev = NULL;

    if (getifaddrs(&ifa))
    {
        return NULL;
    }

    for (ifap = ifa; ifap; ifap = ifap->ifa_next)
    {
        /*
         * Interface needs to have a local address and netmask
         */
        if (ifap->ifa_addr && ifap->ifa_netmask)
        {
            /*
             * Find a suitable interface
             */
            if (ifap->ifa_flags & IFF_UP && ifap->ifa_flags & IFF_RUNNING
                && !(ifap->ifa_flags & IFF_LOOPBACK)
                && ifap->ifa_addr->sa_family == AF_INET)
            {
                /*
                 * If device name has been specified, make sure the suitable
                 * interface has the desired name.
                 */
                if (dev_name)
                {
                    if (strcmp(ifap->ifa_name, dev_name))
                    {
                        continue;
                    }
                }

                break;
            }
        }
    }

    /*
     * No such interface
     */
    if (!ifap)
    {
        freeifaddrs(ifa);
        
        /*
         * Make sure errno is not set
         */
        errno = 0;
        return NULL;
    }

    dev = malloc(sizeof(*dev));
    
    if (!dev)
    {
        freeifaddrs(ifa);
        return NULL;
    }

    memset(dev, 0, sizeof(*dev));
    strncpy(dev->name, ifap->ifa_name, IFNAMSIZ);
    memcpy(&dev->local, ifap->ifa_addr, sizeof(dev->local));
    memcpy(&dev->netmask, ifap->ifa_netmask, sizeof(dev->netmask));

    /*
     * Allocate and store broadcast address if interface has one
     */
    if (ifap->ifa_dstaddr && ifap->ifa_flags & IFF_BROADCAST)
    {
        dev->bcast = malloc(sizeof(*dev->bcast));

        if (!dev->bcast)
        {
            free(dev);
            freeifaddrs(ifa);
            return NULL;
        }

        memset(dev->bcast, 0, sizeof(*dev->bcast));
        memcpy(dev->bcast, ifap->ifa_dstaddr, sizeof(*dev->bcast));
    }

    return dev;
}

void
free_device(struct device *dev)
{
    if (dev)
    {
        if (dev->bcast)
        {
            free(dev->bcast);
        }

        free(dev);
        dev = NULL;
    }
}

