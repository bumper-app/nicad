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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <netinet/in.h>
#include <time.h>

#include "protocol.h"

/**
 *  Check the server response, check the time it returns and accept a 
 *  TIME_TOLERANCE sec delta with the current system time.
 *
 *  This test is based on RFC868. Rdate returns number of seconds since
 *  00:00:00 UTC, January 1, 1900.
 *
 *  @author Francois Isabelle <Francois.Isabelle@ca.kontron.com>
 *
 *  @version \$Id: rdate.c,v 1.6 2004/01/29 17:52:12 martinp Exp $
 *
 *  @file
 */
int check_rdate(Socket_T s) {
 
/* Offset of 00:00:00 UTC, January 1, 1970 from 00:00:00 UTC, January 1, 1900 */
#define  TIME_OFFSET    2208988800UL
#define  TIME_TOLERANCE (time_t)3

  time_t delta;
  time_t rdatet;
  time_t systemt;
	  
  ASSERT(s);
  
  if(socket_read(s,(char*) &rdatet, sizeof(time_t)) <= 0) {
    log("RDATE: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }
  
  /* Get remote time and substract offset to allow unix time comparision */
  rdatet = ntohl(rdatet) - TIME_OFFSET;
  
  if((systemt = time(NULL)) == -1) {
    log("RDATE error: cannot get system time -- %s\n", STRERROR);
    return FALSE;
  }
   
  if(rdatet >= systemt)
    delta = (rdatet-systemt);
  else
    delta= (systemt-rdatet);
 
  if(delta > TIME_TOLERANCE) {
    log("RDATE error: time does not match system time -- %s\n", STRERROR);
    return FALSE;
  }
  
  return TRUE;
  
}

