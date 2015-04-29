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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "monitor.h"
#include "engine.h"
#include "md5.h"
#include "sha.h"
#include "base64.h"
#include "alert.h"
#include "process.h"


/* Private prototypes */
static char x2c(char *hex);
static char *is_str_defined(char *);
static int is_unsafe(unsigned char *c);


/**
 *  General purpose utility methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author "Martin Pala" <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *  @author Michael Amster, <mamster@webeasy.com> 
 *
 *  @version \$Id: util.c,v 1.115 2004/02/29 22:24:44 martinp Exp $
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * @return TRUE if the string parameter is defined, otherwise FALSE
 */
int is_strdefined(char *p) {
  
  return(p && *p);
  
}


/**
 * Strip the path and return only the filename
 * @param path A file path string
 * @return the basename
 */
char *stripfilename(char* path) {
  
  char *fname;

  ASSERT(path);

  fname= strrchr(path, '/');
  
  return(fname ? ++fname : path);
  
}


/**
 * Removes everything from the first newline (CR|LF)
 * @param string A string to be chomped
 */
void chomp(char *string, int len) {
  
  char *p=string;
  int   i;

  ASSERT(string);

  for (i=0;i<len;i++,p++) {
    if((*p== '\r') || (*p=='\n')) {
      *p= 0;
      return;
    }
  }
  
  string[len-1]=0;
}


/**
 * Remove leading and trailing space from the string
 * @param s A string
 * @return s with leading and trailing spaces removed
 */
char *trim(char *s) {

  ASSERT(s);
  
  ltrim(s);
  rtrim(s);

  return s;
  
}


/**
 * Remove leading white space [ \t\r\n] from the string.
 * @param s A string
 * @return s with leading spaces removed
 */
char *ltrim(char *s) {

  char *t= s;

  ASSERT(s);

  while(*t==' ' || *t=='\t' || *t=='\r' || *t=='\n') t++;

  return strcpy(s, t);

}


/**
 * Remove trailing white space [ \t\r\n] from the string
 * @param s A string
 * @return s with trailing spaces removed
 */
char *rtrim(char *s) {

  char *t= s;

  ASSERT(s);

  while(*s) s++;
  while(*--s==' ' || *s=='\t' || *s=='\r' || *s=='\n') *s= '\0';

  return t;

}

/**
 * Remove any enclosing quotes ["'] from the string
 * @param s A string
 * @return s with any enclosed quotes removed
 */

void trim_quotes(char *s) {

  char *t= s;
  char tmp=0;

  ASSERT(s);

  if(*t==39 || *t==34 ) {

    tmp=*t;
    t++;

  } else {

    return;
    
  }

  while ( *t != tmp && *t != '\0' ) {

    *(t-1) = *t;
    t++;
    
  }

  *(t-1) = '\0';
  
  return;

}


/**
 * Replace all occurrences of the <code>old</code> char in the string
 * <code>s</code> with the <code>new</code> char.
 * @param s A string
 * @param old The old char
 * @param new The new char
 * @return s where all occurrence of old are replaced with new
 */
char *replace_char(char *s, char old, char new) {

  char *t= s;

  while (s&&*s) { if(*s==old) *s=new; s++; }

  return (t);

}


/**
 * Replace all occurrences of the sub-string old in the string src
 * with the sub-string new. The method is case sensitive for the
 * sub-strings new and old. The string parameter src must be an
 * allocated string, not a character array.
 * @param src An allocated string reference (e.g. &string)
 * @param old The old sub-string
 * @param new The new sub-string
 * @return src where all occurrences of the old sub-string are
 * replaced with the new sub-string. 
 */
char *replace_string(char **src, const char *old, const char *new) {

  int i;
  int d;

  ASSERT(src && *src && old && new);
  
  i= count_words(*src, old);
  d= strlen(new)-strlen(old);
  
  if(i==0)
      return *src;
  if(d>0)
      d*= i;
  else
      d= 0;
  
  {
    char *p, *q;
    int l= strlen(old);
    char *buf= xmalloc(strlen(*src)+d+1);

    q= *src;
    *buf= 0;
    
    while((p= strstr(q, old))) {
      
      *p= '\0';
      strcat(buf, q);
      strcat(buf, new);
      p+= l;
      q= p;
      
    }
    
    strcat(buf, q);
    FREE(*src);
    *src= buf;
  }

  return *src;
  
}


/**
 * Count the number the sub-string word occurs in s.
 * @param s The String to search for word in
 * @param word 	The sub-string to count in s
 */
int count_words(char *s, const char *word) {

  int i= 0;
  char *p= s;

  ASSERT(s && word);
  
  while((p= strstr(p, word))) { i++;  p++; }

  return i;

}


/**
 * Return TRUE if the string <i>a</i> starts with the string
 * <i>b</i>. The test is <i>case-insensitive</i> but depends on that
 * all characters in the two strings can be translated in the current
 * locale.
 * @param a The string to search for b in
 * @param b The sub-string to test a against
 * @return TRUE if a starts with b, otherwise FALSE
 */
int starts_with(const char *a, const char *b) {

  if((!a || !b) || *a!=*b) return FALSE;

  while(*a && *b) {
    
    if(toupper(*a++) != toupper(*b++)) return FALSE;
    
  }

  return TRUE;

}


/**
 * Exchanges \escape sequences in a string
 * @param buf A string
 */
void handle_string_escapes(char *buf) {

  int editpos;
  int insertpos;

  ASSERT(buf);

  for(editpos=insertpos=0; *(buf+editpos)!='\0'; editpos++, insertpos++) {

    if(*(buf+editpos) == '\\' ) {
      
      switch(*(buf+editpos+1)) {

      case 'n': 
        *(buf+insertpos)='\n';
        editpos++;
	break;

      case 't':
        *(buf+insertpos)='\t';
        editpos++;
	break;

      case 'r':
        *(buf+insertpos)='\r';
        editpos++;
	break;

      case ' ':
        *(buf+insertpos)=' ';
        editpos++;
	break;

      case '\\':
        *(buf+insertpos)='\\';
        editpos++;
	break;

      default:
        *(buf+insertpos)=*(buf+editpos);

      }  

    } else {

      *(buf+insertpos)=*(buf+editpos);

    }  

  }
  *(buf+insertpos)='\0';

}


/**
 * @param name A service name as stated in the config file
 * @return the named service or NULL if not found
 */
Service_T get_service(const char *name) {

  Service_T s;

  ASSERT(name);

  for(s= servicelist; s; s= s->next) {
    if(IS(s->name, name)) {
      return s;
    }
  }

  return NULL;

}


/**
 * @param name A service name as stated in the config file
 * @return TRUE if the service name exist in the
 * servicelist, otherwise FALSE
 */
int exist_service(const char *name) {

  Service_T s;

  ASSERT(name);

  for(s= servicelist; s; s= s->next)
      if(IS(s->name, name))
	  return TRUE;

  return FALSE;

}


/**
 * Get the length of the service list, that is; the number of
 * services in the list.
 * @return The length of the service list
 */
int get_service_list_length() {

  int i= 0;
  Service_T s;

  for(s= servicelist; s; s= s->next) i+=1;

  return i;

}


/**
 * Print the Runtime object
 */
void printrunlist() {
  
  printf("Runtime constants:\n");
  printf(" %-18s = %s\n", "Control file", is_str_defined(Run.controlfile));
  printf(" %-18s = %s\n", "Log file", is_str_defined(Run.logfile));
  printf(" %-18s = %s\n", "Pid file", is_str_defined(Run.pidfile));
  printf(" %-18s = %s\n", "Debug", Run.debug?"True":"False");
  printf(" %-18s = %s\n", "Log", Run.dolog?"True":"False");
  printf(" %-18s = %s\n", "Use syslog", Run.use_syslog?"True":"False");
  printf(" %-18s = %s\n", "Is Daemon", Run.isdaemon?"True":"False");
  printf(" %-18s = %s\n", "Use process engine", Run.doprocess?"True":"False");
  printf(" %-18s = %d seconds\n", "Poll time", Run.polltime);
  printf(" %-18s = ", "Mail server(s)");

  if(Run.mailservers) {
    MailServer_T mta= Run.mailservers;
    for(mta= Run.mailservers; mta; mta= mta->next)
	printf("%s%s", mta->host, mta->next?", ":" ");
    printf("\n");
  } else {
    printf("localhost\n");
  }

  printf(" %-18s = %s\n", "Mail from", is_str_defined(Run.MailFormat.from));
  printf(" %-18s = %s\n", "Mail subject",
	 is_str_defined(Run.MailFormat.subject));
  printf(" %-18s = %-.20s%s\n", "Mail message",
	 Run.MailFormat.message?
	 Run.MailFormat.message:"(not defined)",
	 Run.MailFormat.message?"..(truncated)":"");

  printf(" %-18s = %s\n", "Start monit httpd", Run.dohttpd?"True":"False");
  
  if(Run.dohttpd) {
    
    printf(" %-18s = %s\n", "httpd bind address",
	   Run.bind_addr?Run.bind_addr:"Any/All");
    printf(" %-18s = %d\n", "httpd portnumber", Run.httpdport);
    printf(" %-18s = %s\n", "httpd signature", Run.httpdsig?"True":"False");
    printf(" %-18s = %s\n", "Use ssl encryption", Run.httpdssl?"True":"False");

    if(Run.httpdssl) {

      printf(" %-18s = %s\n", "PEM key/cert file", Run.httpsslpem);

      if(Run.httpsslclientpem!=NULL) {
	printf(" %-18s = %s\n", "Client cert file", Run.httpsslclientpem);
      } else {
	printf(" %-18s = %s\n", "Client cert file", "None");
      } 

      printf(" %-18s = %s\n", "Allow self certs", 
	     Run.allowselfcert?"True":"False");

    }

    printf(" %-18s = %s\n", "httpd auth. style",
	   (Run.credentials!=NULL)&&has_hosts_allow()?
	   "Basic Authentication and Host allow list":
	   (Run.credentials!=NULL)?"Basic Authentication":
	   has_hosts_allow()?"Host allow list":
	   "No authentication!");
     
  }

  {
    Mail_T list;
    for(list= Run.maillist; list; list= list->next) {
      printf(" %-18s = %s\n", "Alert mail to", is_str_defined(list->to));
      printf(" %-18s = ", "Alert on");
      if(list->events == (~((unsigned int)0))) {
        printf("All events");
      } else {
        if(IS_EVENT_SET(list->events, EVENT_START))
	  printf("Start ");
        if(IS_EVENT_SET(list->events, EVENT_STOP))
	  printf("Stop ");
        if(IS_EVENT_SET(list->events, EVENT_RESTART))
	  printf("Restart ");
        if(IS_EVENT_SET(list->events, EVENT_CHECKSUM))
	  printf("Checksum ");
        if(IS_EVENT_SET(list->events, EVENT_RESOURCE))
	  printf("Resource ");
        if(IS_EVENT_SET(list->events, EVENT_TIMEOUT))
	  printf("Timeout ");
        if(IS_EVENT_SET(list->events, EVENT_TIMESTAMP))
	  printf("Timestamp ");
        if(IS_EVENT_SET(list->events, EVENT_SIZE))
	  printf("Size ");
        if(IS_EVENT_SET(list->events, EVENT_CONNECTION))
	  printf("Connection ");
        if(IS_EVENT_SET(list->events, EVENT_PERMISSION))
	  printf("Permission ");
        if(IS_EVENT_SET(list->events, EVENT_UID))
	  printf("Uid ");
        if(IS_EVENT_SET(list->events, EVENT_GID))
	  printf("Gid ");
        if(IS_EVENT_SET(list->events, EVENT_UNMONITOR))
	  printf("Unmonitor ");
      }
      printf("\n");
    }
  }

  printf("\n");
  
}


/**
 * Print a service object
 * @param p A Service_T object
 */
void printservice(Service_T s) {
  
  Port_T n;
  Icmp_T i;
  Mail_T r;
  Device_T dl;
  Resource_T q;
  Timestamp_T t;
  Size_T sl;
  Dependant_T d;

  ASSERT(s);
 
  printf("%-21s = %s\n", servicenames[s->type], s->name);
  printf(" %-20s = %s\n", "Group", is_str_defined(s->group));
  if(s->type == TYPE_PROCESS)
    printf(" %-20s = %s\n", "Pid file", s->path);
  else if(s->type != TYPE_REMOTE)
    printf(" %-20s = %s\n", "Path", s->path);
  printf(" %-20s = %s\n", "Monitoring mode", modenames[s->mode]);
  if(s->start) {
    int i = 0;

    printf(" %-20s =", "Start program");
    while(s->start->arg[i])
      printf(" %s", s->start->arg[i++]);
    printf("\n");
  }
  if(s->stop) {
    int i = 0;

    printf(" %-20s =", "Stop program");
    while(s->stop->arg[i])
      printf(" %s", s->stop->arg[i++]);
    printf("\n");
  }

  for(d= s->dependantlist; d; d= d->next)
    if(d->dependant != NULL)
      printf(" %-20s = %s\n", "Depends on Service", d->dependant);

  if(s->checksum) {
    switch (s->checksum->type) {
    case HASH_MD5:
        printf(" %-20s = if failed MD5(%s) then %s\n",
               "Checksum", s->checksum->hash,
               actionnames[s->checksum->action]);
        break;
    case HASH_SHA1:
        printf(" %-20s = if failed SHA1(%s) then %s\n",
               "Checksum", s->checksum->hash,
               actionnames[s->checksum->action]);
        break;
    default:
        printf(" %-20s = if failed UNKNOWN(%s) then %s\n",
               "Checksum", s->checksum->hash,
               actionnames[s->checksum->action]);
        break;
    }
  }
  
  if(s->perm)
    printf(" %-20s = if failed %o then %s\n",
           "Permission", s->perm->perm, actionnames[s->perm->action]);

  if(s->uid)
    printf(" %-20s = if failed %d then %s\n",
           "UID", (int)s->uid->uid, actionnames[s->uid->action]);

  if(s->gid)
    printf(" %-20s = if failed %d then %s\n",
           "GID", (int)s->gid->gid, actionnames[s->gid->action]);

  if(s->portlist) {
    
    for(n= s->portlist; n; n= n->next) {
      
      if(n->family == AF_INET) {
	
	if(n->SSL.use_ssl) {
	  
	  printf(" %-20s = %s:%d%s [protocol %s via SSL] with timeout %d seconds\n",
	        "Host:Port", n->hostname, n->port, n->request?n->request:"",
		 n->protocol->name, n->timeout);
	  
	  if(n->SSL.certmd5 != NULL)
	      printf(" %-20s = %s\n", "Server cert md5 sum", n->SSL.certmd5);
	  
        } else {

	  printf(" %-20s = %s:%d%s [protocol %s] with timeout %d seconds\n",
	         "Host:Port", n->hostname, n->port, n->request?n->request:"",
		 n->protocol->name, n->timeout);
	  
	}
	
      } else if(n->family == AF_UNIX) {
	
        printf(" %-20s = %s [protocol %s] with timeout %d seconds\n",
	       "Unix Socket", n->pathname, n->protocol->name, n->timeout);
	
      }
      
    }
    
  }
  
  if(s->icmplist)
    for(i= s->icmplist; i; i= i->next)
      printf(" %-20s = if failed %s with timeout %d seconds then %s\n",
             "ICMP", icmpnames[i->type], i->timeout, actionnames[i->action]);

  for(t= s->timestamplist; t; t= t->next) {
    
    if(t->test_changes) {
      printf(" %-20s = if changed then %s\n",
	     "Timestamp",
	     actionnames[t->action]);
    } else {
      printf(" %-20s = if %s %d second(s) then %s\n",
	     "Timestamp",
	     operatornames[t->operator],
	     t->time,
	     actionnames[t->action]);
    }
    
  }

  for(sl= s->sizelist; sl; sl= sl->next) {
    
    if(sl->test_changes) {
      printf(" %-20s = if changed then %s\n",
	     "Size",
	     actionnames[sl->action]);
    } else {
      printf(" %-20s = if %s %lu byte(s) then %s\n",
	     "Size",
	     operatornames[sl->operator],
	     sl->size,
	     actionnames[sl->action]);
    }
    
  }

  for(dl= s->devicelist; dl; dl= dl->next) {
    
    if(dl->resource == RESOURCE_ID_INODE) {

      printf(" %-20s = if %s %ld %s then %s\n",
           "Inodes usage limit",
           operatornames[dl->operator],
           (dl->limit_absolute > -1)?dl->limit_absolute:dl->limit_percent,
           (dl->limit_absolute > -1)?"":"%",
           actionnames[dl->action]);

    } else if(dl->resource == RESOURCE_ID_SPACE) {

      printf(" %-20s = if %s %ld %s then %s\n",
           "Space usage limit",
           operatornames[dl->operator],
           (dl->limit_absolute > -1)?dl->limit_absolute:dl->limit_percent,
           (dl->limit_absolute > -1)?"blocks":"%",
           actionnames[dl->action]);

    }
    
  }

  for(q= s->resourcelist; q; q= q->next) {

    switch(q->resource_id) {

    case RESOURCE_ID_CPU_PERCENT: 

      printf(" %-20s = if %s %.1f%% for %d cycle(s) then %s\n", 
	     "CPU usage limit", 
	     operatornames[q->operator], 
	     q->limit/10.0, q->max_cycle, actionnames[q->action]);
      break;

    case RESOURCE_ID_MEM_PERCENT: 

      printf(" %-20s = if %s %.1f%% for %d cycle(s) then %s\n", 
	     "Memory usage limit", 
	     operatornames[q->operator], q->limit/10.0, q->max_cycle, 
	     actionnames[q->action]);
      break;

    case RESOURCE_ID_MEM_KBYTE: 

      printf(" %-20s = if %s %ldkB for %d cycle(s) then %s\n", 
	     "Memory amount limit", 
	     operatornames[q->operator], q->limit, q->max_cycle, 
	     actionnames[q->action]);
      break;

    case RESOURCE_ID_LOAD1: 

      printf(" %-20s = if %s %.1f for %d cycle(s) then %s\n", 
	     "Load avg. (1min)", 
	     operatornames[q->operator], q->limit/10.0, q->max_cycle, 
	     actionnames[q->action]);
      break;

    case RESOURCE_ID_LOAD5: 

      printf(" %-20s = if %s %.1f for %d cycle(s) then %s\n", 
	     "Load avg. (5min)", 
	     operatornames[q->operator], q->limit/10.0, q->max_cycle, 
	     actionnames[q->action]);
      break;

    case RESOURCE_ID_LOAD15: 

      printf(" %-20s = if %s %.1f for %d cycle(s) then %s\n", 
	     "Load avg. (15min)", 
	     operatornames[q->operator], q->limit/10.0, q->max_cycle, 
	     actionnames[q->action]);
      break;

    }    
  }

  if(s->def_every)
    printf(" %-20s = Check service every %d cycles\n", "Every", s->every);
  

  if(s->def_timeout)
    printf(" %-20s = Do timeout if %d restart within %d cycles\n",
	   "Timeout", s->to_start, s->to_cycle);

  for(r= s->maillist; r; r= r->next) {
    
    printf(" %-20s = %s\n", "Alert mail to", is_str_defined(r->to));
    printf(" %-20s = ", "Alert on");

    if(r->events == (~((unsigned int)0))) {
      
      printf("All events");

    } else {
      if(IS_EVENT_SET(r->events, EVENT_START))
	  printf("Start ");
      if(IS_EVENT_SET(r->events, EVENT_STOP))
	  printf("Stop ");
      if(IS_EVENT_SET(r->events, EVENT_RESTART))
	  printf("Restart ");
      if(IS_EVENT_SET(r->events, EVENT_CHECKSUM))
	  printf("Checksum ");
      if(IS_EVENT_SET(r->events, EVENT_RESOURCE))
	  printf("Resource ");
      if(IS_EVENT_SET(r->events, EVENT_TIMEOUT))
	  printf("Timeout ");
      if(IS_EVENT_SET(r->events, EVENT_TIMESTAMP))
	  printf("Timestamp ");
      if(IS_EVENT_SET(r->events, EVENT_SIZE))
	  printf("Size ");
      if(IS_EVENT_SET(r->events, EVENT_CONNECTION))
	  printf("Connection ");
      if(IS_EVENT_SET(r->events, EVENT_PERMISSION))
	  printf("Permission ");
      if(IS_EVENT_SET(r->events, EVENT_UID))
	  printf("Uid ");
      if(IS_EVENT_SET(r->events, EVENT_GID))
	  printf("Gid ");
      if(IS_EVENT_SET(r->events, EVENT_UNMONITOR))
	  printf("Unmonitor ");
    }
    
    printf("\n");
    
  }

  printf("\n");
  
}


/**
 * Print all the services in the servicelist
 */
void printservicelist() {

  Service_T s;
  char ruler[STRLEN];
  
  printf("The service list contains the following entries:\n\n");
  
  for(s= servicelist_conf; s; s= s->next_conf) {
    
    printservice(s);
    
  }

  memset(ruler, '-', STRLEN);
  printf("%-.79s\n", ruler);
  
}


/**
 * Open and read the pid from the given pidfile.
 * @param pidfile A pidfile with full path
 * @return the pid (TRUE) or FALSE if the pid could
 * not be read from the file
 */
pid_t get_pid(char *pidfile) {
  
  FILE *file= NULL;
  int pid= -1;

  ASSERT(pidfile);

  if(! exist_file(pidfile)) {
    
    return(FALSE);
    
  }

  if(! isreg_file(pidfile)) {
    
    log("%s: pidfile '%s' is not a regular file\n",prog, pidfile);
    return(FALSE);
    
  }
  
  if((file= fopen(pidfile,"r")) == (FILE *)NULL) {
    
    log("%s: Error opening the pidfile '%s' -- %s\n",
	prog, pidfile, STRERROR);
    return(FALSE);
    
  }

  fscanf(file, "%d", &pid);
  fclose(file);

  if(pid == -1) {
    
    log("%s: pidfile `%s' does not contain a valid pidnumber\n",
	prog, pidfile);
    
    return (FALSE);
    
  }

  return (pid_t)pid;
  
}


/**
 * @return TRUE (i.e. the running pid id)  if
 * the process is running, otherwise FALSE
 */
int is_process_running(Service_T s) {
  
  pid_t  pid;

  ASSERT(s);
  
  errno= 0;

  if((pid= get_pid(s->path)))
    if( (getpgid(pid) > -1) || (errno == EPERM) )
      return pid;

  memset(s->procinfo, 0, sizeof *(s->procinfo));

  return FALSE;
  
}


/**
 * Returns a RFC822 Date string. If the given date is NULL compute the
 * date now.
 * @param date
 * @return a date string or NULL if an error occured
 */
char *get_RFC822date(long *date) {

  char D[STRLEN];
  struct tm *tm_now;
  time_t now= (date && *date>0)?*date:time(NULL);

  tm_now= localtime(&now);

  if(strftime(D, STRLEN, "%a, %d %b %Y %H:%M:%S %z", tm_now) <= 0) {

    return NULL;

  }

  return xstrdup(D);
  
}


/**
 * Compute an uptime string for a process based on the ctime
 * from the pidfile. The caller must free the returned string.
 * @param pidfile A process pidfile
 * @return an uptime string
 */
char *get_process_uptime(char *pidfile, char *sep) {

  time_t ctime;

  ASSERT(pidfile);

  if( (ctime= get_timestamp(pidfile, S_IFREG)) ) {

    time_t now= time(&now);
    time_t since= now-ctime;

    return get_uptime(since, sep);

  }

  return xstrdup("");

}

  
/**
 * Compute an uptime string based on the delta time in seconds. The
 * caller must free the returned string.
 * @param delta seconds. 
 * @param sep string separator
 * @return an uptime string
 */
char *get_uptime(time_t delta, char *sep) {

  static int min= 60;
  static int hour= 3600;
  static int day= 86400;
  long rest_d;
  long rest_h;
  long rest_m;
  char buf[STRLEN];
  char *p= buf;

  *buf= 0;

  if((rest_d= delta/day)>0) {
    p+= snprintf(p, STRLEN-(p-buf), "%ldd%s", rest_d,sep);
    delta-= rest_d*day;
  }
  if((rest_h= delta/hour)>0 || (rest_d > 0)) {
    p+= snprintf(p, STRLEN-(p-buf),"%ldh%s", rest_h,sep);
    delta-= rest_h*hour;
  }

  rest_m= delta/min;
  p+= snprintf(p, STRLEN-(p-buf),"%ldm%s", rest_m,sep);
  delta-= rest_m*min;

  return xstrdup(buf);

}


/**
 * @return a checksum for the given file, or NULL if error.
 */
char *get_checksum(char *file, int hashtype) {

  int hashlength=16;

  ASSERT(file);

  switch(hashtype) {
  case HASH_MD5:
      hashlength=16;
      break;

  case HASH_SHA1:
      hashlength=20;
      break;
      
  default:
      return NULL;
  }

  if(isreg_file(file)) {
    
    FILE *f= fopen(file, "r");
    
    if(f) {
      
      int i;
      unsigned char buf[hashlength];
      char result[STRLEN];
      char *r= result;
      int fresult=0;

      *result=0;

      switch(hashtype) {
      case HASH_MD5:
          fresult=md5_stream(f, buf);
          break;
          
      case HASH_SHA1:
          fresult=sha_stream(f, buf);
          break;
      }
      
      if(fresult) {
	
	fclose(f);
	
	return NULL;
	
      }
      
      fclose(f);
      
      for(i= 0; i < hashlength; ++i)
	r+= snprintf(r, STRLEN-(r-result) ,"%02x", buf[i]);
    
      return (xstrdup(result));
      
    }
    
  }
  
  return NULL;

}


/**
 * @param file A file to open and compute a checksum for
 * @param sum A previous checksum computed for this file
 * @return TRUE if computation of hash on the given file, is equal
 * to the given sum, otherwise return FALSE. FALSE is also returned if
 * *hash*_stream fails.
 */
int check_hash(char *file, char *sum, int hashtype) {

  char *newSum;

  ASSERT(file);
  ASSERT(sum);

  newSum= get_checksum(file, hashtype);


  if(newSum) {
    
    int rv;
    
    switch(hashtype) {
    case HASH_MD5:
        rv= (!strncmp(sum, newSum, 32));
        break;
        
    case HASH_SHA1:
        rv= (!strncmp(sum, newSum, 40));
        break;

    default:
        rv= FALSE;
    }
    
    FREE(newSum);
    
    return (rv);
    
  }

  return FALSE;
  
}


/**
 * Escape an uri string converting unsafe characters to a hex (%xx)
 * representation.  The caller must free the returned string.
 * @param uri an uri string
 * @return the escaped string
 */
char *url_encode(char *uri) {

  register int x, y;
  unsigned char *str;
  static unsigned char hexchars[]= "0123456789ABCDEF";

  ASSERT(uri);

  str= (unsigned char *)xmalloc(3 * strlen(uri) + 1);

  for(x = 0, y = 0; uri[x]; x++, y++) {
    if(is_unsafe((unsigned char*) &uri[x])) {
      str[y++] = '%';
      str[y++] = hexchars[(unsigned char) uri[x] >> 4];
      str[y] = hexchars[(unsigned char) uri[x] & 0xf];
    } else str[y]= (unsigned char)uri[x];
  }

  str[y] = '\0';

  return ((char *) str);

}


/**
 * Unescape an url string. The <code>url</code> parameter is modified
 * by this method.
 * @param url an escaped url string
 * @return A pointer to the unescaped <code>url</code>string
 */
char *url_decode(char *url) {

  register int x,y;

  if(!(url&&*url)) return url;
  replace_char(url, '+', ' ');
  for(x=0,y=0;url[y];++x,++y) {
    if((url[x] = url[y]) == '%') {
      url[x]= x2c(&url[y+1]);
      y+=2;
    }
  }
  url[x]= 0;
  return url;
}


/**
 * @return a Basic Authentication Authorization string (RFC 2617),
 * with credentials from the Run object, The string "\r\n" if
 * credentials are not defined.
 */
char *get_basic_authentication_header() {

  Auth_T c=Run.credentials;

  if (c==NULL) {

    return xstrdup("\r\n");

  }
  
  /* We find the first cleartext credential for authorization */
  
  while (c!= NULL) {

    if (c->digesttype == DIGEST_CLEARTEXT) {

      break;
      
    }
    
    c=c->next;
    
  }
  
  if(c!=NULL) {

    char *auth, *b64;
    char  buf[STRLEN];

    snprintf(buf, STRLEN, "%s:%s",
             c->uname,
             c->passwd);

    if(! (b64= encode_base64(strlen(buf), (unsigned char *)buf)) ) {
      log("Failed to base64 encode authentication header\n");
      return NULL;
    }

    auth= xcalloc(sizeof(char), STRLEN+1);
    snprintf(auth, STRLEN, "Authorization: Basic %s\r\n", b64);
    FREE(b64);

    return auth;

  }

  log("Cleattext credentials needed for basic authorization!\n");
  return NULL;

}


/**
 * Do printf style format line parsing
 * @param s format string
 * @param ap variable argument list
 * @param len The lenght of the bytes written,
 * may be different from the returned allocated buffer size
 * @return buffer with parsed string
 */
char *format(const char *s, va_list ap, long *len) {

  int n;
  int size= STRLEN;
  char *buf= xmalloc(size);

  ASSERT(s);
    
  while(TRUE) {
      
    n= vsnprintf(buf, size, s, ap);
    
    if(n > -1 && n < size)
        break;
    
    if(n > -1)
        size= n+1;
    else
        size*= 2;
    
    buf= xresize(buf, size);
    
  }

  *len= n;
  
  return buf;

}


/**
 * Redirect the standard file descriptors to /dev/null and route any
 * error messages to the log file.
 */
void redirect_stdfd() {

  int i;
  
  for(i= 0; i < 3; i++)
    if(close(i) == -1 || open("/dev/null", O_RDWR) != i)
      log("Cannot reopen standard file descriptor (%d) -- %s\n", i, STRERROR);
  
}


/*
 * Close all filedescriptors except standard. Everything
 * seems to have getdtablesize, so we'll use it here, and back
 * out to use 1024 if getdtablesize not available.
 */
void fd_close() {

  int i;
#ifdef HAVE_UNISTD_H
  int max_descriptors = getdtablesize();
#else
  int max_descriptors = 1024;
#endif

  for(i = 3; i < max_descriptors; i++)
    (void) close(i);

  errno= 0;

}

/*
 * Check if monit does have credentials for this user.  If successful
 * a pointer to the password is returned.
 */
Auth_T get_user_credentials(char *uname) {

  Auth_T c= Run.credentials;
  
  while ( c != NULL ) {

    if ( strcmp(c->uname, uname) == 0 ) {

      return c;
      
    }
    
    c=c->next;
    
  }

  return NULL;
}


int compare_user_credentials(char *uname, char *outside) {

  Auth_T c= get_user_credentials(uname);
  char outside_crypt[STRLEN];
  
  if ( c==NULL ) {

    return FALSE;
    
  }
  switch (c->digesttype) {
  case DIGEST_CLEARTEXT:
  {
    strncpy(outside_crypt, outside, STRLEN); 

    break;
  }   
  case DIGEST_MD5:
  {
    char id[STRLEN];
    char salt[STRLEN];
    char * temp;

    /* A password looks like this,
     *   $id$salt$digest
     * the '$' around the id are still part of the id. 
     */

    strncpy(id, c->passwd, STRLEN);
    temp= strchr(id+1, '$')+1;
    *temp= '\0';

    strncpy(salt, c->passwd+strlen(id), STRLEN);
    temp= strchr(salt, '$');
    *temp= '\0';

    if (md5_crypt(outside, id, salt, outside_crypt, STRLEN) == NULL) {

      log("Cannot generate MD5 digest error.\n");
      return FALSE;
      
    }
    
    break;
  }
  case DIGEST_CRYPT:
  {
    char salt[3];
    char * temp;
    snprintf(salt, 3, "%c%c", c->passwd[0], c->passwd[1]);
    temp= crypt(outside, salt);
    strncpy(outside_crypt, temp, STRLEN); 
    
    break;
  }
  
  default:
      log("Unknown password digestion method.\n");
      return FALSE;
  }

  if (strcmp(outside_crypt,c->passwd)==0) {
    return TRUE;
  }

  return FALSE;
}


/* ----------------------------------------------------------------- Private */


/**
 * Returns the value of the variable if defined or the String
 * "(not defined)"
 */
static char *is_str_defined(char *var) {
  
  return (is_strdefined(var)?var:"(not defined)");
  
}


/**
 * Returns TRUE if the given char is url unsafe
 * @param c A unsigned char
 * @return TRUE if the char is in the set of unsafe URL Characters
 */
static int is_unsafe(unsigned char *c) {

  int i;
  static unsigned char unsafe[]= "<>\"#{}|\\^~[]`";

  ASSERT(c);
  
  if(33>*c || *c>176)
      return TRUE;
  
  if(*c=='%') {
    if( isxdigit(*(c + 1)) && isxdigit(*(c + 2)) ) return FALSE;
    return TRUE;
  }

  for(i=0; unsafe[i]; i++)
    if(*c==unsafe[i]) return TRUE;

  return FALSE;

}


/**
 * Convert a hex char to a char
 */
static char x2c(char *hex) {
  
  register char digit;
  
  digit = ((hex[0] >= 'A') ? ((hex[0] & 0xdf) - 'A')+10 : (hex[0] - '0'));
  digit *= 16;
  digit += (hex[1] >= 'A' ? ((hex[1] & 0xdf) - 'A')+10 : (hex[1] - '0'));
  
  return(digit);
  
}

