#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "probe.h"
#include "socket_util.h"

/*
 * Default settings
 */
#define DEFAULT_PORT            80
#define DEFAULT_TIMEOUT_SEC     0
#define DEFAULT_TIMEOUT_USEC    10000

int flag;

/*
 * Set global flag if a marked signal has been caught
 */
void
sighandler(int signum)
{
    /*
     * Write newline to stdout if keyboard interrupt has been sent
     */
     if (signum == SIGINT)
     {
         putchar('\n');
     }

     flag++;
}

/*
 * Catch all catchable signals that terminate process (plus SIGQUIT)
 */
void
catch_signals(void)
{
    struct sigaction act = {{0}};

    sigemptyset(&act.sa_mask);
    act.sa_handler = sighandler;

    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
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
        perror("[!] Failed to retrieve network interface list");
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
        fprintf(stderr, "[!] Failed to find suitable network interface\n");
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

    flag = 0;
    catch_signals();

    /*
     * To increment target address, need to convert IP number from network byte
     * order to host byte order, add one, and convert back
     *
     * Also stop if flag != 0, meaning signal has been caught
     */
    for (target = start; target <= end && !flag;
         target = htonl(ntohl(target) + 1))
    {
        /*
         * Don't scan local address or broadcast address
         */
        if (target == local || target == bcast)
        {
            if (!INET_STR(&addr, addrbuf, INET_ADDRSTRLEN))
            {
                perror("[!] Failed to convert address to string");
                printf("0x%08x [%s]\n", target,
                       target == local ? "YOU" : "BROADCAST");
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
            /*
             * If errno is EINTR, signal has been caught, which should not be
             * considered an error condition for the purposes of this program
             */
            if (ret == -1 && errno != EINTR)
            {
                perror("[!] Probe of remote host failed");
            }

            continue;
        }

        if (!INET_STR(&addr, addrbuf, INET_ADDRSTRLEN))
        {
            perror("[!] Failed to convert remote host address to string");

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

