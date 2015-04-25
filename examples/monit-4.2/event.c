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

#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif


#include "alert.h"
#include "event.h"


/**
 * Implementation of the event interface.
 *
 * @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * @author Martin Pala <martinp@tildeslash.com>
 * @version \$Id: event.c,v 1.35 2004/02/14 22:04:41 hauk Exp $
 * @file
 */


/* ------------------------------------------------------------- Definitions */


struct Event_T {
  int id;
  char *message;
  Service_T source;
};


struct Event_Table {
  int id;
  char description[STRLEN];
} Event_Table[]= {
    {EVENT_NULL,       "No Event"},
    {EVENT_FAILED,     "Failed"},
    {EVENT_START,      "Started"},
    {EVENT_STOP,       "Stopped"},
    {EVENT_RESTART,    "Restarted"},
    {EVENT_CHECKSUM,   "Checksum changed"},
    {EVENT_RESOURCE,   "Resource limit matched"},
    {EVENT_TIMEOUT,    "Timeout"},
    {EVENT_TIMESTAMP,  "Timestamp rule matched"},
    {EVENT_SIZE,       "Size rule matched"},
    {EVENT_CONNECTION, "Connection test failed"},
    {EVENT_PERMISSION, "Permission error"},
    {EVENT_UID,        "UID error"},
    {EVENT_GID,        "GID error"},
    {EVENT_UNMONITOR,  "Monitoring disabled"}
};

static pthread_mutex_t handle_mutex= PTHREAD_MUTEX_INITIALIZER;


/* -------------------------------------------------------------- Prototypes */


static void handle_event(Event_T E);

static void handle_uid(Event_T E);
static void handle_gid(Event_T E);
static void handle_size(Event_T E);
static void handle_stop(Event_T E);
static void handle_start(Event_T E);
static void handle_restart(Event_T E);
static void handle_timeout(Event_T E);
static void handle_checksum(Event_T E);
static void handle_resource(Event_T E);
static void handle_timestamp(Event_T E);
static void handle_connection(Event_T E);
static void handle_permission(Event_T E);
static void handle_unmonitor(Event_T E);


/* ------------------------------------------------------------------ Public */


/**
 * Construct and post a new Event
 * @param source The Service the event occured on
 * @param id The event type
 * @param s Optional message describing the event
 */
void Event_post(Service_T source, long id, char *s, ...) {

  long l;
  Event_T e;
  
  ASSERT(source);

  NEW(e);
  e->id= id;
  e->message= NULL;
  e->source= source;

  if(s) {
    va_list ap;
    va_start(ap, s);
    e->message= format(s, ap, &l);
    va_end(ap);
  }

  LOCK(handle_mutex)
    handle_event(e);
  END_LOCK;

}


/**
 * Destroy an Event object and release allocated resources. 
 * @param E An Event object reference
 */
void Event_free(Event_T *E) {

  ASSERT(E && *E);
  FREE((*E)->message);
  FREE(*E);
  (*E)= NULL;

}


/* -------------------------------------------------------------- Properties */


/**
 * Get the Service where the event orginated
 * @param E An event object
 * @return The Service where the event orginated
 */
Service_T Event_get_source(Event_T E) {

  ASSERT(E);

  return E->source;

}


/**
 * Get the Event type
 * @param E An event object
 * @return The Event type
 */
int Event_get_id(Event_T E) {

  ASSERT(E);
  
  return E->id;

}


/**
 * Get the optionally Event message describing why the event was
 * fired.
 * @param E An event object
 * @return The Event message. May be NULL
 */
const char *Event_get_message(Event_T E) {

  ASSERT(E);

  return E->message;

}


/**
 * Get a textual description of the event type. For instance if the
 * event type is EVENT_RESTART, the textual description is
 * "Restart". Likewise if the event type is EVENT_CHECKSUM the textual
 * description is "Checksum error" and so on.
 * @param E An event object
 * @return A string describing the event type in clear text. If the
 * event type is not found NULL is returned.
 */
const char *Event_get_description(Event_T E) {

  int i, event;
  int size= sizeof(Event_Table)/sizeof(Event_Table[0]);

  ASSERT(E);

  /* In the case of passive mode we replace the description of start, stop
   * or restart event for failed, because these actions are passive in
   * this mode */
  event= (E->source->mode == MODE_PASSIVE &&
          ((E->id == EVENT_START)||
           (E->id == EVENT_STOP) ||
           (E->id == EVENT_RESTART))
         )?EVENT_FAILED:E->id;

  for(i= 0; i < size; i++)
    if(event == Event_Table[i].id)
      return Event_Table[i].description;
  
  return NULL;

}


/* ----------------------------------------------------------------- Private */


/*
 * Handle the event
 * @param E An event
 */
static void handle_event(Event_T E) {

  ASSERT(E);

  if(E->message) {
    log("%s", E->message);
  }

  switch(E->id) {
  case EVENT_UID:        handle_uid(E); break;
  case EVENT_GID:        handle_gid(E); break;
  case EVENT_SIZE:       handle_size(E); break;
  case EVENT_STOP:       handle_stop(E); break;
  case EVENT_START:      handle_start(E); break;
  case EVENT_RESTART:    handle_restart(E); break;
  case EVENT_TIMEOUT:    handle_timeout(E); break;
  case EVENT_RESOURCE:   handle_resource(E); break;
  case EVENT_CHECKSUM:   handle_checksum(E); break;
  case EVENT_TIMESTAMP:  handle_timestamp(E); break;
  case EVENT_CONNECTION: handle_connection(E); break;
  case EVENT_PERMISSION: handle_permission(E); break;
  case EVENT_UNMONITOR:  handle_unmonitor(E); break;
  }
  
  if(E->id != EVENT_NULL) {
    handle_alert(E);
  }
  
  Event_free(&E);

}


/* ---------------------------------------------------------- Event handlers */


static void handle_stop(Event_T E) {

  if((E->source->mode != MODE_PASSIVE)) {
    check_service(E->source->name, "stop");
  } 
  
}


static void handle_start(Event_T E) {
  
  if(E->source->def_timeout)
      E->source->nstart++;
  
  if((E->source->mode != MODE_PASSIVE)) {
    check_service(E->source->name, "start");
  } 

}


static void handle_restart(Event_T E) {

  if(E->source->def_timeout)
      E->source->nstart++;
  
  if((E->source->mode != MODE_PASSIVE)) {
    check_service(E->source->name, "restart");
  }

}


static void handle_timeout(Event_T E) {

  E->source->do_monitor= FALSE;
  
}


static void handle_resource(Event_T E) {

  Device_T d;
  Resource_T r;
  Service_T s= E->source;

  for(d= s->devicelist; d; d= d->next) {
    if(d->event_flag && d->exec != NULL) {
      d->event_flag= FALSE; 
      spawn(s, d->exec, EVENT_DESCRIPTION(E));
    }
  }

  for(r= s->resourcelist; r; r= r->next) {
    if(r->event_flag && r->exec != NULL) {
      /* Reset the event_flag so the command is not executed again
       * unless the flag is explicit set */
      r->event_flag= FALSE; 
      spawn(s, r->exec, EVENT_DESCRIPTION(E));
    }
  }

}


static void handle_timestamp(Event_T E) {

  Timestamp_T t;
  Service_T s= E->source;
  
  /* Check for executables to run */
  for(t= s->timestamplist; t; t= t->next) {
    if(t->event_flag && t->exec != NULL) {
      /* Reset the event_flag so the command is not executed again
       * unless the flag is explicit set */
      t->event_flag= FALSE; 
      spawn(s, t->exec, EVENT_DESCRIPTION(E));
    }
  }

}


static void handle_size(Event_T E) {

  Size_T sl;
  Service_T s= E->source;
  
  /* Check for executables to run */
  for(sl= s->sizelist; sl; sl= sl->next) {
    if(sl->event_flag && sl->exec != NULL) {
      /* Reset the event_flag so the command is not executed again
       * unless the flag is explicit set */
      sl->event_flag= FALSE; 
      spawn(s, sl->exec, EVENT_DESCRIPTION(E));
    }
  }

}


static void handle_connection(Event_T E) {
  
  Port_T p;
  Icmp_T i;
  Service_T s= E->source;
  
  /* Check for executables to run in the port list*/
  for(p= s->portlist; p; p= p->next) {
    if(p->event_flag && p->exec != NULL) {
      /* Reset the event_flag so the command is not executed again
       * unless the flag is explicit set */
      p->event_flag= FALSE; 
      spawn(s, p->exec, EVENT_DESCRIPTION(E));
    }
  }
  
  /* Check for executables to run in the icmp list */
  for(i= s->icmplist; i; i= i->next) {
    if(i->event_flag && i->exec != NULL) {
      i->event_flag= FALSE; 
      spawn(s, i->exec, EVENT_DESCRIPTION(E));
    }
  }

}


static void handle_checksum(Event_T E) {

  Service_T s= E->source;

  if(s->checksum->event_flag && s->checksum->exec != NULL) {
    s->checksum->event_flag= FALSE; 
    spawn(s, s->checksum->exec, EVENT_DESCRIPTION(E));
  }

}


static void handle_permission(Event_T E) {

  Service_T s= E->source;

  s->perm->has_error= TRUE;

  if(s->perm->event_flag && s->perm->exec != NULL) {
    s->perm->event_flag= FALSE; 
    spawn(s, s->perm->exec, EVENT_DESCRIPTION(E));
  }

}


static void handle_uid(Event_T E) {

  Service_T s= E->source;

  s->uid->has_error= TRUE;

  if(s->uid->event_flag && s->uid->exec != NULL) {
    s->uid->event_flag= FALSE; 
    spawn(s, s->uid->exec, EVENT_DESCRIPTION(E));
  }

}


static void handle_gid(Event_T E) {

  Service_T s= E->source;

  s->gid->has_error= TRUE;
  if(s->gid->event_flag && s->gid->exec != NULL) {
    s->gid->event_flag= FALSE; 
    spawn(s, s->gid->exec, EVENT_DESCRIPTION(E));
  }

}


static void handle_unmonitor(Event_T E) {

  check_service(E->source->name, "unmonitor");

}


/* ------------------------------------------------------------------------- */


