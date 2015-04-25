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

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif


#include "monitor.h"
#include "net.h"
#include "ssl.h"
#include "process.h"

/**
 *  DESCRIPTION
 *    monit - system for monitoring services on a Unix system
 *
 *  SYNOPSIS
 *    monit [options] {arguments}
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala <martinp@tildeslash.com>
 *  @author Christian Hopp, <chopp@iei.tu-clausthal.de>
 *
 *  @version \$Id: monitor.c,v 1.100 2004/03/13 00:10:13 martinp Exp $
 *
 *  @file
 */


/* -------------------------------------------------------------- Prototypes */


static void  do_init();                       /* Initialize this application */
static RETSIGTYPE do_reload(int);       /* Signalhandler for a daemon reload */
static void  do_reinit();           /* Re-initialize the runtime application */
static void  do_action(char **);         /* Dispatch to the submitted action */
static RETSIGTYPE  do_destroy(int);  /* Signalhandler for monit finalization */
static void  do_exit();                                    /* Finalize monit */
static void  do_default();                              /* Do default action */
static RETSIGTYPE  do_wakeup(int); /* Signalhandler for a daemon wakeup call */
static int   do_wakeupcall();              /* Wakeup a sleeping monit daemon */
static void  do_signal_block();           /* Block all signals used by monit */
static void  handle_options(int, char **);         /* Handle program options */
static void  help();                 /* Print program help message to stdout */
static void  version();                         /* Print version information */


/* ------------------------------------------------------------------ Public */


/**
 * The Prime mover
 */
int main(int argc, char **argv) {

  prog= stripfilename(argv[0]);
  init_env();
  handle_options(argc, argv);
 
  do_init();
  do_action(argv); 
  do_signal_block();
  do_exit();

  exit(0);
  
}


/* ----------------------------------------------------------------- Private */


/**
 * Initialize this application - Register signal handlers,
 * Parse the control file and initialize the program's
 * datastructures and the log system.
 */
static void do_init() {

  int status;

  /*
   * Register interest for the SIGTERM signal,
   * in case we run in daemon mode this signal
   * will terminate a running daemon.
   */
  signal(SIGTERM, do_destroy);

  /*
   * Register interest for the SIGUSER1 signal,
   * in case we run in daemon mode this signal
   * will wakeup a sleeping daemon.
   */
  signal(SIGUSR1, do_wakeup);

  /*
   * Register interest for the SIGINT signal,
   * in case we run as a server but not as a daemon
   * we need to catch this signal if the user pressed
   * CTRL^C in the terminal
   */
  signal(SIGINT, do_destroy);

  /*
   * Register interest for the SIGHUP signal,
   * in case we run in daemon mode this signal
   * will reload the configuration.
   */
  signal(SIGHUP, do_reload);

  /*
   * Register no interest for the SIGPIPE signal,
   */
  signal(SIGPIPE, SIG_IGN);

  /*
   * Initialize the Runtime mutex. This mutex
   * is used to synchronize handling of global
   * service data
   */
  status= pthread_mutex_init(&Run.mutex, NULL);
  if(status != 0) {
    log("%s: Cannot initialize mutex -- %s\n", prog, strerror(status));
    exit(1);
  }

  /* 
   * Get the position of the control file 
   */
  if(! Run.controlfile) {
    
    Run.controlfile= find_rcfile();
    
  }
  
  /*
   * Initialize the process information gathering interface
   */
  Run.doprocess= init_process_info();

  /*
   * Start the Parser and create the service list. This will also set
   * any Runtime constants defined in the controlfile.
   */
  if(! parse(Run.controlfile)) {
    
    exit(1);
    
  }

  /*
   * Stop and report success if we are just validating the Control
   * file syntax. The previous parse statement exits the program with
   * an error message if a syntax error is present in the control
   * file.
   */
  if(Run.testing) {

    log("Control file syntax OK\n");
    exit(0);

  }

  /*
   * Initialize the log system 
   */
  if(! log_init()) {
    
    exit(1);
    
  }

  /* 
   * Did we find any service ?  
   */
  if(! servicelist) {
    
    log("%s: No services has been specified\n", prog);
    exit(0);
    
  }
  
  /* 
   * Initialize Runtime file variables 
   */
  init_files();

  /* 
   * Should we print debug information ? 
   */
  if(Run.debug) {
    
    printrunlist();
    printservicelist();
    
  }

}


/**
 * Re-Initialize the application - called if a
 * monit daemon receives the SIGHUP signal.
 */
static void do_reinit() {

  char *bind_addr;
  int port= Run.httpdport;

  Run.doreload= FALSE;
  
  bind_addr= Run.bind_addr?xstrdup(Run.bind_addr):NULL;

  log("Awakened by the SIGHUP signal\n");
  log("Reinitializing %s - Control file '%s'\n", prog, Run.controlfile);
  
  /* Run the garbage collector */
  gc();

  if(! parse(Run.controlfile)) {
    
    log("%s daemon died\n", prog);
    exit(1);
    
  }

  /* Close the current log */
  log_close();

  /* Reinstall the log system */
  if(! log_init()) {
    
    exit(1);
    
  }

  /* Did we find any services ?  */
  if(! servicelist) {
    
    log("%s: No services has been specified\n", prog);
    exit(0);
    
  }
  
  /* Reinitialize Runtime file variables */
  init_files();

  if(! create_pidfile(Run.pidfile)) {
      
    log("%s daemon died\n", prog);
    exit(1);
      
  }

  /* Update service data from the state repository */
  state_update();
  
  if(! can_http()) {
	  
    monit_http(STOP_HTTP);
	  
  } else if(!IS(bind_addr, Run.bind_addr) || port != Run.httpdport) {
	  
    monit_http(STOP_HTTP);
    monit_http(START_HTTP);
	  
  } else if(! check_httpd()) {

    monit_http(START_HTTP);
	  
  }

  FREE(bind_addr);

}


/**
 * Dispatch to the submitted action - actions are program arguments
 */
static void do_action(char **args) {
  
  char *action= args[optind];
  char *P= args[++optind];

  Run.once= TRUE;

  if(! action) {
    do_default();
  } else if(IS(action, "start")     ||
            IS(action, "stop")      ||
            IS(action, "monitor")   ||
            IS(action, "unmonitor") ||
            IS(action, "restart")      ) {
    if(P) {
      if(! IS(P, "all")) {
        control_process(P, action);
      } else {
	if(Run.mygroup)
	  control_group(Run.mygroup, action);
	else
	  control(action);
      }
    } else {
      goto error;
    }
  } else if(IS(action, "reload")) {
    log("Reinitializing monit daemon\n", prog);
    kill_daemon(SIGHUP);
  } else if(IS(action, "status")) {
    status();
  } else if(IS(action, "quit")) {
    kill_daemon(SIGTERM);
  } else if(IS(action, "validate")) {
    validate();
  } else {
  error:
    log("%s: invalid argument -- %s  (-h will show valid arguments)\n",
        prog, action);
    exit(1);
  }
  reset_depend();
  
}


/**
 * Signalhandler for a daemon wakeup call
 */
static RETSIGTYPE do_wakeup(int sig) {

  log("Awakened by User defined signal 1\n");

}


/**
 * Signalhandler for a daemon reload call
 */
static RETSIGTYPE do_reload(int sig) {

  Run.doreload= TRUE;
  
}


/**
 * Wakeup a sleeping monit daemon.
 * Returns TRUE on success otherwise FALSE
 */
static int do_wakeupcall() {

  pid_t pid;
  
  if((pid= exist_daemon()) > 0) {
    
    kill(pid, SIGUSR1);
    log("%s daemon at %d awakened\n", prog, pid);
    
    return TRUE;
    
  }
  
  return FALSE;
  
}


/**
 * Signalhandler for monit finalization
 */
static RETSIGTYPE do_destroy(int sig) {
  
  Run.stopped= TRUE;
  
}


/**
 * Finalize monit
 */
static void do_exit() {
  
  Run.stopped= TRUE;

  if(Run.isdaemon && !Run.once) {

    if(Run.dohttpd)
      monit_http(STOP_HTTP);

    log("%s daemon with pid [%d] killed\n", prog, (int)getpid());

  }
  gc();
  exit(0);
 
}


/**
 * Default action - become a daemon if defined in the Run object and
 * run validate() between sleeps. If not, just run validate() once.
 * Also, if specified, start the monit http server if in deamon mode.
 */
static void do_default() {

  if(Run.isdaemon) {
    
    if(do_wakeupcall())
      exit(0);
  
    Run.once= FALSE;

    log("Starting %s daemon\n", prog);
    
    if(can_http()) {
      log("Starting httpd at [%s:%d]\n",
	  Run.bind_addr?Run.bind_addr:"*", Run.httpdport);
    }
    
    if(Run.init != TRUE)
      daemonize(); 
    
    if(! create_pidfile(Run.pidfile)) {
      log("%s daemon died\n", prog);
      exit(1);
    }

    if(state_should_update())
      state_update();

    atexit(finalize_files);
  
    if(can_http())
      monit_http(START_HTTP);
    
    for(;;) {

      validate();
      state_save();
      sleep(Run.polltime);

      if(Run.doreload)
	do_reinit();
      
      if(Run.stopped) {
        do_signal_block();
	do_exit();
      }
      
    }
    
  } else {
    
    validate();
    
  }

}


/*
 * Block all signals used by monit. This method is called at
 * application termination to avoid signals to interrupt a graceful
 * shutdown.
 */
static void  do_signal_block() {
  
  sigset_t block;

  set_signal_block(&block, NULL);
  
}


/**
 * Handle program options - Options set from the commandline
 * takes precedence over those found in the control file
 */
static void handle_options(int argc, char **argv) {
  
  int opt;

  opterr= 0;

  Run.mygroup=0;

  while((opt= getopt(argc,argv,"c:d:g:l:p:s:iItvVh")) != -1) {

    switch(opt) {

    case 'c':
        Run.controlfile= xstrdup(optarg);
        break;
	
    case 'd':
	Run.isdaemon= TRUE;
 	sscanf(optarg, "%d", &Run.polltime);
	if(Run.polltime<1) {
	  log("%s: option -%c requires a natural number\n", prog, opt);
	  exit(1);
	}
	break;

    case 'g':
        Run.mygroup= xstrdup(optarg);
        break;
	
    case 'l':
        Run.logfile= xstrdup(optarg);
	if(IS(Run.logfile, "syslog"))
	    Run.use_syslog= TRUE;
	Run.dolog= TRUE;
        break;
   
    case 'p':
        Run.pidfile= xstrdup(optarg);
        break;

    case 's':
        Run.statefile= xstrdup(optarg);
        break;

    case 'i':
	log("%s: option -i is obsolete\n", prog);
	break;
      
    case 'I':
	Run.init= TRUE;
	break;
      
    case 't':
        Run.testing= TRUE;
        break;
	
    case 'v':
        Run.debug= TRUE;
        break;
	
    case 'V':
        version();
        exit(0);
	break;
	
    case 'h':
        help();
        exit(0);
	break;
	
    case '?':
	switch(optopt) {
	  
	case 'c':
	case 'd':
	case 'g':
	case 'l':
	case 'p':
	case 's':
	    log("%s: option -- %c requires an argument\n", prog, optopt);
	    break;
	default:
	    log("%s: invalid option -- %c  (-h will show valid options)\n",
		  prog, optopt);
	    
	}
	
	exit(1);
	
    }
    
  }
  
}


/**
 * Print the program's help message
 */
static void help() {
  
  printf("Usage: %s [options] {arguments}\n", prog);
  printf("Options are as follows:\n");
  printf(" -c file      Use this control file\n");
  printf(" -d n         Run as a daemon once per n seconds\n");
  printf(" -g name      Set group name for start, stop, restart and status\n");
  printf(" -l logfile   Print log information to this file\n");
  printf(" -p pidfile   Use this lock file in daemon mode\n");
  printf(" -s statefile Set the file monit should write state information to\n");
  printf(" -I           Do not run in background (needed for run from init)\n");
  printf(" -t           Run syntax check for the control file\n");
  printf(" -v           Verbose mode, work noisy (diagnostic output)\n");
  printf(" -V           Print version number and patchlevel\n");
  printf(" -h           Print this text\n");
  printf("Optional action arguments for non-daemon mode are as follows:\n");
  printf(" start all      - Start all services\n");
  printf(" start name     - Only start the named service\n");
  printf(" stop all       - Stop all services\n");
  printf(" stop name      - Only stop the named service\n");
  printf(" restart all    - Stop and start all services\n");
  printf(" restart name   - Only restart the named service\n");
  printf(" monitor all    - Enable monitoring of all services\n");
  printf(" monitor name   - Only enable monitoring of the named service\n");
  printf(" unmonitor all  - Disable monitoring of all services\n");
  printf(" unmonitor name - Only disable monitoring of the named service\n");
  printf(" reload         - Reinitialize monit\n");
  printf(" status         - Print status information for each service\n");
  printf(" quit           - Kill monit daemon process\n");
  printf(" validate       - Check all services and start if not running\n");
  printf("\n");
  printf("(Action arguments operate on services defined in the control file)\n");

}


/**
 * Print version information
 */
static void version() {
  
  printf("This is monit version %s\n", VERSION);
  printf("Copyright (C) 2000-2004 by the monit project group.");
  printf(" All Rights Reserved.\n");
 
}

