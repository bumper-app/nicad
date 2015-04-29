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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "monitor.h"

/**
 *  Utilities used for managing files used by monit.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: files.c,v 1.49 2004/02/17 18:52:08 hauk Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Initialize the programs file variables
 */
void init_files() {

  char pidfile[STRLEN];
  char statefile[STRLEN];

  /* Check if the pidfile was already set during configfile parsing */
  if(Run.pidfile == NULL) {

    /* Set the location of this programs pidfile */
    if(! getuid()) {

      snprintf(pidfile, STRLEN, "%s/%s", MYPIDDIR, MYPIDFILE);

    } else {

      snprintf(pidfile, STRLEN, "%s/.%s", Run.Env.home, MYPIDFILE);

    }

    Run.pidfile= xstrdup(pidfile);

  }

  /* Set the location of the programs state file */
  if(Run.statefile == NULL) {

    snprintf(statefile, STRLEN, "%s/.%s", Run.Env.home, MYSTATEFILE);
    
    Run.statefile= xstrdup(statefile);

  }
  
}


/**
 * Remove temporary files
 */
void finalize_files() {
  
  unlink(Run.pidfile);
  unlink(Run.statefile);

}


/**
 * Get a object's last modified timestamp.
 * @param object A object to stat
 * @param type Requested object's type
 * @return Max of either st_mtime or st_ctime or
 * FALSE if not found or different type of object
 */
time_t get_timestamp(char *object, mode_t type) {
  
  struct stat buf;

  ASSERT(object);

  if(! stat(object, &buf)) {
    
    if(((type == S_IFREG) && S_ISREG(buf.st_mode)) ||
       ((type == S_IFDIR) && S_ISDIR(buf.st_mode)) ||
       ((type == (S_IFREG|S_IFDIR)) && (S_ISREG(buf.st_mode) ||
					S_ISDIR(buf.st_mode)))
       ) {
      
       return MAX(buf.st_mtime, buf.st_ctime);

     } else {

       log("%s: Invalid object type - %s\n", prog, object);

     }

  }

  return FALSE;
  
}


/**
 * Search the system for the monit control file. Try first ~/.monitrc,
 * if that fails try /etc/monitrc and finally ./monitrc.  Exit the
 * application if the control file is not found.
 * @return The location * of monits control file (monitrc)
 */
char *find_rcfile() {

  char *rcfile= xmalloc(STRLEN);
  
  snprintf(rcfile, STRLEN, "%s/.%s", Run.Env.home, MONITRC);
  
  if(exist_file(rcfile)) {
    
    return (rcfile);
    
  }
  
  memset(rcfile, 0, STRLEN);
  snprintf(rcfile, STRLEN, "/etc/%s", MONITRC);
  
  if(exist_file(rcfile)) {
    
    return (rcfile);
    
  }
  
  if(exist_file(MONITRC)) {
    
    memset(rcfile, 0, STRLEN);
    snprintf(rcfile, STRLEN, "%s/%s", Run.Env.cwd, MONITRC);
    
    return (rcfile);
    
  }
  
  log("%s: Cannot find the control file at ~/.%s, /etc/%s or at ./%s \n",
      prog, MONITRC, MONITRC, MONITRC);
  
  exit(1);
  
}


/**
 * Create a program's pidfile - Such a file is created when in daemon
 * mode. The file is created with mask = MYPIDMASK (usually 644).  
 * @param pidfile The name of the pidfile to create
 * @return TRUE if the file was created, otherwise FALSE. 
 */
int create_pidfile(char *pidfile) {
  
  FILE *F= NULL;

  ASSERT(pidfile);
  
  umask(MYPIDMASK);

  unlink(pidfile);
 
  if ((F= fopen(pidfile,"w")) == (FILE *)NULL) {
    
    log("%s: Error opening pidfile '%s' for writing -- %s\n",
	prog, pidfile, STRERROR);
    
    return(FALSE);
    
  }
  
  fprintf(F, "%d\n", (int)getpid());
  fclose(F);

  return TRUE;
  
}


/**
 * Secure check the monitrc file. The run control file must have the
 * same uid as the REAL uid of this process, it must have permissions
 * no greater than 700 and it must not be a symbolic link.  We check
 * these conditions here.
 * @param rcfile The monitrc file
 * @return TRUE if the test passed otherwise FALSE
 */
int check_rcfile(char *rcfile) {

  ASSERT(rcfile);
  
  return check_file_stat(rcfile, "control file", S_IRUSR | S_IWUSR | S_IXUSR);
  
}


/**
 * Check if the file is a regular file
 * @param file A path to the file to check
 * @return TRUE if file exist and is a regular file, otherwise FALSE
 */
int isreg_file(char *file) {
  
  struct stat buf;
  
  ASSERT(file);

  return (stat(file, &buf) == 0 && S_ISREG(buf.st_mode));
  
}


/**
 * Check if the file exist on the system
 * @file A path to the file to check
 * @return TRUE if file exist otherwise FALSE
 */
int exist_file(char *file) {
  
  struct stat buf;
  
  ASSERT(file);

  return (stat(file, &buf) == 0);
  
}


/**
 * Security check for files. The files must have the same uid as the
 * REAL uid of this process, it must have permissions no greater than
 * "maxpermission" and it must not be a symbolic link.  We check these
 * conditions here.
 * @param filename The filename of the checked file
 * @param description The description of the checked file
 * @param permmask The permission mask for the file
 * @return TRUE if the test passed otherwise FALSE
 */
int check_file_stat(char *filename, char *description, int permmask) {

  struct stat buf;
  errno= 0;

  ASSERT(filename);
  ASSERT(description);

  if(lstat(filename, &buf) < 0) {
    
    log("%s: Cannot stat the %s '%s' -- %s\n",
	prog, description, filename, STRERROR);

    return FALSE;
    
  }
    
  if(S_ISLNK(buf.st_mode)) {
    
    log("%s: The %s '%s' must not be a symbolic link.\n",
	prog, description, filename);
    
    return(FALSE);
    
  }

  if(!S_ISREG(buf.st_mode)) {
    
    log("%s: The %s '%s' is not a regular file.\n", 
	prog, description,  filename);
    
    return FALSE;

  }

  if(buf.st_uid != geteuid())  {
    
    log("%s: The %s '%s' must be owned by you.\n", 
	prog, description, filename);
	  
    return FALSE;
    
  }

  if((buf.st_mode & 0777 ) & ~permmask) {

    /* 
       Explanation: 

           buf.st_mode & 0777 ->  We just want to check the
                                  permissions not the file type... 
                                  we did it already!
           () & ~permmask ->      We check if there are any other
                                  permissions set than in permmask 
    */

    log("%s: The %s '%s' must have permissions no more "
	"than -%c%c%c%c%c%c%c%c%c (0%o); "
	"right now permissions are -%c%c%c%c%c%c%c%c%c (0%o).\n", 
	prog, description, filename, 
	permmask&S_IRUSR?'r':'-',
	permmask&S_IWUSR?'w':'-',
	permmask&S_IXUSR?'x':'-',
	permmask&S_IRGRP?'r':'-',
	permmask&S_IWGRP?'w':'-',
	permmask&S_IXGRP?'x':'-',
	permmask&S_IROTH?'r':'-',
	permmask&S_IWOTH?'w':'-',
	permmask&S_IXOTH?'x':'-',
	permmask&0777,
	buf.st_mode&S_IRUSR?'r':'-',
	buf.st_mode&S_IWUSR?'w':'-',
	buf.st_mode&S_IXUSR?'x':'-',
	buf.st_mode&S_IRGRP?'r':'-',
	buf.st_mode&S_IWGRP?'w':'-',
	buf.st_mode&S_IXGRP?'x':'-',
	buf.st_mode&S_IROTH?'r':'-',
	buf.st_mode&S_IWOTH?'w':'-',
	buf.st_mode&S_IXOTH?'x':'-',
	buf.st_mode& 0777);
    
    return FALSE;
	
  }

  return TRUE;

}

