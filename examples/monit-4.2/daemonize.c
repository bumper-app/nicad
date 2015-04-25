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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "monitor.h"


/**
 *  Transform this program into a daemon and provide methods for
 *  managing the daemon.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: daemonize.c,v 1.17 2004/01/29 17:52:11 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Transform a program into a daemon. Inspired by code from Stephen
 * A. Rago's book, Unix System V Network Programming.
 */
void  daemonize() {

  pid_t pid;
  
  /*
   * Clear file creation mask
   */
  umask(0);

  /*
   * Become a session leader to lose our controlling terminal
   */
  if((pid= fork ()) < 0) {
    
    log("Cannot fork of a new process\n");  
    exit (1);
    
  }  
  else if(pid != 0) {
    
    _exit(0);
    
  }
  
  setsid();

  if((pid= fork ()) < 0) {
    
    log("Cannot fork of a new process\n");  
    exit (1);
    
  }  
  else if(pid != 0) {
    
    _exit(0);
    
  }

  
  /*
   * Change current directory to the root so that other file systems
   * can be unmounted while we're running
   */
  if(chdir("/") < 0) {
    
    log("Cannot chdir to / -- %s\n", STRERROR);
    exit(1);
    
  }

  /*
   * Attach standard descriptors to /dev/null. Other descriptors
   * should be closed in env.c
   */
  redirect_stdfd();

} 


/**
 * Send signal to a daemon process
 * @param sig Signal to send daemon to
 * @return TRUE if signal was send, otherwise FALSE
 */
int kill_daemon(int sig) {
  
  pid_t pid;

  if ( (pid= exist_daemon()) > 0 ) {
    
    if ( kill(pid, sig) < 0 ) {
      
      log("%s: Cannot send signal to daemon process -- %s\n",prog, STRERROR);
      return FALSE;
      
    }
    
  } else {
    
    log("%s: No daemon process found\n", prog);
    return TRUE;
    
  }
  
  if(sig == SIGTERM) {
	  
    fprintf(stdout, "%s daemon with pid [%d] killed\n", prog, (int)pid);
    fflush(stdout);

  }
  
  return TRUE;
  
}


/**
 * @return TRUE (i.e. the daemons pid) if a daemon process is running,
 * otherwise FALSE
 */
int exist_daemon() {

  pid_t pid;

  errno= 0;

  if( (pid= get_pid(Run.pidfile)) )
    if( (getpgid(pid)) > -1 || (errno == EPERM) )
      return( (int)pid );
  
  return(FALSE);

}
    
