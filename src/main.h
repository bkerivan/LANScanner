#ifndef MAIN_H
#define MAIN_H

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
#define DEFAULT_PORT            31337
#define DEFAULT_TIMEOUT_SEC     0
#define DEFAULT_TIMEOUT_USEC    10000
#define DEFAULT_SCAN_TYPE       SCAN_TYPE_CONNECT

#endif /* MAIN_H */

