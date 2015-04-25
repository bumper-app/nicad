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

#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_PROCFS_H
#include <procfs.h>
#endif

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"

/**
 *  System dependent resource gathering code for Solaris.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: sysdep_SOLARIS.c,v 1.8 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */


int init_process_info_sysdep(void) {

  num_cpus= sysconf( _SC_NPROCESSORS_ONLN);
  mem_kbyte_max=(long)(((double)sysconf(_SC_PHYS_PAGES)) *
			((double)sysconf(_SC_PAGESIZE)) / 1024.0); 

  return (getuid()==0);

}

double timestruc_to_tseconds(timestruc_t t) {
  return  t.tv_sec * 10 + t.tv_nsec / 100000000.0;
}

int get_process_info_sysdep(ProcInfo_T p) {

  char buf[4096];
  psinfo_t  * psinfo=  (psinfo_t *)&buf;
  pstatus_t * pstatus= (pstatus_t *)&buf;

  if (!read_proc_file(buf,4096, "psinfo", p->pid)) {

    return FALSE;

  }

  /* If we don't have any light-weight processes (LWP) then we
     are definitely a zombie */

  if ( psinfo->pr_nlwp == 0 ) {

    p->status_flag = PROCESS_ZOMBIE;

  }

  p->ppid = pstatus->pr_ppid;

  if ( p->status_flag != PROCESS_ZOMBIE ) {
    /* We can't access /proc/$pid/status of a zombie */
    /* and does it anyway matter? */

    p->mem_percent = psinfo->pr_pctmem * 1000 / 0x8000;
    p->mem_kbyte = psinfo->pr_rssize;

    if (!read_proc_file(buf,4096, "status", p->pid)) {

      return FALSE;

    }

    p->cputime_prev= p->cputime;
    p->cputime= ( timestruc_to_tseconds(pstatus->pr_utime) +
		  timestruc_to_tseconds(pstatus->pr_stime) );

    if( include_children ) {

      p->cputime+= ( timestruc_to_tseconds(pstatus->pr_cutime) +
		     timestruc_to_tseconds(pstatus->pr_cstime) );

    }
    /* first run ? */

    if ( p->time_prev == 0.0 ) {

      p->cputime_prev= p->cputime;

    }

  } else {

    p->cputime_prev= p->cputime = 0;
    p->mem_kbyte= 0;
    p->mem_percent= 0.0;

  }

  return TRUE;

}


/**
 * Read all processes of the proc files system to initilize
 * the process tree (sysdep version... but should work for
 * all procfs based unices)
 * @param reference  reference of ProcessTree
 * @return treesize>=0 if succeeded otherwise <0.
 */
int initprocesstree_sysdep(ProcessTree_T ** reference) {

  int      pid;
  int      i;
  int      treesize;

  ProcessTree_T *  pt;

  glob_t   globbuf;

  ASSERT(reference);

  /* Find all processes in the /proc directory */

  if (glob("/proc/[0-9]*",  NULL , NULL, &globbuf) != 0 ) {

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
