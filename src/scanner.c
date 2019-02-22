#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "device.h"
#include "probe.h"
#include "scanner.h"
#include "util.h"


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
            sc->probe = connect_probe;
            break;
        default:
            ret = -1;
    }

    /*
     * So we don't have to repeat this for each case in switch statement
     */
    if (ret != -1)
    {
        sc->scan_type = scan_type;
    }

    /*
     * This will be extended to check for all scan types where port is relevant
     */
    if (scan_type == SCAN_TYPE_CONNECT)
    {
        sc->port = port ? port : (uint16_t)(nanorand() % 65535);
        sc->target.sin_port = htons(sc->port);
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

    sc = zmalloc(sizeof(*sc));

    if (!sc)
    {
        return NULL;
    }

    sc->dev = get_live_device(dev_name);

    if (!sc->dev)
    {
        free(sc);
        return NULL;
    }

    /*
     * Set the scan type as well as the probe method and the port
     */
    if (set_scan_type(sc, scan_type, port))
    {
        free_scanner(sc);

        /*
         * An error in set_scan_type means an invalid scan type was specified
         */
        errno = EINVAL;

        return NULL;
    }

    /*
     * If timeout is NULL, use the default value
     */
    sc->timeout.tv_sec = !timeout ? SCANNER_DEFAULT_TIMEOUT_SEC
                                  : timeout->tv_sec;
    sc->timeout.tv_usec = !timeout ? SCANNER_DEFAULT_TIMEOUT_USEC
                                   : timeout->tv_usec;

    sc->target.sin_family = AF_INET;

    /*
     * Determine subnet range
     */
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
    int ret = 0;

    if (!sc)
    {
        return;
    }

    /*
     * Iterate through subnet, calling the appropriate callbacks.
     *
     * To increment target IP number, must convert from network byte order to
     * host byte order, add one, and convert back.
     */
    for (sc->target.sin_addr.s_addr = sc->start;
         sc->target.sin_addr.s_addr <= sc->end && !signal_flag;
         sc->target.sin_addr.s_addr = htonl(ntohl(sc->target.sin_addr.s_addr)
                                            + 1))
    {
        ret = sc->probe(sc);

        switch (ret)
        {
            case 0:
                if (down_callback)
                {
                    down_callback(sc);
                }
                break;
            case 1:
                if (up_callback)
                {
                    up_callback(sc);
                }
                break;
            default:    // ret is -1
                /*
                 * EINTR means a signal has been caught, which should not be
                 * considered an error condition
                 */
                if (error_callback && errno != EINTR)
                {
                    error_callback(sc);
                }
        }
    }
}

