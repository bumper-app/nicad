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


#ifndef MONIT_ALERT_H
#define MONIT_ALERT_H

#include "event.h"


/** Default mail from string */
#define ALERT_FROM    "monit@$HOST"

/** Default mail subject */
#define ALERT_SUBJECT "monit alert --  $EVENT $SERVICE"

/** Default mail message */
#define ALERT_MESSAGE "$EVENT Service $SERVICE \r\n\r\n\tDate: $DATE\r\n"\
	      "\tHost: $HOST\r\n\r\nYour faithful employee,\r\nmonit\r\n"


/**
 *  This module is used for event notifications. Users may register
 *  interest for certain events in the monit control file. When an
 *  event occurs this module is called from the event processing
 *  machinery to notify users who have asked to be alerted for
 *  particular events.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: alert.h,v 1.16 2004/02/06 20:02:20 martinp Exp $
 *  @file
 */


/**
 * Notify registred users about the event
 * @param E An Event object
 */
void handle_alert(Event_T E);


#endif
