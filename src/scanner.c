#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "device.h"
#include "probe.h"
#include "scanner.h"


/*
* Set signal flag to 0 so it can be used as terminating condition
*/
int signal_flag = 0;

int
set_scan_type(struct scanner *sc, uint8_t scan_type, uint16_t port)
{
    int ret = 0;

    if (!sc)
    {
        return -1;
    }

    switch (scan_type)
    {
        case SCAN_TYPE_CONNECT:
            sc->scan_type = scan_type;
            sc->probe = connect_probe;
            sc->port = htons(port);
            sc->target.sin_port = sc->port;
            break;
        default:
            ret = -1;
    }

    return ret;
}

void
free_scanner(struct scanner *sc)
{
    if (sc)
    {
        free_device(sc->dev);
        free(sc);
        sc = NULL;
    }
}

struct scanner *
init_scanner(uint8_t scan_type, const char *dev_name, struct timeval *timeout,
             uint16_t port)
{
    struct scanner *sc = NULL;

    sc = malloc(sizeof(*sc));

    if (!sc)
    {
        return NULL;
    }

    memset(sc, 0, sizeof(*sc));

    sc->dev = get_live_device(dev_name);

    if (!sc->dev)
    {
        free(sc);
        return NULL;
    }

    if (set_scan_type(sc, scan_type, port))
    {
        free_scanner(sc);
        errno = EINVAL;
        return NULL;
    }

    sc->timeout.tv_sec = !timeout ? SCANNER_DEFAULT_TIMEOUT_SEC
                                  : timeout->tv_sec;
    sc->timeout.tv_usec = !timeout ? SCANNER_DEFAULT_TIMEOUT_USEC
                                   : timeout->tv_usec;

    sc->target.sin_family = AF_INET;

    sc->start = sc->dev->local.sin_addr.s_addr
                & sc->dev->netmask.sin_addr.s_addr;
    sc->end = sc->start | (~sc->dev->netmask.sin_addr.s_addr);

    sc->fd = -1;

    return sc;
}

void
run_scan(struct scanner *sc, probe_up_callback_t up_callback,
         probe_down_callback_t down_callback,
         probe_error_callback_t error_callback)
{
    if (!sc)
    {
        return;
    }

    sc->up_callback = up_callback;
    sc->down_callback = down_callback;
    sc->error_callback = error_callback;

    for (sc->target.sin_addr.s_addr = sc->start;
         sc->target.sin_addr.s_addr != sc->end && !signal_flag;
         sc->target.sin_addr.s_addr = htonl(ntohl(sc->target.sin_addr.s_addr)
                                            + 1))
    {
        if (sc->target.sin_addr.s_addr == sc->dev->local.sin_addr.s_addr)
        {
            if (sc->up_callback)
            {
                sc->up_callback(sc);
            }

            continue;
        }
        
        if (sc->dev->bcast)
        {
            if (sc->target.sin_addr.s_addr == sc->dev->bcast->sin_addr.s_addr)
            {
                if (sc->up_callback)
                {
                    sc->up_callback(sc);
                }

                continue;
            }
        }

        sc->probe(sc);
    }
}

