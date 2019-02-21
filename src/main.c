#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Gets the 32-bit IPv4 number from a sockaddr structure
 */
#define EXTRACT_INET(sa) (((struct sockaddr_in *) (sa))->sin_addr.s_addr) 

/*
 * Sets IPv4 number of sockaddr structure
 */
#define SET_INET(sa, in) (EXTRACT_INET(sa) = (in))

/*
 * Sets port (IPv4) of a sockaddr structure
 */
#define SET_PORT(sa, port) (((struct sockaddr_in *) \
                             (sa))->sin_port = htons(port))

/*
 * Gets IPv4 string representation of sockaddr
 */
#define INET_STR(sa, buf, len) (inet_ntop(AF_INET, \
                                          &((struct sockaddr_in *) \
                                            (sa))->sin_addr, buf, len))

/*
 * Default settings
 */
#define DEFAULT_PORT            80
#define DEFAULT_TIMEOUT_SEC     0
#define DEFAULT_TIMEOUT_USEC    10000

/*
 * Closes socket, ignoring errors
 */
void
close_sock(int fd)
{
    int serrno = errno;
    close(fd);
    errno = serrno;
}

/*
 * Shuts down socket, ignoring errors
 */
void
shutdown_sock(int fd)
{
    int serrno = errno;
    shutdown(fd, SHUT_RDWR);
    errno = serrno;
}

/*
 * Toggles socket nonblocking mode
 * Returns -1 upon error, 0 otherwise
 * errno set upon error
 */
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

/*
 * Probes remote host at specified port to determine if host is up
 * Attempts full TCP connection with specified timeout 
 * Returns 1 if host is up, 0 if not, and -1 upon error
 * errno set upon error
 */
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

int
main(void)
{
    struct ifaddrs *ifa = NULL, *ifap = NULL;
    in_addr_t local = 0, netmask = 0, bcast = 0, start = 0, end = 0, target;
    struct sockaddr addr = {0};
    char addrbuf[INET_ADDRSTRLEN] = {0};
    struct timeval timeout = {DEFAULT_TIMEOUT_SEC, DEFAULT_TIMEOUT_USEC};
    int ret = 0;

    if (getifaddrs(&ifa))
    {
        perror("getifaddrs");
        return 1;
    }

    for (ifap = ifa; ifap; ifap = ifap->ifa_next)
    {
        if (ifap->ifa_addr && ifap->ifa_netmask)
        {
            /*
             * Find a running, non-loopback IPv4 interface with resources
             * allocated
             */
            if (ifap->ifa_flags & IFF_UP && ifap->ifa_flags & IFF_RUNNING
                && !(ifap->ifa_flags & IFF_LOOPBACK)
                && ifap->ifa_addr->sa_family == AF_INET)
            {
                break;
            }
        }
    }

    /*
     * No such interface
     */
    if (!ifap)
    {
        fprintf(stderr, "Failed to find suitable interface\n");
        freeifaddrs(ifa);
        return 1;
    }

    local = EXTRACT_INET(ifap->ifa_addr); 
    netmask = EXTRACT_INET(ifap->ifa_netmask);

    /*
     * Store broadcast address if there is one
     */
    if (ifap->ifa_dstaddr && ifap->ifa_flags & IFF_BROADCAST)
    {
        bcast = EXTRACT_INET(ifap->ifa_dstaddr);
    }

    printf("Scanning %s\n\n", ifap->ifa_name);

    freeifaddrs(ifa);

    /*
     * Determine subnet range
     */
    start = local & netmask;
    end = start | (~netmask);

    addr.sa_family = AF_INET;

    /*
     * To increment target address, need to convert IP number from network byte
     * order to host byte order, add one, and convert back
     */
    for (target = start; target <= end; target = htonl(ntohl(target) + 1))
    {
        /*
         * Don't scan local address or broadcast address
         */
        if (target == local || target == bcast)
        {
            if (!INET_STR(&addr, addrbuf, INET_ADDRSTRLEN))
            {
                perror("INET_STR");
            }
            else
            {
                printf("%s [%s]\n", addrbuf,
                       target == local ? "YOU" : "BROADCAST");
            }

            memset(addrbuf, 0, INET_ADDRSTRLEN);
            continue;
        }

        SET_INET(&addr, target);

        ret = tcp_probe(&addr, DEFAULT_PORT, &timeout);

        if (ret <= 0)
        {
            if (ret == -1)
            {
                perror("tcp_probe");
            }

            continue;
        }

        if (!INET_STR(&addr, addrbuf, INET_ADDRSTRLEN))
        {
            perror("INET_STR");

            /*
             * If we can't get the IPv4 string, just display the hexidecimal
             * IPv4 number
             */
            printf("0x%08x\n", target);
        }
        else
        {
            puts(addrbuf);
        }

        memset(addrbuf, 0, INET_ADDRSTRLEN);
    }

    return 0;
}

