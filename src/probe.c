#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "packets.h"
#include "probe.h"
#include "socket_util.h"

int
connect_probe(struct scanner *sc)
{
    int ret = 0;
    socklen_t len = sizeof(ret);
    fd_set fds;
    in_addr_t bcast = 0;

    if (!sc)
    {
        errno = EINVAL;
        return -1;
    }

    /*
     * Make sure scanner's file descriptor is set to -1 for error handling.
     */
    sc->fd = -1;

    if (sc->dev->bcast)
    {
        bcast = sc->dev->bcast->sin_addr.s_addr;
    }

    /*
     * Return host up for local address or broadcast address
     */
    if (sc->target.sin_addr.s_addr == sc->dev->local.sin_addr.s_addr
        || sc->target.sin_addr.s_addr == bcast)
    {
        return 1;
    }

    sc->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sc->fd == -1)
    {
        return -1;
    }

    if (toggle_sock_nonblock(sc->fd))
    {
        close_sock(sc->fd);
        return -1;
    }

    connect(sc->fd, (struct sockaddr *) &sc->target, sizeof(sc->target));

    FD_ZERO(&fds);
    FD_SET(sc->fd, &fds);

    ret = select(sc->fd + 1, NULL, &fds, NULL, &sc->timeout);

    /*
     * Connection timed out (host is not up) or error occurred in select
     */
    if (ret <= 0)
    {
        close_sock(sc->fd);

        /*
         * Either ret is -1 and error occurred or ret is 0 and connection timed
         * out, meaning host should be considered down.
         */
        return ret;
    }

    if (getsockopt(sc->fd, SOL_SOCKET, SO_ERROR, &ret, &len))
    {
        close_sock(sc->fd);
        return -1;
    }

    /*
     * Shut down socket if conection was successful
     */
    if (!ret)
    {
        shutdown_sock(sc->fd);
    }

    close_sock(sc->fd);

    /*
     * If connection was successful or if remote host refused or reset the
     * connection, host is up; otherwise, host is not up.
     */
    return ret == ECONNREFUSED || ret == ECONNRESET || !ret ? 1 : 0;
}

int
icmp_probe(struct scanner *sc)
{
    int ret = 0;
    socklen_t len = sizeof(ret);
    fd_set fds;
    in_addr_t bcast = 0;
    void *dgram = NULL;
    struct sockaddr_in remote_host = {0};
    
    /*
     * Sending just 1 byte of data
     */
    size_t dgram_len = sizeof(struct icmp_echo_header) + 1;

    if (!sc)
    {
        errno = EINVAL;
        return -1;
    }

    /*
     * Make sure scanner's file descriptor is set to -1 for error handling.
     */
    sc->fd = -1;

    if (sc->dev->bcast)
    {
        bcast = sc->dev->bcast->sin_addr.s_addr;
    }

    /*
     * Return host up for local address or broadcast address
     */
    if (sc->target.sin_addr.s_addr == sc->dev->local.sin_addr.s_addr
        || sc->target.sin_addr.s_addr == bcast)
    {
        return 1;
    }
    
    /*
     * ICMP datagram sockets (unprivilegd) are only supported on MacOS 
     */
#ifdef __APPLE__    
    sc->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
#else
    sc->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
#endif /* __APPLE__ */

    if (sc->fd == -1)
    {
        return -1;
    }

    /*
     * Might as well just send a character
     */
    dgram = build_icmp_echo("A", 1);

    if (!dgram)
    {
        close_sock(sc->fd);
        return -1;
    }

    if (sendto(sc->fd, dgram, dgram_len, 0, (struct sockaddr *) &sc->target,
               sizeof(sc->target)) == -1)
    {
        free(dgram);
        close_sock(sc->fd);
        return -1;
    }

    FD_ZERO(&fds);
    FD_SET(sc->fd, &fds);

    ret = select(sc->fd + 1, &fds, NULL, NULL, &sc->timeout);

    if (ret <= 0)
    {
        free(dgram);
        close_sock(sc->fd);
        return ret;
    }

    if (getsockopt(sc->fd, SOL_SOCKET, SO_ERROR, &ret, &len))
    {
        free(dgram);
        close_sock(sc->fd);
        return -1;
    }

    if (ret)
    {
        free(dgram);
        close_sock(sc->fd);
        errno = ret;
        return -1;
    }

    len = sizeof(remote_host);
    memset(dgram, 0, dgram_len); 

    if (recvfrom(sc->fd, dgram, dgram_len, 0, (struct sockaddr *) &remote_host,
                 &len) == -1)
    {
        free(dgram);
        close_sock(sc->fd);
        return -1;
    }

    free(dgram);
    close_sock(sc->fd);

    /*
     * Host is only up if an ICMP packet was received from the target host
     */
    return memcmp(&remote_host.sin_addr, &sc->target.sin_addr,
                  sizeof(struct in_addr)) ? 0 : 1;
}

