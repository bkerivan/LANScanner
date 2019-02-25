#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"

/*
 * Program information
 */
#define PROGRAM_NAME    "LANScanner"
#define VERSION_STRING  "v1.0.0"
#define CODE_URL        "https://github.com/bkerivan/LANScanner"

/*
 * Default settings
 */
#define DEFAULT_PORT            80
#define DEFAULT_SCAN_TYPE       SCAN_TYPE_CONNECT

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

void
print_version(void)
{
    printf("%s %s (%s)\n", PROGRAM_NAME, VERSION_STRING, CODE_URL);
}

void
print_usage(const char *binary_name)
{
    printf("%s %s (%s)\n\n", PROGRAM_NAME, VERSION_STRING, CODE_URL); 
    printf("Usage: %s [OPTIONS]\n\n", binary_name);
    puts("OPTIONS:\n"
         "\t-d, --device_name <device>\n"
         "\t-p, --port <port>\n"
         "\t-t, --timeout <timeout in milliseconds>\n"
         "\t-s, --scan-type <single character scan type>\n"
         "\t-v, --version\n"
         "\t-h, --help\n\n"
         "SCAN TYPES:\n"
         "\t-sC, --scan-type=C\t\tTCP connect scan\n");
}

int
parse_scan_type(const char *scan_type_str)
{
    int scan_type = SCAN_TYPE_INVALID;

    if (scan_type_str)
    {
        /*
         * Scan type is specified by a single character.
         */
        if (strlen(scan_type_str) == 1)
        {
            switch (*scan_type_str)
            {
                case 'C':
                    scan_type = SCAN_TYPE_CONNECT;
                    break;
                default:
                    /*
                     * Unrecognized scan type. Do nothing since scan_type is
                     * already initialized to SCAN_TYPE_INVALID.
                     */
                    break;
            }
        }
    }

    return scan_type;
}

int
main(int argc, char *argv[])
{
    struct option options[] = {
        {"device",    required_argument, NULL, 'd'},
        {"port",      required_argument, NULL, 'p'},
        {"timeout",   required_argument, NULL, 't'}, 
        {"scan-type", required_argument, NULL, 's'},
        {"version",   no_argument,       NULL, 'v'},
        {"help",      no_argument,       NULL, 'h'},
        {NULL,        0,                 NULL,   0}
    };

    int opt = 0, scan_type = DEFAULT_SCAN_TYPE;
    char *device_name = NULL;
    struct scanner *sc = NULL;

    /*
     * Disable automatic error messages
     */
    opterr = 0;

    while ((opt = getopt_long(argc, argv, ":d:p:t:s:vh", options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'd':
                device_name = optarg;
                break;
            /*
             * TODO: Add support for port and timeout options
             */
            case 'p':
            case 't':
                fprintf(stderr, "[!] \"%s\" option not yet supported\n\n",
                        argv[optind - 2]);
                print_usage(argv[0]);
                return 1;
            case 's':
                scan_type = parse_scan_type(optarg);
                if (scan_type == SCAN_TYPE_INVALID)
                {
                    fprintf(stderr, "[!] Invalid scan type: %s\n\n", optarg);
                    print_usage(argv[0]);
                    return 1;
                }
                break;
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case ':':
                fprintf(stderr, "[!] \"%s\" option requires an argument\n\n",
                        argv[optind - 1]);
                print_usage(argv[0]);
                return 1;
            case '?':
                fprintf(stderr, "[!] Invalid option: \"%s\"\n\n",
                        argv[optind - 1]);
                print_usage(argv[0]);
                return 1;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    sc = init_scanner(scan_type, device_name, NULL, DEFAULT_PORT);

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

