#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

#include <arpa/inet.h>
#include <netinet/in.h>

/*
 * Gets the 32-bit IPv4 number from a sockaddr structure
 */
#define EXTRACT_INET(sa) (((struct sockaddr_in *) (sa))->sin_addr.s_addr) 

/*
 * Sets IPv4 number of sockaddr structure
 */
#define SET_INET(sa, in) (EXTRACT_INET(sa) = (in))

/*
 * Sets port (IPv4) of a sockaddr structure
 */
#define SET_PORT(sa, port) (((struct sockaddr_in *) \
                             (sa))->sin_port = htons(port))

/*
 * Gets IPv4 string representation of sockaddr
 */
#define INET_STR(sa, buf, len) (inet_ntop(AF_INET, \
                                          &((struct sockaddr_in *) \
                                            (sa))->sin_addr, buf, len))

/*
 * Closes socket, ignoring errors
 */
void
close_sock(int fd);

/*
 * Shuts down socket, ignoring errors
 */
void
shutdown_sock(int fd);

/*
 * Toggles socket nonblocking mode
 * Returns -1 upon error, 0 otherwise
 * errno set upon error
 */
int
toggle_sock_nonblock(int fd);

#endif /* SOCKET_UTIL_H */

