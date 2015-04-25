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

#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
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

#ifdef HAVE_SYS_TIME
#include <sys/time.h>
#endif
#else
#include <time.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <stdio.h>

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"

/**
 *  General purpose /proc methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: process.c,v 1.1 2004/02/29 22:24:44 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */

char actionnames[][STRLEN]=   {"ignore", "alert", "restart", "stop", "exec",
	                       "unmonitor"};
char modenames[][STRLEN]=     {"active", "passive", "manual"};
char operatornames[][STRLEN]= {"greater than", "less than", "equal to",
                               "not equal to"};
char operatorshortnames[][3]= {">", "<", "=", "!="};
char statusnames[][STRLEN]=   {"not monitored", "monitored" };
char servicenames[][STRLEN]=  {"Device Name", "Directory Name", "File Name",
	                       "Process Name", "Remote Host" };
char icmpnames[][STRLEN]=     {"Echo Reply", "", "", "Destination Unreachable",
                               "Source Quench", "Redirect", "", "", "Echo Request",
			       "", "", "Time Exceeded", "Parameter Problem",
			       "Timestamp Request", "Timestamp Reply",
			       "Information Request", "Information Reply",
			       "Address Mask Request", "Address Mask Reply" };

int   include_children= TRUE;
int   num_cpus=1;
long  mem_kbyte_max=0;


/**
 * Initialize the proc information code
 * @return TRUE if succeeded otherwise FALSE.
 */
int init_process_info(void) {

  return (init_process_info_sysdep());

}


/**
 * Get the proc infomation (CPU percentage, MEM in MByte and percent,
 * status), enduser version. 
 * @param p A Service object
 * @param pid The process id
 * @return TRUE if succeeded otherwise FALSE.
 */
int update_process_data(Service_T s, ProcessTree_T *pt, int treesize, pid_t pid) {

  ProcInfo_T pi;
  ProcessTree_T *leaf;

  ASSERT(s);
  
  pi= s->procinfo;
  pi->pid=pid;

  if(! get_process_info(pi))
    return FALSE;

  if ((leaf = findprocess(pid, pt, treesize)) != NULL ) {
 
    pi->children=leaf->children_sum;
    pi->total_mem_kbyte=leaf->mem_kbyte_sum;

    if ( mem_kbyte_max == 0 ) {
      pi->total_mem_percent=0;
    } else {
      pi->total_mem_percent=(int) ((double) leaf->mem_kbyte_sum * 1000.0 / mem_kbyte_max);
    }

  } else {

    pi->children=0;
    pi->total_mem_kbyte=pi->mem_kbyte;
    pi->total_mem_percent=pi->mem_percent;

  }

  return TRUE;
}


/**
 * Updates the loadavg list
 * @return TRUE if successful, otherwise FALSE
 */
int update_loadavg(void) {

  if( -1 == getloadavg_sysdep( Run.loadavg, 3) )
    return FALSE;

  return TRUE;
}


/**
 * Initilize the process tree 
 * @return treesize>=0 if succeeded otherwise <0.
 */
int initprocesstree(void) {
  
  int i;

  if((ptreesize = initprocesstree_sysdep(&ptree)) <= 0)
    return 0;

  for( i = 0; i < ptreesize; i ++ ) {

    if( ptree[i].ppid == 0 )
      continue;

    if(NULL == (ptree[i].parent= (ProcessTree_T *)findprocess(ptree[i].ppid, ptree, ptreesize))) {
      /* inconsitency found, most probably a race condition
	 we might lack accuracy but we remain stable! */
      ptree[i].pid=0;
      continue;
    }
    
    if(! connectchild((ProcessTree_T *) ptree[i].parent, &ptree[i])) {
      /* connection to parent process has failed, this is
	 usually caused in the part above */
      ptree[i].pid=0;
      continue;
    }

  }

  fillprocesstree((ProcessTree_T *) findprocess(1, ptree, ptreesize));

  return ptreesize;

}


/**
 * Search a leaf in the processtree
 * @param pid  pid of the process
 * @param pt  processtree
 * @param treesize  size of the processtree
 * @return pointer of the process if succeeded otherwise NULL.
 */
ProcessTree_T * findprocess(int pid, ProcessTree_T * pt, int size) {

  int i;

  ASSERT(pt);

  if(( pid == 0  ) || ( size <= 0 ))
    return NULL;

  for( i = 0; i < size; i ++ )
    if( pid == pt[i].pid )
      return &pt[i];

  return NULL;

}

/**
 * Delete the process tree 
 */
void delprocesstree(void) {

  int i;

  ASSERT(ptree);

  if( ptreesize <= 0 )
    return;

  for( i = 0; i < ptreesize; i ++ )
    if( ptree[i].children!=NULL )
      FREE(ptree[i].children);

  FREE(ptree);

  return;

}
