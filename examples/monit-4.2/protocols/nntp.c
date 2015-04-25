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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "protocol.h"

/**
 *  Check the server for greeting code 200 and then send a QUIT and
 *  check for code 205. If alive return TRUE, else, return FALSE.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: nntp.c,v 1.14 2004/02/18 22:31:42 chopp Exp $
 *
 *  @file
 */
int check_nntp(Socket_T s) {

  int status;
  char buf[STRLEN];
  char msg[STRLEN];

  ASSERT(s);

  if(socket_read(s, buf, sizeof(buf)) <= 0) {
    log("NNTP: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }

  chomp(buf, STRLEN);
  
  /* RATS: ignore */ /* chomp does zero termination */
  sscanf(buf, "%d %s", &status, msg);
  if(status != 200) {
    log("NNTP error: %s\n", buf);
    return FALSE;
  }

  if(socket_print(s, "QUIT\r\n") < 0) {
    log("NNTP: error sending data -- %s\n", STRERROR);
    return FALSE;
  }

  if(socket_read(s, buf, sizeof(buf)) <= 0) {
    log("NNTP: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }

  chomp(buf, STRLEN);
  
  /* RATS: ignore */ /* chomp does zero termination */
  sscanf(buf, "%d %s", &status, msg);
  if(status != 205) {
    log("NNTP error: %s\n", buf);
    return FALSE;
  }

  return TRUE;
  
}

