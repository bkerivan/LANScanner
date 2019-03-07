#ifndef MAIN_CALLBACKS_H
#define MAIN_CALLBACKS_H

#include "scanner.h"

/*
 * Probe callbacks (as described in scanner.h) for use in main program
 */

void
print_up_host(struct scanner *sc);

void
print_probe_error(struct scanner *sc);

#endif /* MAIN_CALLBACKS_H */

