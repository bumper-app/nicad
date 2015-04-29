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

#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif 

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "net.h"
#include "ssl.h"
#include "monitor.h"
#include "socket.h"



/**
 * Implementation of the socket interface.
 * 
 * @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * @version \$Id: socket.c,v 1.34 2004/02/18 14:05:54 hauk Exp $
 * @file
 */


/* ------------------------------------------------------------- Definitions */

#define TYPE_LOCAL  0
#define TYPE_ACCEPT 1

struct Socket_T {
  int port;
  int type;
  int socket;
  char *host;
  Port_T Port;
  int timeout;
  int connection_type;
  ssl_connection *ssl;
  ssl_server_connection *sslserver;
};


/* ------------------------------------------------------------------ Public */


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
Socket_T socket_new(const char *host, int port, int type, int use_ssl) {

  return socket_create_t(host, port, type, use_ssl, NET_TIMEOUT);
  
}


/**
 * Factory method for creating a new Socket from a monit Port object
 * @param port The port object to create a socket from
 * @return The connected Socket or NULL if an error occurred
 */
Socket_T socket_create(void *port) {

  int s;
  Socket_T S;
  Port_T p= port;

  ASSERT(port);

  if((s= create_generic_socket(p)) != -1) {

    NEW(S);
    S->socket= s;
    S->type= p->type;
    S->port= p->port;
    S->timeout= p->timeout;
    S->connection_type= TYPE_LOCAL;
    
    if(p->family==AF_UNIX) {
      S->host= xstrdup(LOCALHOST);
    } else {
      S->host= xstrdup(p->hostname);
    }

    if(p->SSL.use_ssl) {
      if(! (S->ssl= new_ssl_connection(NULL, p->SSL.version))) {
	goto ssl_error;
      }
      if(! embed_ssl_socket(S->ssl, S->socket)) {
	goto ssl_error;
      }
      if(p->SSL.certmd5) {
	if(! check_ssl_md5sum(S->ssl, p->SSL.certmd5)) {
	  goto ssl_error;
	}
      }
    }

    S->Port= port;

    return S;

    ssl_error:
    socket_free(&S);
    return NULL;

  }

  return NULL;

}


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
			 int timeout) {

  int s;
  Socket_T S;
  int proto= type==SOCKET_UDP?SOCK_DGRAM:SOCK_STREAM;
  
  ASSERT(host);
  ASSERT((type==SOCKET_UDP)||(type==SOCKET_TCP));
  if(use_ssl) {
    ASSERT(type==SOCKET_TCP);
  }
  ASSERT(timeout>0);

  if((s= create_socket(host, port, proto, timeout)) != -1) {
    
    NEW(S);
    S->socket= s;
    S->port= port;
    S->type= proto;
    S->timeout= timeout;
    S->host= xstrdup(host);
    S->connection_type= TYPE_LOCAL;
    
    if(use_ssl) {
      if(! (S->ssl= new_ssl_connection(NULL, SSL_VERSION_AUTO))) {
	goto ssl_error;
      }
      if(! embed_ssl_socket(S->ssl, S->socket)) {
	goto ssl_error;
      }
    }
    
    return S;
    
    ssl_error:
    socket_free(&S);
    return NULL;

  }
  
  return NULL;

}


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
			 int port, void *sslserver) {

  Socket_T S;

  ASSERT(socket>=0);
  ASSERT(remote_host);

  NEW(S);
  S->port= port;
  S->socket= socket;
  S->type= SOCK_STREAM;
  S->timeout= NET_TIMEOUT;
  S->host= xstrdup(remote_host);
  S->connection_type= TYPE_ACCEPT;
  
  if(sslserver) {
    S->sslserver= sslserver;
    if(! (S->ssl= insert_accepted_ssl_socket(S->sslserver))) {
      goto ssl_error;
    }
    if(! embed_accepted_ssl_socket(S->ssl, S->socket)) {
      goto ssl_error;
    }
  }
  
  return S;
  
  ssl_error:
  socket_free(&S);
  return NULL;
  
}


/**
 * Destroy a Socket object. Close the socket and release allocated
 * resources. 
 * @param S A Socket object reference
 */
void socket_free(Socket_T *S) {

  ASSERT(S && *S);

#ifdef HAVE_OPENSSL
  if((*S)->ssl && (*S)->ssl->handler) {
    if((*S)->connection_type==TYPE_LOCAL) {
      close_ssl_socket((*S)->ssl);
      delete_ssl_socket((*S)->ssl);
    } else if((*S)->connection_type==TYPE_ACCEPT && (*S)->sslserver) {
      close_accepted_ssl_socket((*S)->sslserver, (*S)->ssl);
    }
  }
#endif

  close_socket((*S)->socket);
  FREE((*S)->host);
  FREE(*S);
  (*S)= NULL;
  
}


/* -------------------------------------------------------------- Properties */


/**
 * Returns TRUE if the socket is ready for i|o
 * @param S A Socket object
 * @return TRUE if the socket is ready otherwise FALSE 
 */
int socket_is_ready(Socket_T S) {

  ASSERT(S);
  
  switch(S->type) {
  case SOCK_STREAM: return check_socket(S->socket);
  case SOCK_DGRAM:  return check_udp_socket(S->socket);
  default:          break;
  }

  return FALSE;

}


/**
 * Get the underlying socket descriptor
 * @param S A Socket object
 * @return The socket descriptor
 */
int socket_get_socket(Socket_T S) {

  ASSERT(S);
  
  return S->socket;

}


/**
 * Get the Port object used to create this socket. If no Port object
 * was used this method returns NULL.
 * @param S A Socket object
 * @return The Port object or NULL
 */
void *socket_get_Port(Socket_T S) {

  ASSERT(S);
  
  return S->Port;

}


/**
 * Get the remote port number the socket is connected to
 * @param S A Socket object
 * @return The remote host's port number
 */
int socket_get_remote_port(Socket_T S) {

  ASSERT(S);
  
  return S->port;

}


/**
 * Get the remote host this socket is connected to. The host is either
 * a host name in DNS or an IP address string.
 * @param S A Socket object
 * @return The remote host
 */
const char *socket_get_remote_host(Socket_T S) {
  
  ASSERT(S);
  
  return S->host;

}


/* ------------------------------------------------------------------ Public */


/**
 * Writes a character string. Use this function to send text based
 * messages to a client.
 * @param S A Socket_T object
 * @param m A String to send to the client
 * @return The bytes sent or -1 if an error occured
 */
int socket_print(Socket_T S, const char *m, ...) {

  int n;
  long l;
  va_list ap;
  char *buf= NULL;

  ASSERT(S);
  ASSERT(m);

  va_start(ap, m);
  buf= format(m, ap, &l);
  va_end(ap);
  
  n= socket_write(S, buf, l);
  FREE(buf);
  
  return n;

}


/**
 * Write size bytes from the buffer b.
 * @param S A Socket_T object
 * @param b The data to be written
 * @param size The size of the data in b
 * @return The bytes sent or -1 if an error occured
 */
int socket_write(Socket_T S, void *b, int size) {

  int n= 0;
  void *p= b;
  
  ASSERT(S);

  while(size > 0) {
    
    if(S->ssl) {
      n= send_ssl_socket(S->ssl, p, size);
    } else {
      n= sock_write(S->socket,  p, size);
    }

    if(n <= 0) break;
    p+= n;
    size-= n;

  }

  if(n < 0) {
    /* No write or a partial write is an error */
    return -1;
  }
  
  return  (int) p - (int) b;

}


/**
 * Reads size bytes and stores them into the byte buffer pointed to by b.
 * @param S A Socket_T object
 * @param b A Byte buffer
 * @param size The size of the buffer b
 * @return The bytes read or -1 if an error occured
 */
int socket_read(Socket_T S, void *b, int size) {

  int n= 0;
  void *p= b;
  int timeout= 0;
  
  ASSERT(S);
  
  timeout= S->timeout;
  
  while(size > 0) {
    if(S->ssl) {
      n= recv_ssl_socket(S->ssl, p, size, timeout);
    } else {
      n= sock_read(S->socket, p, size, timeout);
    }
    if(n <= 0) break;
    p+= n;
    size-= n;
    /* We managed to read some bytes and we only keep on reading
     * available data, otherwise we return with what we have read
     */
    timeout= 0;
  }

  if(n < 0 && p==b) {
    return -1;
  }
  
  return (int) p - (int) b;
 
}
 

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
char *socket_readln(Socket_T S, char *s, int size) {

  char *p= s;
  
  ASSERT(S);

  /*
   * TODO: We should use an internal buffer and read a large chunk
   * from the socket not just one by one byte.
   */
  while(--size && ((socket_read(S, s, 1)) > 0)) {
    if(*s++ == 10)
	break;
  }
  
  *s= 0;
  
  if(*p)
      return p;
  
  return NULL;
  
}
