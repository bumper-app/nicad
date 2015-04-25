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


#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <config.h>

#include "monitor.h"
#include "socket.h"

/* Protocols supported */
#define P_DEFAULT  1
#define P_HTTP     2
#define P_FTP      3
#define P_SMTP     4
#define P_POP      5
#define P_IMAP     6
#define P_NNTP     7
#define P_SSH      8
#define P_DWP      9
#define P_LDAP2    10
#define P_LDAP3    11
#define P_RDATE    12
#define P_RSYNC    13
#define P_GENERIC  14

/* Protocol Factory routines */
void  gc_protocols();
void* create_default();
void* create_http();
void* create_ftp();
void* create_smtp();
void* create_pop();
void* create_imap();
void* create_nntp();
void* create_ssh();
void* create_dwp();
void* create_ldap2();
void* create_ldap3();
void* create_rdate();
void* create_rsync();
void* create_generic();

/* "Package" locale Protocol routines */
int check_default(Socket_T);
int check_ssh(Socket_T);
int check_dwp(Socket_T);
int check_pop(Socket_T);
int check_ftp(Socket_T);
int check_http(Socket_T);
int check_smtp(Socket_T);
int check_imap(Socket_T);
int check_nntp(Socket_T);
int check_ldap2(Socket_T);
int check_ldap3(Socket_T);
int check_rdate(Socket_T);
int check_rsync(Socket_T);
int check_generic(Socket_T);
#endif
