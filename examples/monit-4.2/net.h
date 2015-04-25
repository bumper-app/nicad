/*
 * Copyright (C), 2000-2004 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef NET_H
#define NET_H

#include <config.h>
#include "monitor.h"


/**
 *  General purpose Network and Socket methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: net.h,v 1.22 2004/02/06 20:02:20 martinp Exp $
 *
 *  @file
 */


/**
 * Standard seconds to wait for a socket connection or for socket read
 * i/o before aborting
 */
#define NET_TIMEOUT 5


/**
 * Test INET Socket connection to host on port with the specified
 * socket protocol type. The protocol should be either SOCK_STREAM or
 * SOCK_DGRAM.
 * @param hostname The host to connect to
 * @param port The portnumber to connect to
 * @param protocol The socket protocol to use
 * @return TRUE if the connect succeeded otherwise FALSE.
 */
int check_connect(const char *hostname, int port, int protocol);


/**
 * Check if the hostname resolves
 * @param hostname The host to check
 * @return TRUE if hostname resolves, otherwise FALSE
 */
int check_host(const char *hostname);


/**
 * Verify that the socket is ready for i|o
 * @param socket A socket
 * @return TRUE if the socket is ready, otherwise FALSE.
 */
int check_socket(int socket);


/**
 * Verify that the udp socket is ready for i|o. The given socket must
 * be a connected udp socket if we should be able to test the udp
 * server. The test is conducted by sending one byte to the server and
 * check for a returned ICMP error when reading from the socket. A
 * better test would be to send an empty SYN udp package to avoid
 * possibly raising an error from the server we are testing but
 * assembling an udp by hand requires SOCKET_RAW and running the
 * program as root.
 * @param socket A socket
 * @return TRUE if the socket is ready, otherwise FALSE.
 */
int check_udp_socket(int socket);


/**
 * Create a non-blocking socket against hostname:port with the given
 * protocol. The protocol should be either SOCK_STREAM or SOCK_DGRAM.
 * @param hostname The host to open a socket at
 * @param port The port number to connect to
 * @param protocol Socket protocol to use (SOCK_STREAM|SOCK_DGRAM)
 * @param timeout If not connected within timeout seconds abort and return -1
 * @return The socket or -1 if an error occured.
 */
int create_socket(const char *hostname, int port, int protocol, int timeout);


/**
 * Open a socket using the given Port_T structure. The protocol,
 * destination and type are selected appropriately.
 * @param p connection description
 * @return The socket or -1 if an error occured.
 */
int create_generic_socket(Port_T p);


/**
 * Create a non-blocking UNIX socket.
 * @param pathname The pathname to use for the unix socket
 * @param timeout If not connected within timeout seconds abort and return -1
 * @return The socket or -1 if an error occured.
 */
int create_unix_socket(const char *pathname, int timeout);


/**
 * Create a blocking server socket and bind it to the specified local
 * port number, with the specified backlog. Set a socket option to
 * make the port reusable again. If a bind address is given the socket
 * will only accept connect requests to this addresses. If the bind
 * address is NULL it will accept connections on any/all local
 * addresses
 * @param port The localhost port number to open
 * @param backlog The maximum queue length for incomming connections
 * @param bindAddr the local address the server will bind to
 * @return The socket ready for accept, or -1 if an error occured.
 */
int create_server_socket(int port, int backlog, const char *bindAddr);


/**
 * Shutdown a socket and close the descriptor.
 * @param socket The socket to shutdown and close
 * @return TRUE if the close succeed otherwise FALSE
 */
int close_socket(int socket);


/**
 * Enable nonblocking i|o on the given socket.
 * @param socket A socket
 * @return TRUE if success, otherwise FALSE
 */
int set_noblock(int socket);


/**
 * Disable nonblocking i|o on the given socket
 * @param socket A socket
 * @return TRUE if success, otherwise FALSE
 */
int set_block(int socket);


/**
 * Check if data is available, if not, wait timeout seconds for data
 * to be present.
 * @param socket A socket
 * @param timeout How long to wait before timeout (value in seconds)
 * @return Return TRUE if the event occured, otherwise FALSE.
 */
int can_read(int socket, int timeout);


/**
 * Check if data can be sent to the socket, if not, wait timeout
 * seconds for the socket to be ready.
 * @param socket A socket
 * @param timeout How long to wait before timeout (value in seconds)
 * @return Return TRUE if the event occured, otherwise FALSE.
 */
int can_write(int socket, int timeout);


/**
 * Write <code>size</code> bytes from the <code>buffer</code> to the
 * <code>socket</code> 
 * @param socket the socket to write to
 * @param buffer The buffer to write
 * @param size Number of bytes to send
 * @return The number of bytes sent or -1 if an error occured.
 */
int sock_write(int socket, const void *buffer, int size);


/**
 * Read up to size bytes from the <code>socket</code> into the
 * <code>buffer</code>. If data is not available wait for
 * <code>timeout</code> seconds.
 * @param socket the Socket to read data from
 * @param buffer The buffer to write the data to
 * @param size Number of bytes to read from the socket
 * @param timeout Seconds to wait for data to be available
 * @return The number of bytes read or -1 if an error occured. 
*/
int sock_read(int socket, void *buffer, int size, int timeout);

/**
 * Create a ICMP socket against hostname, send echo and wait for response.
 * @param hostname The host to open a socket at
 * @param timeout If response will not come within timeout seconds abort
 * @return TRUE on succes, FALSE on error
 */
int icmp_echo(const char *hostname, int timeout);

#endif
