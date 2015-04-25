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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "monitor.h"
#include "cervlet.h" 
#include "engine.h"
#include "processor.h"
#include "base64.h"
#include "event.h" 
#include "alert.h"
#include "process.h"
#include "device.h"

#define ACTION(c) !strncasecmp(req->url, c, sizeof(c))

/* URL Commands supported */
#define HOME        "/"
#define STOP        "/_stop"
#define TEST        "/_monit"
#define ABOUT       "/_about"
#define PIXEL       "/_pixel"
#define STATUS      "/_status"
#define RUN         "/_runtime"
#define VIEWLOG     "/_viewlog"

/* Status output formats */
#define XML_STATUS   0
#define TEXT_STATUS  1

/* Private prototypes */
static int is_readonly(HttpRequest);
static void printPixel(HttpResponse);
static void doGet(HttpRequest, HttpResponse);
static void doPost(HttpRequest, HttpResponse);
static void do_home(HttpRequest, HttpResponse);
static void do_home_device(HttpRequest, HttpResponse);
static void do_home_directory(HttpRequest, HttpResponse);
static void do_home_file(HttpRequest, HttpResponse);
static void do_home_process(HttpRequest, HttpResponse);
static void do_home_remote(HttpRequest, HttpResponse);
static void do_about(HttpRequest, HttpResponse);
static void not_found(HttpRequest, HttpResponse);
static void do_runtime(HttpRequest, HttpResponse);
static void do_viewlog(HttpRequest, HttpResponse);
static void handle_action(HttpRequest, HttpResponse);
static void is_monit_running(HttpRequest, HttpResponse);
static void do_service(HttpRequest, HttpResponse, char *);
static void do_service_device(HttpRequest, HttpResponse, Service_T);
static void do_service_directory(HttpRequest, HttpResponse, Service_T);
static void do_service_file(HttpRequest, HttpResponse, Service_T);
static void do_service_process(HttpRequest, HttpResponse, Service_T);
static void do_service_host(HttpRequest, HttpResponse, Service_T);
static void print_alerts(HttpResponse, Mail_T);
static void print_buttons(HttpRequest, HttpResponse, Service_T);
static void print_service_common_params(HttpResponse, Service_T);
static void print_status(HttpResponse, int);
static void status_start(HttpResponse, int);
static void status_stop(HttpResponse, int);
static void status_service_xml(Service_T, HttpResponse);
static void status_service_text(Service_T, HttpResponse);
static int get_service_status(Service_T);

static  struct {
  const char *type;
  const char *up;
  const char *down;
} status_text[]= {
  {"Device", "accessible", "not accessible or has errors"},
  {"Directory", "accessible", "not accessible or has errors"},
  {"File", "accessible", "not accessible or has errors"},
  {"Process", "running", "not running or has errors"},
  {"Host", "online with all services", "not online or service errors"}
};


/**
 *  Implementation of doGet and doPost routines used by the cervlet
 *  processor module. This particilary cervlet will provide
 *  information about the monit deamon and programs monitored by
 *  monit.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: cervlet.c,v 1.126 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Callback hook to the Processor module for registering this modules
 * doGet and doPost methods.
 */
void init_service() {
  
  add_Impl((void *) doGet, (void *) doPost);
  
}


/* ----------------------------------------------------------------- Private */


/**
 * Called by the Processor (via the service method)
 * to handle a POST request.
 */
static void doPost(HttpRequest req, HttpResponse res) {

  doGet(req, res);

}


/**
 * Called by the Processor (via the service method)
 * to handle a GET request.
 */
static void doGet(HttpRequest req, HttpResponse res) {

  set_content_type(res, "text/html");

  if(ACTION(HOME)) {
    LOCK(Run.mutex)
	do_home(req, res);
    END_LOCK;
  } else if(ACTION(RUN)) {
    LOCK(Run.mutex)
	do_runtime(req, res);
    END_LOCK;
  } else if(ACTION(TEST)) {
    is_monit_running(req, res);
  } else if(ACTION(VIEWLOG)) {
    do_viewlog(req, res);
  } else if(ACTION(ABOUT)) {
    do_about(req, res);
  } else if(ACTION(STOP)) {
    if(is_readonly(req)) {
      send_error(res, SC_FORBIDDEN,
		 "You do not have sufficent privilegs to access this page");
    } else {
      send_error(res, SC_SERVICE_UNAVAILABLE,
		 "The monit http server is stopped");
      stop_httpd();
    }
  } else if(ACTION(PIXEL)) {
    printPixel(res);
  } else if(ACTION(STATUS)) {
    const char *format= get_parameter(req, "format");
    if(format && starts_with(format, "xml"))
      print_status(res, XML_STATUS);
    else {
      /* Text status output is default */
      print_status(res, TEXT_STATUS);
    }
  } else {
    handle_action(req, res);
  }
   
}


/* ----------------------------------------------------------------- Helpers */


static void is_monit_running(HttpRequest req, HttpResponse res) {

  int status;
  int monit= exist_daemon();

  if(monit) {
    status= SC_OK;
  } else {
    status= SC_GONE;
  }

  set_status(res, status);
  
}
    

static void do_home(HttpRequest req, HttpResponse res) {

  char *uptime= get_process_uptime(Run.pidfile, "&nbsp;");
 
  HEAD("", Run.polltime)
  out_print(res,
    "<table cellspacing=\"0\" cellpadding=\"5\" width=\"100%%\" border=\"0\">"
    " <tr bgcolor=\"#BBDDFF\">"
    "  <td colspan=2 valign=\"top\" align=\"left\" bgcolor=\"#EFF7FF\" width=\"100%%\">"
    "  <br><h2 align=\"center\">Monit Service Manager</h2>"
    "  <p align=\"center\">Monit is <a href='/_runtime'>running</a> on %s "
    "  with <i>uptime, %s</i> and monitoring:</p><br>"
    "  </td>"
    " </tr>"
    "</table>"
    "<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%%\" border=\"0\">"
    "  <tr valign=\"middle\" bgcolor=\"#6F6F6F\">"
    "    <td><img src=\"/_pixel\" width=\"1\" height=\"1\" alt=\"\"></td>"
    "  </tr>"
    "</table>", Run.localhostname, uptime);

  FREE(uptime);
  
  do_home_process(req, res);
  do_home_device(req, res);
  do_home_file(req, res);
  do_home_directory(req, res);
  do_home_remote(req, res);
  
  FOOT
      
}


static void do_about(HttpRequest req, HttpResponse res) {

  out_print(res,
    "<html><head><title>about monit</title><body bgcolor=white>"
    "<br><br><center><a href='http://www.tildeslash.com/monit/'><font size=+2>"
    "monit " VERSION "</font></a></center><br><br>");
  out_print(res,
    "<ul>"
    "<li>Copyright &copy; 2000-2004 by <a "
    "href=\"http://www.tildeslash.com/monit/who.html\">the monit "
    "project group</a>. All Rights Reserved.<br> </small><p>"
    "<li>Portions of this software are copyright &copy; 1995, 1996 "
    "<a href='http://www.gnu.org/'>Free Software Foundation, Inc.</a></ul>");
  out_print(res,
   "<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%%\" border=\"0\">"
   " <tr bgcolor=\"#6F6F6F\">"
   " <td valign=\"bottom\"><img src=\"/_pixel\" width=\"1\" height=\"1\" "
   "alt=\"\"></td></tr></table>");
  out_print(res,
    "<p>This program is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License "
    "as published by the Free Software Foundation; either version 2 of "
    "the License, or (at your option) any later version.<p>"
    "This program is distributed in the hope that it will be useful, but "
    "WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "<a href='http://www.tildeslash.com/gpl.html'>GNU "
    "General Public License</a> for more details."
    "<p><br><center><p><i>Please consider making a "
    "<a class=grey href=\"http://www.tildeslash.com/monit/index.html#support\">"
    "donation</a> and support our continued work with monit.</i></center>");

}


static void do_runtime(HttpRequest req, HttpResponse res) {

  int pid=  exist_daemon();
  
  HEAD("_runtime", 1000)
  out_print(res,
	    "<center><h3>monit runtime status</h3><center><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"40%\"><b>Parameter</b></td>"
	    "<td width=\"60%\"><b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Host</td><td>%s</td></tr>",
	    Run.localhostname);
  out_print(res,
	    "<tr><td>Process id</td><td>%d</td></tr>", pid);
  out_print(res,
	    "<tr><td>Effective user running monit</td>"
	    "<td>%s</td></tr>", Run.Env.user);
  out_print(res,
	    "<tr><td>Controlfile</td><td>%s</td></tr>", Run.controlfile);
  if(Run.logfile)
    out_print(res,
	    "<tr><td>Logfile</td><td>%s</td></tr>", Run.logfile);
  out_print(res,
	    "<tr><td>Pidfile</td><td>%s</td></tr>", Run.pidfile);
  out_print(res,
	    "<tr><td>State file</td><td>%s</td></tr>", Run.statefile);
  out_print(res,
	    "<tr><td>Debug</td><td>%s</td></tr>",
	    Run.debug?"True":"False");
  out_print(res,
	    "<tr><td>Log</td><td>%s</td></tr>", Run.dolog?"True":"False");
  out_print(res,
	    "<tr><td>Use syslog</td><td>%s</td></tr>",
	    Run.use_syslog?"True":"False");
  
  out_print(res, "<tr><td>Mail server(s)</td><td>");
  if(Run.mailservers) {
    MailServer_T mta= Run.mailservers;
    for(mta= Run.mailservers; mta; mta= mta->next)
	out_print(res, "%s&nbsp;", mta->host);
    out_print(res, "</td></tr>");
  } else {
    out_print(res, "localhost</td></tr>");
  }
  
  if(Run.MailFormat.from)
    out_print(res,
	    "<tr><td>Default mail from</td><td>%s</td></tr>",
	    Run.MailFormat.from);
  if(Run.MailFormat.subject)
    out_print(res,
	    "<tr><td>Default mail subject</td><td>%s</td></tr>",
	    Run.MailFormat.subject);
  if(Run.MailFormat.message)
    out_print(res,
	    "<tr><td>Default mail message</td><td>%s</td></tr>",
            Run.MailFormat.message);

  out_print(res,
	    "<tr><td>Poll time</td><td>%d seconds</td></tr>",
	    Run.polltime);
  out_print(res,
	    "<tr><td>httpd bind address</td><td>%s</td></tr>",
	    Run.bind_addr?Run.bind_addr:"Any/All");
  out_print(res,
	    "<tr><td>httpd portnumber</td><td>%d</td></tr>", Run.httpdport);
  out_print(res,
	    "<tr><td>httpd signature</td><td>%d</td></tr>",
	    Run.httpdsig?"True":"False");
  out_print(res,
	    "<tr><td>Use ssl encryption</td><td>%s</td></tr>",
	    Run.httpdssl?"True":"False");
  if (Run.httpdssl) {
    out_print(res,
	      "<tr><td>PEM key/certificate file</td><td>%s</td></tr>",
	      Run.httpsslpem);
    
    if (Run.httpsslclientpem!=NULL) {
      out_print(res,
		"<tr><td>Client PEM key/certification"
		"</td><td>%s</td></tr>", "Enabled");
      out_print(res,
		"<tr><td>Client PEM key/certificate file"
		"</td><td>%s</td></tr>", Run.httpsslclientpem);
    } else {
      out_print(res,
		"<tr><td>Client PEM key/certification"
		"</td><td>%s</td></tr>", "Disabled");
    }
    out_print(res,
	      "<tr><td>Allow self certified certificates "
	      "</td><td>%s</td></tr>", Run.allowselfcert?"True":"False");
  }
  
  out_print(res,
	    "<tr><td>httpd auth. style</td><td>%s</td></tr>",
	    (Run.credentials!=NULL)&&has_hosts_allow()?
	    "Basic Authentication and Host allow list":
	    (Run.credentials!=NULL)?"Basic Authentication":
	    has_hosts_allow()?"Host allow list":
	    "No authentication");

  print_alerts(res, Run.maillist);

  out_print(res,"</table>");

  if(!is_readonly(req)) {
    out_print(res, "<table cellspacing=16><tr nowrap><td><font size=+1>"
	      "<font color='#f0000'>Stop monit http server?</font></font>"
	      "<td align=right><form method=GET action=%s>"
	      "<input type=submit value=Go style='font-size: 12pt'></font>"
	      "</form></td>", STOP);
    if(Run.dolog && !Run.use_syslog) {
      out_print(res, "<td><font size=+1>"
		"View monit logfile?</font>"
		"<td align=right><form method=GET action=%s>"
		"<input type=submit value=Go style='font-size: 12pt'></font>"
		"</form></td>", VIEWLOG);
    }
  }
  out_print(res, "</tr></table>");
  
  FOOT

}


static void do_viewlog(HttpRequest req, HttpResponse res) {
  
  if(is_readonly(req)) {
    send_error(res, SC_FORBIDDEN,
	       "You do not have sufficent privilegs to access this page");
    return;
  }
  
  HEAD("_viewlog", 100)
      
  if(Run.dolog && !Run.use_syslog) {
    
    struct stat sb;
    
    if(!stat(Run.logfile, &sb)) {
      
      FILE *f= fopen(Run.logfile, "r");
      if(f) {
	
	int n;
	char buf[8192];
	
	out_print(res, "<br><p><form><textarea cols=80 rows=30>");
	
	while((n= fread(buf, sizeof(char), 8192, f)) > 0) {
	  
	  buf[n]= 0;
	  out_print(res, "%s", buf);
	  
	}
	
        fclose(f);
	out_print(res, "</textarea></form>");
	
      } else {
	
	out_print(res, "Error opening logfile: %s", STRERROR);
	
      }
      
    } else {
      
      out_print(res, "Error stating logfile: %s", STRERROR);
      
    }
    
  } else {
    
    out_print(res,
    "<b>Cannot view logfile:</b><br>");
    if(!Run.dolog) {
      
      out_print(res, "monit was started without logging");
      
    } else {
      
      out_print(res, "monit uses syslog");
      
    }
    
  }
  
  FOOT
  
}


static void handle_action(HttpRequest req, HttpResponse res) {

  char *name= req->url;
  const char *action= get_parameter(req, "action");
  
  if(exist_service(++name)) {
    if(action) {
      
      Service_T s= get_service(name);

      if(is_readonly(req)) {
	send_error(res, SC_FORBIDDEN,
		   "You do not have sufficent privilegs to access this page");
	return;
      }

      if(IS(action, "start")) {
	if(s->start) {
	  LOCK(s->mutex)
	      check_service(name, action);
	      if(s->type==TYPE_PROCESS) {
		/* Wait for the service to start (or fail) */
		int max_tries= Run.polltime;
		while(max_tries-- && !Run.stopped) {
		  if(is_process_running(s))
		      break;
		  sleep(1);
		}
	      }
	  END_LOCK;
	  if(s->type==TYPE_PROCESS && !is_process_running(s)) {
	    send_error(res, SC_INTERNAL_SERVER_ERROR,
		       "Could not start the service");
	    goto quit;
	  }
	} else {
          send_error(res, SC_BAD_REQUEST,
	             "Start method not defined for the service");
	  goto quit;
	}
      } else if(IS(action, "stop")) {
	if(s->stop) {
	  LOCK(s->mutex)
	      check_service(name, action);
	  END_LOCK;
	} else {
          send_error(res, SC_BAD_REQUEST,
	             "Stop method not defined for the service");
	  goto quit;
	}
      } else if(IS(action, "restart")) {
	if(s->start && s->stop) {
 	  LOCK(s->mutex)
	      check_service(name, action);
	      if(s->type==TYPE_PROCESS) {
		/* Wait for the service to restart (or fail) */
		int max_tries= Run.polltime;
		while(max_tries-- && !Run.stopped) {
		  if(is_process_running(s))
		      break;
		  sleep(1);
		}
	      }
	  END_LOCK;
	  if(s->type==TYPE_PROCESS && !is_process_running(s)) {
	    send_error(res, SC_INTERNAL_SERVER_ERROR,
		       "Could not restart the service");
	    goto quit;
	  }
	} else {
          send_error(res, SC_BAD_REQUEST,
		     "Start or stop method not defined for the service");
	  goto quit;
	}
      } else if(IS(action, "monitor")) {
	  LOCK(s->mutex)
	    check_service(name, action);
	  END_LOCK;
      } else if(IS(action, "unmonitor")) {
	  LOCK(s->mutex)
	    check_service(name, action);
	  END_LOCK;
      }
      
    }
    
    LOCK(Run.mutex)
	do_service(req, res, name);
    END_LOCK;
    
  } else {
    
    not_found(req, res);
    
  }

  quit:
  reset_depend();
  
}


static void do_service(HttpRequest req, HttpResponse res, char *name) {
  
  Service_T s= get_service(name);

  HEAD(name, 1000)

    switch (s->type) {

    case TYPE_DEVICE:
	do_service_device(req, res, s);
	break;
	
    case TYPE_DIRECTORY:
	do_service_directory(req, res, s);
	break;
	
    case TYPE_FILE:
	do_service_file(req, res, s);
	break;
	
    case TYPE_PROCESS:
	do_service_process(req, res, s);
	break;
	
    case TYPE_REMOTE:
	do_service_host(req, res, s);
	break;

    default:
      break;

    }

  FOOT

}





static void printPixel(HttpResponse res) {

  static int l;
  Socket_T S= res->S;
  static unsigned char *pixel= NULL;
  
  if(! pixel) {
    pixel= xcalloc(sizeof(unsigned char), strlen(PIXEL_GIF));
    l= decode_base64(pixel, PIXEL_GIF);
  }
  if (l) {
    res->is_committed= TRUE;
    socket_print(S, "HTTP/1.0 200 OK\r\n");
    socket_print(S, "Content-length: %d\r\n", l);
    socket_print(S, "Content-Type: image/gif\r\n");
    socket_print(S, "Connection: close\r\n\r\n");
    socket_write(S, pixel, l);
  }
  
}


static void not_found(HttpRequest req, HttpResponse res) {
  
  send_error(res, SC_NOT_FOUND,
	     "The requested URL was not found on this server");
  
}


static void do_home_process(HttpRequest req, HttpResponse res) {

  int on= TRUE;
  int header= TRUE;
  Service_T s;

  for(s= servicelist_conf; s; s= s->next_conf) {
    
    int isrunning;
    char *uptime;
    
    if(s->type != TYPE_PROCESS) continue;
    
    if(header) {
      
      if(Run.doprocess) {
	
        out_print(res,
          "<br><p>&nbsp;</p>"
          "<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
          "<tr>"
          "<td width=\"20%\"><h3><b>Process</b></h3></td>"
          "<td align=\"left\"><h3><b>Status</b></h3></td>"
          "<td align=\"right\"><h3><b>Uptime</b></h3></td>"
          "<td align=\"right\"><h3><b>CPU</b></h3></td>"
          "<td align=\"right\"><h3><b>Memory</b></h3></td>"
          "</tr>");

      } else {

        out_print(res,
          "<br><p>&nbsp;</p>"
          "<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
          "<tr>"
          "<td width=\"20%\"><h3><b>Service</b></h3></td>"
          "<td align=\"left\"><h3><b>Status</b></h3></td>"
          "</tr>");

      }

      header= FALSE;

    }

    isrunning= is_process_running(s);
    uptime= get_process_uptime(s->path, "&nbsp;");
    
    if(Run.doprocess) {
      ProcInfo_T pi= s->procinfo;
      if(isrunning) {
        out_print(res,
		  "<tr %s>"
		  "<td width=\"20%\"><a href='/%s'>%s</a></td>"
		  "<td align=\"left\">%s</td><td align=\"right\">%s</td>"
		  "<td align=\"right\">%.1f%%</td>"
		  "<td align=\"right\">%.1f%% [%ld&nbsp;kB]</td></tr>",
		  on?"bgcolor=\"#EFEFEF\"":"",
		  s->name, s->name,
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  isrunning?
		  "<font color='#00ff00'>running</font>":
		  "<font color='#ff0000'>not running</font>",
		  uptime,
		  pi->cpu_percent/10.0,
		  pi->mem_percent/10.0,pi->mem_kbyte
		  );
      } else {
        out_print(res,
		  "<tr %s>"
		  "<td width=\"20%\"><a href='/%s'>%s</a></td>"
		  "<td align=\"left\">%s</td>"
		  "<td align=\"right\">-</td>"
		  "<td align=\"right\">- </td>"
		  "<td align=\"right\">- [-]</td></tr>",
		  on?"bgcolor=\"#EFEFEF\"":"",
		  s->name, s->name,
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  "<font color=red>not running</font>"
		  );
      }
    } else {
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s %s</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		isrunning?
		"<font color=green>running</font>":
		"<font color=red>not running</font>",
		isrunning?uptime:"");
    }

    on= on?FALSE:TRUE;
    FREE(uptime);
    
  }
  
  if(!header)
      out_print(res, "</table>");
  
}


static void do_home_device(HttpRequest req, HttpResponse res) {
  
  Service_T s;
  int on= TRUE;
  int header= TRUE;
  
  for(s= servicelist_conf; s; s= s->next_conf) {
    
    if(s->type != TYPE_DEVICE) continue;
    
    if(header) {
      
      out_print(res,
		"<br><p>&nbsp;</p>"
		"<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
		"<tr>"
		"<td width=\"20%\"><h3><b>Device</b></h3></td>"
		"<td align=\"left\"><h3><b>Status</b></h3></td>"
		"<td align=\"right\"><h3><b>Space usage</b></h3></td>"
		"<td align=\"right\"><h3><b>Inodes usage</b></h3></td>"
		"</tr>");
      
      header= FALSE;
      
    }
    
    if(!DeviceInfo_Usage(s->devinfo, s->path)) {
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s</td>"
		"<td align=\"right\">- [-]</td>"
		"<td align=\"right\">- [-]</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		"<font color=red>not accessible</font>");
    } else {
      
      if(s->devinfo->f_files > 0) {
	
        out_print(res,
		  "<tr %s>"
		  "<td width=\"20%\"><a href='/%s'>%s</a></td>"
		  "<td align=\"left\">%s</td>"
		  "<td align=\"right\">%.1f%% [%.1f&nbsp;MB]</td>"
		  "<td align=\"right\">%.1f%% [%ld&nbsp;objects]</td>"
		  "</tr>",
		  on?"bgcolor=\"#EFEFEF\"":"",
		  s->name, s->name,
		  (s->perm&&s->perm->has_error)?
		  "<font color='#ff0000'>Permission Error</font>":
		  (s->uid&&s->uid->has_error)?
		  "<font color='#ff0000'>UID Error</font>":
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  (s->gid&&s->gid->has_error)?
		  "<font color='#ff0000'>GID Error</font>":
		  "<font color='#00ff00'>accessible</font>",
		  (float) 100 *
		  (s->devinfo->f_blocks - s->devinfo->f_blocksfreetotal) /
		  s->devinfo->f_blocks,
		  (float)(s->devinfo->f_blocks - s->devinfo->f_blocksfreetotal)
		  / 1048576 * s->devinfo->f_bsize,
		  (float) 100 * (s->devinfo->f_files - s->devinfo->f_filesfree)/
		  s->devinfo->f_files,
		  s->devinfo->f_files - s->devinfo->f_filesfree);
      } else {
	
        out_print(res,
		  "<tr %s>"
		  "<td width=\"20%\"><a href='/%s'>%s</a></td>"
		  "<td align=\"left\">%s</td>"
		  "<td align=\"right\">%.1f%% [%.1f&nbsp;MB]</td>"
		  "<td align=\"right\"><font color=red>not supported by "
		  "filesystem</font></td></tr>",
		  on?"bgcolor=\"#EFEFEF\"":"",
		  s->name, s->name,
		  (s->perm&&s->perm->has_error)?
		  "<font color='#ff0000'>Permission Error</font>":
		  (s->uid&&s->uid->has_error)?
		  "<font color='#ff0000'>UID Error</font>":
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  (s->gid&&s->gid->has_error)?
		  "<font color='#ff0000'>GID Error</font>":
		  "<font color='#00ff00'>accessible</font>",
		  (float) 100 * (s->devinfo->f_blocks -
				 s->devinfo->f_blocksfreetotal) /
		  s->devinfo->f_blocks,
		  (float) (s->devinfo->f_blocks -
			   s->devinfo->f_blocksfreetotal) /
		  1048576 * s->devinfo->f_bsize);
      }
    }
    
    on= on?FALSE:TRUE;

  }
  
  if(!header)
      out_print(res, "</table>");
  
}


static void do_home_file(HttpRequest req, HttpResponse res) {
  
  Service_T s;
  int on= TRUE;
  int header= TRUE;
  struct stat stat_buf;
  
  for(s= servicelist_conf; s; s= s->next_conf) {
    
    if(s->type != TYPE_FILE) continue;
    
    if(header) {
      
      out_print(res,
		"<br><p>&nbsp;</p>"
		"<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
		"<tr>"
		"<td width=\"20%\"><h3><b>File</b></h3></td>"
		"<td align=\"left\"><h3><b>Status</b></h3></td>"
		"<td align=\"right\"><h3><b>Size</b></h3></td>"
		"<td align=\"right\"><h3><b>Permission</b></h3></td>"
		"<td align=\"right\"><h3><b>UID</b></h3></td>"
		"<td align=\"right\"><h3><b>GID</b></h3></td>"
		"</tr>");
      
      header= FALSE;
      
    }
    
    if((stat(s->path, &stat_buf) != 0) || !S_ISREG(stat_buf.st_mode)) {
      
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s</td>"
		"<td align=\"right\">-</td>"
		"<td align=\"right\">-</td>"
		"<td align=\"right\">-</td>"
		"<td align=\"right\">-</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		"<font color=red>doesn't exist</font>");

    } else {
      
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s</td>"
		"<td align=\"right\">%lu&nbsp;B</td>"
		"<td align=\"right\">%o</td>"
		"<td align=\"right\">%d</td>"
		"<td align=\"right\">%d</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		(s->perm&&s->perm->has_error)?
		"<font color='#ff0000'>Permission Error</font>":
		(s->uid&&s->uid->has_error)?
		"<font color='#ff0000'>UID Error</font>":
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		(s->gid&&s->gid->has_error)?
		"<font color='#ff0000'>GID Error</font>":
		"<font color='#00ff00'>accessible</font>",
		(unsigned long)stat_buf.st_size,
		stat_buf.st_mode & 07777,
		stat_buf.st_uid,
		stat_buf.st_gid
		);

    }
    
    on= on?FALSE:TRUE;
    
  }
  
  if(!header)
      out_print(res, "</table>");
  
}


static void do_home_directory(HttpRequest req, HttpResponse res) {
  
  Service_T s;
  int on= TRUE;
  int header= TRUE;
  struct stat stat_buf;
  
  for(s= servicelist_conf; s; s= s->next_conf) {
    
    if(s->type != TYPE_DIRECTORY) continue;

    if(header) {

      out_print(res,
		"<br><p>&nbsp;</p>"
		"<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
		"<tr>"
		"<td width=\"20%\"><h3><b>Directory</b></h3></td>"
		"<td align=\"left\"><h3><b>Status</b></h3></td>"
		"<td align=\"right\"><h3><b>Permission</b></h3></td>"
		"<td align=\"right\"><h3><b>UID</b></h3></td>"
		"<td align=\"right\"><h3><b>GID</b></h3></td>"
		"</tr>");
      
      header= FALSE;
      
    }
    
    if( (stat(s->path, &stat_buf) != 0) || !S_ISDIR(stat_buf.st_mode)) {
      
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s</td>"
		"<td align=\"right\">-</td>"
		"<td align=\"right\">-</td>"
		"<td align=\"right\">-</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		"<font color=red>doesn't exist</font>");
      
    } else {
      
      out_print(res,
		"<tr %s>"
		"<td width=\"20%\"><a href='/%s'>%s</a></td>"
		"<td align=\"left\">%s</td>"
		"<td align=\"right\">%o</td>"
		"<td align=\"right\">%d</td>"
		"<td align=\"right\">%d</td>"
		"</tr>",
		on?"bgcolor=\"#EFEFEF\"":"",
		s->name, s->name,
		(s->perm&&s->perm->has_error)?
		"<font color='#ff0000'>Permission Error</font>":
		(s->uid&&s->uid->has_error)?
		"<font color='#ff0000'>UID Error</font>":
		!s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		(s->gid&&s->gid->has_error)?
		"<font color='#ff0000'>GID Error</font>":
		"<font color='#00ff00'>accessible</font>",
		stat_buf.st_mode & 07777,
		stat_buf.st_uid,
		stat_buf.st_gid
		);
      
    }
    
    on= on?FALSE:TRUE;
    
  }
  
  if(!header)
      out_print(res, "</table>");
  
}


static void do_home_remote(HttpRequest req, HttpResponse res) {

  Port_T port;
  Service_T s;
  Icmp_T icmp;
  int on= TRUE;
  int header= TRUE;
  
  for(s= servicelist_conf; s; s= s->next_conf) {
    
    if(s->type != TYPE_REMOTE) continue;

    if(header) {
      
      out_print(res,
		"<br><p>&nbsp;</p>"
		"<table cellspacing=0 cellpadding=3 border=0 width=\"70%\">"
		"<tr>"
		"<td width=\"20%\"><h3><b>Host</b></h3></td>"
		"<td align=\"left\"><h3><b>Status</b></h3></td>"
		"<td align=\"right\"><h3><b>Protocol(s)</b></h3></td>"
		"</tr>");
      
      header= FALSE;
      
    }
    
    out_print(res,
	      "<tr %s><td width=\"20%\"><a href='/%s'>%s</a></td>",
	      on?"bgcolor=\"#EFEFEF\"":"", s->name, s->name);
    
    if(s->portlist) {
      int available= TRUE;

      for(port= s->portlist; port; port= port->next) {
	if(!port->is_available) {
	  available= FALSE;
	  break;
	}
      }
      
      if(available) {
	out_print(res, "<td align=\"left\">%s</td>"
		  "<td align=\"right\">",
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  "<font color='#00ff00'>available</font>");
	for(port= s->portlist; port; port= port->next) {
	  if(port != s->portlist)
	      out_print(res, "&nbsp;&nbsp;<b>|</b>&nbsp;&nbsp;");
	  out_print(res, "[%s] at port %d", port->protocol->name, port->port);
	}
	out_print(res, "</td>");
      } else {
	out_print(res, "<td align=\"left\">%s</td>"
		  "<td align=\"right\">[%s] at port %d</td>",
		  !s->do_monitor?"<font color='#ff8800'>not monitored</font>":
		  "<font color='#ff0000'>not available</font>",
		  port->protocol->name, port->port);
      }
    } else if(s->icmplist) {
      int available= TRUE;
      
      for(icmp= s->icmplist; icmp; icmp= icmp->next) {
	if(!icmp->is_available) {
	  available= FALSE;
	  break;
	}
      }
      if(available) {
	out_print(res, "<td align=\"left\">"
		  "<font color='#00ff00'>available</font></td>"
		  "<td align=\"right\">[ICMP PING]</td>");
      } else {
	out_print(res, "<td align=\"left\">"
		  "<font color='#ff0000'>not available</font></td>"
		  "<td align=\"right\">[ICMP PING]</td>");
      }
    }
    
    out_print(res, "</tr>");
    on= on?FALSE:TRUE;
    
  }
  
  if(!header)
      out_print(res, "</table>");

}


static void do_service_device(HttpRequest req, HttpResponse res, Service_T s) {
  
  struct stat stat_buf;
  
  int state= ( DeviceInfo_Usage(s->devinfo, s->path) &&
	       stat(s->path, &stat_buf) == 0 );
  
  out_print(res,
	    "<p><br><h3>Device status</h3><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"30%\"><b>Parameter</b></td><td width=\"70%\">"
	    "<b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Name</td><td>%s</td></tr>", s->name);

  out_print(res,
	    "<tr><td>Device status</td><td>%s</td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>Permission error</font>":
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>UID Error</font>":
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>GID Error</font>":
	    state?
	    "<font color='#00ff00'>Accessible</font>":
	    "<font color='#ff0000'>Not Accessible</font>");
  out_print(res,
	    "<tr><td>Path</td><td>%s</td></tr>",
	    s->path);
  
  print_service_common_params(res, s);
  
  if(s->perm)
      out_print(res, "<tr><td>Associated permission</td>"
		"<td>if failed %o then %s</td></tr>",
		s->perm->perm, actionnames[s->perm->action]);
  if(s->uid)
      out_print(res, "<tr><td>Associated UID</td><td>if failed %d then %s</td>"
		"</tr>", (int)s->uid->uid, actionnames[s->uid->action]);
  if(s->gid)
      out_print(res, "<tr><td>Associated GID</td><td>if failed %d then %s</td>"
		"</tr>", (int)s->gid->gid, actionnames[s->gid->action]);
  out_print(res,
	    "<tr><td>Permission</td><td>%s%o</font></td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    stat_buf.st_mode & 07777);
  out_print(res,
	    "<tr><td>UID</td><td>%s%d</font></td></tr>",
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_uid);
  out_print(res,
	    "<tr><td>GID</td><td>%s%d</font></td></tr>",
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_gid);

  {
    
    Device_T dl;
    
    if(s->devicelist) {
      
      for(dl= s->devicelist; dl; dl= dl->next) {
	
        if(dl->resource == RESOURCE_ID_INODE) {
	  
          out_print(res,
		    "<tr><td>Inodes usage limit</td><td>if %s %ld %s "
		    "then %s</td></tr>", operatornames[dl->operator],
		    (dl->limit_absolute > -1)?
		    dl->limit_absolute:dl->limit_percent,
		    (dl->limit_absolute > -1)?"":"%",
		    actionnames[dl->action]);
	  
        } else if(dl->resource == RESOURCE_ID_SPACE) {
	  
          out_print(res,
		    "<tr><td>Space usage limit</td><td>if %s %ld %s "
		    "then %s</td></tr>", operatornames[dl->operator],
		    (dl->limit_absolute > -1)?
		    dl->limit_absolute:dl->limit_percent,
		    (dl->limit_absolute > -1)?"blocks":"%",
		    actionnames[dl->action]);
        }

      }
      
    }
    
    if(state) {
      out_print(res,
		"<tr><td>Blocks total</td><td>%ld [%.1f MB]</td></tr>",
		s->devinfo->f_blocks,
		(float) s->devinfo->f_blocks / 1048576 * s->devinfo->f_bsize);
      out_print(res,
		"<tr><td>Blocks free for non superuser</td>"
		"<td>%ld [%.1f MB] [%.1f%%]</font></td></tr>",
		s->devinfo->f_blocksfree,
		(float) s->devinfo->f_blocksfree /
		1048576 * s->devinfo->f_bsize,
		(float) 100 * s->devinfo->f_blocksfree / s->devinfo->f_blocks);
      out_print(res,
		"<tr><td>Blocks free total</td><td>%ld [%.1f MB] [%.1f%%]"
		"</td></tr>", s->devinfo->f_blocksfreetotal,
		(float) s->devinfo->f_blocksfreetotal /
		1048576 * s->devinfo->f_bsize,
		(float) 100 * s->devinfo->f_blocksfreetotal /
		s->devinfo->f_blocks);
      out_print(res,
		"<tr><td>Block size</td><td>%ld B</td></tr>",
		s->devinfo->f_bsize);
      if(s->devinfo->f_files > 0) {
        out_print(res,
		  "<tr><td>Inodes total</td><td>%ld</td></tr>",
		  s->devinfo->f_files);
        out_print(res,
		  "<tr><td>Inodes free</td><td>%ld [%.1f%%]</font>"
		  "</td></tr>", s->devinfo->f_filesfree,
		  (float) 100 * s->devinfo->f_filesfree / s->devinfo->f_files);
      }
    } else {
      out_print(res,
		"<tr><td>Blocks total</td><td>-</td></tr>");
      out_print(res,
		"<tr><td>Blocks free for non superuser</td>"
		"<td>-</font></td></tr>");
      out_print(res,
		"<tr><td>Blocks free total</td><td>-</td></tr>");
      out_print(res,
		"<tr><td>Block size</td><td>-</td></tr>");
      out_print(res,
		"<tr><td>Inodes total</td><td>-</td></tr>");
      out_print(res,
		"<tr><td>Inodes free</td><td>-</font></td></tr>");
    }
  }
  
  print_alerts(res, s->maillist);
  
  out_print(res, "</table>");
  
  print_buttons(req, res, s);
  
}


static void do_service_directory(HttpRequest req,
				 HttpResponse res, Service_T s) {
  
  struct stat stat_buf;
  int state=((stat(s->path, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode));
  
  out_print(res,
	    "<p><br><h3>Directory status</h3><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"30%\"><b>Parameter</b></td>"
	    "<td width=\"70%\"><b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Name</td><td>%s</td></tr>", s->name);

  out_print(res,
	    "<tr><td>Directory status</td><td>%s</td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>Permission Error</font>":
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>UID Error</font>":
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>GID Error</font>":
	    state?
	    "<font color='#00ff00'>Exist</font>":
	    "<font color='#ff0000'>Doesn't Exist</font>");
  out_print(res,
	    "<tr><td>Path</td><td>%s</td></tr>",
	    s->path);
  
  print_service_common_params(res, s);
  
  {
    struct mytimestamp *t;
    for(t= s->timestamplist; t; t= t->next) {
      if(t->test_changes) {
	out_print(res,
		  "<tr><td>Associated timestamp</td>"
		  "<td>If changed then %s</td></tr>",
		  actionnames[t->action]);
      } else {
	out_print(res,
		  "<tr><td>Associated timestamp</td>"
		  "<td>If %s %d second(s) then %s</td></tr>",
		  operatornames[t->operator],
		  t->time, actionnames[t->action]);
      }
    }
  }
  if(s->perm)
      out_print(res,
		"<tr><td>Associated permission</td><td>"
		"if failed %o then %s</td></tr>",
		s->perm->perm, actionnames[s->perm->action]);
  if(s->uid)
      out_print(res, "<tr><td>Associated UID</td><td>"
		"if failed %d then %s</td></tr>",
		(int)s->uid->uid, actionnames[s->uid->action]);
  if(s->gid)
      out_print(res, "<tr><td>Associated GID</td><td>"
		"if failed %d then %s</td></tr>",
		(int)s->gid->gid, actionnames[s->gid->action]);
  out_print(res,
	    "<tr><td>Permission</td><td>%s%o</font></td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    stat_buf.st_mode & 07777);
  out_print(res,
	    "<tr><td>UID</td><td>%s%d</font></td></tr>",
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_uid);
  out_print(res,
	    "<tr><td>GID</td><td>%s%d</font></td></tr>",
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_gid);
  
  print_alerts(res, s->maillist);
  
  out_print(res, "</table>");
  
  print_buttons(req, res, s);
  
}


static void do_service_file(HttpRequest req, HttpResponse res, Service_T s) {
  
  struct stat stat_buf;
  int state=((stat(s->path, &stat_buf) == 0) && S_ISREG(stat_buf.st_mode));
  
  out_print(res,
	    "<p><br><h3>File status</h3><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"30%\"><b>Parameter</b></td><td width=\"70%\">"
	    "<b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Name</td><td>%s</td></tr>", s->name);
  out_print(res,
	    "<tr><td>File status</td><td>%s</td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>Permission Error</font>":
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>UID Error</font>":
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>GID Error</font>":
	    state?
	    "<font color='#00ff00'>Exist</font>":
	    "<font color='#ff0000'>Doesn't Exist</font>");
  out_print(res,
	    "<tr><td>Path</td><td>%s</td></tr>",
	    s->path);
  
  print_service_common_params(res,s);
  
  {
    struct mytimestamp *t;
    for(t= s->timestamplist; t; t= t->next) {
      if(t->test_changes) {
	out_print(res,
		  "<tr><td>Associated timestamp</td>"
		  "<td>If changed then %s</td></tr>",
		  actionnames[t->action]);
      } else {
	out_print(res,
		  "<tr><td>Associated timestamp</td>"
		  "<td>If %s %d second(s) then %s</td></tr>",
		  operatornames[t->operator],
		  t->time, actionnames[t->action]);
      }
    }
  }
  {
    Size_T sl;
    for(sl= s->sizelist; sl; sl= sl->next) {
      if(sl->test_changes) {
	out_print(res,
		  "<tr><td>Associated size</td>"
		  "<td>If changed then %s</td></tr>",
		  actionnames[sl->action]);
      } else {
	out_print(res,
		  "<tr><td>Associated size</td>"
		  "<td>If %s %lu byte(s) then %s</td></tr>",
		  operatornames[sl->operator],
		  sl->size, actionnames[sl->action]);
      }
    }
  }

  if(s->checksum) {
    switch (s->checksum->type) {
    case HASH_MD5:
        out_print(res, "<tr><td>Associated checksum</td><td>"
                  "if failed MD5(%s) then %s</td></tr>",
                  s->checksum->hash, actionnames[s->checksum->action]);
        break;
    case HASH_SHA1:
        out_print(res, "<tr><td>Associated checksum</td><td>"
                  "if failed SHA1(%s) then %s</td></tr>",
                  s->checksum->hash, actionnames[s->checksum->action]);
        break;
    default:
        out_print(res, "<tr><td>Associated checksum</td><td>"
                  "if failed UNKNOWN(%s) then %s</td></tr>",
                  s->checksum->hash, actionnames[s->checksum->action]);
        break;
    }
  }
  
  if(s->perm)
      out_print(res, "<tr><td>Associated permission</td><td>"
		"if failed %o then %s</td></tr>",
		s->perm->perm, actionnames[s->perm->action]);
  if(s->uid)
      out_print(res, "<tr><td>Associated UID</td><td>"
		"if failed %d then %s</td></tr>",
		(int)s->uid->uid, actionnames[s->uid->action]);
  if(s->gid)
      out_print(res, "<tr><td>Associated GID</td><td>"
		"if failed %d then %s</td></tr>",
		(int)s->gid->gid, actionnames[s->gid->action]);
  out_print(res,
	    "<tr><td>Size</td><td>%lu B</td></tr>",
	    (unsigned long)stat_buf.st_size);
  out_print(res,
	    "<tr><td>Permission</td><td>%s%o</td></tr>",
	    (s->perm&&s->perm->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    stat_buf.st_mode & 07777);
  out_print(res,
	    "<tr><td>UID</td><td>%s%d</font></td></tr>",
	    (s->uid&&s->uid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_uid);
  out_print(res,
	    "<tr><td>GID</td><td>%s%d</font></td></tr>",
	    (s->gid&&s->gid->has_error)?
	    "<font color='#ff0000'>":
	    "<font>",
	    (int)stat_buf.st_gid);

  print_alerts(res, s->maillist);

  out_print(res, "</table>");

  print_buttons(req, res, s);
  
}


static void do_service_process(HttpRequest req, HttpResponse res, Service_T s) {
  
  int run= is_process_running(s);
  
  out_print(res,
	    "<p><br><h3>Process status</h3><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"30%\"><b>Parameter</b></td>"
	    "<td width=\"70%\"><b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Name</td><td>%s</td></tr>", s->name);
  out_print(res,
	    "<tr><td>Process id </td><td>%d</td></tr>", run);
  out_print(res,
	    "<tr><td>Process status</td><td>%s</td></tr>",
	    run?"<font color='#00ff00'>Running</font>":
	    "<font color='#ff0000'>Not Running</font>");
  out_print(res,
	    "<tr><td>Pid file</td><td>%s</td></tr>",
	    s->path);

  print_service_common_params(res, s);
  
  if(Run.doprocess) {
    ProcInfo_T pi= s->procinfo;
    
    out_print(res,
	      "<tr><td>CPU usage</td><td>%.1f%%</td></tr>",
	      pi->cpu_percent/10.0);
    out_print(res,
	      "<tr><td>Memory usage</td><td>%.1f%% [%ldkB]</td></tr>",
	      pi->mem_percent/10.0,pi->mem_kbyte);
    out_print(res,
              "<tr><td>Children</td><td>%d</td></tr>",
              pi->children);
    out_print(res,
              "<tr><td>Total memory usage (incl. children) </td>"
	      "<td>%.1f%% [%ldkB]</td></tr>",
              pi->total_mem_percent/10.0,pi->total_mem_kbyte);
  }
  {
    Port_T n;
    for(n= s->portlist; n; n= n->next) {
      if(n->family == AF_INET) {
	if(n->SSL.use_ssl) {
	  out_print(res,
		    "<tr><td>Host:Port</td>"
		    "<td>if failed %s:%d%s [%s via SSL] then %s</td>"
		    "</tr>", n->hostname, n->port, n->request?n->request:"",
		    n->protocol->name, actionnames[n->action]);
	  
	  if(n->SSL.certmd5 != NULL) {
	    out_print(res,
	      "<tr><td>Server certificate md5 sum</td><td>%s</td></tr>",
		      n->SSL.certmd5);
	  }
	} else {
	  out_print(res,
		    "<tr><td>Host:Port</td>"
		    "<td>if failed %s:%d%s [%s] then %s</td></tr>",
		    n->hostname, n->port, n->request?n->request:"",
		    n->protocol->name, actionnames[n->action]);
	}
      } else if(n->family == AF_UNIX) {
	out_print(res,
		  "<tr><td>Unix Socket</td>"
		  "<td>if failed %s [%s] then %s</td></tr>",
		  n->pathname, n->protocol->name, actionnames[n->action]);
      }
    }
  }
  {
    Resource_T q;
    
    for (q= s->resourcelist; q; q= q->next) {
      switch (q->resource_id) {
	
      case RESOURCE_ID_CPU_PERCENT: 
	  
	  out_print(res,"<tr><td>CPU usage limit</td>"
		    "<td>If %s %.1f%% for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_MEM_PERCENT: 
	  
	  out_print(res,"<tr><td>Memory usage limit</td>"
		    "<td>If %s %.1f%% for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_MEM_KBYTE: 
	  
	  out_print(res,"<tr><td>Memory amount limit</td>"
		    "<td>If %s %ld for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_LOAD1: 
	  
	  out_print(res,"<tr><td>Load average (1min)</td>"
		    "<td>If %s %.1f for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_LOAD5: 
	  
	  out_print(res,"<tr><td>Load average (5min)</td>"
		    "<td>If %s %.1f for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_LOAD15: 
	  out_print(res,"<tr><td>Load average (15min)</td>"
		    "<td>If %s %.1f for %d cycle(s) then %s</td></tr>", 
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle, 
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_CHILDREN:
	  out_print(res,"<tr><td>Children</td>"
		    "<td>If %s %d for %d cycle(s) then %s</td></tr>",
		    operatornames[q->operator],
		    q->limit, q->max_cycle,
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_TOTAL_MEM_KBYTE:
	  out_print(res,"<tr><td>Memory amount limit (incl. children)</td>"
		    "<td>If %s %d for %d cycle(s) then %s</td></tr>",
		    operatornames[q->operator],
		    q->limit, q->max_cycle,
		    actionnames[q->action]);
	  break;
	  
      case RESOURCE_ID_TOTAL_MEM_PERCENT:
	  out_print(res,"<tr><td>Memory usage limit (incl. children)</td>"
		    "<td>If %s %.1f%% for %d cycle(s) then %s</td></tr>",
		    operatornames[q->operator],
		    q->limit/10.0, q->max_cycle,
		    actionnames[q->action]);
	  break;
	  
      }    
    }
  }
  
  print_alerts(res, s->maillist);
  
  out_print(res, "</table>");
  
  print_buttons(req, res, s);

}
 

static void do_service_host(HttpRequest req, HttpResponse res, Service_T s) {

  Icmp_T icmp;
  Port_T port;
  int available= TRUE;
  
  for(icmp= s->icmplist; icmp; icmp= icmp->next) {
    if(!icmp->is_available) {
      available= FALSE;
      break;
    }
  }

  for(port= s->portlist; port; port= port->next) {
    if(!port->is_available) {
      available= FALSE;
      break;
    }
  }

  out_print(res,
	    "<p><br><h3>Host status</h3><br>");
  out_print(res,"<table cellspacing=0 cellpadding=3 border=1 width=\"90%\">"
	    "<tr><td width=\"30%\"><b>Parameter</b></td>"
	    "<td width=\"70%\"><b>Value</b></td></tr>");
  out_print(res,
	    "<tr><td>Name</td><td>%s</td></tr>", s->name);
  out_print(res,
	    "<tr><td>Remote status</td><td>%s</td></tr>",
	    available?"<font color='#00ff00'>Available</font>":
	    "<font color='#ff0000'>Not available</font>");
  
  print_service_common_params(res, s);

  {
    Port_T n;
    for(n= s->portlist; n; n= n->next) {
      if(n->family == AF_INET) {
	if(n->SSL.use_ssl) {
	  out_print(res,
		    "<tr><td>Host:Port</td>"
		    "<td>if failed %s:%d%s [%s via SSL] then %s</td></tr>",
		    n->hostname, n->port, n->request?n->request:"",
		    n->protocol->name, actionnames[n->action]);
	  
	  if(n->SSL.certmd5 != NULL) {
	    out_print(res,
	      "<tr><td>Server certificate md5 sum</td><td>%s</td></tr>",
		      n->SSL.certmd5);
	  }
	} else {
	  out_print(res,
		    "<tr><td>Host:Port</td>"
		    "<td>if failed %s:%d%s [%s] then %s</td></tr>",
		    n->hostname, n->port, n->request?n->request:"",
		    n->protocol->name, actionnames[n->action]);
	}
      } else if(n->family == AF_UNIX) {
	out_print(res,
		  "<tr><td>Unix Socket</td>"
		  "<td>if failed %s [%s] then %s</td></tr>",
		  n->pathname, n->protocol->name, actionnames[n->action]);
      }
    }
  }
  {
    Icmp_T n;
    for(n= s->icmplist; n; n= n->next)
      out_print(res,
		"<tr><td>ICMP</td>"
		"<td>if failed %s with timeout %d seconds then %s</td></tr>",
		icmpnames[n->type], n->timeout, actionnames[n->action]);
  }
  
  print_alerts(res, s->maillist);
  
  out_print(res, "</table>");

  print_buttons(req, res, s);

}


/* ------------------------------------------------------------------------- */


static void print_alerts(HttpResponse res, Mail_T s) {

  Mail_T r;
  
  for(r= s; r; r= r->next) {
    out_print(res,
	      "<tr bgcolor=\"#EFEFEF\"><td>Alert mail to</td>"
	      "<td>%s</td></tr>", r->to?r->to:"");
    out_print(res, "<tr><td>Alert on</td><td>");

    if(r->events == (~((unsigned int)0))) {
      
      out_print(res, "All events");

    } else {
      if(IS_EVENT_SET(r->events, EVENT_START))
	  out_print(res, "Start ");
      if(IS_EVENT_SET(r->events, EVENT_STOP))
	  out_print(res, "Stop ");
      if(IS_EVENT_SET(r->events, EVENT_RESTART))
	  out_print(res, "Restart ");
      if(IS_EVENT_SET(r->events, EVENT_CHECKSUM))
	  out_print(res, "Checksum ");
      if(IS_EVENT_SET(r->events, EVENT_RESOURCE))
	  out_print(res, "Resource ");
      if(IS_EVENT_SET(r->events, EVENT_TIMEOUT))
	  out_print(res, "Timeout ");
      if(IS_EVENT_SET(r->events, EVENT_TIMESTAMP))
	  out_print(res, "Timestamp ");
      if(IS_EVENT_SET(r->events, EVENT_SIZE))
	  out_print(res, "Size ");
      if(IS_EVENT_SET(r->events, EVENT_CONNECTION))
	  out_print(res, "Connection ");
      if(IS_EVENT_SET(r->events, EVENT_PERMISSION))
	  out_print(res, "Permission ");
      if(IS_EVENT_SET(r->events, EVENT_UID))
	  out_print(res, "Uid ");
      if(IS_EVENT_SET(r->events, EVENT_GID))
	  out_print(res, "Gid ");
      if(IS_EVENT_SET(r->events, EVENT_UNMONITOR))
	  out_print(res, "Unmonitor ");
    }
      
    out_print(res, "</td></tr>");

  }

}


static void print_buttons(HttpRequest req, HttpResponse res, Service_T s) {

  char *action[]= {"monitor", "unmonitor"};
  char *label[]= {"Enable monitoring", "Disable monitoring"};

  if(is_readonly(req)) {
    /*
     * A read-only REMOTE_USER will not get access to the buttons
     */
    return;
  }
  
  out_print(res, "<table cellspacing=16><tr nowrap><td><font size=+1>");
  /* Start program */
  if(s->start)
      out_print(res, 
		"<td><form method=GET action=/%s>"
		"<input type=hidden value='start' name=action>"
		"<input type=submit value='Start service' style='font-size: "
		"12pt'></form></td>", s->name);
  /* Stop program */
  if(s->stop)
      out_print(res, 
		"<td><form method=GET action=/%s>"
		"<input type=hidden value='stop' name=action>"
		"<input type=submit value='Stop service' style='font-size: "
		"12pt'></form></td>", s->name);
  /* Restart program */
  if(s->start && s->stop)
      out_print(res, 
		"<td><form method=GET action=/%s>"
		"<input type=hidden value='restart' name=action>"
		"<input type=submit value='Restart service' style='font-size: "
		"12pt'></form></td>", s->name);
  /* (un)monitor */
  out_print(res, 
	    "<td><form method=GET action=/%s>"
	    "<input type=hidden value='%s' name=action>"
	    "<input type=submit value='%s' style='font-size: 12pt'>"
	    "</form></td></tr></table>",
	    s->name, action[s->do_monitor], label[s->do_monitor]);

}


static void print_service_common_params(HttpResponse res, Service_T s) {

  Dependant_T d;

  if(s->group)
      out_print(res,
		"<tr><td>Group</td><td><font color='#0000ff'>%s"
		"</font></td></tr>", s->group);
  
  out_print(res,
	    "<tr><td>Monitoring mode</td><td>%s</td></tr>",
	    modenames[s->mode]);
  
  out_print(res,
	    "<tr><td>Monitoring status</td><td><font color='#ff8800'>%s"
	    "</font></td></tr>",
	    statusnames[s->do_monitor]);

  for(d= s->dependantlist; d; d= d->next) {
    if(d->dependant != NULL) {
      out_print(res,"<tr><td>Depends on service </td>"
		"<td> <a href=%s> %s </a></td></tr>",
		d->dependant, d->dependant);
    }
  }
  
  if(s->start) {
    int i= 0;
    out_print(res, "<tr><td>Start program</td><td>");
    while(s->start->arg[i])
      out_print(res, "%s ", s->start->arg[i++]);
    out_print(res, "</td></tr>");
  }
  
  if(s->stop) {
    int i= 0;
    out_print(res, "<tr><td>Stop program</td><td>");
    while(s->stop->arg[i])
      out_print(res, "%s ", s->stop->arg[i++]);
    out_print(res, "</td></tr>");
  }
  
  out_print(res,
	    "<tr><td>Check service</td><td>Every %d cycle</td></tr>",
	    s->every?s->every:1);
  
  if(s->def_timeout) {
    out_print(res,
	      "<tr><td>Timeout</td><td>"
	      "Timeout if %d restart within %d cycles</td></tr>",
	      s->to_start, s->to_cycle);
  }

}


static int is_readonly(HttpRequest req) {

  if(req->remote_user) {
    return ((Auth_T)get_user_credentials(req->remote_user))->is_readonly;
  }

  return FALSE;

}


/* ----------------------------------------------------------- Status output */


static void print_status(HttpResponse res, int format) {

  Service_T s;

  set_content_type(res, "text/plain");
  status_start(res, format);
  for(s= servicelist_conf; s; s= s->next_conf) {
    if(format==XML_STATUS) {
      status_service_xml(s, res);
    } else {
      /* Text status is default */
      status_service_text(s, res);
    }
  }
  status_stop(res, format);

}


static void status_start(HttpResponse res, int format) {
  
  char *uptime= get_process_uptime(Run.pidfile, " ");
  
  if(format==XML_STATUS) {
    out_print(res, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n");
    out_print(res, "<monit>\r\n\t<server>\r\n\t\t<version>%s</version>"
	      "\r\n\t\t<uptime>%s</uptime>\r\n\t</server>\r\n", 
	      VERSION, trim(uptime));
  } else {
    /* Text status is default */
    out_print(res, "The monit daemon uptime: %s\n\n", uptime);
  }

  FREE(uptime);
  
}


static void status_stop(HttpResponse res, int format) {
 
  if(format==XML_STATUS)
    out_print(res, "</monit>\r\n");
  
}


static void status_service_xml(Service_T s, HttpResponse res) {
  
  out_print(res, "\t<service>\r\n\t\t<type>%s</type>\r\n\t\t<name>%s</name>"
	    "\r\n\t\t<status>%d</status>\r\n\t\t<monitored>%d</monitored>"
	    "\r\n\t</service>\r\n",
	    status_text[s->type].type, 
	    s->name, get_service_status(s), s->do_monitor);
  
}


static void status_service_text(Service_T s, HttpResponse res) {
  
  char prefix[STRLEN];
  
  snprintf(prefix, STRLEN, "%s '%s'", status_text[s->type].type, s->name);
  out_print(res, "%-35s %s\n", prefix,
	    !s->do_monitor?"not monitored":
	    get_service_status(s)?status_text[s->type].up:
	    status_text[s->type].down);
  
}


static int get_service_status(Service_T s) {

  struct stat stat_buf;
  
  switch(s->type) {
  case TYPE_PROCESS:
    return (is_process_running(s)>0);
    break;
  case TYPE_DEVICE:
    if(DeviceInfo_Usage(s->devinfo, s->path) && 
       stat(s->path, &stat_buf) == 0) {
      if(s->uid&&s->uid->has_error)
	return FALSE;
      else if(s->gid&&s->gid->has_error)
	return FALSE;
      else
	return TRUE;
    } else {
      return FALSE;
    }
    break;
  case TYPE_FILE:
      if((stat(s->path, &stat_buf) == 0) && S_ISREG(stat_buf.st_mode)) {
	if(s->perm&&s->perm->has_error)
	  return FALSE;
	else if(s->uid&&s->uid->has_error)
	  return FALSE;
	else if(s->gid&&s->gid->has_error)
	  return FALSE;
	else 
	  return TRUE;
      } else {
	return FALSE;
      }
      break;
  case TYPE_DIRECTORY:
    if((stat(s->path, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode)) {
      if(s->perm&&s->perm->has_error)
	return FALSE;
      else if(s->uid&&s->uid->has_error)
	return FALSE;
      else if(s->gid&&s->gid->has_error)
	return FALSE;
      else
	return TRUE;
    } else {
      return FALSE;
    }
    break;
  case TYPE_REMOTE:
    if(s->portlist) {
	Port_T port;
	int available= TRUE;
	for(port= s->portlist; port; port= port->next) {
	  if(!port->is_available) {
	    available= FALSE;
	    break;
	  }
	}
	return available;
    } else {
      return FALSE;
    }
    break;
  default:
    break;
  }
  
  return FALSE;

}
