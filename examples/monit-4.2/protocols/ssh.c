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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "protocol.h"

/**
 *  A simple 'SSH protocol version exchange' implemetation based on
 *  RFC (http://www.openssh.com/txt/draft-ietf-secsh-transport-14.txt)
 * 
 *  @author Igor Homyakov, <homyakov@altlinux.ru>
 *
 *  @version \$Id: ssh.c,v 1.8 2004/01/29 17:52:12 martinp Exp $
 *
 *  @file
 */
int check_ssh(Socket_T s) {

  int n;
  char  buf[STRLEN];

  ASSERT(s);
    
  if((n= socket_read(s, buf, sizeof(buf))) <= 0) {
    log("SSH: error receiving identification string -- %s\n", STRERROR);
    return FALSE;
  }
  
  if(! starts_with(buf, "SSH-")) {
    log("SSH: protocol error %s\n", buf);
    return FALSE;
  }

  /* send identification string back to server */
  if(socket_write(s, buf, n) <= 0) {
    log("SSH: error sending identification string -- %s\n", STRERROR);
    return FALSE;
  }
   
  return TRUE;
  
}
