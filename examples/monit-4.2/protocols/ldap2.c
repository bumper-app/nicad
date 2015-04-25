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
 *  Simple LDAPv2 protocol test.
 *
 *  Try anonymous bind to the server.
 *
 *  BindRequest based on RFC1777. Request and response are ASN.1
 *  BER encoded strings. To make the test as simple as possible
 *  we work with BER encoded data.
 *
 *  The test checks only if the bind was successfull - in the
 *  case of failure it don't provide any erroneous message
 *  analysis.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: ldap2.c,v 1.12 2004/02/06 20:02:20 martinp Exp $
 *
 *  @file
 */
int check_ldap2(Socket_T s) {

  unsigned char buf[STRLEN];

  unsigned char request[14] = {
    0x30,	                 /** Universal Sequence TAG */
    0x0c,	       /** Length of the packet's data part */

    0x02,	                  /** Universal Integer TAG */
    0x01,			   	 /** Integer length */
    0x00,		   	              /** MessageID */

    0x60,		    /** Application BindRequest TAG */
    0x07,		        /** Length of the data part */

    0x02,                         /** Universal Integer TAG */
    0x01,			         /** Integer length */
    0x02,			       /** Protocol version */	

    0x04,                    /** Universal Octet string TAG */
    0x00,      			    /** Octet string length */
    /* NULL */     	     	       /** Anonymous BindDN */

    0x80,	        /** Context specific SimpleAuth TAG */
    0x00     	       /** SimpleAuth (octet string) length */
    /* NULL */     	     	  /** Anonymous Credentials */
  };
    
  unsigned char response[14] = {
    0x30,	                 /** Universal Sequence TAG */
    0x0c,	       /** Length of the packet's data part */

    0x02,	                  /** Universal Integer TAG */
    0x01,			   	 /** Integer length */
    0x00,		   	              /** MessageID */

    0x61,		   /** Application BindResponse TAG */
    0x07,		        /** Length of the data part */

    0x0a,                      /** Universal Enumerated TAG */
    0x01,			      /** Enumerated length */
    0x00,			                /** Success */	

    0x04,                    /** Universal Octet string TAG */
    0x00,      			    /** Octet string length */
    /* NULL */     	     	              /** MatchedDN */

    0x04,                    /** Universal Octet string TAG */
    0x00      			    /** Octet string length */
    /* NULL */     	     	           /** ErrorMessage */
  };

  unsigned char unbind[7] = {
    0x30,                        /** Universal Sequence TAG */
    0x05,              /** Length of the packet's data part */

    0x02,                         /** Universal Integer TAG */
    0x01,                                /** Integer length */
    0x01,                                     /** MessageID */

    0x42,                 /** Application UnbindRequest TAG */
    0x00                        /** Length of the data part */
    /* NULL */

  };

  ASSERT(s);


  if(socket_write(s, (unsigned char *)request, sizeof(request)) < 0) {
    log("LDAP: error sending data -- %s\n", STRERROR);
    return FALSE;
  }

  if(socket_read(s, (unsigned char *)buf, sizeof(buf)) <= 0) {
    log("LDAP: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }

  if(memcmp((unsigned char *)buf,
	    (unsigned char *)response,
	    sizeof(response))) {
    log("LDAP: anonymous bind failed\n");
    return FALSE;
  }

  if(socket_write(s, (unsigned char *)unbind, sizeof(unbind)) < 0) {
    log("LDAP: error sending data -- %s\n", STRERROR);
    return FALSE;
  }

  return TRUE;
    
}

