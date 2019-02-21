#include <errno.h>
#include <sys/select.h>

#include "probe.h"
#include "socket_util.h"

int
tcp_probe(struct sockaddr *target, uint16_t port, struct timeval *timeout)
{
    int fd = 0, ret = 0;
    socklen_t len = sizeof(ret);
    fd_set fds;

    if (!target)
    {
        errno = EINVAL;
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == -1)
    {
        return -1;
    }

    if (toggle_sock_nonblock(fd))
    {
        close_sock(fd);
        return -1;
    }

    SET_PORT(target, port);
    connect(fd, target, sizeof(*target));

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    ret = select(fd + 1, NULL, &fds, NULL, timeout);

    /*
     * Connection timed out (host is not up) or error occurred in select
     */
    if (ret <= 0)
    {
        close_sock(fd);
        return ret;
    }

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len))
    {
        close_sock(fd);
        return -1;
    }

    /*
     * Shut down socket if conection was successful
     */
    if (!ret)
    {
        shutdown_sock(fd);
    }

    close_sock(fd);

    /*
     * If connection was successful or if remote host refused or reset the
     * connection, host is up; otherwise, host is not up
     */
    return ret == ECONNREFUSED || ret == ECONNRESET || !ret ? 1 : 0;
}

