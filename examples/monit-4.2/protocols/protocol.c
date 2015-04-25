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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "protocol.h"

/* Private variables */
static Protocol_T mydefault= NULL;
static Protocol_T myhttp= NULL;
static Protocol_T myftp= NULL;
static Protocol_T mysmtp= NULL;
static Protocol_T mypop= NULL;
static Protocol_T myimap= NULL;
static Protocol_T mynntp= NULL;
static Protocol_T myssh= NULL;
static Protocol_T mydwp= NULL;
static Protocol_T myldap2= NULL;
static Protocol_T myldap3= NULL;
static Protocol_T myrdate= NULL;
static Protocol_T myrsync= NULL;
static Protocol_T mygeneric= NULL;


/**
 *  Factory module for vending protocol objects. Using lazy
 *  initialization, and dishing out only one copy of the object.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: protocol.c,v 1.18 2004/01/29 17:52:12 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


void gc_protocols() {

  FREE(mydefault); 
  FREE(myhttp);    
  FREE(myftp);     
  FREE(mysmtp);    
  FREE(mypop);     
  FREE(myimap);    
  FREE(mynntp);    
  FREE(myssh);     
  FREE(mydwp);     
  FREE(myldap2);   
  FREE(myldap3);   
  FREE(myrdate);   
  FREE(myrsync);   
  FREE(mygeneric); 

}


void *create_default() {

  if(mydefault == NULL) {
    NEW(mydefault);
    mydefault->name= "DEFAULT";
    mydefault->check= check_default;
  }

  return mydefault;

}


void *create_http() {

  if(myhttp == NULL) {
    NEW(myhttp);
    myhttp->name= "HTTP";
    myhttp->check= check_http;
  }

  return myhttp;

}


void *create_ftp() {

  if(myftp == NULL) {
    NEW(myftp);
    myftp->name= "FTP";
    myftp->check= check_ftp;
  }

  return myftp;

}


void *create_smtp() {

  if(mysmtp == NULL) {
    NEW(mysmtp);
    mysmtp->name= "SMTP";
    mysmtp->check= check_smtp;
  }

  return mysmtp;

}


void *create_pop() {

  if(mypop == NULL) {
    NEW(mypop);
    mypop->name= "POP";
    mypop->check= check_pop;
  }

  return mypop;

}


void *create_imap() {

  if(myimap == NULL) {
    NEW(myimap);
    myimap->name= "IMAP";
    myimap->check= check_imap;
  }

  return myimap;

}


void *create_nntp() {

  if(mynntp == NULL) {
    NEW(mynntp);
    mynntp->name= "NNTP";
    mynntp->check= check_nntp;
  }

  return mynntp;

}


void *create_ssh() {
  
  if(myssh == NULL) {
    NEW(myssh);
    myssh->name= "SSH";
    myssh->check= check_ssh;
  }
  
  return myssh;

}


void *create_dwp() {
  
  if(mydwp == NULL) {
    NEW(mydwp);
    mydwp->name= "DWP";
    mydwp->check= check_dwp;
  }
  
  return mydwp;

}


void *create_ldap2() {
  
  if(myldap2 == NULL) {
    NEW(myldap2);
    myldap2->name= "LDAP2";
    myldap2->check= check_ldap2;
  }
  
  return myldap2;

}


void *create_ldap3() {
  
  if(myldap3 == NULL) {
    NEW(myldap3);
    myldap3->name= "LDAP3";
    myldap3->check= check_ldap3;
  }
  
  return myldap3;

}


void *create_rdate() {
  
  if(myrdate == NULL) {
    NEW(myrdate);
    myrdate->name= "RDATE";
    myrdate->check= check_rdate;
  }
  
  return myrdate;
  
}


void *create_rsync() {
  
  if(myrsync == NULL) {
    NEW(myrsync);
    myrsync->name= "RSYNC";
    myrsync->check= check_rsync;
  }
  
  return myrsync;
  
}

void *create_generic() {
  
  if(myrsync == NULL) {
    NEW(myrsync);
    myrsync->name= "generic";
    myrsync->check= check_generic;
  }
  
  return myrsync;
  
}


