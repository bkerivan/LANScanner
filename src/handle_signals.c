#include <signal.h>
#include <stdio.h>

#include "handle_signals.h"

/*
 * Set signal flag to 0 so it can be used as a terminating condition when it
 * is set
 */
int signal_flag = 0;

void
sighandler(int signum)
{
    /*
     * Write newline to stdout if keyboard interrupt has been sent to separate
     * future output from control character
     */
    if (signum == SIGINT)
    {
        putchar('\n');
    }

    signal_flag++;
}

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

