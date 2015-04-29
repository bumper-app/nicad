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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "engine.h"
#include "socket.h"


/**
 *  A naive http 1.0 server. The server delegates handling of a HTTP
 *  request and response to the processor module.
 *
 *  NOTE
 *    This server does not use threads or forks; Requests are
 *    serialized and pending requests will be popped from the
 *    connection queue when the current request finish.
 *
 *    Since this server is written for monit, low traffic is expected.
 *    Connect from not-authenicated clients will be closed down
 *    promptly. The authentication schema or access control is based
 *    on client name/address and only requests from known clients are
 *    accepted. Hosts allowed to connect to this server should be
 *    added to the access control list by calling add_host_allow().
 *
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *  @author Martin Pala <martinp@tildeslash.com>
 *
 *  @version \$Id: engine.c,v 1.56 2004/03/13 00:10:18 martinp Exp $
 *
 *  @file 
 */


/* ------------------------------------------------------------- Definitions */


static int myServerSocket= 0;
static HostsAllow hostlist= NULL;
static volatile int stopped= FALSE;
ssl_server_connection *mySSLServerConnection= NULL;
static pthread_mutex_t hostlist_mutex= PTHREAD_MUTEX_INITIALIZER;


/* -------------------------------------------------------------- Prototypes */


static void check_Impl();
static void initialize_service();
static int  authenticate(const char*);
static int  is_host_allow(const char *);
static void destroy_host_allow(HostsAllow);
static Socket_T socket_producer(int, int, void*);


/* ------------------------------------------------------------------ Public */


/**
 * Start the HTTPD server
 * @param port The Port number to start the server at
 * @param backlog The maximum length of the incomming connection queue 
 * @param bindAddr the local address the server will bind to
 */
void start_httpd(int port, int backlog, char *bindAddr) {

  Socket_T S;

  stopped= Run.stopped;

  if((myServerSocket= create_server_socket(port, backlog, bindAddr)) < 0) {
    
    log("http server: Could not create a server socket at port %d -- %s\n",
	port, STRERROR);
    
    log("monit HTTP server not available\n");
    
    if(Run.init) {
      
      sleep(1);
      kill_daemon(SIGTERM);
      
    }
    
  } else {
    
    initialize_service();
    
    if(Run.httpdssl) {
      
      mySSLServerConnection= init_ssl_server( Run.httpsslpem,
					      Run.httpsslclientpem);
      
      if(mySSLServerConnection == NULL) {
	
	log("http server: Could not initilize SSL engine\n");
	
	log("monit HTTP server not available\n");
	
	return;
      }
      
#ifdef HAVE_OPENSSL
      mySSLServerConnection->server_socket= myServerSocket;
#endif
    }
    
    while(! stopped) {
      
      if(!(S= socket_producer(myServerSocket, port, mySSLServerConnection))) {
	continue;
      }

      http_processor(S);
      
    }

    if(S) {
      socket_free(&S);
    }
    
    delete_ssl_server_socket(mySSLServerConnection);  
    close_socket(myServerSocket);

  }

}


/**
 * Stop the HTTPD server. 
 */
void stop_httpd() {

  stopped= TRUE;

}


/* -------------------------------------------------------------- Properties */


/**
 * Add hosts allowed to connect to this server.
 * @param name A hostname (A-Record) or IP address to be added to the
 * hosts allow list
 * @return FALSE if the given host does not resolve, otherwise TRUE
 */
int add_host_allow(char *name) {

  struct hostent *hp;

  if(! (hp= gethostbyname(name))) {
    
    return FALSE;
    
  } else  {
    
    HostsAllow h;

    NEW(h);
    
    while(*hp->h_addr_list) {

      h->name= xstrdup( inet_ntoa( *(struct in_addr *) *hp->h_addr_list++));
    
      LOCK(hostlist_mutex)
	
      if(hostlist) {
      
        HostsAllow p, n;
      
        for(n= p= hostlist; p; n= p, p= p->next) {
	
	  if(!strcasecmp(p->name, name)) {
	  
	    destroy_host_allow(h);
	    goto done;
	  
	  }
	
        }
      
        n->next= h;
      
      } else {
      
        hostlist= h;
      
      }

      done:
      END_LOCK;

    }

  }

  return TRUE;

}


/**
 * Are any hosts present in the host allow list?
 * @return TRUE if the host allow list is non-empty, otherwise FALSE
 */
int has_hosts_allow() {

  int rv;

  LOCK(hostlist_mutex)
      rv= (hostlist != NULL);
  END_LOCK;

  return rv;

}


/** 
 * Free the host allow list
 */
void destroy_hosts_allow() {

  if(has_hosts_allow()) {
    
    LOCK(hostlist_mutex)
	destroy_host_allow(hostlist);
        hostlist= NULL;
    END_LOCK;
    
  }

}


/* ----------------------------------------------------------------- Private */


/**
 * Setup the cervlet service and verify that a cervlet implementation
 * exist. Only one cervlet is supported in this version. In a standalone
 * versions this function will load cervlets from a repository and
 * initialize each cervlet.
 */
static void initialize_service() {

  init_service();
  check_Impl();

}


/**
 * Abort if no Service implementors are found
 */
static void check_Impl() {

  if((Impl.doGet == 0) || (Impl.doPost == 0)) {
    
    log("http server: Service Methods not implemented\n");
    _exit(1);
    
  }
  
}


/**
 * Returns TRUE if remote host is allowed to connect, otherwise return
 * FALSE. If allow Basic Authentication is defined in the Run.Auth
 * object, authentication is delegated to the processor module.
 */
static int authenticate(const char *host) {

  if(is_host_allow(host)) {
    
    return TRUE;
    
  }

  if(! has_hosts_allow() && (Run.credentials!=NULL)) {

    return TRUE;

  }

  log("%s: Denied connection from non-authorized client [%s]\n", prog, host);
  
  return FALSE;

}


/**
 * Returns TRUE if host is allowed to connect to
 * this server
 */
static int is_host_allow(const char *name) { 

  HostsAllow p;
  int rv= FALSE;

  LOCK(hostlist_mutex)
  
  for(p= hostlist; p; p= p->next) {

    if(!strncasecmp(p->name, name, STRLEN)) {
      
      rv= TRUE;
      break;
      
    }
    
  }

  END_LOCK;

  return rv;

}


/* --------------------------------------------------------------- Factories */


/**
 * Accept connections from Clients and create a Socket_T object for
 * each successful accept. If accept fails, return a NULL object
 */
static Socket_T socket_producer(int server, int port, void *sslserver) {
  
  int client;
  struct sockaddr_in in;
  int len= sizeof(struct sockaddr_in);
  
  if(can_read(server, 1)) {
    
    if( (client= accept(server, (struct sockaddr*)&in, &len)) < 0) {

      if(stopped) {
        log("http server: service stopped\n");
      }  else {
        log("http server: cannot accept connection -- %s\n", STRERROR);
      }

      return NULL;

    }

  } else {

    /* If timeout or error occured, return NULL to allow the caller to
     * handle various states (such as stopped) which can occure in the
     * meantime */
    return NULL;

  }

  if(set_noblock(client) < 0) {
    goto error;
  }
  
  if(!check_socket(client)) {
    goto error;
  }
  
  if(! authenticate(inet_ntoa(in.sin_addr))) {
    goto error;
  }


  return socket_create_a(client, inet_ntoa(in.sin_addr), port, sslserver);

  error:
  close_socket(client);
  return NULL;

}


/* ----------------------------------------------------------------- Cleanup */


/**
 * Free a (linked list of) host_allow ojbect(s). 
 */
static void destroy_host_allow(HostsAllow p) {
  
  HostsAllow a= p; 
  
  if(a->next) {
    destroy_host_allow(a->next);
  }

  FREE(a->name);
  FREE(a);
 
}

