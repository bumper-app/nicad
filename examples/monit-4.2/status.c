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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "net.h"
#include "socket.h"
#include "monitor.h"
#include "process.h"
#include "device.h"


/**
 *  Obtain status from the monit daemon
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: status.c,v 1.53 2004/02/29 22:24:44 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Show all services in the service list
 */
void status() {

#define LINE 1024

  if(exist_daemon()) {
    Socket_T sock= socket_new(Run.bind_addr?Run.bind_addr:"localhost",
			      Run.httpdport, SOCKET_TCP, Run.httpdssl);
    if(!sock) {
      log("%s: error connecting to the monit daemon\n", prog);
      return;
    } else {
      char buf[LINE];
      int status= FALSE;
      char *auth= get_basic_authentication_header();
      
      socket_print(sock, 
		   "GET /_status?format=text HTTP/1.0\r\n%s\r\n",
		   auth?auth:"");
      FREE(auth);
      /* Read past HTTP headers and check status */
      while(socket_readln(sock, buf, LINE)) {
	if(IS(buf, "\n") || IS(buf, "\r\n"))
	  break;
	if(starts_with(buf, "HTTP/1.0 200"))
	  status= TRUE;
      }
      if(!status) {
	log("%s: cannot read status from the monit daemon\n", prog);
	goto error;
      }
      while(socket_readln(sock, buf, LINE)) {
	printf("%s", buf);
      }
    error:
      socket_free(&sock);
    }
  } else {
    log("%s: no status available -- the monit daemon is not running\n", prog);
  }
  
}
