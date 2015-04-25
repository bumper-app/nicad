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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#else
#include <time.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ASM_PARAM_H
#include <asm/param.h>
#endif

#ifdef HAVE_ASM_PAGE_H
#include <asm/page.h>
#endif

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"

/**
 *  System dependent resource gathering code for Linux.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Arkadiusz Miskiewicz <arekm@pld-linux.org>
 *
 *  @version \$Id: sysdep_LINUX.c,v 1.12 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */

#define PAGE_TO_KBYTE_SHIFT PAGE_SHIFT-10
#define MEMTOTAL            "MemTotal:"

int init_process_info_sysdep(void) {

  int  memfd;
  char buf[1024], *ptr;

  if( (memfd = open("/proc/meminfo", O_RDONLY)) == -1 ) 
    return FALSE;

  if( read(memfd, buf, sizeof(buf)) < 0 ) {
    close(memfd);
    return FALSE;
  }
  close(memfd);

  if(! (ptr= strstr(buf, MEMTOTAL)) )
    return FALSE;

  if( sscanf(ptr+strlen(MEMTOTAL), "%ld", &mem_kbyte_max) != 1 )
    return FALSE;

  num_cpus= sysconf(_SC_NPROCESSORS_CONF);

  return TRUE;

}


int get_process_info_sysdep(ProcInfo_T p) {

  char buf[4096];
  char* tmp;
  char stat_item_state;
  unsigned long stat_item_utime;
  unsigned long stat_item_stime;
  long stat_item_cutime;
  long stat_item_cstime;
  long stat_item_rss;
  int statm_item_share;
  int statm_item_trs;
  int statm_item_drs;
  int stat_ppid;

  if (!read_proc_file(buf,4096, "stat", p->pid)) {

    return FALSE;

  }

  /* Move along the buffer to get past the process name */

  tmp = strrchr(buf, ')') + 2;

  /* This implementation is done by using fs/procfs/array.c as a basis
     it is also worth looking into the source of the procps utils */

  sscanf(tmp,"%c %d %*d %*d %*d %*d %*u %*u"
	 "%*u %*u %*u %lu %lu %ld %ld %*d %*d %*d "
	 "%*d %*u %*u %ld %*u %*u %*u %*u %*u "
	 "%*u %*u %*u %*u %*u %*u %*u %*u %*d %*d\n",
	 &stat_item_state, &stat_ppid,
	 &stat_item_utime, &stat_item_stime,
	 &stat_item_cutime, &stat_item_cstime, &stat_item_rss);

  /* abs to please the compiler... we dont want to shift negatively.
     why doesn't C understand this??? */

  p->ppid= stat_ppid;
  
  /* jiffies -> seconds = 1 / HZ
     HZ is defined in "asm/param.h"  and it is usually 1/100s but on
     alpha system it is 1/1024s */

  p->cputime_prev = p->cputime;
  p->cputime =  ( stat_item_utime + stat_item_stime ) * 10 / HZ;

  if ( include_children ) {

    p->cputime += ( stat_item_cutime + stat_item_cstime ) * 10 / HZ;

  }

  /* first run ? */

  if ( p->time_prev == 0.0 ) {

      p->cputime_prev = p->cputime;

  }

  /* State is Zombie -> then we are a Zombie ... clear or? (-: */

  if ( stat_item_state == 'Z' ) {

    p->status_flag |= PROCESS_ZOMBIE;

  }


  if (!read_proc_file(buf,4096, "statm", p->pid)) {

    return FALSE;

  }

  /* This implementation is done by using fs/procfs/array.c as a basis
     it is also worth looking into the source of the procps utils.

     Beware then proc man page has a wrong sorting of the entries!  */
  
  sscanf(buf,"%*d %*d %d %d %*d %d %*d\n",
         &statm_item_share,&statm_item_trs, &statm_item_drs);

  if ( PAGE_TO_KBYTE_SHIFT < 0 ) {

    p->mem_kbyte= ( statm_item_trs >> abs(PAGE_TO_KBYTE_SHIFT) ) +
        ( statm_item_drs >> abs(PAGE_TO_KBYTE_SHIFT) ) -
        ( statm_item_share >> abs(PAGE_TO_KBYTE_SHIFT) );

  } else {
    
    p->mem_kbyte= ( statm_item_trs << abs(PAGE_TO_KBYTE_SHIFT) ) +
        ( statm_item_drs << abs(PAGE_TO_KBYTE_SHIFT) ) -
        ( statm_item_share << abs(PAGE_TO_KBYTE_SHIFT) );

  }

  p->mem_percent = (int)( (double)p->mem_kbyte * 1000.0 / mem_kbyte_max);
  
  return TRUE;

}


/**
 * Read all processes of the proc files system to initilize
 * the process tree (sysdep version... but should work for
 * all procfs based unices) 
 * @param reference  reference of ProcessTree
 * @return treesize>0 if succeeded otherwise =0.
 */
int initprocesstree_sysdep(ProcessTree_T ** reference) {

  int      pid;
  int      i;
  int      treesize;

  ProcessTree_T *  pt;

  glob_t   globbuf;

  ASSERT(reference);

  /* Find all processes in the /proc directory */

  if ( glob("/proc/[0-9]*",  GLOB_ONLYDIR, NULL, &globbuf) != 0 ) {

    return 0;

  } 

  treesize = globbuf.gl_pathc;

  /* Allocate the tree */

  pt = xcalloc(sizeof(ProcessTree_T), treesize);

  /* Insert data from /proc directory */

  for ( i = 0; i < treesize; i ++ ) {

    pid=atoi(globbuf.gl_pathv[i]+strlen("/proc/"));
    
    if (! getdatafromproc(pid, &pt[i] )) {

      continue;

    }

  }
  
  * reference = pt;

  /* Free globbing buffer */

  globfree(&globbuf);

  return treesize;

}

/**
 * This routine returns 'nelem' double precision floats containing
 * the load averages in 'loadv'; at most 3 values will be returned.
 * @param loadv destination of the load averages
 * @param nelem number of averages
 * @return: 0 if successful, -1 if failed (and all load averages are 0).
 */
int getloadavg_sysdep (double *loadv, int nelem) {

  return getloadavg(loadv, nelem);
  
}
