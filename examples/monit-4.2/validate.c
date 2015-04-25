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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef HAVE_SOL_IP
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

#ifdef HAVE_NETINET_IP_ICMP_H
#include <netinet/ip_icmp.h>
#endif

#include "monitor.h"
#include "alert.h"
#include "event.h"
#include "socket.h"
#include "net.h"
#include "device.h"
#include "process.h"


/**
 *  Implementation of validation engine
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Olivier Beyssac, <ob@r14.freenix.org> (check_skip)
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: validate.c,v 1.109 2004/02/29 22:24:44 martinp Exp $
 *  @file
 */


/* -------------------------------------------------------------- Prototypes */


static int  compare_value(int, int, int);
static void reset_resource_counter(Service_T);

static int  check_process_state(Service_T, char *);
static int  check_process_resources(Service_T, Resource_T, char *);
static int  check_process_connection(Service_T, Port_T, char *);

static int  check_icmp_connection(Service_T, Icmp_T, char *);

static int  check_device_resources(Service_T, Device_T, char *);

static int  check_timestamps(Service_T);
static int  check_timestamp_item(Service_T, Timestamp_T, char *);

static int  check_size_item(Service_T, Size_T, unsigned long, char *);

static int  check_timeout(Service_T);
static int  check_skip(Service_T);

static int  check_checksum(Service_T, char *);
static int  check_perm(Service_T, mode_t, char *);
static int  check_uid(Service_T, uid_t, char *);
static int  check_gid(Service_T, gid_t, char *);

static int  eval_actions(int, Service_T, char *, char *, int);


/* ---------------------------------------------------------------- Public */


/**
 *  This function contains the main check machinery for  monit. The
 *  validate function check services in the service list to see if
 *  they will pass all defined tests.
 */
void validate() {

  Service_T s;
  sigset_t ns, os;

  if(! update_loadavg())
    log("Update of loadavg has failed!\n");

  if(Run.doprocess)
    initprocesstree();

  for(s= servicelist; s; s= s->next) {
    if(s->visited)
      continue;
    LOCK(s->mutex)
      set_signal_block(&ns, &os);
      if(s->do_monitor && !check_skip(s) && !check_timeout(s))
        s->check(s);
      unset_signal_block(&os);
    END_LOCK;
  }

  if(Run.doprocess)
    delprocesstree();

  reset_depend();

}


/**
 * Validate a given process service s. Events are posted according to 
 * its configuration. In case of a fatal event FALSE is returned.
 */
int check_process(Service_T s) {

  pid_t  pid= -1;
  Port_T pp= NULL;
  Resource_T pr= NULL;
  char report[STRLEN]={0};
  
  /* Test for running process */
  if(!(pid= is_process_running(s))) {
    /* Reset the proc info object to prevent false data in the first run */
    memset(s->procinfo, 0, sizeof *(s->procinfo));
    Event_post(s, EVENT_START, "Event: Process '%s' is not running.\n",
	       s->name);
    return FALSE;
  } else {
    DEBUG("'%s' is running with pid %d\n", s->name, (int)pid);
  }

  if(Run.doprocess) {
    if(update_process_data(s, ptree, ptreesize, pid)) {
      if(! check_process_state(s, report)) {
	Event_post(s, EVENT_RESOURCE, "Event: %s\n", report);
      } else {
	DEBUG("'%s' check_process_state() passed.\n", s->name);
      }
      
      for(pr= s->resourcelist; pr; pr= pr->next) {
	if(!check_process_resources(s, pr, report)) {
	  pr->cycle=0;
	  if(! pr->event_handled) {
	    pr->event_flag= TRUE;
	    pr->event_handled= TRUE;
	    if(! eval_actions(pr->action, s, report, "resource",
			      EVENT_RESOURCE)) {
	      reset_resource_counter(s);
	      return FALSE;
	    }
	  }
	} else {
	  pr->event_handled= FALSE;
	}
      }
    } else {
      log("'%s' failed to get service data\n", s->name);
    }
  }

  /* Test each host:port and protocol in the service's portlist */
  for(pp= s->portlist; pp; pp= pp->next) {
    if(!check_process_connection(s, pp, report)) {
      if(! pp->event_handled) {
	pp->event_flag= TRUE;
	pp->event_handled= TRUE;
	if(! eval_actions(pp->action, s, report, "connection",
			  EVENT_CONNECTION)) {
	  return FALSE;
	}
      }
    } else {
      pp->event_handled= FALSE;
    }
  }
  
  return TRUE;
  
}


/**
 * Validate a given device service s.  Events are posted according to 
 * its configuration.  In case of a fatal event FALSE is returned.
 */
int check_device(Service_T s) {

  Device_T td;
  struct stat stat_buf;
  char report[STRLEN]= {0};

  if(stat(s->path, &stat_buf) != 0) {
    if(! s->event_handled) {
      Event_post(s, EVENT_START, "Event: device '%s' doesn't exist\n", s->name);
      s->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->event_handled= FALSE;
  }
  
  if(check_perm(s, stat_buf.st_mode, report)) {
    s->perm->event_flag= TRUE;
    if(! eval_actions(s->perm->action, s, report, "permission",
		      EVENT_PERMISSION))
      return FALSE;
  }

  if(check_uid(s, stat_buf.st_uid, report)) {
    s->uid->event_flag= TRUE;
    if(! eval_actions(s->uid->action, s, report, "uid", EVENT_UID))
      return FALSE;
  }

  if(check_gid(s, stat_buf.st_gid, report)) {
    s->gid->event_flag= TRUE;
    if(! eval_actions(s->gid->action, s, report, "gid", EVENT_GID))
      return FALSE;
  }

  if(!DeviceInfo_Usage(s->devinfo, s->path)) {
    if(! s->devinfo->event_handled) {
      Event_post(s, EVENT_START,
		 "Event: unable to read device '%s' state\n",
		 s->path);
      s->devinfo->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->devinfo->event_handled= FALSE;
    DEBUG("'%s' succeeded getting device statistic for %s\n", s->name, s->path);
  }

  /* Test devices */
  if(s->devicelist) {
    for(td= s->devicelist; td; td= td->next) {
      if(!check_device_resources(s, td, report)) {
	if(! td->event_handled) {
	  td->event_flag= TRUE; /* Turn on the object's event_flag
				 * to indicate that the resource
				 * event occured on this particular
				 * object */
	  td->event_handled= TRUE; /* Turn on the object's
				    * event_handled flag so this
				    * test is not handled in
				    * subsequent poll cycles until
				    * the error condition was reset.
				    */

	  if(! eval_actions(td->action, s, report, "device", EVENT_RESOURCE)) {
	    return FALSE;
	  }
	}
      } else {
	td->event_handled= FALSE; 
      }
    }
    
  }
  
  return TRUE;

}


/**
 * Validate a given file service s.  Events are posted according to 
 * its configuration.  In case of a fatal event FALSE is returned.
 */
int check_file(Service_T s) {

  Size_T sl;
  struct stat stat_buf;
  char report[STRLEN]= {0};

  if(stat(s->path, &stat_buf) != 0) {
    if(! s->event_handled) {
      Event_post(s, EVENT_START, "Event: file '%s' doesn't exist\n", s->name);
      s->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->event_handled= TRUE;
  }
  
  if(!S_ISREG(stat_buf.st_mode)) {
    if(! s->event_handled) {
      Event_post(s, EVENT_UNMONITOR,
		 "Event: '%s' is not regular file\n", s->name);
      s->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->event_handled= FALSE;
  }
  
  if(check_checksum(s, report)) {
      s->checksum->event_flag= TRUE;
      if(! eval_actions(s->checksum->action, s, report, "checksum",
			EVENT_CHECKSUM))
	  return FALSE;
  }
  
  if(check_perm(s, stat_buf.st_mode, report)) {
    s->perm->event_flag= TRUE;
    if(! eval_actions(s->perm->action, s, report, "permission",
		      EVENT_PERMISSION))
	return FALSE;
  }

  if(check_uid(s, stat_buf.st_uid, report)) {
    s->uid->event_flag= TRUE;
    if(! eval_actions(s->uid->action, s, report, "uid", EVENT_UID))
	return FALSE;
  }
  
  if(check_gid(s, stat_buf.st_gid, report)) {
    s->gid->event_flag= TRUE;
    if(! eval_actions(s->gid->action, s, report, "gid", EVENT_GID))
	return FALSE;
  }

  for(sl= s->sizelist; sl; sl= sl->next) {
    if(!check_size_item(s, sl, (unsigned long)stat_buf.st_size, report)) {
      if(! sl->event_handled) {
	/* Turn on the object's event_flag to indicate that the size event
	 * occured on this particular object */
	sl->event_flag= TRUE;
	sl->event_handled= TRUE; /* Turn on the object's
				  * event_handled flag so this
				  * test is not handled in
				  * subsequent poll cycles until
				  * the error condition was reset.
				  */
	if(! eval_actions(sl->action, s, report, "size", EVENT_SIZE)) {
	  return FALSE;
	}
      }
    } else {
      sl->event_handled= FALSE;
    }
  }

  if(!check_timestamps(s))
    return FALSE;

  return TRUE;

}


/**
 * Validate a given directory service s.  Events are posted according to 
 * its configuration.  In case of a fatal event FALSE is returned.
 */
int check_directory(Service_T s) {

  struct stat stat_buf;
  char report[STRLEN]= {0};

  if(stat(s->path, &stat_buf) != 0) {
    if(! s->event_handled) {
      Event_post(s, EVENT_START,
		 "Event: directory '%s' doesn't exist\n", s->name);
      s->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->event_handled= FALSE;
  }

  if(!S_ISDIR(stat_buf.st_mode)) {
    if(!s->event_handled) {
      Event_post(s, EVENT_UNMONITOR,
		 "Event: '%s' is not directory\n", s->name);
      s->event_handled= TRUE;
    }
    return FALSE;
  } else {
    s->event_handled= FALSE;
  }
  
  if(check_perm(s, stat_buf.st_mode, report)) {
    s->perm->event_flag= TRUE;
    if(! eval_actions(s->perm->action, s, report, "permission",
		      EVENT_PERMISSION))
	return FALSE;
  }
  
  if(check_uid(s, stat_buf.st_uid, report)) {
    s->uid->event_flag= TRUE;
    if(! eval_actions(s->uid->action, s, report, "uid", EVENT_UID))
	return FALSE;
  } 
  
  if(check_gid(s, stat_buf.st_gid, report)) {
    s->gid->event_flag= TRUE;
    if(! eval_actions(s->gid->action, s, report, "gid", EVENT_GID))
	return FALSE;
  } 

  if(!check_timestamps(s))
    return FALSE;

  return TRUE;

}


/**
 * Validate a remote service.
 * @param s The remote service to validate
 * @return FALSE if there was an error otherwise TRUE
 */
int check_remote_host(Service_T s) {

  Port_T p= NULL;
  char report[STRLEN]={0};
  Icmp_T icmp, last_ping= NULL;

  /* Test each icmp type in the service's icmplist */
  for(icmp= s->icmplist; icmp; icmp= icmp->next) {
    if(!check_icmp_connection(s, icmp, report)) {
      icmp->is_available= FALSE;
      last_ping= icmp;
      if(! icmp->event_handled) {
	icmp->event_handled= TRUE;
	icmp->event_flag= TRUE;
	if(! eval_actions(icmp->action, s, report, "icmp",
			  EVENT_CONNECTION)) {
	  return FALSE;
	}
      }
    } else {
      icmp->is_available= TRUE;
      icmp->event_handled= FALSE;
    }
  }

  /* If we could not ping the host we assume it's down and do not
   * continue to check any port connections  */
  if(last_ping && !last_ping->is_available) {
    DEBUG("'%s' icmp ping failed, skipping any port connection tests\n",
	  s->name);
    return FALSE;
  }
    

  /* Test each host:port and protocol in the service's portlist */
  for(p= s->portlist; p; p= p->next) {
    if(!check_process_connection(s, p, report)) {
      p->is_available= FALSE;
      if(! p->event_handled) {
	p->event_flag= TRUE; 
	p->event_handled= TRUE;
	if(! eval_actions(p->action, s, report, "connection",
			  EVENT_CONNECTION)) {
	  return FALSE;
	}
      }
    } else {
      p->is_available= TRUE;
      p->event_handled= FALSE; 
    }
  }

  return TRUE;
  
}


/* --------------------------------------------------------------- Private */


/**
 * Evaluate actions of a service.  Post appropriate actions if necessary.
 * Return FALSE in case of a fatal event.
 */
static int eval_actions(int action, Service_T s, char *report, char *check,
			int event) {

  ASSERT(s);
  ASSERT(check);
  ASSERT(report);

  switch(action) {
  case ACTION_ALERT:
      Event_post(s, event, "Event: %s\n", report);
      break; /* continue */
    
  case ACTION_STOP:
      Event_post(s, EVENT_STOP, "Event: %s\n", report);
      return FALSE;
      
  case ACTION_RESTART:
      Event_post(s, EVENT_RESTART, "Event: %s\n", report);
      return FALSE;
      
  case ACTION_EXEC:
      Event_post(s, event, "Event: %s\n", report);
      break; /* continue */

  case ACTION_UNMONITOR:
      Event_post(s, event, "Event: %s\n", report);
      Event_post(s, EVENT_UNMONITOR, "Event: %s\n", report);
      return FALSE;

  default:
      log("'%s' error -- unknown failure action: [%s]\n", s->name, check);
      break;
    
  }

  return TRUE;
}


/**
 * Returns TRUE if the connection and protocol test succeeded
 * otherwise FALSE.
 */
static int check_process_connection(Service_T s, Port_T p, char *report) {

  Socket_T socket;
  volatile int rv= TRUE;

  ASSERT(s && p);

  /* Open a socket to the destination INET[hostname:port] or UNIX[pathname] */
  socket= socket_create(p);
  if(!socket) {
    snprintf(report, STRLEN, 
	     "'%s' failed, cannot open a connection to %s",
	     s->name, p->address);
    rv= FALSE;
    goto error;
  } else {
    DEBUG("'%s' succeeded connecting to %s\n", s->name, p->address);
  }

  /* Verify that the socket is ready for i|o */
  if(! socket_is_ready(socket)) {
    snprintf(report, STRLEN, 
	     "'%s' failed, the socket at %s is not ready for i|o -- %s",
	     s->name, p->address, STRERROR);
    rv= FALSE;
    goto error;
  }

  /* Run the protocol verification routine through the socket */
  if(! p->protocol->check(socket)) {
    snprintf(report, STRLEN, 
	     "'%s' failed protocol test [%s] at %s.",
	     s->name, p->protocol->name, p->address);
    rv= FALSE;
    goto error;
  } else {
    DEBUG("'%s' succeeded testing protocol [%s] at %s\n",
	  s->name, p->protocol->name, p->address);
  }

  error:
  if(socket) socket_free(&socket);
  
  return rv;
      
}


/**
 * Returns TRUE if the ICMP test succeeded otherwise FALSE.
 */
static int check_icmp_connection(Service_T s, Icmp_T i, char *report) {

  if(i->type == ICMP_ECHO && !icmp_echo(s->path, i->timeout)) {

    snprintf(report, STRLEN, "'%s' failed ICMP test [%s]", s->name,
             icmpnames[i->type]);

    return FALSE;

  }

  DEBUG("'%s' succeeded testing ICMP [%s]\n", s->name, icmpnames[i->type]);

  return TRUE;

}


/**
 * Returns TRUE if the process state (e.g. Zombie) test succeeded
 * otherwise FALSE.
 */
static int check_process_state(Service_T s, char *report) {

  ProcInfo_T pi;

  ASSERT(s);
  
  pi= s->procinfo;
  
  if(pi->status_flag & PROCESS_ZOMBIE) {
    snprintf(report, STRLEN, "process with pid %d is a zombie", pi->pid);
    /* We do not check the process anymore if it's a zombie
       since such a process is (usually) unmanageable */
    LOCK(Run.mutex)
      s->do_monitor= FALSE;
    END_LOCK;
    return FALSE;
  } else {
    DEBUG("'%s' zombie check passed [status_flag=%04x]\n",
          s->name,  pi->status_flag);
  }
  
  return TRUE;

}


/**
 * Resets the resource counter
 */
static void reset_resource_counter(Service_T s) {
  
  Resource_T pr;

  ASSERT(s);
  
  for(pr= s->resourcelist; pr; pr= pr->next) {
    pr->cycle=0;
  }
  
}


/**
 * Returns TRUE if the proc test succeeded
 * otherwise FALSE.
 */
static int check_process_resources(Service_T s, Resource_T pr, char *report) {

  ProcInfo_T pi;
  int okay= TRUE;

  ASSERT(s);
  ASSERT(pr);

  pi= s->procinfo;
  
  switch(pr->resource_id) {
  case RESOURCE_ID_CPU_PERCENT: 
      if(compare_value(pr->operator, pi->cpu_percent, pr->limit)) {
	snprintf(report, STRLEN,
	 "cpu usage of %.1f%% matches resource limit [cpu usage%s%.1f%%]",
		 pi->cpu_percent/10.0, operatorshortnames[pr->operator],
		 pr->limit/10.0);
	okay= FALSE;
      } else {
	DEBUG("'%s' cpu usage check passed [current cpu usage=%.1f%%]\n", 
	      s->name, pi->cpu_percent/10.0);
      }
      break;

  case RESOURCE_ID_MEM_PERCENT:
      if(compare_value(pr->operator, pi->mem_percent, pr->limit)) {
	snprintf(report, STRLEN,
	   "mem usage of %.1f%% matches resource limit [mem usage%s%.1f%%]",
		 pi->mem_percent/10.0, operatorshortnames[pr->operator],
		 pr->limit/10.0);
	okay= FALSE;
      } else {
	DEBUG("'%s' mem usage check passed [current mem usage=%.1f%%]\n", 
	      s->name, pi->mem_percent/10.0);
      }
      break;
      
  case RESOURCE_ID_MEM_KBYTE: 
    if(compare_value(pr->operator, pi->mem_kbyte, pr->limit)) {
      snprintf(report, STRLEN,
	   "mem amount of %ldkB matches resource limit [mem amount%s%ldkB]", 
	       pi->mem_kbyte, operatorshortnames[pr->operator],
	       pr->limit);      
      okay= FALSE;
    } else {
      DEBUG("'%s' mem amount check passed [current mem amount=%ldkB]\n", 
	    s->name, pi->mem_kbyte);
    }
    break;

  case RESOURCE_ID_LOAD1: 
    if(compare_value(pr->operator, (int)(Run.loadavg[0]*10.0), pr->limit)) {
      snprintf(report, STRLEN,
	       "loadavg(1min) of %.1f matches resource limit "
	       "[loadavg(1min)%s%.1f]", 
	       Run.loadavg[0], operatorshortnames[pr->operator],
	       pr->limit/10.0);      
      okay= FALSE;
    } else {
      DEBUG("'%s' loadavg(1min) check passed [current loadavg(1min)=%.1f]\n", 
	    s->name, Run.loadavg[0]);
    }
    break;

  case RESOURCE_ID_LOAD5: 
    if(compare_value(pr->operator, (int)(Run.loadavg[1]*10.0), pr->limit)) {
      snprintf(report, STRLEN,
	       "loadavg(5min) of %.1f matches resource limit "
	       "[loadavg(5min)%s%.1f]", 
	       Run.loadavg[1], operatorshortnames[pr->operator],
	       pr->limit/10.0);      
      okay= FALSE;
    } else {
      DEBUG("'%s' loadavg(5min) check passed [current loadavg(5min)=%.1f]\n", 
	    s->name, Run.loadavg[1]);
    }
    break;
    
  case RESOURCE_ID_LOAD15: 
      if(compare_value(pr->operator, (int)(Run.loadavg[2]*10.0), pr->limit)) {
	snprintf(report, STRLEN,
		 "loadavg(15min) of %.1f matches resource limit "
		 "[loadavg(15min)%s%.1f]", 
		 Run.loadavg[2], operatorshortnames[pr->operator],
		 pr->limit/10.0);      
	okay= FALSE;
      } else {
	DEBUG("'%s' loadavg(15min) check passed "
	      "[current loadavg(15min)=%.1f]\n", 
	      s->name, Run.loadavg[2]);
      }
      break;
      
  case RESOURCE_ID_CHILDREN:
      if(compare_value(pr->operator, pi->children, pr->limit)) {
	snprintf(report, STRLEN,
		 "children of %i matches resource limit [children%s%ld]",
		 pi->children, operatorshortnames[pr->operator],
		 pr->limit);
	okay= FALSE;
      } else {
	DEBUG("'%s' children check passed [current children=%i]\n",
	      s->name, pi->children);
      }
    break;

  case RESOURCE_ID_TOTAL_MEM_KBYTE:
    if(compare_value(pr->operator, pi->total_mem_kbyte, pr->limit)) {
      snprintf(report, STRLEN,
	       "total mem amount of %ldkB matches resource limit"
	       " [total mem amount%s%ldkB]",
	       pi->total_mem_kbyte, operatorshortnames[pr->operator],
	       pr->limit);
      okay= FALSE;
    } else {
      DEBUG("'%s' total mem amount check passed "
	    "[current total mem amount=%ldkB]\n",
	    s->name, pi->total_mem_kbyte);
    }
    break;

  default:
      log("'%s' error -- unknown resource ID: [%d]\n", s->name, pr->resource_id);
  }

  if(okay && pr->cycle > 0) {
    pr->cycle--;
  } else if(! okay) {
    pr->cycle++;
  }

  if(pr->cycle >= pr->max_cycle) {
    return FALSE;
  }
    
  return TRUE;
  
}


/**
 * Returns TRUE if the service timed out, otherwise FALSE. 
 */
static int check_timeout(Service_T s) {

  ASSERT(s);
  
  if(!s->def_timeout) {
    return FALSE;
  }
  
  /*
   * Start counting cycles
   */
  if(s->nstart > 0) {
    s->ncycle++;
  }
  
  /*
   * Check timeout
   */
  if(s->nstart >= s->to_start && s->ncycle <= s->to_cycle) {
    Event_post(s, EVENT_TIMEOUT,
	       "Service '%s' timed out and will not be checked anymore.\n",
	       s->name);
    return TRUE;
  }
  
  /*
   * Stop counting and reset if the
   * cycle interval is passed
   */
  if(s->ncycle > s->to_cycle) {
    s->ncycle= 0;
    s->nstart= 0;
  }

  return FALSE;
  
}


/**
 * Returns TRUE if validation should be skiped for
 * this service in this cycle, otherwise FALSE
 */
static int check_skip(Service_T s) {

  ASSERT(s);
  
  if(!s->def_every) {
    return FALSE;
  }
  
  if(++s->nevery < s->every) {
      return TRUE;
  }
  s->nevery= 0;

  return FALSE;

}


/**
 * Returns TRUE if the checksum was changed for associated
 * path and send an alert warning.
 * Returns FALSE if the checksum is the same or not defined
 * for this service.
 */
static int check_checksum(Service_T s, char *report) {

  ASSERT(s);

  if(!s->checksum)
    return FALSE;

  if( !check_hash(s->path, s->checksum->hash, s->checksum->type) ) {
    snprintf(report, STRLEN, "checksum test failed for %s", s->path);
    /* Set the old checksum to the new value so we do not report this
     * more than once per change */
    FREE(s->checksum->hash);
    s->checksum->hash= get_checksum(s->path, s->checksum->type);
    return TRUE;
  }
  
  DEBUG("'%s' has valid checksums\n", s->name);
  
  return FALSE;

}


/**
 * Returns TRUE if the permission was changed for associated
 * path and send an alert warning.
 * Returns FALSE if the permission is the same or not defined
 * for this service.
 */
static int check_perm(Service_T s, mode_t mode, char *report) {

  ASSERT(s);

  if(!s->perm)
    return FALSE;

  if((mode & 07777) != s->perm->perm) {
    snprintf(report, STRLEN,
             "permission test failed for %s -- current permission is %o",
             s->path, mode&07777);
    s->perm->perm= (mode & 07777);
    return TRUE;
  }

  DEBUG("'%s' file permission check passed [current permission=%o]\n",
	s->name, mode&07777);

  return FALSE;

}


/**
 * Returns TRUE if the uid was changed for associated
 * path and send an alert warning.
 * Returns FALSE if the uid is the same or not defined
 * for this service.
 */
static int check_uid(Service_T s, uid_t uid, char *report) {

  ASSERT(s);

  if(!s->uid)
    return FALSE;

  if( uid != s->uid->uid ) {
    snprintf(report, STRLEN,
             "uid test failed for %s -- current uid is %d",
             s->path, (int)uid);
    s->uid->uid= uid;
    return TRUE;
  }

  DEBUG("'%s' device uid check passed [current uid=%d]\n",
	s->name, (int)uid);

  return FALSE;

}


/**
 * Returns TRUE if the gid was changed for associated
 * path and send an alert warning.
 * Returns FALSE if the gid is the same or not defined
 * for this service.
 */
static int check_gid(Service_T s, gid_t gid, char *report) {

  ASSERT(s);

  if(!s->gid)
    return FALSE;

  if( gid != s->gid->gid ) {
    snprintf(report, STRLEN,
             "gid test failed for %s -- current gid is %d",
             s->path, (int)gid);
    s->gid->gid= gid;
    return TRUE;
  }

  DEBUG("'%s' device gid check passed [current gid=%d]\n",
	s->name, (int)gid);

  return FALSE;

}


/**
 * Validate all given timestamps of a service s.  Events are posted according 
 * to its configuration.  In case of a fatal event FALSE is returned.
 */
static int check_timestamps(Service_T s) {

  Timestamp_T tl;
  char report[STRLEN];
  
  for(tl= s->timestamplist; tl; tl= tl->next) {
    if(!check_timestamp_item(s, tl, report)) {
      tl->event_flag= TRUE; /* Turn on the object's event_flag to
			     * indicate that the timestamp event
			     * occured on this particular object */
      if(! eval_actions(tl->action, s, report, "timestamp",
			EVENT_TIMESTAMP)) {
	return FALSE;
      }
    }
  }
  
  return TRUE;

}


/**
 * Returns TRUE if the timestamp test succeded, otherwise FALSE
 */
static int check_timestamp_item(Service_T s, Timestamp_T t, char *report) {

  time_t now;
  time_t timestamp;

  ASSERT(s);
  ASSERT(t);

  if((int)time(&now) == -1) {
    snprintf(report, STRLEN, "can't get actual time");      
    return FALSE;
  }

  if(!(timestamp= get_timestamp(s->path, S_IFDIR|S_IFREG))) {
    snprintf(report, STRLEN, "can't get timestamp for %s", s->path);
    return FALSE;
  }

  if(t->test_changes) {
    if(t->timestamp != timestamp) {
      snprintf(report, STRLEN, "timestamp was changed for %s", s->path);
      /* Set the old timestamp to the new value so we do not report
       * this more than once per change */
      t->timestamp= timestamp;
      return FALSE;
    }
  } else {
    if(compare_value(t->operator, (int)(now - timestamp), t->time)) {
      if(! t->event_handled) {
	snprintf(report, STRLEN, "timestamp test failed for %s", s->path);
	t->event_handled= TRUE;
	return FALSE;
      }
    } else {
      t->event_handled= FALSE;
    }
  }

  DEBUG("'%s' timestamp test passed for %s\n", s->name, s->path); 
      
  return TRUE;

}


/**
 * Returns TRUE if the size test succeded, otherwise FALSE
 */
static int check_size_item(Service_T s, Size_T sl, unsigned long size,
			   char *report) {

  ASSERT(s);
  ASSERT(sl);

  if(sl->test_changes) {
    if(sl->runsize != size) {
      snprintf(report, STRLEN, "size was changed for %s", s->path);
      /* Set the old size to the new value so we do not report
       * this more than once per change */
      sl->runsize= size;
      return FALSE;
    }
  } else {
    if(compare_value(sl->operator, size, sl->size)) {
      snprintf(report, STRLEN,
               "size test failed for %s -- current size is %lu B",
               s->path, size);
      return FALSE;
    }
  }

  DEBUG("'%s' file size check passed [current size=%lu B]\n",
	s->name, size);

  return TRUE;

}


/**
 * Returns TRUE if the device test succeeded
 * otherwise FALSE.
 */
static int check_device_resources(Service_T t, Device_T td, char *report) {

  ASSERT(t);
  ASSERT(td);

  if( (td->limit_percent < 0) && (td->limit_absolute < 0) ) {
    log("'%s' error: device limit not set\n", t->name);
    return FALSE;
  }

  switch(td->resource) {

  case RESOURCE_ID_INODE:
      if(t->devinfo->f_files <= 0) {
	DEBUG("'%s' filesystem doesn't support inodes\n", t->name);
	return TRUE;
      }

      if(td->limit_percent >= 0) {
	if(compare_value(
	       td->operator,
	       100 * (t->devinfo->f_files - t->devinfo->f_filesfree) /
	       t->devinfo->f_files, td->limit_percent)) {
	  snprintf(report, STRLEN,
          "inode usage %ld%% matches resource limit [inode usage%s%d%%]",
		   100 * (t->devinfo->f_files - t->devinfo->f_filesfree) /
		   t->devinfo->f_files,
		   operatorshortnames[td->operator],
		   td->limit_percent);
	  return FALSE;
	}
      } else {
	if(compare_value(td->operator, t->devinfo->f_files -
			 t->devinfo->f_filesfree, td->limit_absolute)) {
	  snprintf(report, STRLEN,
		   "inode usage %ld matches resource limit [inode usage%s%ld]",
		   t->devinfo->f_files - t->devinfo->f_filesfree,
		   operatorshortnames[td->operator],
		   td->limit_absolute);
	  return FALSE;
	}
      }
      
      DEBUG("'%s' inode usage check passed [current inode usage=%.1f%%]\n",
	    t->name,
	    (float) 100 * (t->devinfo->f_files - t->devinfo->f_filesfree) /
	    t->devinfo->f_files);
      
      return TRUE;

  case RESOURCE_ID_SPACE:
      if(td->limit_percent >= 0) {
        if(compare_value(
	       td->operator,
	       100 * (t->devinfo->f_blocks - t->devinfo->f_blocksfreetotal) /
	       t->devinfo->f_blocks, td->limit_percent)) {
          snprintf(report, STRLEN,
	       "space usage %ld%% matches resource limit [space usage%s%d%%]",
		   100*(t->devinfo->f_blocks - t->devinfo->f_blocksfreetotal) /
		   t->devinfo->f_blocks, operatorshortnames[td->operator],
		   td->limit_percent);
          return FALSE;
        }
      } else {
        if(compare_value(td->operator, t->devinfo->f_blocks -
			 t->devinfo->f_blocksfreetotal, td->limit_absolute)) {
	  snprintf(report, STRLEN,
      "space usage %ld blocks matches resource limit [space usage%s%ld blocks]",
		   t->devinfo->f_blocks - t->devinfo->f_blocksfreetotal,
		   operatorshortnames[td->operator],
		   td->limit_absolute);
	  return FALSE;
        }
      }
      DEBUG("'%s' space usage check passed [current space usage=%.1f%%]\n",
	    t->name,
	    (float) 100*(t->devinfo->f_blocks - t->devinfo->f_blocksfreetotal) /
	    t->devinfo->f_blocks);
      return TRUE;
      
  default:
      log("'%s' error -- unknown resource type: [%d]\n", t->name, td->resource);
      return FALSE;
  }
  
}


/**
 * Comparison of values. Returns TRUE if comparison matches, otherwise
 * FALSE.
 */
static int compare_value(int operator, int left, int right) {

  switch(operator) {
  case OPERATOR_GREATER:
      if(left > right)
	  return TRUE;
      break;
  case OPERATOR_LESS:
      if(left < right)
	  return TRUE;
      break;
  case OPERATOR_EQUAL:
      if(left == right)
	  return TRUE;
      break;
  case OPERATOR_NOTEQUAL:
      if(left != right)
	  return TRUE;
      break;
  default:
      log("Unknown comparison operator\n");
      return FALSE;
  }

  return FALSE;
    
}

