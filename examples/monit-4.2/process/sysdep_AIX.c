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

#ifdef HAVE_PROCINFO_H
#include <procinfo.h>
#endif

#ifdef HAVE_SYS_PROC_H
#include <sys/proc.h>
#endif

#ifdef HAVE_PROCFS_H
#include <procfs.h>
#endif


#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"

/**
 *  System dependent resource gathering code for AIX
 *
 *  @author Richard Schwaninger <risc@vmilch.at>
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: sysdep_AIX.c,v 1.7 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 * most of the code is based on monitor-2.1.5 available on various sources
 * (see below).
 * The source code version of monitor program can be get from ftp-server
 *    ftp://ftp.mesa.nl/pub/monitor/monitor-2.1.5.tar.gz
 * or via the web from
 *    http://www.mesa.nl/pub/monitor/monitor-2.1.5.tar.gz.
 */

#define NPROCS 10000
struct procinfo procs[NPROCS];
int kmemfd = -1;
int nlistdone = 0;

int init_process_info_sysdep(void) {

  mem_kbyte_max = 0;
  num_cpus= sysconf( _SC_NPROCESSORS_ONLN);
  return (getuid()==0);

}

double timestruc_to_tseconds(struct timeval t) {

  return  t.tv_sec * 10 + t.tv_usec * 1.0e-7;

}


int get_process_info_sysdep(ProcInfo_T p) {
    
  int nproc;
  int i,rv;
  struct userinfo user;
  
  memset(&user,0,sizeof(struct userinfo));
  nproc=getproc(procs,NPROCS,sizeof(struct procinfo));
  for(i=0;i<nproc;i++) {
    
    if(p->pid==procs[i].pi_pid) {
      
      if(procs[i].pi_stat==SZOMB) {

empty:

	p->cputime_prev= p->cputime = 0;
	p->mem_kbyte= 0;
	p->mem_percent= 0.0;
	p->status_flag |= PROCESS_ZOMBIE;
 
     } else {

       rv=getuser(&(procs[i]),sizeof(struct procinfo),
		  &user,sizeof(struct userinfo));
      
       if(rv== -1) {

	 goto empty;

       }

       p->mem_percent = user.ui_prm*10;
       p->mem_kbyte = (user.ui_drss+user.ui_trss)*4;
       
       p->cputime_prev= p->cputime;
       p->cputime= ( timestruc_to_tseconds(user.ui_ru.ru_utime) +
		     timestruc_to_tseconds(user.ui_ru.ru_stime));
       
       if( include_children ) {
	 p->cputime+= ( 
		       timestruc_to_tseconds(user.ui_cru.ru_utime)+
		       timestruc_to_tseconds(user.ui_cru.ru_stime));
       }
       /* first run ? */
       if ( p->time_prev == 0.0 ) {
	
	 p->cputime_prev= p->cputime;

       }       
     }    
      
      return TRUE; 

    }
  }

  return FALSE;
    
}


/***************************************************************************
 *                      DATA CAPTURE FUNCTIONS                             *
 ***************************************************************************/

int getkmemdata(void *buf, int bufsize, caddr_t address) {

  int n;
  
  /*
   * Do stuff we only need to do once per invocation, like opening
   * the kmem file and fetching the parts of the symbol table.
   */
  
  if (kmemfd < 0) {
    if ((kmemfd = open("/dev/kmem", O_RDONLY)) < 0) {
      perror("kmem");
      exit(1);
    }
  }

  /*
   * Get the structure from the running kernel.
   */

  lseek(kmemfd, (off_t) address, SEEK_SET);
  n = read(kmemfd, buf, bufsize);
  
  return(n);

} /* getkmemdata */


/**
 * This routine returns 'nelem' double precision floats containing
 * the load averages in 'loadv'; at most 3 values will be returned.
 * @param loadv destination of the load averages
 * @param nelem number of averages
 * @return: 0 if successful, -1 if failed (and all load averages are 0).
 */
int getloadavg_sysdep (double *loadv, int nelem) {

  /*
  static int initted=0;
  int avenrun[3];
  static int no_avenrun_here = 0;
  static struct nlist kernelnames[] = {
    {"avenrun", 0, 0, 0, 0, 0},
    {NULL, 0, 0, 0, 0, 0}
  };
  
  if (no_avenrun_here)
    return FALSE;
  
  if (!initted) {

    initted = 1;
    if (knlist(kernelnames, 1, sizeof(struct nlist)) == -1){

      no_avenrun_here = 1;
      return -1;

    }

  }

  getkmemdata(&avenrun, sizeof(avenrun), (caddr_t) kernelnames->n_value);
  if (nelem > 0)
    loadv[0] = avenrun[0] / 65536.0;
  if (nelem > 1)
    loadv[1] = avenrun[1] / 65536.0;
  if (nelem > 2)
    loadv[2] = avenrun[2] / 65536.0;
  
  return TRUE;
  */

  return getloadavg(loadv, nelem);

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
  int i,rv;
  struct userinfo user;
  ProcessTree_T *  pt;
  
  memset(&user,0,sizeof(struct userinfo));

  /* Find all processes in the system */

  treesize=getproc(procs,NPROCS,sizeof(struct procinfo));
  
  /* Allocate the tree */
  
  pt = xcalloc(sizeof(ProcessTree_T), treesize);

  /* Insert data from /proc directory */

  for(i=0;i<treesize;i++) {

    pt[i].pid=procs[i].pi_pid;
    pt[i].ppid=procs[i].pi_ppid;

    if(procs[i].pi_stat==SZOMB) {

      pt[i].mem_kbyte= 0;

    } else {

      rv=getuser(&(procs[i]),sizeof(struct procinfo),
		 &user,sizeof(struct userinfo));

      if(rv== -1) {

	pt[i].mem_kbyte= 0;

      } else {

        pt[i].mem_kbyte = (user.ui_drss+user.ui_trss)*4;

      }
    }    
  }

  * reference = pt;

  return treesize;

}
