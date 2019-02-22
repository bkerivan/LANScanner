#include <errno.h>
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

    /*
     * Make sure scanner's file descriptor is set to -1 for error handling.
     */
    sc->fd = -1;

    if (!sc)
    {
        errno = EINVAL;
        goto error;
    }

    sc->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sc->fd == -1)
    {
        goto error;
    }

    if (toggle_sock_nonblock(sc->fd))
    {
        goto error;
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
        /*
         * Error occurred
         */
        if (ret == -1)
        {
            goto error;
        }

        /*
         * Connection timed out so assume host is not up
         */              

        close_sock(sc->fd);

        if (sc->down_callback)
        {
            sc->down_callback(sc);
        }

        return 0;
    }

    if (getsockopt(sc->fd, SOL_SOCKET, SO_ERROR, &ret, &len))
    {
        goto error;
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
     * Set ret to reflect whether host is up or down.
     */
    ret = ret == ECONNREFUSED || ret == ECONNRESET || !ret ? 1 : 0;

    /*
     * Call the proper callback if one is specified
     */
    if (!ret && sc->down_callback)
    {
        sc->down_callback(sc);
    }
    else if (sc->up_callback)
    {
        sc->up_callback(sc);
    }

    /*
     * Return the status of the host to the caller
     */
    return ret;

error:
    if (sc->fd != -1)
    {
        close_sock(sc->fd);
    }

    if (sc->error_callback)
    {
        sc->error_callback(sc);
    }

    return -1;
}

