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

#include "../md5.h"
#include "../sha.h"
#include "protocol.h"


#undef   READ_SIZE
#define  READ_SIZE  8192


/* Private prototypes */
static int check_request(Socket_T s);
static char *get_host_header(Socket_T s, char * host);
static int check_request_checksum(Socket_T s, char *checksum, int);


/**
 *  A simple HTTP test.
 *
 *  We send the following request to the server:
 *  'GET / HTTP/1.1'             ... if request statement isn't defined
 *  'GET /custom/page  HTTP/1.1' ... if request statement is defined
 *  and check the server's status code.
 *
 *  If the host statement defines hostname, it is used in 'Host:'
 *  header.
 *
 *  If the status code is >= 400, an error has occurred.
 *  Return TRUE if the status code is OK, otherwise FALSE.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: http.c,v 1.32 2004/02/25 14:50:56 chopp Exp $
 *
 *  @file
 */
int check_http(Socket_T s) {

  char host[STRLEN];
  char *request= NULL;
  char *request_checksum= NULL;
  int  request_hashtype= 0;
 
  ASSERT(s);
  
  if(socket_get_Port(s)) {
    request= ((Port_T)(socket_get_Port(s)))->request;
    request_checksum= ((Port_T)(socket_get_Port(s)))->request_checksum;
    request_hashtype= ((Port_T)(socket_get_Port(s)))->request_hashtype;
  }

  request= request?request:"/";
 
  if(socket_print(s, "GET %s HTTP/1.1\r\n"
		  "Host: %s\r\n"
		  "Accept: */*\r\n"
		  "User-Agent: %s/%s\r\n"
		  "Connection: close\r\n\r\n",
		  request, get_host_header(s, host), prog, VERSION) < 0) {
    log("HTTP: error sending data -- %s\n", STRERROR);
    return FALSE;
  }

  if(request_checksum) {
    return check_request_checksum(s, request_checksum, request_hashtype);
  }

  return check_request(s);
  
}


/* ----------------------------------------------------------------- Private */


/**
 * @return a "hostname:port" or a void string if host
 * equals LOCALHOST or if it is an IP address
 */
static char *get_host_header(Socket_T s, char *hostbuf) {

  if(! strcmp(LOCALHOST, socket_get_remote_host(s)) ||
     inet_aton(socket_get_remote_host(s), NULL)) {

    *hostbuf= 0;

  } else {

    snprintf(hostbuf, STRLEN, "%s:%d",
	     socket_get_remote_host(s),
	     socket_get_remote_port(s));
    
  }

  return hostbuf;

}


/**
 * Check that the server returns a valid HTTP response
 * @param s A socket
 * @return TRUE if the respons is valid otherwise FALSE
 */
static int check_request(Socket_T s) {

  int n;
  int status;
  char buf[STRLEN];
  char msg[STRLEN];
  char proto[STRLEN];
 
  if(! socket_readln(s, buf, sizeof(buf))) {
    log("HTTP: error receiving data -- %s\n", STRERROR);
    return FALSE;
  }

  chomp(buf, STRLEN);

  /* RATS: ignore */ /* chomp does zero termination */
  n= sscanf(buf, "%s %d %s", proto, &status, msg);
  if(n!=3 || (status >= 400)) {
    log("HTTP error: %s\n", buf);
    return FALSE;
  }

  return TRUE;
  
}


/**
 * Check that the server returns a valid HTTP response and that the
 * returned entity's checksum is equal to the expected checksum. If
 * the response does not contain the Content-Length header, the
 * checksum is not tested and this method returns TRUE.
 * @param s A socket
 * @param checksum expected checksum
 * @return TRUE if the respons is valid and the checksum match,
 * otherwise FALSE
 */
static int check_request_checksum(Socket_T s, char *checksum, int hashtype) {

  int i;
  char line[STRLEN];
  long content_length= 0;

  if(! check_request(s))
      return FALSE;

  while(NULL != socket_readln(s, line, STRLEN)) {
    if(starts_with(line, "\r\n") || starts_with(line, "\n"))
	break;
    if(starts_with(line, "Content-Length")) {
      /* RATS: ignore */ /* socket_readln does zero termination */      
      if(1 != sscanf(line, "%*s%*[: ]%ld", &content_length)) {
	chomp(line, STRLEN);
	log("HTTP error: parsing Content-Length response header '%s'\n", line);
	return FALSE;
      }
    }
  }
  
  if(! content_length) {
    
    DEBUG("HTTP warning: Response does not contain Content-Length\n");
    
  } else {
    
    int n;
    char *r;
    long size;
    char result[STRLEN];
    char buf[READ_SIZE];
    unsigned char hash[STRLEN];
    int  keylength=0;

    switch (hashtype) {
    case HASH_MD5:
    {
      struct md5_ctx ctx;
      md5_init_ctx(&ctx);
      while(content_length > 0) {
        size= content_length>READ_SIZE?READ_SIZE:content_length;
        n= socket_read(s, buf, size);
        if(n<0) break;
        md5_process_bytes(buf, n, &ctx);
        content_length -= n; 
      }
      md5_finish_ctx(&ctx, hash);
      keylength=16; /* Raw key bytes not string chars! */
      break;
    }
    case HASH_SHA1: 
    {
      struct sha_ctx ctx;
      sha_init_ctx(&ctx);
      while(content_length > 0) {
        size= content_length>READ_SIZE?READ_SIZE:content_length;
        n= socket_read(s, buf, size);
        if(n<0) break;
        sha_process_bytes(buf, n, &ctx);
        content_length -= n; 
      }
      sha_finish_ctx(&ctx, hash);
      keylength=20; /* Raw key bytes not string chars! */
      break;
    }
    default:
        DEBUG("HTTP warning: Unknown hash type\n");
        return FALSE;
    }
    
    r= result;
    for(i= 0; i < keylength; ++i)
	r+= snprintf(r, STRLEN-(r-result) ,"%02x", hash[i]);
    
    if(strncasecmp(result, checksum, keylength*2) != 0) {
      DEBUG("HTTP warning: Document checksum mismatch\n");
      return FALSE;
    } else {
      DEBUG("HTTP: Succeeded testing document checksum\n");
    }
    
  }
  
  return TRUE;

}
