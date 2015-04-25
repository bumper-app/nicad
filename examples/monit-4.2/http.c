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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "monitor.h"
#include "net.h"
#include "engine.h"

/* Private prototypes */
static void *thread_wrapper(void *arg);

/* The HTTP Thread */
static pthread_t thread;


/**
 *  Facade functions for the cervlet sub-system. Start/Stop the monit
 *  http server and check if monit http can start.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: http.c,v 1.1 2004/02/29 22:24:44 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
* Check if the HTTPD server is running.
*/
int check_httpd() {

  return check_connect(Run.bind_addr?Run.bind_addr:"localhost",
		       Run.httpdport, SOCK_STREAM);

}


/**
 * @return TRUE if the monit http can start and is specified in the
 * controlfile to start, otherwise return FALSE. Print an error
 * message if monit httpd _should_ start but can't.
 */
int can_http() {

  if(Run.dohttpd && Run.isdaemon) {
    
    if(! has_hosts_allow() && ! (Run.credentials!=NULL)) {
      
      log("%s: monit httpd not started since no connect allowed\n",
	  prog);
      
      return FALSE;
      
    }
    
    return TRUE;
    
  }
  
  return FALSE;
  
}


/**
 * Start and stop the monit http server
 * @param action START_HTTP or STOP_HTTP
 */
void monit_http(int action) {

  int status;

  switch(action) {
    
  case STOP_HTTP:
    log("Shutting down %s HTTP server\n", prog);
    stop_httpd();
    if( (status= pthread_join(thread, NULL)) != 0) {
      log("%s: Failed to stop the http server. Thread error -- %s.\n",
          prog, strerror(status));
    } else {
      log("%s HTTP server stopped\n", prog);
    }
    break;

  case START_HTTP:
    if( (status= pthread_create(&thread, NULL, thread_wrapper, NULL)) != 0) {
      log("%s: Failed to create the http server. Thread error -- %s.\n",
          prog, strerror(status));
    }
    break;

  default:
    log("%s: Unknown http server action\n", prog);
    break;
      
  }

  return;

}


/* ----------------------------------------------------------------- Private */


static void *thread_wrapper(void *arg) {

  sigset_t ns;

  /* Block collective signals in the http thread. The http server is
   * taken down gracefully by signaling the main monit thread */
  set_signal_block(&ns, NULL);
  start_httpd(Run.httpdport, 10, Run.bind_addr);

  return NULL;

}

  
