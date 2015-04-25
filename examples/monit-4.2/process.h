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

#ifndef MONIT_PROCESS_H
#define MONIT_PROCESS_H

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define PROCESS_ZOMBIE        1

extern int  include_children;
extern char actionnames[][STRLEN];
extern char modenames[][STRLEN];
extern char operatornames[][STRLEN];
extern char operatorshortnames[][3];
extern char statusnames[][STRLEN];
extern char servicenames[][STRLEN];
extern char icmpnames[][STRLEN];
extern int  num_cpus;
extern long mem_kbyte_max;

int update_process_data(Service_T s, ProcessTree_T *, int treesize, pid_t pid);
int init_process_info(void);
int update_loadavg(void);

ProcessTree_T *findprocess(int, ProcessTree_T *, int);
int            initprocesstree(void);
void           delprocesstree(void);


#endif

