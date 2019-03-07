#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "main_callbacks.h"
#include "scanner.h"

/*
 * Program information
 */
#define PROGRAM_NAME        "LANScanner"
#define VERSION_STRING      "v1.0.1"
#define PROGRAM_URL         "https://github.com/bkerivan/LANScanner"
#define BUG_REPORT_EMAIL    "bkerivan@villanova.edu"

/*
 * More information used in version message
 */
#define COPYRIGHT_YEAR      2019
#define COPYRIGHT_HOLDER    "Brendan Kerivan"
#define LICENSE_STRING      "License: MIT License"
#define LICENSE_URL         "https://opensource.org/licenses/MIT"
#define SOFTWARE_STATEMENT  "This is free software: " \
                            "you are free to change and redistribute it.\n" \
                            "There is NO WARRANTY, " \
                            "to the extent permitted by law."

/*
 * Character code for the copyright symbol -- used in version message
 */
#define COPYRIGHT_CHARCODE  0xA9


/*
 * Default settings
 */
#define DEFAULT_PORT            80
#define DEFAULT_TIMEOUT_SEC     0
#define DEFAULT_TIMEOUT_USEC    10000
#define DEFAULT_SCAN_TYPE       SCAN_TYPE_CONNECT

void
print_version(void)
{
    printf("%s %s\n", PROGRAM_NAME, VERSION_STRING);
    
    if (isprint(COPYRIGHT_CHARCODE))
    {
        printf("Copyright %c ", COPYRIGHT_CHARCODE);
    }
    else
    {
        printf("Copyright (C) ");
    }

    printf("%d %s\n%s <%s>\n%s\n", COPYRIGHT_YEAR, COPYRIGHT_HOLDER,
           LICENSE_STRING, LICENSE_URL, SOFTWARE_STATEMENT);
}

void
print_usage(const char *binary_name)
{
    printf("%s %s\n\n"
           "Usage: %s [OPTIONS]\n\n", PROGRAM_NAME, VERSION_STRING,
           binary_name); 
    puts("OPTIONS:\n"
         "\t-d, --device_name <device>\n"
         "\t-p, --port <port (1-65535)>\n"
         "\t-t, --timeout <timeout in milliseconds>\n"
         "\t-s, --scan-type <single character scan type>\n"
         "\t-v, --version\n"
         "\t-h, --help\n\n"
         "SCAN TYPES:\n"
         "\t-sC, --scan-type=C\t\tTCP connect scan\n\n");
    printf("Report bugs to: %s\n"
           "%s home page: <%s>\n", BUG_REPORT_EMAIL, PROGRAM_NAME,
           PROGRAM_URL);
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
    long ret = 0;
    char *device_name = NULL, *endptr = NULL;
    struct scanner *sc = NULL;
    uint16_t port = DEFAULT_PORT;
    struct timeval timeout = {DEFAULT_TIMEOUT_SEC, DEFAULT_TIMEOUT_USEC};

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
            case 'p':
                /*
                 * Must use errno to check for overflow/underflow in strtol
                 */
                errno = 0;

                ret = strtol(optarg, &endptr, 10);
                if (errno || *endptr || ret < 0 || ret > 65535)
                {
                    fprintf(stderr, "[!] Invalid port: \"%s\"\n", optarg);
                    fprintf(stderr, "[*] Using default port: %d\n\n",
                            DEFAULT_PORT);
                    endptr = NULL;
                }
                else
                {
                    port = (uint16_t) ret;
                }
                break;
            case 't':
                errno = 0;
                ret = strtol(optarg, &endptr, 10);
                if (errno || *endptr || ret < 0)
                {
                    fprintf(stderr, "[!] Invalid timeout: \"%s\"\n", optarg);
                    fprintf(stderr, "[*] Using default timeout: %d msec\n\n", 
                            DEFAULT_TIMEOUT_SEC * 1000
                            + DEFAULT_TIMEOUT_USEC / 1000);
                    endptr = NULL;
                }
                else
                {
                    timeout.tv_sec = ret / 1000;
                    timeout.tv_usec = (ret % 1000) * 1000;
                }
                break;
            case 's':
                scan_type = parse_scan_type(optarg);
                if (scan_type == SCAN_TYPE_INVALID)
                {
                    fprintf(stderr, "[!] Invalid scan type: %s\n", optarg);
                    fputs("[*] Using default scan type: TCP connect\n\n",
                          stderr);
                    scan_type = DEFAULT_SCAN_TYPE;
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

    sc = init_scanner(scan_type, device_name, &timeout, port);

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

