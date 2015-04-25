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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "monitor.h"
#include "net.h"
#include "socket.h"
#include "event.h"


/**
 *  Methods for controlling services managed by monit.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Rory Toma, <rory@digeo.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: control.c,v 1.73 2004/02/06 20:02:20 martinp Exp $
 *
 *  @file
 */


/* -------------------------------------------------------------- Prototypes */


static void  do_start(Service_T);
static int   do_stop(Service_T);
static void  do_monitor(Service_T);
static void  do_unmonitor(Service_T);
static void  monitor_set(Service_T);
static void  monitor_unset(Service_T);
static void *wait_start(void *);
static int   wait_stop(Service_T);
static void  do_depend(Service_T, const char *);


/* ------------------------------------------------------------------ Public */


/**
 * Handle the action for all processes in service list
 * @param action A string describing the action to execute
 */
void control(const char *action) {

  Service_T s;

  ASSERT(action);

  for(s= servicelist; s; s= s->next) {
    if(s->visited)
	continue;
    if(exist_daemon()) {
      d_check_service(s->name, action);
    } else {
      check_service(s->name, action);
    }
  }
  
  reset_depend();

}


/*
 * Handle the action for all processes in a group
 * @param G group name
 * @param action A string describing the action to execute
 */
void control_group(const char *G, const char *action) {

  Service_T s;

  ASSERT(G);
  ASSERT(action);

  for(s= servicelist; s; s= s->next) {
    if(s->visited)
	continue;
    if(IS(s->group, G)) {
      if(exist_daemon()) {
	d_check_service(s->name, action);
      } else {
	check_service(s->name, action);
      }
    }
  }

  reset_depend();
 
}


/*
 * Handle the action for a specified process
 * @param P process name
 * @param action A string describing the action to execute
 */
void control_process(const char *P, const char *action) {

  Service_T s;

  ASSERT(P);
  ASSERT(action);

  if(NULL==(s= get_service(P))) {
    log("%s: Cannot %s program '%s' -- not found in %s\n",
          prog, action, P, Run.controlfile);
    return;
  }

  if(exist_daemon()) {
    d_check_service(P, action);
  } else {
    check_service(P, action);
  }
 
}


/**
 * Pass on to methods in http/cervlet.c to start/stop services
 * @param P A service name as stated in the config file
 * @param action A string describing the action to execute
 */
void d_check_service(const char *P, const char *action) {

  Socket_T s;
  char *auth= get_basic_authentication_header();

  ASSERT(P);
  ASSERT(action);


  s= socket_new(Run.bind_addr?Run.bind_addr:"localhost",
		Run.httpdport, SOCKET_TCP, Run.httpdssl);
  
  if(!s) {
    
    log("%s: Cannot connect to the monit daemon. "
          "Did you start it with http support?\n", prog);
    goto error;
    
  } else {

    socket_print(s, "GET /%s?action=%s HTTP/1.0\r\n%s\r\n", P, action, auth);
    socket_free(&s);
      
  }
  
  error:
  FREE(auth);
  
}


/**
 * Check to see if we should try to start/stop service
 * @param P A service name as stated in the config file
 * @param action A string describing the action to execute
 * @return TRUE if the service was handled successfully otherwise FALSE
 */
void check_service(const char *P, const char *action) {

  Service_T s= NULL;

  ASSERT(P);
  ASSERT(action);

  if(NULL==(s= get_service(P))) {
    log("%s: Cannot %s program '%s' -- not found in %s\n",
          prog, action, P, Run.controlfile);
    return;
  }

  if(IS(action, "start")) {
    
    if(s->type==TYPE_PROCESS && is_process_running(s)) {
      DEBUG("%s: Process already running -- process %s\n", prog, P);
      monitor_set(s);
      return;
    }
    
    if(s->type==TYPE_PROCESS && !s->start) {
      DEBUG("%s: Start method not defined -- process %s\n", prog, P);
      monitor_set(s);
      return;
    }
    
    do_depend(s, "stop");
    do_start(s);
    do_depend(s, "start");
    
  } else if(IS(action, "stop")) {
    
    if(s->type==TYPE_PROCESS && !s->stop) {
      DEBUG("%s: Stop method not defined -- process %s\n", prog, P);
      monitor_unset(s);
      return;
    }
    
    do_depend(s, "stop");
    do_stop(s);
    
  } else if(IS(action, "restart")) {
    
    if(s->type==TYPE_PROCESS && (!s->start || !s->stop)) {
      DEBUG("%s: Start or stop method not defined -- process %s\n", prog, P);
      monitor_set(s);
      return;
    } else {
      log("Trying to restart '%s'\n", s->name);
    }
    
    do_depend(s, "stop");
    if(do_stop(s)) {
      /* Only start if stop succeeded */
      do_start(s);
      do_depend(s, "start");
    }
    
  } else if(IS(action, "monitor")) {
    
    /* We only enable monitoring of this service and all prerequisite
     * services. Chain of services which depends on this service keep
     * its state */
    do_monitor(s);
    
  } else if(IS(action, "unmonitor")) {
    
    /* We disable monitoring of this service and all services which
     * depends on it */
    do_depend(s, "unmonitor");
    do_unmonitor(s);
    
  }
    
}


/*
 * Reset the visited flags used when handling dependencies
 */
void reset_depend() {

  Service_T s;
  
  for(s= servicelist; s; s= s->next) {
    s->visited= FALSE;
    s->depend_visited= FALSE;
  }

}


/* ----------------------------------------------------------------- Private */


/*
 * This is a post- fix recursive function for starting every service
 * that s depends on before starting s.
 * @param s A Service_T object
 */
static void do_start(Service_T s) {

  ASSERT(s);

  if(s->visited)
    return;
  
  s->visited= TRUE;
  
  if(s->dependantlist) {
    
    Dependant_T d;
    
    for(d= s->dependantlist; d; d= d->next ) {
      
      Service_T parent= get_service(d->dependant);
      ASSERT(parent);
      do_start(parent);
      
    }

  }
  
  if(s->start && (s->type!=TYPE_PROCESS || !is_process_running(s))) {
    int status;
    pthread_t thread;

    log("start: (%s) %s\n", s->name, s->start->arg[0]);
    spawn(s, s->start, "Started");
    if(s->type==TYPE_PROCESS) {
      /* We only wait for a process type, other service types does not
       * have a pid file to watch */
      status= pthread_create(&thread, NULL, wait_start, s);
      if(status != 0) {
	log("Warning: Failed to create the start controller thread. "
	    "Thread error -- %s.\n", strerror(status));
      }
    }
  }

  monitor_set(s);

}


/*
 * This function simply stops the service p.
 * @param s A Service_T object
 * @return TRUE if the service was stopped otherwise FALSE
 */
static int do_stop(Service_T s) {

  ASSERT(s);

  if(s->depend_visited)
    return TRUE;
 
  s->depend_visited= TRUE;
 
  monitor_unset(s);

  if(s->type==TYPE_PROCESS) {
    /* Reset the proc info object in case of a later restart */
    memset(s->procinfo, 0, sizeof *(s->procinfo));
  }

  if(s->stop && (s->type!=TYPE_PROCESS || is_process_running(s))) {
    log("stop: (%s) %s\n", s->name, s->stop->arg[0]);
    spawn(s, s->stop, "Stopped");

    if(s->type==TYPE_PROCESS) {
      /* Only wait for process service types */
      return wait_stop(s);
    }

  }

  return TRUE;
 
}


/*
 * This is a post- fix recursive function for enabling monitoring every service
 * that s depends on before monitor s.
 * @param s A Service_T object
 */
static void do_monitor(Service_T s) {

  ASSERT(s);

  if(s->visited)
    return;
  
  s->visited= TRUE;
  
  if(s->dependantlist) {
    
    Dependant_T d;
    
    for(d= s->dependantlist; d; d= d->next ) {
      
      Service_T parent= get_service(d->dependant);
      ASSERT(parent);
      do_monitor(parent);
      
    }
  }
  
  monitor_set(s);
   
}


/*
 * This is a function for disabling monitoring
 * @param s A Service_T object
 */
static void do_unmonitor(Service_T s) {

  ASSERT(s);

  if(s->depend_visited)
    return;
 
  s->depend_visited= TRUE;
  
  monitor_unset(s);
   
  if(s->type==TYPE_PROCESS) {
    /* Reset the proc info object in case of a later restart */
    memset(s->procinfo, 0, sizeof *(s->procinfo));
  }
  
}


/*
 * This will enable service monitoring in the case that it was disabled
 * @param s A Service_T object
 */
static void monitor_set(Service_T s) {

  ASSERT(s);

  if(!s->do_monitor) {
    LOCK(Run.mutex)
      s->do_monitor= TRUE;
    END_LOCK;
    DEBUG("Monitoring enabled -- service %s\n", s->name);
  }

}


/*
 * This will disable service monitoring in the case that it is enabled
 * @param s A Service_T object
 */
static void monitor_unset(Service_T s) {

  ASSERT(s);

  if(s->do_monitor) {
    LOCK(Run.mutex)
      s->do_monitor= FALSE;
    END_LOCK;
    DEBUG("Monitoring disabled -- service %s\n", s->name);
  }

}


/*
 * This is an in-fix recursive function called before s is started to
 * stop every service that depends on s, in reverse order *or* after s
 * was started to start again every service that depends on s. The
 * action parametere controls if this function should start or stop
 * the procceses that depends on s.
 * @param s A Service_T object
 * @param action An action to do on the dependant services
 */
static void do_depend(Service_T s, const char *action) {

  Service_T child;
  
  ASSERT(s);

  for(child= servicelist; child; child= child->next) {
    
    if(child->dependantlist) {

      Dependant_T d;
    
      for(d= child->dependantlist; d; d= d->next) {

	if(IS(d->dependant, s->name)) {
	
	  if(IS(action, "start"))
	    do_start(child);
          else if(IS(action, "monitor"))
	    do_monitor(child);
	
	  do_depend(child, action);
	
	  if(IS(action, "stop"))
	    do_stop(child);
          else if(IS(action, "unmonitor"))
	    do_unmonitor(child);

	  break;

	}
      }
    }
  }
}
    

/*
 * This function runs in it's own thread and waits for the service to
 * start running. If the service did not start a failed event is
 * posted to notify the user.
 * @param service A Service to wait for
 */
static void *wait_start(void *service) {

  Service_T s= service;
  int max_tries= Run.polltime;
  
  ASSERT(s);

  pthread_detach(pthread_self());

  while(max_tries-- && !Run.stopped) {
    if(is_process_running(s))
      break;
    sleep(1);
  }
  
  if(!is_process_running(s)) {
    Event_post(s, EVENT_FAILED, "Failed to start '%s'\n", s->name);
  }

  return NULL;
  
}


/*
 * This function waits for the service to stop running. If the service
 * did not stop a failed event is posted to notify the user. This
 * function does purposefully not run in its own thread because, if we
 * did a restart we need to know if we successfully managed to stop
 * the service first before we can do a start.
 * @param service A Service to wait for
 * @return TRUE if the service was stopped otherwise FALSE
 */
static int wait_stop(Service_T s) {

  int max_tries= Run.polltime;
  
  ASSERT(s);

  while(max_tries-- && !Run.stopped) {
    if(!is_process_running(s))
      break;
    sleep(1);
  }

  if(is_process_running(s)) {
    Event_post(s, EVENT_FAILED, "Failed to stop '%s'\n", s->name);
    return FALSE;
  }

  return TRUE;

}
