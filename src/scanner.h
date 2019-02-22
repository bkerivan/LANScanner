#ifndef SCANNER_H
#define SCANNER_H

#include <netinet/in.h>
#include <stdint.h>
#include <sys/time.h>

#include "device.h"

#define SCANNER_DEFAULT_TIMEOUT_SEC     0
#define SCANNER_DEFAULT_TIMEOUT_USEC    10000

#define SCAN_TYPE_CONNECT   0x01

/*
 * Flag for catching signals to terminate scan
 */
extern int signal_flag;

struct scanner;

/*
 * All probes return 1 if host is up and 0 if not.
 * Upon error, they return -1 and set errno.
 */
typedef int (*probe_method_t)(struct scanner *sc);

/*
 * Callback function for each scanned host that is up.
 */
typedef void (*probe_up_callback_t)(struct scanner *sc);

/*
 * Callback function for each scanned host that is not up.
 */
typedef void (*probe_down_callback_t)(struct scanner *sc);

/*
 * Callback function for each error that occurs in a probe.
 */
typedef void (*probe_error_callback_t)(struct scanner *sc);

struct scanner
{
    struct device *dev;
    uint8_t scan_type;
    probe_method_t probe;
    struct timeval timeout;
    struct sockaddr_in target;
    in_addr_t start;
    in_addr_t end;
    uint16_t port;
    int fd;
};


/*
 * Sets the scanner to the specified scan type.
 *
 * If timeout is NULL, the scanner's timeout is set to the default value of 10
 * milliseconds.
 *
 * If the port argument is 0, the port to be scanned is set to a random integer
 * between 0 and 65535. For scan types where ports are irrelevant, the port
 * argument is ignored.
 *
 * Returns 0 on success or -1 if scan type is invalid or the scanner pointer is
 * NULL.
 */
int
set_scan_type(struct scanner *sc, uint8_t scan_type, uint16_t port);

/*
 * Frees all resources associated with a scanner structure returned by
 * init_scanner.
 */
void
free_scanner(struct scanner *sc);

/*
 * Initializes a scanner for the specified scan type.
 *
 * If dev_name is not NULL, attempts to initialize scanner for a suitable
 * network device of the specified name.
 *
 * If timeout is NULL, the scanner's timeout is set to the default value
 * defined above.
 *
 * If the port argument is 0, the port to be scanned is set to a random integer
 * between 0 and 65535. For scan types where ports are irrelevant, the port
 * argument is ignored.
 *
 * On success, returns a pointer to an initialized struct scanner; on error,
 * returns NULL and sets errno.
 *
 * If no suitable network device can be determined for scanning, returns NULL
 * and doesn't set errno.
 */
struct scanner *
init_scanner(uint8_t scan_type, const char *dev_name, struct timeval *timeout,
             uint16_t port);

/*
 * Runs a scan of the entire subnet, using the specifications provided to
 * init_scanner.
 *
 * If any of the callbacks are not null: up_callback is called whenever a probe
 * detects an up host, down_callback is called whenever a probe detects a host
 * that is not up, and error_callback is called whenever an error occurs in a
 * probe.
 */
void
run_scan(struct scanner *sc, probe_up_callback_t up_callback,
         probe_down_callback_t down_callback,
         probe_error_callback_t error_callback);

#endif /* SCANNER_H */

