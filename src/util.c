#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void *
zmalloc(size_t size)
{
    void *ptr = malloc(size);
    return ptr ? memset(ptr, 0, size) : NULL;
}

uint32_t
nanorand(void)
{
    struct timeval tv = {0};
    
    gettimeofday(&tv, NULL);

    /*
     * Seed PRNG with more entropy than time(NULL) provides
     */
    srand((unsigned)(tv.tv_sec ^ tv.tv_usec));

    return (uint32_t) rand();
}

