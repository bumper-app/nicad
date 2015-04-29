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

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
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

#include "monitor.h"


/* Private prototypes */
static void set_sandbox(void);
static void set_environment(void);

/**
 *  Setup this program for safer exec, and set required runtime
 *  "environment" variables.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: env.c,v 1.24 2004/03/12 16:27:03 martinp Exp $
 * 
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Initialize the program environment
 */
void init_env() {
  
  /* Setup for safe(r) exec */
  set_sandbox();

  /* Setup program environment */
  set_environment();

  /* Free program environment on exit */
  atexit(destroy_env);
  
}


/**
 * Destroy the program environment
 */
void destroy_env() {
  
  FREE(Run.Env.cwd);
  FREE(Run.Env.home);
  FREE(Run.Env.user);
  FREE(Run.localhostname);
  FREE(Run.controlfile);
  
}


/* ----------------------------------------------------------------- Private */


/**
 *  DESCRIPTION
 *    This code was originally posted by Wietse Venema, years ago, in
 *    a discussion on news on how to create safe suid wrappers. For
 *    those interested in NNTP archeology, here's the post:
 *    
 *  Article 5648 of comp.security.unix:
 *  From: wietse@wzv.win.tue.nl (Wietse Venema)
 *  Newsgroups: comp.security.unix
 *  Subject: Re: [8lgm]-Advisory-7.UNIX.passwd.11-May-1994
 *  Date: 18 May 1994 07:52:05 +0200
 *  Organization: Eindhoven University of Technology, The Netherlands
 *  Lines: 68
 *  
 *  milton@picard.med.miami.edu (H. Milton Johnson) writes:
 *  >OK, I admit it, I'm a totally incompetent sysadmin because I am not
 *  >sure I could write a bullet-proof setuid wrapper.  However, if one of
 *  >the competent sysadmins subscribing to this group could post or point
 *  >the way to an example of a bullet- proof setuid wrapper, I'm sure that
 *  >I could use it as a template to address this/future/other problems.
 *  
 *  Ok, here is a first stab. Perhaps we can make this into a combined
 *  effort and get rid of the problem once and for all.
 *  
 *           Wietse
 *
 *  [code - see the function below, only marginally changed to suit monit]    
 *
 *  @author Wietse Venema <wietse@wzv.win.tue.nl>
 *
 */
static void set_sandbox(void) {

  int    i;
  struct stat st;
  extern char **environ;
  char   *path = "PATH=/bin:/usr/bin:/sbin:/usr/sbin";

  /*
   * Purge the environment. Then make sure PATH is set; some shells default
   * to a path with '.' first. You may have to putenv() other stuff, too,
   * but be careful with importing too much.
   */
  environ[0]= 0;
  
  if(putenv(path)) {
    
    log("%s: cannot set the PATH variable -- %s\n", prog, STRERROR);
    exit(1);
    
  }

  /*
   * Require that file descriptors 0,1,2 are open. Mysterious things
   * can happen if that is not the case.
   */
  for(i= 0; i < 3; i++) {
    
    if(fstat(i, &st) == -1 && open("/dev/null", O_RDWR) != i) {
      
      log("Cannot open /dev/null -- %s\n", STRERROR);
      exit(1);
      
    }
    
  }

  fd_close();

}


/**
 * Get and set required runtime "environment" variables.
 */
static void set_environment(void) {

  char cwd[STRLEN];
  struct passwd *pw;
  char localhost[STRLEN];
  
  /* Get password struct */
  if ( ! (pw= getpwuid(geteuid())) ) {
    
    log("%s: You don't exist. Go away.\n", prog);
    exit(1);
    
  }
  
  /* Get CWD */
  if ( ! (getcwd(cwd, sizeof(cwd))) ) {
    
    log("%s: Cannot read current directory -- %s\n", prog, STRERROR);
    exit(1);
    
  }
  
  /*
   * Get the localhost name
   */
  if(gethostname(localhost, sizeof(localhost)) < 0) {
    snprintf(localhost, STRLEN, "%s", LOCALHOST);
  }

  /*
   * Save and clear the file creation mask
   */
  Run.umask= umask(0);
  
  /*
   * Initialize the runtime environment object
   */
  Run.Env.cwd=  xstrdup(cwd);
  Run.Env.home= xstrdup(pw->pw_dir);
  Run.Env.user= xstrdup(pw->pw_name);
  Run.localhostname= xstrdup(localhost);
  
}

