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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "monitor.h"
#include "protocol.h"
#include "ssl.h"
#include "engine.h"


/* Private prototypes */
static void _gc_service_list(Service_T *);
static void _gc_service(Service_T *);
static void _gc_mail_server(MailServer_T *);
static void _gcppl(Port_T *);
static void _gcdevice(Device_T *);
static void _gcicmp(Icmp_T *);
static void _gcpql(Resource_T *);
static void _gcppil(ProcInfo_T *);
static void _gcptl(Timestamp_T *);
static void _gccmd(Command_T *);
static void _gcpdil(DeviceInfo_T *);
static void _gcpdl(Dependant_T *);
static void _gcso(Size_T *);
static void _gcchecksum(Checksum_T *);
static void _gcperm(Perm_T *);
static void _gcuid(Uid_T *);
static void _gcgid(Gid_T *);
static void _gcgrc(Generic_T *);
static void _gcath(Auth_T *);


/**
 *  Release allocated memory.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: gc.c,v 1.48 2004/03/12 01:04:44 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


void gc() {

  destroy_hosts_allow();

  gc_protocols();

  if(servicelist)
    _gc_service_list(&servicelist);
  
  if(Run.credentials)
    _gcath(&Run.credentials);

  if(Run.maillist)
    gc_mail_list(&Run.maillist);
  
  if(Run.mailservers)
    _gc_mail_server(&Run.mailservers);

  FREE(Run.mygroup);
  FREE(Run.httpsslpem);
  FREE(Run.httpsslclientpem);
  FREE(Run.bind_addr);
  FREE(Run.MailFormat.from);
  FREE(Run.MailFormat.subject);
  FREE(Run.MailFormat.message);
  
}


void gc_mail_list(Mail_T *m) {

  ASSERT(m);
  
  if((*m)->next)
    gc_mail_list(&(*m)->next);
  
  FREE((*m)->to);
  FREE((*m)->from);
  FREE((*m)->subject);
  FREE((*m)->message);
  FREE((*m)->opt_message);
  FREE(*m);
  
}


/* ----------------------------------------------------------------- Private */


static void _gc_service_list(Service_T *s) {
  
  ASSERT(s&&*s);

  if((*s)->next) {
    _gc_service_list(&(*s)->next);
  }
  
  _gc_service(&(*s));
    
}


static void _gc_service(Service_T *s) {

  ASSERT(s&&*s);
  
  if((*s)->portlist) {
    _gcppl(&(*s)->portlist);
  }

  if((*s)->devicelist) {
    _gcdevice(&(*s)->devicelist);
  }

  if((*s)->icmplist) {
    _gcicmp(&(*s)->icmplist);
  }

  if((*s)->maillist) {
    gc_mail_list(&(*s)->maillist);
  }

  if((*s)->resourcelist) {
    _gcpql(&(*s)->resourcelist);
  }

  if((*s)->procinfo) {
    _gcppil(&(*s)->procinfo);
  }
  
  if((*s)->devinfo) {
    _gcpdil(&(*s)->devinfo);
  }
  
  if((*s)->timestamplist) {
    _gcptl(&(*s)->timestamplist);
  }

  if((*s)->sizelist) {
    _gcso(&(*s)->sizelist);
  }

  if((*s)->checksum) {
    _gcchecksum(&(*s)->checksum);
  }

  if((*s)->perm) {
    _gcperm(&(*s)->perm);
  }

  if((*s)->uid) {
    _gcuid(&(*s)->uid);
  }

  if((*s)->gid) {
    _gcgid(&(*s)->gid);
  }

  if((*s)->dependantlist) {
    _gcpdl(&(*s)->dependantlist);
  }

  if((*s)->start) {
    _gccmd(&(*s)->start);
  }
  
  if((*s)->stop) {
    _gccmd(&(*s)->stop);
  }
  
  FREE((*s)->name);
  FREE((*s)->path);
  FREE((*s)->group);
  
  (*s)->next= NULL;

  pthread_mutex_destroy(&(*s)->mutex);

  FREE(*s);

}
  

static void _gc_mail_server(MailServer_T *s) {

  if(!s&&!*s) return;
  
  if((*s)->next) {
    _gc_mail_server(&(*s)->next);
  }
  
  FREE((*s)->host);
  FREE(*s);

}

   
static void _gccmd(Command_T *c) {

  int i;
  
  ASSERT(c&&*c);
  
  for(i= 0; (*c)->arg[i]; i++)
    FREE((*c)->arg[i]);
  FREE(*c);

}


static void _gcppl(Port_T *p) {
  
  ASSERT(p&&*p);

  if((*p)->next) {
    _gcppl(&(*p)->next);
  }

  if((*p)->exec) {
    _gccmd(&(*p)->exec);
  }

  if((*p)->generic) {
    _gcgrc(&(*p)->generic);
  }

  FREE((*p)->address);
  FREE((*p)->request);
  FREE((*p)->hostname);
  FREE((*p)->pathname);
  FREE((*p)->SSL.certmd5);
  FREE((*p)->request_checksum);
  FREE(*p);

}


static void _gcdevice(Device_T *d) {
  
  ASSERT(d&&*d);

  if((*d)->next) {
    _gcdevice(&(*d)->next);
  }

  if((*d)->exec) {
    _gccmd(&(*d)->exec);
  }

  FREE(*d);

}


static void _gcicmp(Icmp_T *i) {
  
  ASSERT(i&&*i);

  if((*i)->next) {
    _gcicmp(&(*i)->next);
  }

  if((*i)->exec) {
    _gccmd(&(*i)->exec);
  }

  FREE(*i);

}


static void _gcpql(Resource_T *q) {

  ASSERT(q);

  if((*q)->next) {
    _gcpql(&(*q)->next);
  }
  
  if((*q)->exec) {
    _gccmd(&(*q)->exec);
  }

  FREE(*q);
  
}


static void _gcppil(ProcInfo_T *pi) {

  ASSERT(pi);

  FREE(*pi);

}


static void _gcpdil(DeviceInfo_T *di) {

  ASSERT(di);

  FREE(*di);

}


static void _gcptl(Timestamp_T *p) {
  
  ASSERT(p);

  if((*p)->next) {
    _gcptl(&(*p)->next);
  }

  if((*p)->exec) {
    _gccmd(&(*p)->exec);
  }
  FREE(*p);

}


static void _gcso(Size_T *s) {
  
  ASSERT(s);

  if((*s)->next)
    _gcso(&(*s)->next);

  if((*s)->exec)
    _gccmd(&(*s)->exec);

  FREE(*s);

}


static void _gcchecksum(Checksum_T *s) {
  
  ASSERT(s);

  if((*s)->exec)
    _gccmd(&(*s)->exec);

  FREE((*s)->hash);
  FREE(*s);

}


static void _gcperm(Perm_T *s) {
  
  ASSERT(s);

  if((*s)->exec)
    _gccmd(&(*s)->exec);

  FREE(*s);

}


static void _gcuid(Uid_T *s) {
  
  ASSERT(s);

  if((*s)->exec)
    _gccmd(&(*s)->exec);

  FREE(*s);

}


static void _gcgid(Gid_T *s) {
  
  ASSERT(s);

  if((*s)->exec)
    _gccmd(&(*s)->exec);

  FREE(*s);

}


static void _gcpdl(Dependant_T *d) {

  ASSERT(d);

  if((*d)->next) {
    _gcpdl(&(*d)->next);
  }

  FREE((*d)->dependant);
  FREE(*d);

}


static void _gcgrc(Generic_T *g) {

  ASSERT(g);

  if((*g)->next) {
    _gcgrc(&(*g)->next);
  }

  FREE((*g)->send);
#ifdef HAVE_REGEX_H
  if ((*g)->expect!=NULL) {
    regfree((*g)->expect);
  }
#endif
  FREE((*g)->expect);
  FREE(*g);
    
}


static void _gcath(Auth_T *c) {

  ASSERT(c);

  if((*c)->next) {
    _gcath(&(*c)->next);
  }

  FREE((*c)->uname);
  FREE((*c)->passwd);
  FREE(*c);
  
}
