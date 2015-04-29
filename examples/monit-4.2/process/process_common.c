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

/**
 *  System independent /proc & kvm methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: process_common.c,v 1.1 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_LOADAVG_H
#include <sys/loadavg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"


/**
 * Reads an process dependent entry or the proc files system
 * @param buf buffer to write to
 * @param buf_size size of buffer "buf"
 * @param name name of proc service
 * @param pid pid number of the process
 * @return TRUE if succeeded otherwise FALSE.
 */
int read_proc_file(char *buf, int buf_size, char * name, int pid) {

  int fd;
  char filename[STRLEN];
  int bytes;

  ASSERT(buf);
  ASSERT(name);

  snprintf(filename, STRLEN, "/proc/%d/%s", pid, name);

  if ( (fd = open(filename, O_RDONLY)) < 0 ) {

    return FALSE;

  }

  if ( (bytes = read(fd, buf, buf_size-1)) < 0 ) {

    close(fd);

    return FALSE;

  }
       
  /* In case it is a string we have to 0 terminate it our self */
  buf[bytes]='\0';

  close(fd);

  return TRUE;
  
}

/**
 * Get the actual time as a floating point number
 * @return time in seconds
 */
double get_float_time(void) {    

  struct timeval t;

  gettimeofday(&t, NULL);
  return (double) t.tv_sec * 10 + (double) t.tv_usec / 100000.0;

}

/**
 * Get the proc infomation (CPU percentage, MEM in MByte and percent,
 * status)
 * @param p proc information structure to work with.
 * @return TRUE if succeeded otherwise FALSE.
 */
int get_process_info(ProcInfo_T p) {
  double temp_time;

  /* get the actual time */

  temp_time = get_float_time();
  
  /* get the system dependant proc data */

  if ( ! get_process_info_sysdep(p) ) {

    return FALSE;

  }

  /* check if it is the first run */
  
  if ( p->time_prev == 0.0 ) {
    
    p->cpu_percent= 0;
    
  } else {
    
    /* Calculate CPU percentage */

    p->cpu_percent=  (int) (( p->cputime - p->cputime_prev ) * 1000) / 
      ( temp_time - p->time_prev ) / num_cpus;
	
  }

  p->time_prev = temp_time;
    

  return TRUE;

}



/**
 * Write process data in processtree entry
 * @param pid pid of the process
 * @param entry process tree
 * @return TRUE if succeeded otherwise FALSE.
 */
int getdatafromproc(int pid, ProcessTree_T *entry) {

  ProcInfo_T pi=NEW(pi);

  pi->pid=pid;

  if (! get_process_info_sysdep(pi)) {

    return FALSE;

  }

  entry->pid=pid;
  entry->mem_kbyte=pi->mem_kbyte;
  entry->ppid=pi->ppid;

  FREE(pi);
  return TRUE;

}


/**
 * Connects child and parent in a process treee
 * @param parent pointer to parents process tree entry
 * @param child pointer to childs process tree entry
 * @return TRUE if succeeded otherwise FALSE.
 */
int connectchild(ProcessTree_T * parent, ProcessTree_T * child) {

  ProcessTree_T ** tmp;

  ASSERT(child);
  ASSERT(parent);

  if ( parent->pid == 0 || child->pid == 0 ) {

    return FALSE;
    
  }

  parent->children_num++;

  tmp = xcalloc(sizeof(ProcessTree_T *), parent->children_num);

  if ( parent->children != NULL ) {

    memcpy(tmp, parent->children,
	   sizeof(ProcessTree_T *) * (parent->children_num - 1));
    FREE(parent->children);

  }

  parent->children = tmp;
  parent->children[parent->children_num - 1] = child;

  return TRUE;

}


/**
 * Fill data in the process tree by recusively walking through it
 * @param pt process tree
 * @return TRUE if succeeded otherwise FALSE.
 */
void fillprocesstree(ProcessTree_T * pt) {

  int i;
  ProcessTree_T  *parent_pt;

  ASSERT(pt);

  if (( pt->pid==0 ) || ( pt->visited == 1 )) {
    
    return;
    
  }

  pt->visited= 1;
  pt->children_sum= pt->children_num;
  pt->mem_kbyte_sum= pt->mem_kbyte;

  for( i = 0; i < pt->children_num; i++) {

    fillprocesstree(pt->children[i]);

  }

  if ( pt->parent != NULL ) {
    
    parent_pt=pt->parent;
    parent_pt->children_sum+=pt->children_sum;
    parent_pt->mem_kbyte_sum+=pt->mem_kbyte_sum;
    
  } 

}


/**
 * Transfer child information from process tree to process info structre
 * @param pi process info structure
 * @param pt process tree
 * @param treesize size of the process tree
 * @return TRUE if succeeded otherwise FALSE.
 */
int enterchildinfo(ProcInfo_T pi, ProcessTree_T * pt, int treesize) { 

  ProcessTree_T  *leaf;

  leaf = findprocess(pi->pid, pt, treesize);

  if (NULL == (leaf = findprocess(pi->pid, pt, treesize))) {
    
    return FALSE;

  }
  
  pi->children = leaf->children_sum;
  pi->total_mem_kbyte = leaf->mem_kbyte_sum;

  return TRUE;

}
