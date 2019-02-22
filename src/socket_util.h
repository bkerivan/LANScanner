#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

/*
 * Closes socket, ignoring errors.
 */
void
close_sock(int fd);

/*
 * Shuts down socket, ignoring errors.
 */
void
shutdown_sock(int fd);

/*
 * Toggles socket nonblocking mode.
 *
 * On success, returns 0; upon error, returns -1 and sets errno.
 */
int
toggle_sock_nonblock(int fd);

#endif /* SOCKET_UTIL_H */

