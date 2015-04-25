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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "monitor.h"
#include "event.h"
#include "net.h"
#include "alert.h"


/**
 *  Implementation of the alert module
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: alert.c,v 1.33 2004/02/18 22:31:42 chopp Exp $
 *  @file
 */


/* -------------------------------------------------------------- Prototypes */


static void copy_mail(Mail_T, Mail_T);
static void replace_bare_linefeed(Mail_T *);
static void substitute(Mail_T*, char *name, const char *event);


/* ------------------------------------------------------------------ Public */


/**
 * Notify registred users about the event
 * @param E An Event object
 */
void handle_alert(Event_T E) {

  Service_T s;
  
  ASSERT(E);

  s= Event_get_source(E);

  if(s->maillist||Run.maillist) {

    Mail_T m;
    Mail_T n;
    sigset_t ns, os;
    Mail_T list= NULL;

    set_signal_block(&ns, &os);
    
    /*
     * Build a mail-list with local recipients that has registered interest
     * for this event.
     */
    for(m= s->maillist; m; m= m->next) {
      
      if(IS_EVENT_SET(m->events, Event_get_id(E)) ||
	 /* Failed events are always reported */
	 (Event_get_id(E) == EVENT_FAILED) ) {

	Mail_T tmp;

	NEW(tmp);
	
	copy_mail(tmp, m);

	if(Event_get_message(E)) {
	  tmp->opt_message= xstrdup(Event_get_message(E));
	}

	substitute(&tmp, s->name, EVENT_DESCRIPTION(E));
	
	replace_bare_linefeed(&tmp);
	
	tmp->next= list;
	list= tmp;
	
	DEBUG("%s notification is sent to %s\n", EVENT_DESCRIPTION(E), m->to);

      }

    }

    /*
     * Build a mail-list with global recipients that has registered interest
     * for this event. Recipients which are defined in the service localy
     * overrides the same recipient events which are registered globaly.
     */
    for(m= Run.maillist; m; m= m->next) {
      
      int skip= FALSE;

      for(n= s->maillist; n; n= n->next) {
        if(IS(m->to, n->to)) {
	  skip= TRUE;
	  break;
	}
      }

      if(!skip &&
           (IS_EVENT_SET(m->events, Event_get_id(E)) ||
	   (Event_get_id(E) == EVENT_FAILED))
	) {

	Mail_T tmp;

	NEW(tmp);
	
	copy_mail(tmp, m);

	if(Event_get_message(E))
	  tmp->opt_message= xstrdup(Event_get_message(E));

	substitute(&tmp, s->name, EVENT_DESCRIPTION(E));
	
	replace_bare_linefeed(&tmp);
	
	tmp->next= list;
	list= tmp;
	
	DEBUG("%s notification is sent to %s\n", EVENT_DESCRIPTION(E), m->to);

      }

    }

    if(list) {
      
      sendmail(list);
      gc_mail_list(&list);
      
    }

    unset_signal_block(&os);

  }

}


static void substitute(Mail_T *m, char *name, const char *event) {

  char host[STRLEN];
  char *now= get_RFC822date(NULL);

  ASSERT(m && name && event);

  if(gethostname(host, sizeof(host)) < 0) {
    snprintf(host, STRLEN, "%s", LOCALHOST);
  }

  replace_string(&(*m)->from, "$HOST", host);
  replace_string(&(*m)->subject, "$DATE", now);
  replace_string(&(*m)->message, "$DATE", now);
  replace_string(&(*m)->subject, "$HOST", host);
  replace_string(&(*m)->message, "$HOST", host);
  replace_string(&(*m)->subject, "$PROGRAM", name);
  replace_string(&(*m)->message, "$PROGRAM", name);
  replace_string(&(*m)->subject, "$SERVICE", name);
  replace_string(&(*m)->message, "$SERVICE", name);
  replace_string(&(*m)->subject, "$EVENT", event);
  replace_string(&(*m)->message, "$EVENT", event);
 
  FREE(now);

}


static void copy_mail(Mail_T n, Mail_T o) {

  ASSERT(n && o);
  
  n->to= xstrdup(o->to);
  n->from=
      o->from?
      xstrdup(o->from):
      Run.MailFormat.from?
      xstrdup(Run.MailFormat.from):
      xstrdup(ALERT_FROM);
  n->subject=
      o->subject?
      xstrdup(o->subject):
      Run.MailFormat.subject?
      xstrdup(Run.MailFormat.subject):
      xstrdup(ALERT_SUBJECT);
  n->message=
      o->message?
      xstrdup(o->message):
      Run.MailFormat.message?
      xstrdup(Run.MailFormat.message):
      xstrdup(ALERT_MESSAGE);
  n->opt_message= NULL;
  
}


static void replace_bare_linefeed(Mail_T *m) {

  replace_string(&(*m)->message, "\r\n", "\n");
  replace_string(&(*m)->message, "\n", "\r\n");
  if((*m)->opt_message) {
    replace_string(&(*m)->opt_message, "\r\n", "\n");
    replace_string(&(*m)->opt_message, "\n", "\r\n");
  }

}
