#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket_util.h"

void
close_sock(int fd)
{
    int serrno = errno;
    close(fd);
    errno = serrno;
}

void
shutdown_sock(int fd)
{
    int serrno = errno;
    shutdown(fd, SHUT_RDWR);
    errno = serrno;
}

int
toggle_sock_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
    {
        return -1;
    }

    flags ^= O_NONBLOCK;

    return fcntl(fd, F_SETFL, flags) == -1 ? -1 : 0;
}

