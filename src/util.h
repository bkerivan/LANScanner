#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

/*
 * Same as malloc but zeroes the data before returning.
 */
void *
zmalloc(size_t size);

/*
 * Returns a random unsigned integer with much lower probability of repeats
 * than most ways of seeding and using the standard library's random number
 * generator.
 */
uint32_t
nanorand(void);

#endif /* UTIL_H */

