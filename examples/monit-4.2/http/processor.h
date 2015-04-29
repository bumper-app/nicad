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


#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <config.h>
#include <stdio.h>

#include "monitor.h"
#include "net.h"
#include "socket.h"

/* Server masquerade */
#define SERVER_NAME        "monit" 
#define SERVER_VERSION     VERSION
#define SERVER_URL         "http://www.tildeslash.com/monit/"
#define SERVER_PROTOCOL    "HTTP/1.0"
#define DATEFMT             "%a, %d %b %Y %H:%M:%S GMT"

/* Protocol methods supported */
#define METHOD_GET         "GET"
#define METHOD_POST        "POST"

/* HTTP Status Codes */
#define SC_CONTINUE                      100
#define SC_SWITCHING_PROTOCOLS           101
#define SC_PROCESSING                    102
#define SC_OK                            200
#define SC_CREATED                       201
#define SC_ACCEPTED                      202
#define SC_NON_AUTHORITATIVE             203
#define SC_NO_CONTENT                    204
#define SC_RESET_CONTENT                 205
#define SC_PARTIAL_CONTENT               206
#define SC_MULTI_STATUS                  207
#define SC_MULTIPLE_CHOICES              300
#define SC_MOVED_PERMANENTLY             301
#define SC_MOVED_TEMPORARILY             302
#define SC_SEE_OTHER                     303
#define SC_NOT_MODIFIED                  304
#define SC_USE_PROXY                     305
#define SC_TEMPORARY_REDIRECT            307
#define SC_BAD_REQUEST                   400
#define SC_UNAUTHORIZED                  401
#define SC_PAYMENT_REQUIRED              402
#define SC_FORBIDDEN                     403
#define SC_NOT_FOUND                     404
#define SC_METHOD_NOT_ALLOWED            405
#define SC_NOT_ACCEPTABLE                406
#define SC_PROXY_AUTHENTICATION_REQUIRED 407
#define SC_REQUEST_TIMEOUT               408
#define SC_CONFLICT                      409
#define SC_GONE                          410
#define SC_LENGTH_REQUIRED               411
#define SC_PRECONDITION_FAILED           412
#define SC_REQUEST_ENTITY_TOO_LARGE      413
#define SC_REQUEST_URI_TOO_LARGE         414
#define SC_UNSUPPORTED_MEDIA_TYPE        415
#define SC_RANGE_NOT_SATISFIABLE         416
#define SC_EXPECTATION_FAILED            417
#define SC_UNPROCESSABLE_ENTITY          422
#define SC_LOCKED                        423
#define SC_FAILED_DEPENDENCY             424
#define SC_INTERNAL_SERVER_ERROR         500
#define SC_NOT_IMPLEMENTED               501
#define SC_BAD_GATEWAY                   502
#define SC_SERVICE_UNAVAILABLE           503
#define SC_GATEWAY_TIMEOUT               504
#define SC_VERSION_NOT_SUPPORTED         505
#define SC_VARIANT_ALSO_VARIES           506
#define SC_INSUFFICIENT_STORAGE          507
#define SC_NOT_EXTENDED                  510

/* Initial buffer sizes */
#define STRLEN             256
#define REQ_STRLEN         1024
#define RES_STRLEN         2048
#define MAX_URL_LENGTH     512

/* Request timeout in seconds */
#define REQUEST_TIMEOUT    30 

#define TRUE               1
#define FALSE              0

struct entry {
  char *name;
  char *value;
  /* For internal use */
  struct entry *next;
};
typedef struct entry *HttpHeader;
typedef struct entry *HttpParameter;

typedef struct request {
  char *url;
  Socket_T S;
  char *method;
  char *protocol;
  char *pathinfo;
  char *remote_user;
  HttpHeader headers;
  ssl_connection *ssl;
  HttpParameter params;
} *HttpRequest;

typedef struct response {
  int status;
  Socket_T S;
  char *protocol;
  size_t bufsize;
  size_t bufused;
  int is_committed;
  HttpHeader headers;
  ssl_connection *ssl;
  const char *status_msg; 
  unsigned char *outputbuffer;
} *HttpResponse;


struct  ServiceImpl {
  void(*doGet)(HttpRequest, HttpResponse);
  void(*doPost)(HttpRequest, HttpResponse);
};

/*
 * An object for implementors of the service functions; doGet and
 * doPost. Implementing modules i.e. CERVLETS, must implement the
 * doGet and doPost functions and the engine will call the add_Impl
 * function to setup the callback to these functions.
 */
struct ServiceImpl Impl;

/* Public prototypes */
void *http_processor(void *);
char *get_headers(HttpResponse res);
void set_status(HttpResponse res, int status);
const char *get_status_string(int status_code);
void add_Impl(void *doGetFunc, void *doPostFunc);
void out_print(HttpResponse res,  const char *, ...);
void set_content_type(HttpResponse res, const char *mime);
const char *get_header(HttpRequest req, const char *header_name);
void send_redirect(HttpResponse res, const char *location);
void send_error(HttpResponse, int status, const char *message);
const char *get_parameter(HttpRequest req, const char *parameter_name);
void set_header(HttpResponse res, const char *name, const char *value);

#endif
