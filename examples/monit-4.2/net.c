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

#include <config.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef NEED_SOCKLEN_T_DEFINED
#define _BSD_SOCKLEN_T_
#endif

#ifdef HAVE_SOL_IP
#define ICMP_SIZE sizeof(struct icmphdr)
#else
#define ICMP_SIZE sizeof(struct icmp)
#include <netinet/in_systm.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif 

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif 

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif

#ifdef HAVE_NETINET_IP_ICMP_H
#include <netinet/ip_icmp.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef __dietlibc__
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
#endif

#include "monitor.h"
#include "net.h"
#include "ssl.h"

/**
 *  General purpose Network and Socket methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: net.c,v 1.51 2004/02/16 22:42:32 hauk Exp $
 *
 *  @file
 */


/* -------------------------------------------------------------- Prototypes */


static int do_connect(int, const struct sockaddr *, socklen_t, int);
static unsigned short checksum_ip(unsigned char *, int);


/* ------------------------------------------------------------------ Public */


/**
 * Test INET Socket connection to host on port with the specified
 * socket protocol type. The protocol should be either SOCK_STREAM or
 * SOCK_DGRAM.
 * @param hostname The host to connect to
 * @param port The portnumber to connect to
 * @param protocol The socket protocol to use
 * @return TRUE if the connect succeeded otherwise FALSE.
 */
int check_connect(const char *hostname, int port, int protocol) {

  int socket;
  int rv= TRUE;

  ASSERT(hostname);
  
  if((socket= create_socket(hostname, port, protocol, NET_TIMEOUT)) < 0) {
    rv= FALSE;
  } else if(! check_socket(socket)) {
    rv= FALSE;
  }
  
  close_socket(socket);
  
  return rv;
  
}


/**
 * Check if the hostname resolves
 * @param hostname The host to check
 * @return TRUE if hostname resolves, otherwise FALSE
 */
int check_host(const char *hostname) {
  
  struct hostent *hp;

  ASSERT(hostname);

  if((hp = gethostbyname(hostname)) == NULL) {
    return FALSE;
  }
  
  return TRUE;
  
}


/**
 * Verify that the socket is ready for i|o
 * @param socket A socket
 * @return TRUE if the socket is ready, otherwise FALSE.
 */
int check_socket(int socket) {

  return (can_read(socket, 0) || can_write(socket, 0));
  
}


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
int check_udp_socket(int socket) {

  int r;
  char buf[1]= {0};

  /*
   * R/W is asynchronous and we should probably loop and wait longer
   * for a possible ICMP error.
   */
  write(socket, buf, 1);
  sleep(2);
  r= read(socket, buf, 1);
  if(0>r) {
    switch(errno) {
    case ECONNREFUSED: return FALSE;
    default:           break;
    }
  }
  
  return TRUE;

}


/**
 * Create a non-blocking socket against hostname:port with the given
 * type. The type should be either SOCK_STREAM or SOCK_DGRAM.
 * @param hostname The host to open a socket at
 * @param port The port number to connect to
 * @param type Socket type to use (SOCK_STREAM|SOCK_DGRAM)
 * @param timeout If not connected within timeout seconds abort and return -1
 * @return The socket or -1 if an error occured.
 */
int create_socket(const char *hostname, int port, int type, int timeout) {

  int s;
  struct hostent *hp;
  struct sockaddr_in sin;
  
  ASSERT(hostname);

  if((hp= gethostbyname(hostname)) == NULL) {
    return -1;
  }

  if((s= socket(AF_INET, type, 0)) < 0) {
    return -1;
  }

  sin.sin_family= AF_INET;
  sin.sin_port= htons(port);
  memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  
  if(! set_noblock(s)) {
    goto error;
  }
 
  if(fcntl(s, F_SETFD, FD_CLOEXEC) == -1)
    goto error; 
  
  if(do_connect(s, (struct sockaddr *)&sin, sizeof(sin), timeout) < 0) {
    goto error;
  }

  return s;

  error:
  close_socket(s);
  return -1;
 
}


/**
 * Open a socket using the given Port_T structure. The protocol,
 * destination and type are selected appropriately.
 * @param p connection description
 * @return The socket or -1 if an error occured.
 */
int create_generic_socket(Port_T p) {

  int socket_fd= -1;

  ASSERT(p);

  switch(p->family) {
  case AF_UNIX:
      socket_fd= create_unix_socket(p->pathname, p->timeout);
      break;
  case AF_INET:
      socket_fd= create_socket(p->hostname, p->port, p->type, p->timeout);
      break;
  default:
      socket_fd= -1;
  }
  
  return socket_fd;
  
}


/**
 * Create a non-blocking UNIX socket.
 * @param pathname The pathname to use for the unix socket
 * @param timeout If not connected within timeout seconds abort and return -1
 * @return The socket or -1 if an error occured.
 */
int create_unix_socket(const char *pathname, int timeout) {

  int s;
  struct sockaddr_un unixsocket;
  
  ASSERT(pathname);

  if((s= socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  unixsocket.sun_family= AF_UNIX;
  snprintf(unixsocket.sun_path, sizeof(unixsocket.sun_path), "%s", pathname);
  
  if(! set_noblock(s)) {
    goto error;
  }
  
  if(do_connect(s, (struct sockaddr *)&unixsocket, sizeof(unixsocket),
		timeout) < 0) {
    goto error;
  }
  
  return s;
  
  error:
  close_socket(s);
  return -1;

}


/**
 * Create a non-blocking server socket and bind it to the specified local
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
int create_server_socket(int port, int backlog, const char *bindAddr) {
  
  int s;
  int flag= 1;
  struct sockaddr_in myaddr;

  if((s= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  memset(&myaddr, 0, sizeof(struct sockaddr_in));
  myaddr.sin_family= AF_INET;
  myaddr.sin_port= htons(port);
  
  if(bindAddr) {
    struct hostent *h= gethostbyname(bindAddr);
    if(h==NULL) {
      errno= h_errno;
      goto error;
    }
    myaddr.sin_addr= *(struct in_addr*)h->h_addr_list[0];
  } else {
    myaddr.sin_addr.s_addr= htonl(INADDR_ANY);
  }
  
  if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) < 0) 
    goto error;
  
  if(! set_noblock(s))
    goto error;
  
  if(fcntl(s, F_SETFD, FD_CLOEXEC) == -1)
    goto error; 
  
  if(bind(s, (struct sockaddr *)&myaddr, sizeof(struct sockaddr_in)) < 0)
    goto error;
  
  if(listen(s, backlog) < 0)
    goto error;
  
  return s;

  error:
  close(s);

  return -1;

}


/**
 * Shutdown a socket and close the descriptor.
 * @param socket The socket to shutdown and close
 * @return TRUE if the close succeed otherwise FALSE
 */
int close_socket(int socket) {

  int r;

  shutdown(socket, 2);
  
  do {
    r= close(socket);
  } while(r == -1 && errno == EINTR);
  
  return r;

}


/**
 * Enable nonblocking i|o on the given socket.
 * @param socket A socket
 * @return TRUE if success, otherwise FALSE
 */
int set_noblock(int socket) {
  
  int flags;

  flags= fcntl(socket, F_GETFL, 0);
  flags |= O_NONBLOCK;

  return (fcntl(socket, F_SETFL, flags) == 0);

}


/**
 * Disable nonblocking i|o on the given socket
 * @param socket A socket
 * @return TRUE if success, otherwise FALSE
 */
int set_block(int socket) {

  int flags;

  flags= fcntl(socket, F_GETFL, 0);
  flags &= ~O_NONBLOCK;

  return (fcntl(socket, F_SETFL, flags) == 0);

}


/**
 * Check if data is available, if not, wait timeout seconds for data
 * to be present.
 * @param socket A socket
 * @param timeout How long to wait before timeout (value in seconds)
 * @return Return TRUE if the event occured, otherwise FALSE.
 */
int can_read(int socket, int timeout) {

  int r= 0;
  fd_set rset;
  struct timeval tv;
  
  FD_ZERO(&rset);
  FD_SET(socket, &rset);
  tv.tv_sec= timeout;
  tv.tv_usec= 0;
  
  do {
    r= select(socket+1, &rset, NULL, NULL, &tv);
  } while(r == -1 && errno == EINTR);

  return (r > 0);

}


/**
 * Check if data can be sent to the socket, if not, wait timeout
 * seconds for the socket to be ready.
 * @param socket A socket
 * @param timeout How long to wait before timeout (value in seconds)
 * @return Return TRUE if the event occured, otherwise FALSE.
 */
int can_write(int socket, int timeout) {

  int r= 0;
  fd_set wset;
  struct timeval tv;

  FD_ZERO(&wset);
  FD_SET(socket, &wset);
  tv.tv_sec= timeout;
  tv.tv_usec= 0;

  do {
    r= select(socket+1, NULL, &wset, NULL, &tv);
  } while(r == -1 && errno == EINTR);

  return (r > 0);
  
}


/**
 * Write <code>size</code> bytes from the <code>buffer</code> to the
 * <code>socket</code> 
 * @param socket the socket to write to
 * @param buffer The buffer to write
 * @param size Number of bytes to send
 * @return The number of bytes sent or -1 if an error occured.
 */
int sock_write(int socket, const void *buffer, int size) {

  ssize_t n= 0;
  
  if(size<=0)
      return 0;
  
  errno= 0;
  do {
    n= write(socket, buffer, size);
  } while(n == -1 && errno == EINTR);
  
  if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    if(! can_write(socket, NET_TIMEOUT)) {
      return -1;
    }
    do {
      n= write(socket, buffer, size);
    } while(n == -1 && errno == EINTR);
  }
  
  return n;

}


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
int sock_read(int socket, void *buffer, int size, int timeout) {
  
  ssize_t n;

  if(size<=0)
      return 0;
  
  errno= 0;
  do {
    n= read(socket, buffer, size);
  } while(n == -1 && errno == EINTR);
  
  if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    if(! can_read(socket, timeout)) {
      return -1;
    }
    do {
      n= read(socket, buffer, size);
    } while(n == -1 && errno == EINTR);
  }

  return n;

}


/**
 * Create a ICMP socket against hostname, send echo and wait for response.
 * @param hostname The host to open a socket at
 * @param timeout If response will not come within timeout seconds abort
 * @return TRUE on succes, FALSE on error
 */
int icmp_echo(const char *hostname, int timeout) {

  struct hostent *hp;
  struct sockaddr_in sin;
  struct sockaddr_in sout;
#ifdef HAVE_SOL_IP
  struct iphdr *iphdrin;
  struct icmphdr *icmphdrin= NULL;
  struct icmphdr *icmphdrout= NULL;
#else
  struct ip *iphdrin;
  struct icmp *icmphdrin= NULL;
  struct icmp *icmphdrout= NULL;
#endif
  struct timeval tv;
  size_t size;
  fd_set rset;
  int s;
  int n= 0;
  int rv= FALSE;
  int sol_ip;
  unsigned ttl= 255;
  char buf[STRLEN];
  
  ASSERT(hostname);

  if((hp= gethostbyname(hostname)) == NULL)
    return FALSE;

  if((s= socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    return FALSE;

#ifdef HAVE_SOL_IP
  sol_ip = SOL_IP;
#else
  {
    struct protoent *pent;
    pent = getprotobyname( "ip" );
    sol_ip = ( pent != NULL ) ? pent->p_proto : 0;
  }
#endif

  if(setsockopt(s, sol_ip, IP_TTL, (char *)&ttl, sizeof(ttl)) < 0)
    goto error;

  NEW(icmphdrout);
#ifdef HAVE_SOL_IP
  icmphdrout->code= 0;
  icmphdrout->type= ICMP_ECHO;
  icmphdrout->un.echo.id= getpid();
  icmphdrout->un.echo.sequence= 0;
  icmphdrout->checksum= checksum_ip((unsigned char *)icmphdrout,
                                    ICMP_SIZE);
#else
  icmphdrout->icmp_code= 0;
  icmphdrout->icmp_type= ICMP_ECHO;
  icmphdrout->icmp_id= getpid();
  icmphdrout->icmp_seq= 0;
  icmphdrout->icmp_cksum= checksum_ip((unsigned char *)icmphdrout,
                                    ICMP_SIZE);
#endif
  sout.sin_family= AF_INET;
  sout.sin_port= 0;
  memcpy(&sout.sin_addr, hp->h_addr, hp->h_length);

  do {
    n= sendto(s, (char *)icmphdrout, ICMP_SIZE, 0,
	      (struct sockaddr *)&sout, sizeof(struct sockaddr));
  } while(n == -1 && errno == EINTR);
  
  tv.tv_sec= timeout;
  tv.tv_usec= 0;

  do {

    FD_ZERO(&rset);
    FD_SET(s, &rset);

    do {
      n= select(s+1, &rset, NULL, NULL, &tv);
    } while(n == -1 && errno == EINTR);

    if(n <= 0)
      goto error;

    size= sizeof(struct sockaddr_in);

    do {
      n= recvfrom(s, buf, STRLEN, 0, (struct sockaddr *)&sin, &size);
    } while(n == -1 && errno == EINTR);
    
    if(n < 0)
	goto error;
    
#ifdef HAVE_SOL_IP
    iphdrin= (struct iphdr *)buf;
    icmphdrin= (struct icmphdr *)(buf + iphdrin->ihl * 4);
    if( (icmphdrin->un.echo.id == icmphdrout->un.echo.id) &&
        (icmphdrin->type == ICMP_ECHOREPLY) &&
        (icmphdrin->un.echo.sequence == icmphdrout->un.echo.sequence) ) {
#else
    iphdrin= (struct ip *)buf;
    icmphdrin= (struct icmp *)(buf + iphdrin->ip_hl * 4);
    if( (icmphdrin->icmp_id == icmphdrout->icmp_id) &&
        (icmphdrin->icmp_type == ICMP_ECHOREPLY) &&
        (icmphdrin->icmp_seq == icmphdrout->icmp_seq) ) {
#endif

      rv= TRUE;
      break;

    }

  } while(TRUE);

  error:
  FREE(icmphdrout);
  close_socket(s);

  return rv;
 
}


/* ----------------------------------------------------------------- Private */


/*
 * Do a non blocking connect, timeout if not connected within timeout seconds
 */
static int do_connect(int s, const struct sockaddr *addr,
		      socklen_t addrlen, int timeout) {

  int n, error;
  fd_set wset, rset;
  struct timeval tv;

  errno= 0;
  error= 0;
  
  if(0 > (n= connect(s, addr, addrlen)))
    if(errno != EINPROGRESS)
      return -1;
  
  FD_ZERO(&rset);
  FD_SET(s, &rset);
  wset= rset;
  tv.tv_sec= timeout;
  tv.tv_usec= 0;
  
  if(select(s+1, NULL, &rset, &wset, &tv)==0) {
    close(s);
    errno= ETIMEDOUT;
    return -1;
  }

  if(FD_ISSET(s, &rset) || FD_ISSET(s, &wset)) {
    int len= sizeof(error);
    if(getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
      return -1;
  } else
    return -1;

  if(error) {
    errno= error;
    return -1;
  }

  return 0;
  
}


/*
 * Compute Internet Checksum for "count" bytes beginning at location "addr".
 * Based on RFC1071.
 */
static unsigned short checksum_ip(unsigned char *addr, int count) {

  register long sum= 0;

  while(count > 1) {
    sum += *((unsigned short *)addr)++;
    count -= 2;
  }

  /* Add left-over byte, if any */
  if(count > 0)
    sum += *(unsigned char *)addr;

  /* Fold 32-bit sum to 16 bits */
  while(sum >> 16)
    sum= (sum & 0xffff) + (sum >> 16);

  return ~sum;

}

