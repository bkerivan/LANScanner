#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>

#include "scanner.h"
/*
 * Default settings
 */
#define DEFAULT_PORT            80

void
print_up_host(struct scanner *sc)
{
    char addrbuf[INET_ADDRSTRLEN] = {0};
    in_addr_t bcast = 0;

    if (!sc)
    {
        return;
    }

    if (sc->dev->bcast)
    {
        bcast = sc->dev->bcast->sin_addr.s_addr;
    }

    if (!inet_ntop(AF_INET, &sc->target.sin_addr, addrbuf, INET_ADDRSTRLEN))
    {
        perror("[!] Failed to convert host address to string");
        printf("0x%08x", sc->target.sin_addr.s_addr);
    }
    else
    {
        printf("%s", addrbuf);
    }
    
    if (sc->target.sin_addr.s_addr == sc->dev->local.sin_addr.s_addr
        || sc->target.sin_addr.s_addr == bcast)
    {
        printf(" [%s]\n", sc->target.sin_addr.s_addr == bcast ? "BROADCAST"
                                                              : "YOU");
    }
    else
    {
        putchar('\n');
    }
}

void
print_probe_error(struct scanner *sc)
{
    perror("[!] Probe of remote host failed");
}

int
main(void)
{
    struct scanner *sc = init_scanner(SCAN_TYPE_CONNECT, NULL, NULL,
                                      DEFAULT_PORT); 

    if (!sc)
    {
        if (errno)
        {
            perror("[!] Failed to initialize scanner");
        }
        else
        {
            fputs("[!] Failed to find suitable network device\n", stderr);
        }

        return 1;
    }

    run_scan(sc, print_up_host, NULL, print_probe_error);
    free_scanner(sc);

    return 0;
}

