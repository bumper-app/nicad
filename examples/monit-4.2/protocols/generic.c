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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

#include "protocol.h"

/**
 *  Generic test service test.
 *
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: generic.c,v 1.7 2004/01/29 17:52:12 martinp Exp $
 *
 *  @file
 */
int check_generic(Socket_T s) {

  Generic_T g= NULL;
  char buf[STRLEN];
#ifdef HAVE_REGEX_H
  int regex_return;
#endif

  
  ASSERT(s);

  if(socket_get_Port(s)) {
    g= ((Port_T)(socket_get_Port(s)))->generic;
  }
    
  while (g != NULL) {
    if (g->send != NULL) {

      if(socket_print(s, "%s", g->send) < 0) {

        log("GENERIC: error sending data -- %s\n", STRERROR);
        return FALSE;

      } else {

        DEBUG("GENERIC: successfully sent: '%s'\n", g->send); 
        
      }
                
    } else if (g->expect != NULL) {

      if(socket_readln(s, buf, STRLEN) == NULL) {
        
        log("GENERIC: error receiving data -- %s\n", STRERROR);
        return FALSE;
        
      }

#ifdef HAVE_REGEX_H
      regex_return=regexec( g->expect,
                            buf,
                            0,
                            NULL,
                            0);
      if (regex_return != 0) {

        log("GENERIC: receiving unexpected data [%s]\n", buf);
        return FALSE;

      } else {

        DEBUG("GENERIC: successfully received: '%s'\n", buf); 
        
      }
      
#else
      /* w/o regex support */

      if ( strncmp(buf, g->expect, strlen(g->expect)) != 0 ) {

        log("GENERIC: receiving unexpected data\n");
        return FALSE;

      } else {

        DEBUG("GENERIC: successfully received: '%s'\n", buf); 
        
      } 
      
#endif
      
    } else {

      /* This should not happen */
      log("GENERIC: unexpected strageness\n");
      return FALSE;
      
    }
    
    g= g->next;
  }
  
  return TRUE;
    
}

 
