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


%{
  
/*
 * DESCRIPTION
 *   Simple context-free grammar for parsing the control file. 
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Olivier Beyssac, <ob@r14.freenix.org>
 *  @author Kianusch Sayah Karadji <kianusch.sayah.karadji@sk-tech.net>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *  @author Rory Toma <rory@digeo.com>
 *  @version \$Id: p.y,v 1.168 2004/03/12 00:52:18 martinp Exp $
 */

#include <config.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif 

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif 

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif 

#ifdef HAVE_GRP_H
#include <grp.h>
#endif 
  
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
  
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_ASM_PARAM_H
#include <asm/param.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifndef HAVE_SOL_IP
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

#ifdef HAVE_NETINET_IP_ICMP_H
#include <netinet/ip_icmp.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
  
#include "net.h"
#include "monitor.h"
#include "protocol.h"
#include "engine.h"
#include "alert.h"
#include "process.h"
#include "ssl.h"
#include "device.h"

  struct IHavePrecedence {
    int daemon;
    int logfile;
    int pidfile;
  }; 


  struct PortSet {
    int  socket;
    char *hostname;
    int  port;
    int  type;
    int  family;
    int  ssl;
    int  sslversion;
    char *request;
    char *request_checksum;
    Generic_T generic;
    char *pathname;
    char *certmd5;
    Protocol_T protocol;
    int  action;
    int timeout;
  };

  struct ResourceSet {
    int  resource_id;
    int  limit;
    int  operator;
    int  max_cycle;
    int  action;
  };

  struct TimestampSet {
    int  operator;
    int  time;
    int  test_changes;
    int  action;
  };

  struct SizeSet {
    int  operator;
    unsigned int size;
    int  test_changes;
    int  action;
  };

  struct ChecksumSet {
    char *hash;
    int   action;
    int   type;
  };

  struct PermSet {
    int perm;
    int action;
  };

  struct UidSet {
    uid_t uid;
    int   action;
  };

  struct GidSet {
    gid_t gid;
    int   action;
  };

  struct DeviceSet {
    int  resource;
    int  operator;
    long limit_absolute;
    int  limit_percent;
    int  action;
  };

  struct IcmpSet {
    int  type;
    int  timeout;
    int  action;
  };

  /* yacc interface */
  void  yyerror(const char *,...);
  void  yyerror2(const char *,...);
  void  yywarning(const char *,...);
  void  yywarning2(const char *,...);

  /* lexer interface */
  int yylex(void);
  extern FILE *yyin;
  extern int lineno;
  extern int arglineno;
  extern char *yytext;
  extern char *argyytext;
  extern char *currentfile;
  extern char *argcurrentfile;
  extern int buffer_stack_ptr;

  
  /* Local variables */
  static int cfg_errflag= FALSE;
  static Service_T tail= NULL;
  static Service_T current= NULL;
  static struct mymail mailset;
  static unsigned int eventset;
  static Command_T command= NULL;
  static Service_T depend_list= NULL;
  static struct IHavePrecedence ihp= {FALSE, FALSE, FALSE};
  static struct PortSet portset= {-1, NULL, 0, SOCK_STREAM, AF_INET, FALSE,
				  SSL_VERSION_AUTO, NULL,NULL,  NULL, NULL,
				  NULL, NULL, ACTION_ALERT, NET_TIMEOUT};
  static struct ResourceSet resourceset= {0, 0, OPERATOR_EQUAL, 1,
    ACTION_ALERT};
  static struct TimestampSet timestampset= {OPERATOR_EQUAL, 0, 0, ACTION_ALERT};
  static struct SizeSet sizeset= {OPERATOR_EQUAL, 0, 0, ACTION_ALERT};
  static struct ChecksumSet checksumset= {NULL, ACTION_ALERT};
  static struct PermSet permset= {0, ACTION_ALERT};
  static struct UidSet uidset= {0, ACTION_ALERT};
  static struct GidSet gidset= {0, ACTION_ALERT};
  static struct DeviceSet deviceset= {0, OPERATOR_EQUAL, -1, -1, ACTION_ALERT};
  static struct IcmpSet icmpset= {ICMP_ECHO, NET_TIMEOUT, ACTION_ALERT};
  static char * htpasswd_file= NULL;
  static int    digesttype= DIGEST_CLEARTEXT;
  
  /* Private prototypes */
  static void  initialize();
  static void  postparse();
  static void  addservice(Service_T);
  static void  addmail(char *, Mail_T, Mail_T *, unsigned int);
  static void  createservice(int, char *, char *, int (*)(Service_T));
  static void  adddependant(char *);
  static void  addport(struct PortSet *);
  static void  addresource(struct ResourceSet *);
  static void  addtimestamp(struct TimestampSet *);
  static void  addsize(struct SizeSet *);
  static void  adddevice(struct DeviceSet *);
  static void  addicmp(struct IcmpSet *);
  static void *addprotocol(int);
  static void  addgeneric(struct PortSet *, char*, char*);
  static void  addcommand(int);
  static void  addargument(char *);
  static void  addmailserver(char *);
  static int   addcredentials(char *, char *, int, int);
  static void  addhtpasswdentry(char *, char *, int);
  static uid_t get_uid(char *, uid_t);
  static gid_t get_gid(char *, gid_t);
  static void  addchecksum(struct ChecksumSet *);
  static void  addperm(struct PermSet *);
  static void  adduid(struct UidSet *);
  static void  addgid(struct GidSet *);
  static void  addeuid(uid_t);
  static void  addegid(gid_t);
  static void  setlogfile(char *);
  static void  setpidfile(char *);
  static void  reset_mailset();
  static void  reset_portset();
  static void  reset_resourceset();
  static void  reset_timestampset();
  static void  reset_sizeset();
  static void  reset_checksumset();
  static void  reset_permset();
  static void  reset_uidset();
  static void  reset_gidset();
  static void  reset_deviceset();
  static void  reset_icmpset();
  static void  check_name(char *);
  static void  check_timeout(int, int);
  static void  check_every(int);
  static int   check_perm(int);
  static void  check_hostname (char *);
  static void  check_exec(char *);
  static void  createdevinfo();
  static void  createprocinfo();
  static int   cleanup_hash_string(char *);
  static void  check_depend();
  static void  order_depend(Service_T);
  static void  validate_depend(Service_T, int *);
  static void  setsyslog(char *);
%}

%union {
  float real;
  int   number;
  char *string;
}

%token IF THEN FAILED CYCLE
%token SET LOGFILE FACILITY DAEMON SYSLOG MAILSERVER HTTPD ALLOW ADDRESS INIT
%token READONLY CLEARTEXT MD5HASH SHA1HASH CRYPT
%token PEMFILE ENABLE DISABLE HTTPDSSL CLIENTPEMFILE ALLOWSELFCERTIFICATION
%token STATEFILE SEND EXPECT
%token PIDFILE START STOP PATHTOK
%token HOST PORT TYPE UDP TCP TCPSSL PROTOCOL CONNECTION
%token ALERT MAILFORMAT UNIXSOCKET SIGNATURE
%token TIMEOUT RESTART CHECKSUM EXPECT EVERY 
%token DEFAULT HTTP FTP SMTP POP IMAP NNTP SSH DWP LDAP2 LDAP3 RDATE RSYNC
%token <string> STRING PATH MAILADDR MAILFROM MAILSUBJECT 
%token <string> MAILBODY SERVICENAME
%token <number> NUMBER PERCENT
%token <real> REAL
%token CHECKPROC CHECKDEV CHECKFILE CHECKDIR CHECKHOST
%token CPUUSAGE MEMUSAGE MEMKBYTE CHILDREN 
%token RESOURCE MEMORY TOTALMEMORY CPU LOADAVG1 LOADAVG5 LOADAVG15 
%token MODE ACTIVE PASSIVE MANUAL
%token GROUP REQUEST DEPENDS
%token UID GID
%token TIMESTAMP CHANGED SECOND MINUTE HOUR DAY
%token SSLAUTO SSLV2 SSLV3 TLSV1 CERTMD5
%token BYTE KILOBYTE MEGABYTE GIGABYTE
%token INODE SPACE PERMISSION SIZE
%token EXEC UNMONITOR ICMP ICMPECHO

%left GREATER LESS EQUAL NOTEQUAL


%%
cfgfile         : /* EMPTY */
                | statement_list
                ;

statement_list  : statement
                | statement_list statement
                ;

statement       : setalert
                | setdaemon
                | setlog
                | setmailservers
                | setmailformat
                | sethttpd
                | setpid
                | setstate
                | setinit
                | checkproc optproclist
                | checkfile optfilelist
                | checkdev optdevlist
                | checkdir optdirlist
                | checkhost opthostlist
                ;

optproclist     : /* EMPTY */
                | optproclist optproc
                ;

optproc         : start
                | stop
                | connection
                | connectionunix
                | timeout
                | alert
                | every
		| mode
		| group
                | depend
		| resourcesystem
                ;

optfilelist      : /* EMPTY */
                | optfilelist optfile
                ;

optfile         : start
                | stop
		| timestamp
                | timeout
                | every
                | alert
		| permission
		| uid
		| gid
		| checksum
                | size
		| mode
		| group
                | depend
                ;

optdevlist      : /* EMPTY */
                | optdevlist optdev
                ;

optdev          : start
                | stop
                | timeout
                | every
                | alert
		| permission
		| uid
		| gid
		| mode
		| group
                | depend
                | inode
                | space
                ;

optdirlist      : /* EMPTY */
                | optdirlist optdir
                ;

optdir          : start
                | stop
		| timestamp
                | timeout
                | every
                | alert
		| permission
		| uid
		| gid
		| mode
		| group
                | depend
                ;

opthostlist     : opthost
                | opthostlist opthost
                ;

opthost         : start
                | stop
                | connection
		| icmp
                | timeout
                | alert
                | every
		| mode
		| group
                | depend
                ;

setalert        : SET alertmail '{' eventoptionlist '}' formatlist {
                   addmail($<string>2, &mailset, &Run.maillist, eventset);
                 }
                | SET alertmail formatlist {
                   addmail($<string>2, &mailset, &Run.maillist, eventset);
		 }
                ;

setdaemon       : SET DAEMON NUMBER  {
                   if(!Run.isdaemon || ihp.daemon) {
		     ihp.daemon= TRUE;
		     Run.isdaemon= TRUE;
		     Run.polltime= $3;
		   }
                 }
                ;

setinit         : SET INIT {
                   Run.init= TRUE;
                 }
                ;

setlog          : SET LOGFILE PATH   {
                   if(!Run.logfile || ihp.logfile) {
		     ihp.logfile= TRUE;
		     setlogfile($3);
		     Run.use_syslog= FALSE;
		     Run.dolog=TRUE;
		   }
                 }
                | SET LOGFILE SYSLOG { setsyslog(NULL); }
                | SET LOGFILE SYSLOG FACILITY STRING {
		    setsyslog($5); FREE($5);
		  }
		;

setstate        : SET STATEFILE PATH {
                    Run.statefile= $3;
                  }
                ;

setpid          : SET PIDFILE PATH {
                   if(!Run.pidfile || ihp.pidfile) {
		     ihp.pidfile= TRUE;
                     setpidfile($3);
		   }
                 }
                ;

setmailservers  : SET MAILSERVER mailserverlist nettimeout {
                   Run.mailserver_timeout= $<number>4;
                  }
                ;

setmailformat   : SET MAILFORMAT '{' formatoptionlist '}' {
                   Run.MailFormat.from=
			mailset.from?
			mailset.from:
			xstrdup(ALERT_FROM);
                   Run.MailFormat.subject=
			mailset.subject?
			mailset.subject:
			xstrdup(ALERT_SUBJECT);
                   Run.MailFormat.message=
			mailset.message?
			mailset.message:
			xstrdup(ALERT_MESSAGE);
		   reset_mailset();
                 }
                ;

sethttpd        : SET HTTPD PORT NUMBER httpdlist {
                   Run.dohttpd= TRUE;
		   Run.httpdport= $4;
                 }
                ;

mailserverlist  : mailserver
                | mailserverlist mailserver
                ;

mailserver      : STRING { check_hostname($1); addmailserver($1); }
                ;

httpdlist	: /* EMPTY */
                | httpdlist httpdoption
                ; 

httpdoption     : ssl
		| signature
		| bindaddress
                | allow
                ;

ssl             : ssldisable { Run.httpdssl= FALSE; }
                | sslenable pemfile clientpemfile allowselfcert { 
		  Run.httpdssl= TRUE;                   
		  if(!have_ssl()) {
		    yyerror("SSL is not supported");
		  }
		 }
                ;

sslenable       : HTTPDSSL
                | HTTPDSSL ENABLE
                | ENABLE HTTPDSSL
                ;

ssldisable      : HTTPDSSL DISABLE
                | DISABLE HTTPDSSL
                | ssldisable PEMFILE PATH { FREE($3); }
                | ssldisable CLIENTPEMFILE PATH { FREE($3); }
                | ssldisable ALLOWSELFCERTIFICATION
                ;

signature       : sigenable  { Run.httpdsig= TRUE; }
                | sigdisable { Run.httpdsig= FALSE; }
                ;

sigenable       : SIGNATURE ENABLE
                | ENABLE SIGNATURE
                ;

sigdisable      : SIGNATURE DISABLE
                | DISABLE SIGNATURE
                ;

bindaddress     : ADDRESS STRING { Run.bind_addr= $2; }
                ;

pemfile         : PEMFILE PATH {
                     Run.httpsslpem= $2;
                     if(!check_file_stat(Run.httpsslpem,
					 "SSL server PEM file", S_IRWXU))
			 yyerror2("SSL server PEM file has too"
                                  " loose permissions");
                  }
                ;

clientpemfile   : /* EMPTY */ 
                | CLIENTPEMFILE PATH {
                     Run.httpsslclientpem= $2; 
		     Run.clientssl= TRUE;
                     if(!check_file_stat(Run.httpsslclientpem,
					 "SSL client PEM file", 
					 S_IRWXU | S_IRGRP | S_IROTH))
			 yyerror2("SSL client PEM file has too"
                                  " loose permissions");
                  }
                ;

allowselfcert   : /* EMPTY */ { 
                   Run.allowselfcert= FALSE; 
                   config_ssl(Run.allowselfcert); 
                 }
                | ALLOWSELFCERTIFICATION {   
                   Run.allowselfcert= TRUE;
                   config_ssl(Run.allowselfcert); 
                  }
                ;

allow           : ALLOW STRING':'STRING readonly {
                   addcredentials($2,$4, DIGEST_CLEARTEXT, $<number>5);
                  }
                | ALLOW PATH { addhtpasswdentry($2, NULL, DIGEST_CLEARTEXT);
                               FREE($2); }
                | ALLOW CLEARTEXT PATH {
                               addhtpasswdentry($3, NULL, DIGEST_CLEARTEXT);
                               FREE($3); }
                | ALLOW MD5HASH PATH {
                               addhtpasswdentry($3, NULL, DIGEST_MD5);
                               FREE($3); }
                | ALLOW CRYPT PATH {
                               addhtpasswdentry($3, NULL, DIGEST_CRYPT);
                               FREE($3); }
                | ALLOW PATH { htpasswd_file= $2;
                               digesttype= CLEARTEXT; }
                  allowuserlist { FREE(htpasswd_file);}
                | ALLOW CLEARTEXT PATH { htpasswd_file= $3;
                                         digesttype= DIGEST_CLEARTEXT; }
                  allowuserlist { FREE(htpasswd_file);}
                | ALLOW MD5HASH PATH { htpasswd_file= $3;
                                        digesttype= DIGEST_MD5; }
                  allowuserlist { FREE(htpasswd_file);}
                | ALLOW CRYPT PATH { htpasswd_file= $3;
                                     digesttype= DIGEST_CRYPT; }
                  allowuserlist { FREE(htpasswd_file);}
                | ALLOW STRING {
		    if(!add_host_allow($2)) {
		      yyerror2("hostname did not resolve",$2);
		    }
		    FREE($2);
		  }
                ;

allowuserlist   : allowuser
                | allowuserlist allowuser
                ;

allowuser       : STRING { addhtpasswdentry(htpasswd_file, $1, digesttype);
                           FREE($1); }
                ;

readonly        : /* EMPTY */ { $<number>$= FALSE; }
                | READONLY { $<number>$= TRUE; }
                ;

checkproc       : CHECKPROC SERVICENAME PIDFILE PATH {
		   check_name($<string>2);
                   createservice(TYPE_PROCESS, $<string>2, $4, check_process);
                  }
                | CHECKPROC SERVICENAME PATHTOK PATH {
		   check_name($<string>2);
                   createservice(TYPE_PROCESS, $<string>2, $4, check_process);
                  }
                ;

checkfile       : CHECKFILE SERVICENAME PATHTOK PATH {
		   check_name($<string>2);
                   createservice(TYPE_FILE, $<string>2, $4, check_file);
                  }
                ;

checkdev        : CHECKDEV SERVICENAME PATHTOK PATH {
		   check_name($<string>2);
                   createservice(TYPE_DEVICE, $<string>2, $4, check_device);
                  }
                ;

checkdir        : CHECKDIR SERVICENAME PATHTOK PATH {
		   check_name($<string>2);
                   createservice(TYPE_DIRECTORY, $<string>2, $4, check_directory);
                  }
                ;

checkhost       : CHECKHOST SERVICENAME ADDRESS STRING {
                   check_hostname($4); 
		   check_name($<string>2);
                   createservice(TYPE_REMOTE, $<string>2, $4, check_remote_host);
                  }
                ;

start           : START argumentlist { addcommand(START); }
                | START argumentlist useroptionlist { addcommand(START); }
                ;

stop            : STOP argumentlist { addcommand(STOP); }
                | STOP argumentlist useroptionlist { addcommand(STOP); }
                ;

argumentlist    : argument
                | argumentlist argument
                ;

useroptionlist  : useroption
                | useroptionlist useroption
                ;

argument        : STRING { addargument($1); }
                | PATH   { addargument($1); }
                ;

useroption      : UID STRING { addeuid( get_uid($2, 0) ); FREE($2); }
                | GID STRING { addegid( get_gid($2, 0) ); FREE($2); }
                | UID NUMBER { addeuid( get_uid(NULL, $2) ); }
                | GID NUMBER { addegid( get_gid(NULL, $2) ); }
                ;

connection      : IF FAILED host port type protocol nettimeout THEN action {
                   portset.timeout= $<number>7;
                   portset.action= $<number>9;
		   addport(&portset);
                  }
                ;

connectionunix  : IF FAILED unixsocket type protocol nettimeout THEN action {
                   portset.timeout= $<number>6;
                   portset.action= $<number>8;
                   addport(&portset);
                  }
                ;

icmp            : IF FAILED ICMP icmptype nettimeout THEN action {
                   icmpset.type= $<number>4;
                   icmpset.timeout= $<number>5;
                   icmpset.action= $<number>7;
                   addicmp(&icmpset);
                  }
                ;

host            : /* EMPTY */ {
                    if(current->type == TYPE_REMOTE)
                      portset.hostname= xstrdup(current->path);
                    else
                      portset.hostname= xstrdup(LOCALHOST);
                  }
                | HOST STRING { check_hostname($2); portset.hostname= $2; }
		;

port            : PORT NUMBER { portset.port= $2; portset.family= AF_INET; }
                ;

unixsocket      : UNIXSOCKET PATH {
                    portset.pathname= $2; portset.family= AF_UNIX;
                  }
                ;

type            : /* EMPTY */ { portset.type= SOCK_STREAM; }
                | TYPE TCP    { portset.type= SOCK_STREAM;  
		                Run.servicessl= TRUE; }
                | TYPE TCPSSL sslversion certmd5  { portset.type= SOCK_STREAM;
		                                    portset.ssl= TRUE; }
                | TYPE UDP    { portset.type= SOCK_DGRAM; }
                ;

certmd5         : /* EMPTY */    { portset.certmd5= NULL; }
                | CERTMD5 STRING { portset.certmd5= $2; }
                ;

sslversion      : /* EMPTY */  { portset.sslversion= SSL_VERSION_AUTO; }
                | SSLV2        { portset.sslversion= SSL_VERSION_SSLV2; }
                | SSLV3        { portset.sslversion= SSL_VERSION_SSLV3; }
                | TLSV1        { portset.sslversion= SSL_VERSION_TLS; }
                | SSLAUTO      { portset.sslversion= SSL_VERSION_AUTO; }
                ;

protocol        : /* EMPTY */  { portset.protocol= addprotocol(P_DEFAULT);}
                | PROTOCOL DEFAULT { portset.protocol= addprotocol(P_DEFAULT); }
                | PROTOCOL HTTP request {portset.protocol= addprotocol(P_HTTP);}
                | PROTOCOL FTP { portset.protocol= addprotocol(P_FTP); }
                | PROTOCOL SMTP { portset.protocol= addprotocol(P_SMTP); }
                | PROTOCOL POP { portset.protocol= addprotocol(P_POP); }
                | PROTOCOL IMAP { portset.protocol= addprotocol(P_IMAP); }
                | PROTOCOL NNTP { portset.protocol= addprotocol(P_NNTP); }
                | PROTOCOL SSH  { portset.protocol= addprotocol(P_SSH); }
                | PROTOCOL DWP  { portset.protocol= addprotocol(P_DWP); }
                | PROTOCOL LDAP2 { portset.protocol= addprotocol(P_LDAP2); }
                | PROTOCOL LDAP3 { portset.protocol= addprotocol(P_LDAP3); }
                | PROTOCOL RDATE { portset.protocol= addprotocol(P_RDATE); }
                | PROTOCOL RSYNC { portset.protocol= addprotocol(P_RSYNC); }
                | sendexpectlist { portset.protocol= addprotocol(P_GENERIC); }
                ;     

sendexpectlist  : sendexpect
                | sendexpectlist sendexpect
                ;

sendexpect      : SEND STRING { addgeneric(&portset, $2, NULL); FREE($2);}
                | EXPECT STRING { addgeneric(&portset, NULL, $2); FREE($2);}
                ;

request         : /* EMPTY */
                | REQUEST PATH  { portset.request= url_encode($2); FREE($2); }
                | REQUEST PATH CHECKSUM STRING {
		   portset.request= url_encode($2); FREE($2);
		   portset.request_checksum= $4;
		  }
                ;

nettimeout      : /* EMPTY */ {
                   $<number>$= NET_TIMEOUT;
                  }
                | TIMEOUT NUMBER SECOND {
		   $<number>$= $2;
		  }
                ;

timeout         : TIMEOUT NUMBER NUMBER {
                   yywarning("The TIMEOUT statement is deprecated."
                             "\n\tPlease use the new version:"
                             "\n\tIF x RESTARTS WITHIN y CYCLES THEN TIMEOUT"
                             "\n\t");
                   check_timeout($2, $3);
		   current->def_timeout= TRUE;
		   current->to_start= $2;
		   current->to_cycle= $3;
                 }
                | IF NUMBER RESTART NUMBER CYCLE THEN TIMEOUT {
                   check_timeout($2, $4);
		   current->def_timeout= TRUE;
		   current->to_start= $2;
		   current->to_cycle= $4;
                 }
                ;


alert           : alertmail '{' eventoptionlist '}' formatlist {
                   addmail($<string>1, &mailset, &current->maillist, eventset);
                 }
                | alertmail formatlist {
                   addmail($<string>1, &mailset, &current->maillist, eventset);
		 }
                ;

alertmail       : ALERT MAILADDR { $<string>$= $2; }
		;

eventoptionlist : eventoption
                | eventoptionlist eventoption
                ;

eventoption     : START      { eventset |= EVENT_START; }
                | STOP       { eventset |= EVENT_STOP; }
                | RESTART    { eventset |= EVENT_RESTART; }
                | CHECKSUM   { eventset |= EVENT_CHECKSUM; }
                | RESOURCE   { eventset |= EVENT_RESOURCE; }
                | TIMEOUT    { eventset |= EVENT_TIMEOUT; }
                | TIMESTAMP  { eventset |= EVENT_TIMESTAMP; }
                | SIZE       { eventset |= EVENT_SIZE; }
                | CONNECTION { eventset |= EVENT_CONNECTION; }
                | PERMISSION { eventset |= EVENT_PERMISSION; }
                | UID        { eventset |= EVENT_UID; }
                | GID        { eventset |= EVENT_GID; }
                | UNMONITOR  { eventset |= EVENT_UNMONITOR; }
                ;

formatlist      : /* EMPTY */
	        | MAILFORMAT '{' formatoptionlist '}'
                ;

formatoptionlist: formatoption
                | formatoptionlist formatoption
                ;

formatoption    : MAILFROM { mailset.from= $1; }
                | MAILSUBJECT { mailset.subject= $1; }
                | MAILBODY { mailset.message= $1; }
                ;

every           : EVERY NUMBER CYCLE {
                   check_every($2);
		   current->def_every= TRUE;
		   current->every= $2;
                 }
                ;

mode            : MODE ACTIVE  { current->mode= MODE_ACTIVE; }
                | MODE PASSIVE { current->mode= MODE_PASSIVE; }
                | MODE MANUAL  { current->mode= MODE_MANUAL;
		    current->do_monitor= FALSE;
		  }
		;

group		: GROUP STRING { current->group= $2; }
		;

depend          : DEPENDS dependlist
                ;

dependlist      : dependant
                | dependlist dependant
                ;
 
dependant       : SERVICENAME { adddependant($<string>1); }
                ;

resourcesystem  : IF resource resourcecycle THEN action {
                   resourceset.action= $<number>5;
		   addresource(&resourceset);
		  }
                ;

resource        : /* Old syntax */
                  CPUUSAGE operator value { 
		    yyerror("CPUUSAGE is obsolete -"
                            " use the CPU statement instead");
                  }
                | MEMUSAGE operator value { 
		    yyerror("MEMUSAGE is obsolete -"
			   " use the MEMORY statement instead");
		  }
                | MEMKBYTE operator value { 
		    yyerror("MEMKBYTE is obsolete -"
                            " use the MEMORY statement instead");
		  }
                /* New syntax */
                | MEMORY operator value unit {
                    resourceset.resource_id= RESOURCE_ID_MEM_KBYTE;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 
					      ($<number>4 / 1024.0)); 
                  }
                | MEMORY operator PERCENT {
                    resourceset.resource_id= RESOURCE_ID_MEM_PERCENT;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 10); 
                  }
                | TOTALMEMORY operator value unit {
                    resourceset.resource_id= RESOURCE_ID_TOTAL_MEM_KBYTE;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 
					      ($<number>4 / 1024.0)); 
#ifdef LINUX
                    yywarning("TOTALMEMORY statement "
                              "does not work properly on Linux\n",
                              prog, lineno);
#endif
                  }
                | TOTALMEMORY operator PERCENT  {
                    resourceset.resource_id= RESOURCE_ID_TOTAL_MEM_PERCENT;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 10);
#ifdef LINUX
                    yywarning("TOTALMEMORY statement"
                              "does not work properly on Linux.\n",
                              prog, lineno);
#endif
                  }
                | CPU operator PERCENT {
                    resourceset.resource_id= RESOURCE_ID_CPU_PERCENT;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 10); 
                  }
                /* Remaining syntax */
                | CHILDREN operator NUMBER { 
                    resourceset.resource_id= RESOURCE_ID_CHILDREN;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) $3; 
		  }
                | resourceload operator value { 
                    resourceset.resource_id= $<number>1;
                    resourceset.operator= $<number>2;
		    resourceset.limit= (int) ($<real>3 * 10.0); 
		  }
                ;

value           : REAL { $<real>$ = $1; }
                | NUMBER { $<real>$ = (float) $1; }
                ;

resourceload    : LOADAVG1  { $<number>$= RESOURCE_ID_LOAD1; }
                | LOADAVG5  { $<number>$= RESOURCE_ID_LOAD5; }
                | LOADAVG15 { $<number>$= RESOURCE_ID_LOAD15; }
                ;

resourcecycle   : /* EMPTY */
                | NUMBER CYCLE { resourceset.max_cycle= $1; }
                ;

timestamp       : IF TIMESTAMP operator NUMBER time THEN action {
		    timestampset.operator= $<number>3;
		    timestampset.time= ($4 * $<number>5);
		    timestampset.action= $<number>7;
		    timestampset.test_changes= FALSE;
		    addtimestamp(&timestampset);
                  }
                | IF TIMESTAMP CHANGED THEN action {
		    timestampset.test_changes= TRUE;
		    timestampset.action= $<number>5;
		    addtimestamp(&timestampset);
                  }
                ;

operator        : /* EMPTY */ { $<number>$= OPERATOR_EQUAL; }
                | GREATER     { $<number>$= OPERATOR_GREATER; }
                | LESS        { $<number>$= OPERATOR_LESS; }
                | EQUAL       { $<number>$= OPERATOR_EQUAL; }
                | NOTEQUAL    { $<number>$= OPERATOR_NOTEQUAL; }
                | CHANGED     { $<number>$= OPERATOR_NOTEQUAL; }
                ;

time            : /* EMPTY */ { $<number>$= TIME_SECOND; }
                | SECOND      { $<number>$= TIME_SECOND; }
                | MINUTE      { $<number>$= TIME_MINUTE; }
                | HOUR        { $<number>$= TIME_HOUR; }
                | DAY         { $<number>$= TIME_DAY; }
                ;

action          : ALERT       { $<number>$= ACTION_ALERT; }
                | RESTART     { $<number>$= ACTION_RESTART; }
                | STOP        { $<number>$= ACTION_STOP; }
                | EXEC argumentlist { $<number>$= ACTION_EXEC; }
                | EXEC argumentlist useroptionlist { $<number>$= ACTION_EXEC; }
		| UNMONITOR   { $<number>$= ACTION_UNMONITOR; }
                ;

checksum        : IF FAILED hashtype CHECKSUM THEN action {
                    checksumset.action= $<number>6;
                    addchecksum(&checksumset);
                  }
                | IF FAILED hashtype CHECKSUM EXPECT STRING THEN action {
                    checksumset.hash= $6;
                    checksumset.action= $<number>8;
                    addchecksum(&checksumset);
                  }
                ;
hashtype        : /* EMPTY */ { checksumset.type= 0; }
                | MD5HASH { checksumset.type= HASH_MD5; }
                | SHA1HASH { checksumset.type= HASH_SHA1; }
                ;

inode           : IF INODE operator NUMBER THEN action {
                    deviceset.resource= RESOURCE_ID_INODE;
                    deviceset.operator= $<number>3;
                    deviceset.limit_absolute= $4;
                    deviceset.action= $<number>6;
                    adddevice(&deviceset);
                  }
                | IF INODE operator PERCENT THEN action {
                    deviceset.resource= RESOURCE_ID_INODE;
                    deviceset.operator= $<number>3;
                    deviceset.limit_percent= (int) $<real>4;
                    deviceset.action= $<number>6;
                    adddevice(&deviceset);
                  }
                ;

space           : IF SPACE operator value unit THEN action {
                    if(!DeviceInfo_Usage(current->devinfo, current->path)) {
                      yyerror2("cannot read usage of device %s",
                               current->path);
		    }
                    deviceset.resource= RESOURCE_ID_SPACE;
                    deviceset.operator= $<number>3;
                    deviceset.limit_absolute=
			(int)((float)$<real>4 /
			      (float)current->devinfo->f_bsize * 
			      (float)$<number>5 );
                    deviceset.action= $<number>7;
                    adddevice(&deviceset);
                  }
                | IF SPACE operator PERCENT THEN action {
                    deviceset.resource= RESOURCE_ID_SPACE;
                    deviceset.operator= $<number>3;
                    deviceset.limit_percent= (int) $<real>4;
                    deviceset.action= $<number>6;
                    adddevice(&deviceset);
                  }
                ;

unit            : BYTE     { $<number>$= UNIT_BYTE; }
                | KILOBYTE { $<number>$= UNIT_KILOBYTE; }
                | MEGABYTE { $<number>$= UNIT_MEGABYTE; }
                | GIGABYTE { $<number>$= UNIT_GIGABYTE; }
                ;

permission      : IF FAILED PERMISSION NUMBER THEN action {
                    permset.perm= check_perm($4);
		    permset.action= $<number>6;
		    addperm(&permset);
		  }
                ;

size            : IF SIZE operator NUMBER unit THEN action {
		    sizeset.operator= $<number>3;
		    sizeset.size= ((unsigned long)$4 * $<number>5);
		    sizeset.action= $<number>7;
		    sizeset.test_changes= FALSE;
		    addsize(&sizeset);
                  }
                | IF SIZE CHANGED THEN action {
		    sizeset.test_changes= TRUE;
		    sizeset.action= $<number>5;
		    addsize(&sizeset);
                  }
                ;

uid             : IF FAILED UID STRING THEN action {
                    uidset.uid= get_uid($4, 0);
		    uidset.action= $<number>6;
		    adduid(&uidset);
                    FREE($4);
		  }
                | IF FAILED UID NUMBER THEN action {
                    uidset.uid= get_uid(NULL, $4);
		    uidset.action= $<number>6;
		    adduid(&uidset);
		  }
                ;

gid             : IF FAILED GID STRING THEN action {
                    gidset.gid= get_gid($4, 0);
		    gidset.action= $<number>6;
		    addgid(&gidset);
                    FREE($4);
		  }
                | IF FAILED GID NUMBER THEN action {
                    gidset.gid= get_gid(NULL, $4);
		    gidset.action= $<number>6;
		    addgid(&gidset);
		  }
                ;

icmptype        : TYPE ICMPECHO { $<number>$= ICMP_ECHO; }
                ;

%%


/* -------------------------------------------------------- Parser interface */


/**
 * Syntactic error routine
 *
 * This routine is automatically called by the lexer!
 */
void yyerror(const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;

  ASSERT(s);
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);
  
  log("%s:%i: Error: %s '%s'\n", currentfile, lineno, msg, yytext);
  cfg_errflag++;
 
  FREE(msg);
  
}

/**
 * Syntactical warning routine
 */
void yywarning(const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;

  ASSERT(s);
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);
  
  log("%s:%i: Warning: %s '%s'\n", currentfile, lineno, msg, yytext);
 
  FREE(msg);
  
}

/**
 * Argument error routine
 */
void yyerror2(const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;

  ASSERT(s);
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);
  
  log("%s:%i: Error: %s '%s'\n", argcurrentfile, arglineno, msg, argyytext);
  cfg_errflag++;
 
  FREE(msg);
  
}

/**
 * Argument warning routine
 */
void yywarning2(const char *s, ...) {
  
  long len;
  va_list ap;
  char *msg= NULL;

  ASSERT(s);
  
  va_start(ap,s);
  msg= format(s, ap, &len);
  va_end(ap);
  
  log("%s:%i: Warning: %s '%s'\n", argcurrentfile, arglineno, msg, argyytext);
 
  FREE(msg);
  
}

/*
 * The Parser hook - start parsing the control file
 * Returns TRUE if parsing succeeded, otherwise FALSE
 */
int parse(char *controlfile) {

  ASSERT(controlfile);

  servicelist= tail= current= NULL;
  
  if(! check_rcfile(controlfile)) {
    
    return (FALSE);
    
  }
  
  if ((yyin = fopen(controlfile,"r")) == (FILE *)NULL) {
    
    log("%s: Error: cannot open the control file '%s' -- %s\n",
	prog, controlfile, STRERROR);
    return(FALSE);
    
  }

  currentfile=xstrdup(controlfile);
  
  /* Creation of the global service list is synchronized  */
  LOCK(Run.mutex)
      
      initialize();
      yyparse();
      fclose(yyin);

      /* If defined - add the last service to the service list */
      if(current)
	addservice(current);
      
  END_LOCK;

  FREE(currentfile);

  if (argyytext!=NULL)
      FREE(argyytext);
  
  postparse();
    
  return(cfg_errflag == 0);
  
}


/* ----------------------------------------------------------------- Private */


/**
 * Initialize objects. Placeholder for setting values that can change
 * between cycles (changed config file) and not easily supported by
 * the grammar implementation.
 */
static void initialize() {

  /* Reset lexer */
  buffer_stack_ptr=0;
  lineno= 1;
  arglineno= 1;
  argcurrentfile=NULL;
  argyytext=NULL;
  
  /* Reset parser */
  Run.stopped= FALSE;
  Run.dolog= FALSE;
  Run.dohttpd= FALSE;
  Run.httpdsig= TRUE;
  Run.credentials= NULL;
  Run.httpdssl= FALSE;
  Run.servicessl= FALSE;
  Run.clientssl= FALSE;
  Run.mailserver_timeout= NET_TIMEOUT;
  Run.bind_addr= NULL;
  Run.maillist= NULL;
  Run.mailservers= NULL;
  Run.MailFormat.from= NULL;
  Run.MailFormat.subject= NULL;
  Run.MailFormat.message= NULL;
  depend_list= NULL;
  
}


/*
 * Check that values are reasonable after parsing
 */
static void postparse() {

  Service_T s;
  
  if(cfg_errflag || (servicelist==NULL)) {
    return;
  }

  /* Check the sanity of any dependency graph */
  check_depend();

  /* Check that we do not start monit in daemon mode without having a
   * poll time */
  if(!Run.polltime && (Run.isdaemon || Run.init)) {
    log("%s: Error: Poll time not defined. Please define poll time"
	" in the\n control file or use the -d option when starting monit\n",
	prog);
    cfg_errflag++;
  }

  /* Verify that a remote service has a port or an icmp list */
  for(s= servicelist; s; s= s->next) {
    if(s->type != TYPE_REMOTE)
	continue;
    if(!s->portlist && !s->icmplist) {
      log("%s: Error: 'check host' statement is incomplete; Please specify a"
          " port number to test\n or an icmp test at the remote host: '%s'\n",
	  prog, s->name);
      cfg_errflag++;
    }
  }

}


/*
 * Create a new service object and add any current objects to the
 * service list.
 */
static void createservice(int type, char *name, char *value, int (*check)(Service_T s)) {

  ASSERT(name);
  ASSERT(value);

  if(current) {
    addservice(current);
  } else {
    NEW(current);
  }

  /* Reset the current object */
  memset(current, 0, sizeof(*current));

  /* Set default values */
  current->do_monitor= TRUE;
  current->mode= MODE_ACTIVE;

  current->type= type;
  current->name= name;
  current->path= value;
  current->check= check;

  if(type == TYPE_DEVICE) {
    createdevinfo();
  } else if(type == TYPE_PROCESS) {
    createprocinfo();
  }

  pthread_mutex_init(&current->mutex, NULL);

}


/*
 * Add a service object to the servicelist
 */
static void addservice(Service_T s) {
  
  Service_T n;

  ASSERT(s);
 
  NEW(n);
  memcpy(n, s, sizeof(*s));

  /* Add the service to the end of the service list */
  if(tail != NULL) {
    
    tail->next= n;
    tail->next_conf= n;
    
  } else {
    
    servicelist= n;
    servicelist_conf= n;
    
  }
  
  tail= n;

}


/* 
 * Add a dependant entry to the current service dependant list
 *
 */
static void adddependant(char *dependant) {

  Dependant_T d; 

  ASSERT(dependant);
  
  NEW(d);
  
  if (current->dependantlist != NULL) {
    d->next= current->dependantlist;
  }

  d->dependant=dependant;
  current->dependantlist= d;

}


/*
 * Add the given mailaddress with the apropriate alert notification
 * values and mail attributes to the given mailinglist.
 */
static void addmail(char *mailto, Mail_T f, Mail_T *l, unsigned int events) {

  Mail_T m;

  ASSERT(mailto);

  NEW(m);
  if(events != EVENT_NULL) {
    m->events= events;
  } else {
    /* Turn on all events */
    m->events= ~((unsigned int)0);
  }
  
  m->to= mailto;
  m->from= f->from;
  m->subject= f->subject;
  m->message= f->message;
  
  m->next= *l;
  *l= m;

  reset_mailset();

}


/*
 * Add the given portset to the current service's portlist
 */
static void addport(struct PortSet *pp) {

  Port_T p;
  char address[STRLEN];

  ASSERT(pp);

  NEW(p);
  p->port= pp->port;
  p->type= pp->type;
  p->socket= pp->socket;
  p->family= pp->family;
  p->action= pp->action;
  p->timeout= pp->timeout;
  p->request= pp->request;
  p->generic= pp->generic;
  p->protocol= pp->protocol;
  p->pathname= pp->pathname;
  p->hostname= pp->hostname;
  p->request_checksum= pp->request_checksum;

  if (p->request_checksum) {
    cleanup_hash_string(p->request_checksum);

    if (strlen(p->request_checksum)==32) {
      p->request_hashtype=HASH_MD5;
    } else if (strlen(p->request_checksum)==40) {
      p->request_hashtype=HASH_SHA1;
    } else {
      yyerror2("invalid checksum [%s]", p->request_checksum);
    }
  } else {
    p->request_hashtype=0;
  }

  if(pp->family == AF_INET) {
    snprintf(address, STRLEN, "INET[%s:%d]", pp->hostname, pp->port);
  } else if(pp->family == AF_UNIX) {
    snprintf(address, STRLEN, "UNIX[%s]", pp->pathname);
  }
  p->address= xstrdup(address);
  
  if(pp->ssl == TRUE) {
    if(!have_ssl()) {
      yyerror("ssl check cannot be activated. SSL is not supported");
    } else {
      if (pp->certmd5 != NULL) {
	p->SSL.certmd5= pp->certmd5;
	cleanup_hash_string(p->SSL.certmd5);
      }
      p->SSL.use_ssl= TRUE;
      p->SSL.version= pp->sslversion;
    }
  }
  
  if((p->action == ACTION_EXEC) && command) {
    p->exec= command;
    command= NULL;
  }
  
  p->next= current->portlist;
  current->portlist= p;
  
  reset_portset();

}


/*
 * Add a new resource object to the current service resource list
 */
static void addresource(struct ResourceSet *rr) {

  Resource_T r;

  ASSERT(rr);

  NEW(r);
  if(! Run.doprocess) {

    yyerror("cannot activate service check.\n"
            "\t(The process status engine was disabled. On certain"
            " systems you must\n\trun monit as root to utilize this"
            " feature)\n\t");
    
  }

  r->resource_id= rr->resource_id;
  r->limit= rr->limit;
  r->cycle= 0;
  r->max_cycle= rr->max_cycle;
  r->action= rr->action;
  r->operator= rr->operator;
  r->next= current->resourcelist;

  if(r->max_cycle < 1) {

    yyerror2("the number of cycles must be greater then 0");
    
  }

  if((r->action == ACTION_EXEC) && command) {
    r->exec= command;
    command= NULL;
  }
  
  current->resourcelist= r;
  reset_resourceset();

}


/*
 * Add a new file object to the current service timestamp list
 */
static void addtimestamp(struct TimestampSet *ts) {

  Timestamp_T t;

  ASSERT(ts);

  NEW(t);
  t->operator= ts->operator;
  t->time= ts->time;
  t->action= ts->action;
  t->test_changes= ts->test_changes;

  if(t->test_changes) {
    if(!exist_file(current->path)) {
      yyerror2("the path '%s' used in the TIMESTAMP statement"
               " does not exist", current->path);
    } else if(!(t->timestamp= get_timestamp(current->path, S_IFDIR|S_IFREG))) {
      yyerror2("cannot get the timestamp for '%s'", current->path);
    }
  }
  
  if((t->action == ACTION_EXEC) && command) {
    t->exec= command;
    command= NULL;
  }

  t->next= current->timestamplist;
  current->timestamplist= t;

  reset_timestampset();

}


/*
 * Add a new Size object to the current service size list
 */
static void addsize(struct SizeSet *ss) {

  struct stat buf;
  Size_T s;

  ASSERT(ss);

  if(ss->test_changes && stat(current->path, &buf) != 0) {
    yyerror2("cannot get size for '%s'", current->path);
    reset_sizeset();
    return;
  }
 
  NEW(s);
  s->operator= ss->operator;
  s->size= ss->size;
  s->action= ss->action;
  s->test_changes= ss->test_changes;
  if(ss->test_changes)
    s->runsize= (unsigned long)buf.st_size;
 
  if((s->action == ACTION_EXEC) && command) {
    s->exec= command;
    command= NULL;
  }
 
  s->next= current->sizelist;
  current->sizelist= s;

  reset_sizeset();
}


/*
 * Set Checksum object in the current service
 */
static void addchecksum(struct ChecksumSet *cs) {

  Checksum_T c;
  int len;
  ASSERT(cs);

  if(!cs->hash) {
    if (!cs->type) {
      cs->type=DEFAULT_HASH;
    }
    if( !(cs->hash= get_checksum(current->path, cs->type))) {
      yyerror2("cannot compute a checksum for a file %s", current->path);
      reset_checksumset();
      return;
    }
  }

  len=cleanup_hash_string(cs->hash);

  if (!cs->type) {
    if (len==32) {
      cs->type=HASH_MD5;
    } else if (len==40) {
      cs->type=HASH_SHA1;
    } else {
      yyerror2("invalid checksum [%s] for a file %s", cs->hash, current->path);
      reset_checksumset();
      return;
    }
  } else if (( cs->type==HASH_MD5 && len!=32 ) ||
             ( cs->type==HASH_SHA1 && len!=40 )) {
    yyerror2("invalid checksum [%s] for a file %s", cs->hash, current->path);
    reset_checksumset();
    return;
  }

  NEW(c);

  c->type=cs->type;
  c->hash= cs->hash;
  c->action= cs->action;
 
  if((c->action == ACTION_EXEC) && command) {
    c->exec= command;
    command= NULL;
  }
 
  current->checksum= c;

  reset_checksumset();

}


/*
 * Set Perm object in the current service
 */
static void addperm(struct PermSet *ps) {

  Perm_T p;

  ASSERT(ps);

  NEW(p);
  p->perm= ps->perm;
  p->action= ps->action;
 
  if((p->action == ACTION_EXEC) && command) {
    p->exec= command;
    command= NULL;
  }
 
  current->perm= p;

  reset_permset();

}


/*
 * Set Uid object in the current service
 */
static void adduid(struct UidSet *us) {

  Uid_T u;

  ASSERT(us);

  NEW(u);
  u->uid= us->uid;
  u->action= us->action;
 
  if((u->action == ACTION_EXEC) && command) {
    u->exec= command;
    command= NULL;
  }
 
  current->uid= u;

  reset_uidset();

}


/*
 * Set Gid object in the current service
 */
static void addgid(struct GidSet *gs) {

  Gid_T g;

  ASSERT(gs);

  NEW(g);
  g->gid= gs->gid;
  g->action= gs->action;
 
  if((g->action == ACTION_EXEC) && command) {
    g->exec= command;
    command= NULL;
  }
 
  current->gid= g;

  reset_gidset();

}


/*
 * Add a new device to the current service's device list
 */
static void adddevice(struct DeviceSet *ds) {

  Device_T dev;

  ASSERT(ds);
  
  NEW(dev);
  dev->resource= ds->resource;
  dev->operator= ds->operator;
  dev->limit_absolute= ds->limit_absolute;
  dev->limit_percent= ds->limit_percent;
  dev->action= ds->action;

  if((dev->action == ACTION_EXEC) && command) {
    dev->exec= command;
    command= NULL;
  }

  dev->next= current->devicelist;
  current->devicelist= dev;

  reset_deviceset();

}


/*
 * Add a new icmp object to the current service's icmp list
 */
static void addicmp(struct IcmpSet *is) {

  if(!getuid()) {
    
    Icmp_T icmp;
    
    ASSERT(is);
    
    NEW(icmp);
    icmp->type= is->type;
    icmp->timeout= is->timeout;
    icmp->action= is->action;
    
    if((icmp->action == ACTION_EXEC) && command) {
      icmp->exec= command;
      command= NULL;
    }
    
    icmp->next= current->icmplist;
    current->icmplist= icmp;
    
  } else {
    
    yyerror("icmp statements must be run as root");
    
  }
  
  reset_icmpset();

}


/*
 * Adds runtime device info to current service
 */
static void createdevinfo() {

  DeviceInfo_T di;

  NEW(di);
  current->devinfo=di;
  
}


/*
 * Adds runtime process info to current service
 */
static void createprocinfo() {

  ProcInfo_T pi;

  NEW(pi);
  current->procinfo=pi;
  
}


/*
 * Return a protocol object for the given protocol
 */
static void *addprotocol(int protocol) {

  switch (protocol) {
  case P_HTTP:    return create_http();
  case P_FTP:     return create_ftp();
  case P_SMTP:    return create_smtp();
  case P_POP:     return create_pop();
  case P_IMAP:    return create_imap();
  case P_NNTP:    return create_nntp();
  case P_SSH:     return create_ssh();
  case P_DWP:     return create_dwp();
  case P_LDAP2:   return create_ldap2();
  case P_LDAP3:   return create_ldap3();
  case P_RDATE:   return create_rdate();
  case P_RSYNC:   return create_rsync();
  case P_GENERIC: return create_generic();
  }

  return create_default();
   
}

/*
 * Add a generic protocol handler to 
 */
static void addgeneric(struct PortSet *pp, char *send, char *expect) {

  Generic_T g=pp->generic;

  if ( g == NULL ) {

    NEW(g);
    pp->generic=g;
    
  } else {
  
    while (g->next != NULL) {
      
      g=g->next;
    
    }

    NEW(g->next);
    g=g->next;
  }
  
  if ( send != NULL ) {

    g->send=xstrdup(send);
    g->expect=NULL;
        
  } else if ( expect != NULL ) {

#ifdef HAVE_REGEX_H
    int   reg_return;

    NEW(g->expect);
    
    reg_return= regcomp(g->expect, expect, REG_NOSUB|REG_EXTENDED);

    if (reg_return!=0) {

      char errbuf[STRLEN];

      regerror(reg_return, g->expect, errbuf, STRLEN);
      yyerror2("regex parsing error:%s", errbuf);
      
    }

#else

    g->expect=xstrdup(expect);

#endif

    g->send=NULL;
    
  } 
  
}


/*
 * Add the current command object to the current service object's
 * start or stop program.
 */
static void addcommand(int what) {

  switch(what) {
  case START: current->start= command; break;
  case STOP:  current->stop= command; break;
  }
  
  command= NULL;
  
}

  
/*
 * Add a new argument to the argument list
 */
static void addargument(char *argument) {

  ASSERT(argument);

  if(! command) {
    
    NEW(command);
    check_exec(argument);
    
  }
  
  command->arg[command->length++]= argument;
  command->arg[command->length]= NULL;
  
  if(command->length >= ARGMAX) {
    
    yyerror("exceeded maximum number of program arguments");
    
  }
  
}


/*
 * Add a new smtp server to the mail server list
 */
static void addmailserver(char *host) {

  MailServer_T s;
  
  ASSERT(host);

  NEW(s);
  s->host= host;
  s->next= NULL;

  if(Run.mailservers) {
    MailServer_T l;
    for(l= Run.mailservers; l->next; l= l->next) /* empty */;
    l->next= s;
  } else {
    Run.mailservers= s;
  }
  
}


/*
 * Return uid if found on the system. If the parameter user is NULL
 * the uid parameter is used for looking up the user id on the system,
 * otherwise the user parameter is used.
 */
static uid_t get_uid(char *user, uid_t uid) {

  struct passwd *pwd;

  if(user) {

    pwd= getpwnam(user);

    if(pwd == NULL) {
      yyerror2("requested user not found on the system");

      return(0);
    }

  } else {

    if( (pwd= getpwuid(uid)) == NULL ) {
      yyerror2("requested uid not found on the system");

      return(0);
    }
  }

  return(pwd->pw_uid);

}


/*
 * Return gid if found on the system. If the parameter group is NULL
 * the gid parameter is used for looking up the group id on the system,
 * otherwise the group parameter is used.
 */
static gid_t get_gid(char *group, gid_t gid) {

  struct group *grd;

  if(group) {

    grd= getgrnam(group);

    if(grd == NULL) {
      yyerror2("requested group not found on the system");
      
      return(0);
    }

  } else {

    if( (grd= getgrgid(gid)) == NULL ) {
      yyerror2("requested gid not found on the system");

      return(0);
    }

  }

  return(grd->gr_gid);

}


/*
 * Add a new user id to the current command object.
 */
static void addeuid(uid_t uid) {

  if(!getuid()) {

    command->has_uid= TRUE;
    command->uid= uid;

  } else {
    
    yyerror("uid statement requires root privileges");

  }

}


/*
 * Add a new group id to the current command object.
 */
static void addegid(gid_t gid) {

  if(!getuid()) {

    command->has_gid= TRUE;
    command->gid= gid;

  } else {

    yyerror("gid statement requires root privileges");

  }

}


/*
 * Reset the logfile if changed
 */
static void setlogfile(char *logfile) {

  if(Run.logfile) {
    
    if(IS(Run.logfile, logfile)) {
      
      FREE(logfile);
      return;
      
    } else {
      
      FREE(Run.logfile);
      
    }
  }
  
  Run.logfile= logfile;
 
}


/*
 * Reset the pidfile if changed
 */
static void setpidfile(char *pidfile) {

  if(Run.pidfile) {
    
    if(IS(Run.pidfile, pidfile)) {
      
      FREE(pidfile);
      return;
      
    } else {
      
      FREE(Run.pidfile);
      
    }
  }
  
  Run.pidfile= pidfile;
 
}


/*
 * Read a apache htpasswd file and add credentials found for username
 */
static void addhtpasswdentry(char *filename, char *username, int dtype) {

  char *ht_username;
  char *ht_passwd;
  char buf[STRLEN];
  FILE *handle;
  int credentials_added= 0;
  
  ASSERT(filename);

  handle=fopen(filename, "r");

  if ( handle==NULL ) {

    if (username!=NULL) {
      yyerror2("cannot read htpasswd (%s)", filename);
    } else {
      yyerror2("cannot read htpasswd", filename);
    }
    return;
    
  }
  
  while (!feof(handle)) {
    char *colonindex;
    int i;
    
    if (fgets(buf, STRLEN, handle)<=0) {

      continue;
      
    }

    /* strip trailing non visible characters */
    for (i=strlen(buf)-1; i >= 0; i--) {
      if ( buf[i] == ' '  || buf[i] == '\r' ||
           buf[i] == '\n' || buf[i] == '\t' ) {
        
        buf[i]='\0';
        
      } else {
        
        break;
        
      }
    }

    if ( NULL == (colonindex=strchr(buf, ':'))) {

      continue;
      
    }

    ht_passwd=xstrdup(colonindex+1);
    *colonindex='\0';

    /* Incase we have a file in /etc/passwd or /etc/shadow style we
     *  want to remove ":.*$" and Crypt and MD5 hashed dont have a colon
     */ 
    
    if ( (NULL != (colonindex=strchr(ht_passwd, ':'))) &&
         ( dtype != DIGEST_CLEARTEXT) ) {

      *colonindex='\0';
      
    }

    ht_username=xstrdup(buf);

    if (username==NULL) {
      
      if (addcredentials(ht_username, ht_passwd, dtype, FALSE)) {  
        credentials_added++;
      }
      
    } else if (strcmp(username, ht_username) == 0)  {
      
      if (addcredentials(ht_username, ht_passwd, dtype, FALSE)) {
        credentials_added++;
      }
      
    } else {

      FREE(ht_passwd);
      FREE(ht_username);
            
    }
  }

  if (credentials_added==0) {

    if ( username == NULL ) {
      
      yywarning2("htpasswd file (%s) has no usable credentials",
          filename);

    } else {

      yywarning2("htpasswd file (%s) has no usable credentials "
                 "for user %s", filename, username);

    }
    
    
  }

  fclose(handle);
  
}


/*
 * Add Basic Authentication credentials
 */
static int addcredentials(char *uname, char *passwd, int dtype, int readonly) {

  Auth_T c;

  ASSERT(uname);
  ASSERT(passwd);

  if (Run.credentials == NULL) {

    NEW(Run.credentials);
    c=Run.credentials;
    
  } else {

    if (get_user_credentials(uname) != NULL) {

      yywarning2("credentials for user %s were already added, entry ignored",
                 uname);
      FREE(uname);
      FREE(passwd);
      return FALSE;
      
    }

    c=Run.credentials;

    while ( c->next != NULL ) {

      c=c->next;
      
    }

    NEW(c->next);
    c=c->next;
        
  }
  
  c->next=NULL;
  c->uname=uname;
  c->passwd=passwd;
  c->digesttype=dtype;
  c->is_readonly= readonly;
  
  DEBUG("%s: Debug: Adding credenitials for user '%s'.\n", prog, uname); 
  
  return TRUE;
  
}


/*
 * Set the syslog and the facilities to be used
 */
static void setsyslog(char *facility) {

  if (!Run.logfile || ihp.logfile) {

    ihp.logfile= TRUE;
    setlogfile(xstrdup("syslog"));
    Run.use_syslog= TRUE;
    Run.dolog=TRUE;

  }

  if(facility) {
    if(IS(facility,"log_local0")) {
      Run.facility = LOG_LOCAL0;
    } else if(IS(facility, "log_local1")) {
      Run.facility = LOG_LOCAL1;
    } else if(IS(facility, "log_local2")) {
      Run.facility = LOG_LOCAL2;
    } else if(IS(facility, "log_local3")) {
      Run.facility = LOG_LOCAL3;
    } else if(IS(facility, "log_local4")) {
      Run.facility = LOG_LOCAL4;
    } else if(IS(facility, "log_local5")) {
      Run.facility = LOG_LOCAL5;
    } else if(IS(facility, "log_local6")) {
      Run.facility = LOG_LOCAL6;
    } else if(IS(facility, "log_local7")) {
      Run.facility = LOG_LOCAL7;
    } else if(IS(facility, "log_daemon")) {
      Run.facility = LOG_DAEMON;
    } else {
      yyerror2("invalid syslog facility");
    }
  } else {
    Run.facility= LOG_USER;
  }
  
}


/*
 * Reset the current mailset and eventset for reuse
 */
static void reset_mailset() {

  memset(&mailset, 0, sizeof(struct mymail));
  eventset= 0;

}


/*
 * Reset the Port set to default values
 */
static void reset_portset() {

  portset.socket= -1;
  portset.hostname= NULL;
  portset.port= 0;
  portset.type= SOCK_STREAM;
  portset.family= AF_INET;
  portset.ssl= FALSE;
  portset.sslversion= SSL_VERSION_AUTO;
  portset.request= NULL;
  portset.request_checksum= NULL;
  portset.generic= NULL;
  portset.protocol= NULL;
  portset.pathname= NULL;
  portset.action= ACTION_ALERT;
  portset.timeout= NET_TIMEOUT;

}


/*
 * Reset the Proc set to default values
 */
static void reset_resourceset() {

  resourceset.resource_id= 0;
  resourceset.limit= 0;
  resourceset.max_cycle= 1;
  resourceset.action= ACTION_ALERT;
  resourceset.operator= OPERATOR_EQUAL;

}


/*
 * Reset the Timestamp set to default values
 */
static void reset_timestampset() {

  timestampset.operator= OPERATOR_EQUAL;
  timestampset.time= 0;
  timestampset.test_changes= 0;
  timestampset.action= ACTION_ALERT;

}


/*
 * Reset the Size set to default values
 */
static void reset_sizeset() {

  sizeset.operator= OPERATOR_EQUAL;
  sizeset.size= 0;
  sizeset.test_changes= 0;
  sizeset.action= ACTION_ALERT;

}


/*
 * Reset the Checksum set to default values
 */
static void reset_checksumset() {

  checksumset.hash= NULL;
  checksumset.type= 0;
  checksumset.action= ACTION_ALERT;

}


/*
 * Reset the Perm set to default values
 */
static void reset_permset() {

  permset.perm= 0;
  permset.action= ACTION_ALERT;

}


/*
 * Reset the Uid set to default values
 */
static void reset_uidset() {

  uidset.uid= 0;
  uidset.action= ACTION_ALERT;

}


/*
 * Reset the Gid set to default values
 */
static void reset_gidset() {

  gidset.gid= 0;
  gidset.action= ACTION_ALERT;

}


/*
 * Reset the Device set to default values
 */
static void reset_deviceset() {

  deviceset.resource= 0;
  deviceset.operator= OPERATOR_EQUAL;
  deviceset.limit_absolute= -1;
  deviceset.limit_percent= -1;
  deviceset.action= ACTION_ALERT;

}


/*
 * Reset the ICMP set to default values
 */
static void reset_icmpset() {

  icmpset.type= ICMP_ECHO;
  icmpset.timeout= NET_TIMEOUT;
  icmpset.action= ACTION_ALERT;

}


/* ---------------------------------------------------------------- Checkers */


/*
 * Check for unique service name
 */
static void check_name(char *name) {

  ASSERT(name);

  if(exist_service(name) || (current && IS(name, current->name))) {
    yyerror2("service name conflict, %s already defined", name);
  }
  if(name && *name=='/') {
    yyerror2("service name must not start with '/' -- ", name);
  }
  
}


/*
 * Permission statement semantic check
 */
static int check_perm(int perm) {

  long result;
  char *status;
  char buf[STRLEN];

  snprintf(buf, STRLEN, "%d", perm);

  result= strtol(buf, &status, 8);

  if( *status != '\0' || result < 0 || result > 07777 ) {
    
    yyerror2("permission statements must have an octal value "
             "between 0 and 07777");
        
  }

  return result;
  
}


/*
 * Timeout statement semantic check
 */
static void check_timeout(int s, int c) {

  if(s > c) {
    
    yyerror2("the number of restarts must be less than poll cycles");
    
  }

  if(s <= 0 || c <= 0) {
    
    yyerror2("zero or negative values not allowed in a timeout statement");
    
  }
  
}


/*
 * Every statement semantic check
 */
static void check_every(int every) {

  if(every <= 1) {
    
    yyerror2("an EVERY statement must have a value greater than 1");
    
  }
  
}


/*
 * Check hostname 
 */
static void check_hostname(char *hostname) {

  ASSERT(hostname);

  if(!check_host(hostname)) {
    
    yyerror2("hostname did not resolve");

  }
 
}


/*
 * Check the dependency graph for errors and if dependencies are
 * present reshuffle the service list in a depending order.
 */
static void check_depend() {

  Service_T s;
  int has_depend= FALSE;
  
  for(s= servicelist; s; s= s->next) {
    if(s->visited)
	continue;
    validate_depend(s, &has_depend);
    reset_depend();
  }
  
  if(has_depend) {
    
    Service_T d;
    
    for(s= servicelist; s; s= s->next) {
      if(s->visited)
	  continue;
      order_depend(s);
    }

    ASSERT(depend_list);
    servicelist= depend_list;
    
    for(d= depend_list; d; d= d->next_depend)
	d->next= d->next_depend;
    
  }

  reset_depend();
  
}


/*
 * Check if the executable exist
 */
static void check_exec(char *exec) {
  
  if(! exist_file(exec)) {
    yyerror2("the executable does not exist");
  }
 
}
 

/* -------------------------------------------------------------------- Misc */


/*
 * Cleans up an md5 string, tolower and remove byte sperators
 */
static int cleanup_hash_string(char *hashstring) {

  int i= 0, j= 0;

  ASSERT(hashstring);

  while (hashstring[i] != '\0') {

    if (isxdigit((int) hashstring[i])) {
      
      hashstring[j]=tolower(hashstring[i]);
      j++;

    } 

    i++;

  }
  
  hashstring[j]='\0';

  return j;
 
}


/*
 * Search for any errors in the service dependency graph
 */
static void validate_depend(Service_T s, int *has_depend) {

  ASSERT(s);

  if(s->visited)
      return;
  
  if(s->dependantlist) {
    
    Dependant_T d;
    
    for(d= s->dependantlist; d; d= d->next) {
      
      Service_T dp= get_service(d->dependant);
      
      if(!dp) {
	log("%s: Error: Depend service '%s' is not defined in the "
	    "control file\n", prog, d->dependant);
	exit(1);
      }
      
      if(dp->depend_visited) {
	log("%s: Error: Found a depend loop in the control file "
	    "involving the service '%s'\n", prog, s->name);
	exit(1);
      }
      
      *has_depend= TRUE;
      dp->depend_visited= TRUE;
      validate_depend(dp, has_depend);
      
    }
  }
  
  s->visited= TRUE;

}


/*
 * Order the service list with the most "depending" service last and
 * the least first.
 */
static void order_depend(Service_T s) {

  ASSERT(s);
  
  if(s->visited)
      return;

  s->visited= TRUE;

  if(s->dependantlist) {
    
    Dependant_T d;
    
    for(d= s->dependantlist; d; d= d->next) {
      
      Service_T dp= get_service(d->dependant);
      
      order_depend(dp);
      
    }
  }

  s->next_depend= depend_list;
  depend_list= s;

}
 
