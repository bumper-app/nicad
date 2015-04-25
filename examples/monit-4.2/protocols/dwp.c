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

#ifdef HAVE_ERRNO_H
#include <errno.h>
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

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif


#include "protocol.h"

/**
 *  A simple DWP (database wire protocol) test.
 *
 *  We send the following request to the server:
 *  'HEAD / HTTP/1.1'
 *  and check the server's status code.
 *
 *  If the status code is >= 400, an error has occurred.
 *  Return TRUE if the status code is 200, otherwise FALSE.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: dwp.c,v 1.10 2004/02/18 22:31:42 chopp Exp $
 *
 *  @file
 */
int check_dwp(Socket_T s) {

#define REQ_LENGTH  1024

  int n;
  int status;
  char buf[STRLEN];
  char proto[STRLEN];

  ASSERT(s);

  if(socket_print(s, "HEAD / HTTP/1.1\r\n"
		  "Connection: close\r\n\r\n") < 0) {
    log("DWP: error sending data -- %s\n", STRERROR);
    return FALSE;
  }
  
  if(socket_readln(s, buf, sizeof(buf)) <= 0) {
    log("DWP: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }

  chomp(buf,STRLEN);

  /* RATS: ignore */ /* chomp does zero termination */
  n= sscanf(buf, "%s %d", proto, &status);
  if(n!=2 || (status >= 400)) {
    log("DWP error: %s\n", buf);
    return FALSE;
  }
  
  return TRUE;
  
}

