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

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#define _RUSAGE_EXTENDED

#ifdef HAVE_SYS_PSTAT_H
#include <sys/pstat.h>
#endif

#ifdef HAVE_NLIST_H
#include <nlist.h>
#endif

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"

#define NPROCS 10000
static struct pst_status psall[NPROCS];

static int page_size;

/**
 *  System dependent resource gathering code for HP/UX.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *  @author Joe Bryant <JBryant@RiteAid.com>
 *  @version \$Id: sysdep_HPUX.c,v 1.8 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */

/*
 * Helpful guide for implematation:
 * "SunOS to HP-UX 9.05 Porting Guide" at
 *    http://www.interex.org/tech/9000/Tech/sun_hpux_port/portguide.html
 */

int init_process_info_sysdep(void) {
  struct pst_dynamic psd;
  struct pst_static pst;

  if (pstat_getdynamic(&psd,sizeof(psd),(size_t)1,0)!=-1) {

    num_cpus=psd.psd_proc_cnt;

  } else {

    return FALSE;

  }

  if (pstat_getstatic(&pst, sizeof(pst), (size_t) 1, 0) != -1) {

    mem_kbyte_max=(long)(((double) pst.physical_memory) *
			((double) pst.page_size) / 1024.0); 
    page_size=pst.page_size;

  } else {

    return FALSE;

  }

  return TRUE;
}


int get_process_info_sysdep(ProcInfo_T p) {

  struct pst_status ps;

  if (pstat_getproc(&ps,sizeof(ps),(size_t)1,p->pid)==-1) {

    return FALSE;

  }

  /* jiffies -> seconds = 1 / HZ
     HZ is defined in "asm/param.h"  and it is usually 1/100s but on
     alpha system it is 1/1024s */

  p->cputime_prev = p->cputime;
  p->cputime =  ( ps.pst_utime + ps.pst_stime ) * 10 / HZ;
  p->mem_kbyte   = (long)(((double)ps.pst_rssize)/1024.0 * page_size);
  p->mem_percent = (((double)p->mem_kbyte) * 1000.0 / ((double)mem_kbyte_max));

  if ( include_children ) {

    p->cputime += ( ps.pst_child_utime.pst_sec + ps.pst_child_stime.pst_sec ) 
      * 10 / HZ;

  }

  /* first run ? */

  if ( p->time_prev == 0.0 ) {

      p->cputime_prev = p->cputime;

  }

  /* State is Zombie -> then we are a Zombie ... clear or? (-: */

  if ( ps.pst_stat || PS_ZOMBIE ) {

    p->status_flag |= PROCESS_ZOMBIE;

  }

  return TRUE;

}

/**
 * This routine returns 'na' double precision floats containing
 * the load averages in 'a'; at most 3 values will be returned.
 * @param loadv destination of the load averages
 * @param nelem number of averages
 * @return: 0 if successful, -1 if failed (and all load averages are 0).
 */
int getloadavg_sysdep (double *a, int na) {
  
  struct pst_dynamic psd;
	
  if (pstat_getdynamic(&psd,sizeof(psd),(size_t)1,0)!=-1) {
    
    switch (na) {
    case 3:

      a[2]=psd.psd_avg_15_min;

    case 2:

      a[1]=psd.psd_avg_5_min;

    case 1:

      a[0]=psd.psd_avg_1_min;

    }

  } else {

    return FALSE;

  }

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

  int treesize;
  int i;

  ProcessTree_T *  pt;

  ASSERT(reference);

  /* Gather process data */

  if ((treesize=pstat_getproc(psall,(size_t) NPROCS* sizeof(pst_status),(size_t) NPROCS, 0))==-1) {

    return 0;

  }

  /* Allocate the tree */

  pt = xcalloc(sizeof(ProcessTree_T), treesize);

  /* Inspect data */

  for (i = 0; i < tree_size; pp++, i++) {

    pt[i]->pid = psall[i]->pst_pid;
    pt[i]->ppid = psall[i]->pst_ppid;
    p->mem_kbyte   = (long)(((double)psall[i]->pst_rssize)/1024.0 * page_size);
    
  }

  /* Return results */

  * reference = pt;

  return treesize;

}
