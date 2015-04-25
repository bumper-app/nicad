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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "monitor.h"


/**
 *  Implementation of a logger that appends log messages to a file
 *  with a preceding timestamp. Methods support both syslog or own
 *  logfile.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: log.c,v 1.20 2004/02/13 15:29:11 hauk Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------- Definitions */


static FILE *LOG= NULL;
static char time_str[STRLEN];
static pthread_mutex_t log_mutex= PTHREAD_MUTEX_INITIALIZER;


/* -------------------------------------------------------------- Prototypes */


static int  open_log();
static char *timestr(void);


/* ------------------------------------------------------------------ Public */


/**
 * Initialize the log system and 'log' function
 * @return TRUE if the log system was successfully initialized
 */
int log_init() {

  if(!Run.dolog) {
    return TRUE;
  }

  if(!open_log()) {
    return FALSE;
  }

  /* Register log_close to be
     called at program termination */
  atexit(log_close);

  return TRUE;
  
}


/**
 * Log a message to monits logfile or syslog. 
 * @param s A formated (printf-style) string to log
 */
void log_log(const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;

  ASSERT(s);
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);

  if(! Run.dolog) goto nolog;

  if(Run.use_syslog) {
    LOCK(log_mutex)
      syslog(LOG_ERR, "%s", msg);
    END_LOCK;
    
  } else if(LOG) {
    LOCK(log_mutex)
      fprintf(LOG,"[%s] %s", timestr(), msg);
    END_LOCK;
    
  }

  nolog:
  LOCK(log_mutex)
    fprintf(stderr, "%s", msg);
    fflush(stderr);
  END_LOCK;
  
  FREE(msg);
  
}


/**
 * Close the log file or syslog
 */
void log_close() {
  
  if(Run.use_syslog) {
    closelog(); 
  }
  
  if(LOG  && (0 != fclose(LOG))) {
    log("%s: Error closing the log file -- %s\n",
	prog, STRERROR);
  }
  pthread_mutex_destroy(&log_mutex);
  LOG= NULL;
  
}


/* ----------------------------------------------------------------- Private */


/**
 * Open a log file or syslog
 */
static int open_log() {
 
  if(Run.use_syslog) {
    openlog(prog, LOG_PID, Run.facility); 
  } else {
    umask(LOGMASK);
    if((LOG= fopen(Run.logfile,"a+")) == (FILE *)NULL) {
      log("%s: Error opening the log file '%s' for writing -- %s\n",
	  prog, Run.logfile, STRERROR);
      return(FALSE);
    }
    /* Set logger in unbuffered mode */
    setvbuf(LOG, NULL, _IONBF, 0);
  }

  return TRUE;
  
}


/**
 * Returns the current time as a formated string, see the TIMEFORMAT
 * macro in the header file
 */
static char *timestr() {

  time_t now;
  struct tm tm;
  
  time(&now);
  if(!strftime(time_str, STRLEN, TIMEFORMAT, localtime_r(&now, &tm))) {
    memset(time_str, 0, STRLEN);
  }
  
  return time_str;
  
}


  

