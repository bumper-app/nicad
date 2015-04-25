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


#ifndef MONIT_SOCKET_H
#define MONIT_SOCKET_H

#define SOCKET_TCP  1
#define SOCKET_UDP  2


/**
 * This class implements a <b>Socket</b>. A socket is an endpoint for
 * communication between two machines. 
 * 
 * @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * @version \$Id: socket.h,v 1.15 2004/01/29 17:52:11 martinp Exp $
 * @file
 */


typedef struct Socket_T *Socket_T;


/**
 * Create a new Socket opened against host:port. The returned Socket
 * is a connected socket. This method can be used to create either TCP
 * or UDP sockets and the type parameter is used to select the socket
 * type. If the use_ssl parameter is TRUE the socket is created using
 * SSL. Only TCP sockets may use SSL.
 * @param host The remote host to open the Socket against. The host
 * may be a hostname found in the DNS or an IP address string.
 * @param port The port number to connect to
 * @param type The socket type to use (SOCKET_TCP or SOCKET_UPD)
 * @param use_ssl if TRUE the socket is created supporting SSL
 * @return The connected Socket or NULL if an error occurred
 */
Socket_T socket_new(const char *host, int port, int type, int use_ssl);


/**
 * Factory method for creating a new Socket from a monit Port object
 * @param port The port object to create a socket from
 * @return The connected Socket or NULL if an error occurred
 */
Socket_T socket_create(void *port);


/**
 * Create a new Socket opened against host:port with an explicit
 * timeout value for connect and read. Otherwise, same as socket_new()
 * @param host The remote host to open the Socket against. The host
 * may be a hostname found in the DNS or an IP address string.
 * @param port The port number to connect to
 * @param type The socket type to use (SOCKET_TCP or SOCKET_UPD)
 * @param use_ssl if TRUE the socket is created supporting SSL
 * @param timeout The timeout value in seconds
 * @return The connected Socket or NULL if an error occurred
 */
Socket_T socket_create_t(const char *host, int port, int type, int use_ssl,
			 int timeout);


/**
 * Factory method for creating a Socket object from an accepted
 * socket. The given socket must be a socket created from accept(2).
 * If the sslserver context is non-null the socket will support
 * ssl. This method does only support TCP sockets.
 * @param socket The accepted socket
 * @param remote_host The remote host from where the socket connection
 * originated
 * @param port The localhost port number from where the connection
 * arrived.
 * @param sslserver A ssl server connection context, may be NULL
 * @return A Socket or NULL if an error occurred
 */
Socket_T socket_create_a(int socket, const char *remote_host,
			 int port, void *sslserver);


/**
 * Destroy a Socket object. Close the socket and release allocated
 * resources. 
 * @param S A Socket object reference
 */
void socket_free(Socket_T *S);


/**
 * Returns TRUE if the socket is ready for i|o
 * @param S A Socket object
 * @return TRUE if the socket is ready otherwise FALSE 
 */
int socket_is_ready(Socket_T S);


/**
 * Get the underlying socket descriptor
 * @param S A Socket object
 * @return The socket descriptor
 */
int socket_get_socket(Socket_T S);


/**
 * Get the Port object used to create this socket. If no Port object
 * was used this method returns NULL.
 * @param S A Socket object
 * @return The Port object or NULL
 */
void *socket_get_Port(Socket_T S);


/**
 * Get the remote port number the socket is connected to
 * @param S A Socket object
 * @return The remote host's port number
 */
int socket_get_remote_port(Socket_T S);


/**
 * Get the remote host this socket is connected to. The host is either
 * a host name in DNS or an IP address string.
 * @param S A Socket object
 * @return The remote host
 */
const char *socket_get_remote_host(Socket_T S);


/**
 * Writes a character string. Use this function to send text based
 * messages to a client.
 * @param S A Socket_T object
 * @param m A String to send to the client
 * @return The bytes sent or -1 if an error occured
 */
int socket_print(Socket_T S, const char *m, ...);


/**
 * Write size bytes from the buffer b.
 * @param S A Socket_T object
 * @param b The data to be written
 * @param size The size of the data in b
 * @return The bytes sent or -1 if an error occured
 */
int socket_write(Socket_T S, void *b, int size);


/**
 * Reads size bytes and stores them into the byte buffer pointed to by b.
 * @param S A Socket_T object
 * @param b A Byte buffer
 * @param size The size of the buffer b
 * @return The bytes read or -1 if an error occured
 */
int socket_read(Socket_T S, void *b, int size);


/**
 * Reads in at most one less than size <code>characters</code> and
 * stores them into the buffer pointed to by s. Reading stops after
 * an EOF or a newline.  If a newline is read, it is stored into the
 * buffer.  A '\0' is stored after the last character in the buffer.
 * @param S A Socket_T object
 * @param s A character buffer to store the string in
 * @param size The size of the string buffer, s
 * @return s on success, and NULL on error or when end of file occurs
 * while no characters have been read.
 */
char *socket_readln(Socket_T S, char *s, int size);


#endif
