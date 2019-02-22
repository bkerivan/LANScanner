#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "device.h"
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
    struct device *dev = NULL;
    in_addr_t bcast = 0, start = 0, end = 0, target;
    struct sockaddr_in addr = {0};
    char addrbuf[INET_ADDRSTRLEN] = {0};
    struct timeval timeout = {DEFAULT_TIMEOUT_SEC, DEFAULT_TIMEOUT_USEC};
    int ret = 0;

    dev = get_live_device(NULL);

    if (!dev)
    {
        if (errno)
        {
            perror("[!] Failed to retrieve network device");
        }
        else
        {
            fputs("[!] Failed to find suitable network device\n", stderr);
        }

        return 1;
    }

    printf("Scanning %s\n\n", dev->name);

    /*
     * Determine subnet range
     */
    start = dev->local.sin_addr.s_addr & dev->netmask.sin_addr.s_addr;
    end = start | (~dev->netmask.sin_addr.s_addr);

    if (dev->bcast)
    {
        bcast = dev->bcast->sin_addr.s_addr;
    }

    addr.sin_family = AF_INET;

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
        if (target == dev->local.sin_addr.s_addr || target == bcast)
        {
            if (!inet_ntop(AF_INET, &addr.sin_addr, addrbuf, INET_ADDRSTRLEN))
            {
                perror("[!] Failed to convert address to string");
                printf("0x%08x [%s]\n", target,
                       target == bcast ? "BROADCAST" : "YOU");
            }
            else
            {
                printf("%s [%s]\n", addrbuf,
                       target == bcast ? "BROADCAST" : "YOU");
            }

            memset(addrbuf, 0, INET_ADDRSTRLEN);
            continue;
        }

        addr.sin_addr.s_addr = target;

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

        if (!inet_ntop(AF_INET, &addr.sin_addr, addrbuf, INET_ADDRSTRLEN))
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

    free_device(dev);
    return 0;
}

