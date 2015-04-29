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
#include <ctype.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "protocol.h"

/**
 *  Check the server for greeting "@RSYNCD: XX, then send this greeting back
 *  to server, send command '#list' to get a listing of modules.
 * 
 *  @author Igor Homyakov <homyakov@altlinux.ru>
 *
 *  @version \$Id: rsync.c,v 1.7 2004/02/18 22:31:42 chopp Exp $
 *
 *  @file
 */
int check_rsync(Socket_T s) {

  char  buf[STRLEN];
  char  msg[STRLEN];
  int   rc, version;
  char  *rsyncd = "@RSYNCD:";
  
  ASSERT(s);
    
  if(socket_read(s, buf, sizeof(buf)) <= 0) {
    log("RSYNC: did not see server greeting  -- %s\n", STRERROR);
    return FALSE;
  }

  chomp(buf, STRLEN);
  
  /* RATS: ignore */ /* chomp does zero termination */
  rc = sscanf(buf, "%s %d", msg, &version);
  if ((rc == EOF) || (rc == 0)) {
     log("RSYNC: server greeting parse error %s\n", buf);
    return FALSE;
  }
   
  if(strncasecmp(msg, rsyncd, strlen(rsyncd)) != 0) {
    log("RSYNC: server sent \"%s\" rather than greeting\n", buf);
    return FALSE;
  }

  if(snprintf(buf, sizeof(buf), "%s %d\n", rsyncd, version) < 0) {
    log("RSYNC: string copy error -- %s\n", STRERROR);
    return FALSE;
  } 
	
  if(socket_write(s, buf, strlen(buf)) <= 0) {
    log("RSYNC: error sending identification string -- %s\n", STRERROR);
     return FALSE;
  }

  if(socket_print(s, "#list\n") < 0) {
    log("RSYNC: error sending writing #list command  -- %s\n", STRERROR);
    return FALSE;
  }

  if(socket_read(s, buf, sizeof(buf)) <= 0) {
    log("RSYNC: did not see server answer  -- %s\n", STRERROR);
    return FALSE;
  }
  
  return TRUE;
  
}
