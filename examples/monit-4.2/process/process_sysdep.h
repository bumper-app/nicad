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

#ifndef MONIT_PROCESS_SYSDEP_H
#define MONIT_PROCESS_SYSDEP_H

int    get_process_info(ProcInfo_T);
int    get_process_info_sysdep(ProcInfo_T);

int    init_process_info_sysdep(void);
int    init_proc_info_sysdep(void);

int    read_proc_file(char *, int, char *, int);
int    getdatafromproc(int, ProcessTree_T *);
int    getloadavg_sysdep (double *, int);

double get_float_time(void);

int    initprocesstree_sysdep(ProcessTree_T **);
void   fillprocesstree(ProcessTree_T *);

int    connectchild(ProcessTree_T * , ProcessTree_T *);


#endif

