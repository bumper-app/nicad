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

#include "monitor.h"
#include "process.h"
#include "process_sysdep.h"


/**
 *  System dependent resource gathering code for UNKNOWN systems
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: sysdep_UNKNOWN.c,v 1.4 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */


int init_process_info_sysdep(void) {

  mem_kbyte_max = 0;

  return FALSE;

}


int get_process_info_sysdep(ProcInfo_T p) {
    
  return FALSE;
    
}

/**
 * THIS IS JUST A DUMMY!!!
 *
 * Read all processes of the proc files system to initilize
 * the process tree (sysdep version... but should work for
 * all procfs based unices) 
 * @param reference  reference of ProcessTree
 * @return treesize>0 if succeeded otherwise =0.
 */
int initprocesstree_sysdep(ProcessTree_T ** reference) {

  return 0;

}

/**
 * THIS IS JUST A DUMMY!!!
 *
 * This routine returns 'nelem' double precision floats containing
 * the load averages in 'loadv'; at most 3 values will be returned.
 * @param loadv destination of the load averages
 * @param nelem number of averages
 * @return: 0 if successful, -1 if failed (and all load averages are 0).
 */
int getloadavg_sysdep (double *loadv, int nelem) {

  int i;
  
  for ( i = 0; i < nelem; i++ ) {

    loadv[i] = 0.0;

  }

  return -1;

}
