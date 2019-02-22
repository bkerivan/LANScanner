#ifndef HANDLE_SIGNALS_H
#define HANDLE_SIGNALS_H

/*
 * Global signal flag that can be used by various functions, namely run_scan,
 * to terminate execution when a signal is caught
 */
extern int signal_flag;

/*
 * Set signal flag if a marked signal has been caught.
 */
void
sighandler(int signum);

/*
 * Catch all catchable signals that terminate process (plus SIGQUIT). 
 */
void
catch_signals(void);

#endif /* HANDLE_SIGNALS_H */

