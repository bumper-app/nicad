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


#ifndef ENGINE_H
#define ENGINE_H

#include <config.h>

#include "net.h"
#include "processor.h"
#include "cervlet.h"

/*
 * The maximum queue length for incoming connection
 * indications (a request to connect)
 */
#define DEFAULT_QUEUE_LEN  10

typedef struct host_allow {
  char *name;  
  /* For internal use */
  struct host_allow *next;
} *HostsAllow;

/* Public prototypes */
void start_httpd(int port, int backlog, char *bindAddr);
void stop_httpd();
int add_host_allow(char *);
int has_hosts_allow();
void destroy_hosts_allow();


#endif
