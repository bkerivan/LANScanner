#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "probe.h"
#include "socket_util.h"

int
connect_probe(struct scanner *sc)
{
    int ret = 0;
    socklen_t len = sizeof(ret);
    fd_set fds;
    in_addr_t bcast = 0;

    /*
     * Make sure scanner's file descriptor is set to -1 for error handling.
     */
    sc->fd = -1;

    if (!sc)
    {
        errno = EINVAL;
        return -1;
    }

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

