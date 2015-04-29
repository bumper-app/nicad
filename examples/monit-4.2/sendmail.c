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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif 

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif 

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "monitor.h"
#include "net.h"
#include "socket.h"


/**
 *  Connect to a SMTP server and send mail.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: sendmail.c,v 1.29 2004/02/18 22:31:42 chopp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------- Definitions */


typedef struct {
  Socket_T socket;
  sigjmp_buf error;
  const char *server;
  char localhost[STRLEN];
} SendMail_T;


/* -------------------------------------------------------------- Prototypes */


static void do_status(SendMail_T *S);
static void open_server(SendMail_T *S);
static void do_send(SendMail_T *S, const char *, ...);


/* ------------------------------------------------------------------ Public */


/**
 * Send mail messages via SMTP
 * @param mail A Mail object
 */
void sendmail(Mail_T mail) {

  Mail_T m;
  SendMail_T S;
  char *now= get_RFC822date(NULL);
  
  ASSERT(mail);
  
  if(sigsetjmp(S.error, TRUE)) {
    goto exit;
  }
  
  open_server(&S);
  
  if(gethostname(S.localhost, sizeof(S.localhost)) < 0) {
    snprintf(S.localhost, sizeof(S.localhost), "%s", LOCALHOST);
  }

  /*
   * FIXME: Implement this as a small state machine and handle server
   * returned status codes as transition points.
   */
  do_status(&S);
  do_send(&S, "HELO %s\r\n", S.localhost); 
  do_status(&S);

  for(m= mail; m; m= m->next) { 
    
    do_send(&S, "MAIL FROM: <%s>\r\n", m->from);
    do_status(&S);
    do_send(&S, "RCPT TO: <%s>\r\n", m->to);
    do_status(&S);
    do_send(&S, "DATA\r\n");
    do_status(&S);
    do_send(&S, "From: %s\r\n", m->from);
    do_send(&S, "To: %s\r\n", m->to);
    do_send(&S, "Subject: %s\r\n", m->subject);
    do_send(&S, "Date: %s\r\n", now);
    do_send(&S, "X-Mailer: %s %s\r\n", prog, VERSION);
    do_send(&S, "Mime-Version: 1.0\r\n");
    do_send(&S, "Content-Type: text/plain; charset=\"iso-8859-1\"\r\n");
    do_send(&S, "Content-Transfer-Encoding: quoted-printable\r\n");
    do_send(&S, "\r\n");
    do_send(&S, "%s\r\n", m->message);
    if(m->opt_message) {
      do_send(&S, "%s\r\n", m->opt_message);
    }
    do_send(&S, ".\r\n");
    do_status(&S);
    
  }

  do_send(&S, "QUIT\r\n");
  
  exit:
  FREE(now);
  if(S.socket) {
    socket_free(&S.socket);
  }
  
}


/* ----------------------------------------------------------------- Private */


void do_send(SendMail_T *S, const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);
  
  if(socket_write(S->socket, msg, strlen(msg)) <= 0) {

    FREE(msg);
    log("Sendmail: error sending data to the server '%s' -- %s\n",
	S->server, STRERROR);
    siglongjmp(S->error, TRUE);
    
  }

  FREE(msg);

}


static void do_status(SendMail_T *S) {
  
  int  status;
  char buf[STRLEN];
  
  if(socket_read(S->socket, buf, sizeof(buf)) <= 0) {
    
    log("Sendmail: error receiving data from the mailserver '%s' -- %s\n",
	S->server, STRERROR);
    siglongjmp(S->error, TRUE);
    
  }
  
  chomp(buf, STRLEN);
  
  sscanf(buf, "%d", &status);
  if(status >= 400) {
    
    log("Sendmail error: %s\n", buf);
    siglongjmp(S->error, TRUE);
    
  }
  
}


static void open_server(SendMail_T *S) {

  MailServer_T mta= Run.mailservers;

  S->server= mta?mta->host:LOCALHOST;

  do {
    S->socket= socket_create_t(S->server, SMTP_PORT, SOCKET_TCP, FALSE,
			       Run.mailserver_timeout);
    if(S->socket)
	break;
    log("Cannot open a connection to the mailserver '%s' -- %s\n",
	S->server, STRERROR);
    if(mta && (mta= mta->next)) {
      S->server= mta->host;
      log("Trying the next mail server '%s'\n", S->server);
      continue;
    } else {
      log("No mail servers are available, aborting alert\n");
      /* FIXME: Add mail to a queue for later retry */
      siglongjmp(S->error, TRUE);
    }
  } while(TRUE);

}

