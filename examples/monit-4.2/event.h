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

#ifndef MONIT_EVENT_H
#define MONIT_EVENT_H

#include "monitor.h"


#define EVENT_NULL       0x0
#define EVENT_FAILED     0x01
#define EVENT_START      0x02
#define EVENT_STOP       0x04
#define EVENT_RESTART    0x08
#define EVENT_CHECKSUM   0x10
#define EVENT_RESOURCE   0x20
#define EVENT_TIMEOUT    0x40
#define EVENT_TIMESTAMP  0x80
#define EVENT_SIZE       0x100
#define EVENT_CONNECTION 0x200
#define EVENT_PERMISSION 0x400
#define EVENT_UID        0x800
#define EVENT_GID        0x1000
#define EVENT_UNMONITOR  0x2000


#define EVENT_DESCRIPTION(E) Event_get_description(E)
#define IS_EVENT_SET(value, mask) ((value & mask) != 0)


/**
 * This class implements the <b>event</b> processing machinery used by
 * monit. In monit an event is an object containing a Service_T
 * reference indicating the object where the event orginated, an id
 * specifying the event type and an optional message describing why
 * the event was fired.
 *
 * Clients may use the function Event_post() to post events to the
 * event handler for processing.
 * 
 * @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * @version \$Id: event.h,v 1.13 2004/01/29 17:52:11 martinp Exp $
 * @file
 */


typedef struct Event_T *Event_T;


/**
 * Construct and post a new Event
 * @param source The Service the event occured on
 * @param id The event type
 * @param s Optional message describing the event
 */
void Event_post(Service_T source, long id, char *s, ...);


/**
 * Destroy an Event object and release allocated resources. 
 * @param E An Event object reference
 */
void Event_free(Event_T *E);


/**
 * Get the Service where the event orginated
 * @param E An event object
 * @return The Service where the event orginated
 */
Service_T Event_get_source(Event_T E);


/**
 * Get the Event type
 * @param E An event object
 * @return The Event type
 */
int Event_get_id(Event_T E);


/**
 * Get the optionally Event message describing why the event was
 * fired.
 * @param E An event object
 * @return The Event message. May be NULL
 */
const char *Event_get_message(Event_T E);


/**
 * Get a textual description of the event type. For instance if the
 * event type is EVENT_RESTART, the textual description is
 * "Restart". Likewise if the event type is EVENT_CHECKSUM the textual
 * description is "Checksum error" and so on.
 * @param E An event object
 * @return A string describing the event type in clear text. If the
 * event type is not found NULL is returned.
 */
const char *Event_get_description(Event_T E);


#endif
