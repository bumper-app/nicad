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

#include "monitor.h"


/**
 *  Manage service information persistently. Service data is saved to
 *  a state file when monit runs in daemon mode for each poll
 *  cycle. Monit use this file to recover from a crash or to maintain
 *  service data persistently during a reload. The location of the
 *  state file may be set from the command line or set in the monitrc
 *  file, if not set, the default is ~/.monit.state.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @version \$Id: state.c,v 1.18 2004/02/14 22:26:43 hauk Exp $
 *  @file
 */


/* ------------------------------------------------------------- Definitions */


/**
 * Fields from the Service_T object type, which we are interested in
 * when handling the state.
 */
typedef struct mystate {
  char name[STRLEN];   
  int  mode;       
  int  nstart;     
  int  ncycle;     
  int  do_monitor;
  int  has_permission_error;
  int  has_uid_error;
  int  has_gid_error;
} State_T;


/* -------------------------------------------------------------- Prototypes */


static void close_state(FILE *);
static FILE *open_state(const char *mode);
static void clone_state(Service_T, State_T *);
static void update_service_state(Service_T, State_T *);


/* ------------------------------------------------------------------ Public */


/**
 * Save service state information to the state file
 */
void state_save() {

  int l= 0;
  Service_T s;
  State_T state;
  FILE *S= NULL;
  sigset_t ns,os;
  
  if(! (S= open_state("w")))
      return;

  set_signal_block(&ns, &os);

  l= get_service_list_length();
  
  if(fwrite(&l, 1, sizeof (int), S) != sizeof(int)) {
    log("%s: Unable to save monit state information to '%s'\n",
	prog, Run.statefile);
    goto error;
  }
  
  for(s= servicelist; s; s= s->next) {
    clone_state(s, &state);
    if(fwrite(&state, 1, sizeof(State_T), S) != sizeof(State_T)) {
      log("%s: An error occured when saving monit state information "
	  "for the service %s\n", prog, s->name);
      goto error;
    }
  }
  
  error:
  close_state(S);
  unset_signal_block(&os);
  
}


/**
 * Check if we should update current services with persistent state
 * information. The logic is as follows: Iff a state file is present
 * *and* older than the running monit daemon's lock file we have had a
 * crash and should update data from the state file.
 * @return TRUE if the state should be updated otherwise FALSE
 */
int state_should_update() {

  if(exist_file(Run.statefile) && exist_file(Run.pidfile)) {
    
    return (get_timestamp(Run.pidfile, S_IFREG) >
	    get_timestamp(Run.statefile, S_IFREG));
    
  }

  return FALSE;
  
}


/**
 * Update the current service list with data from the state file. We
 * do *only* change services found in *both* the monitrc file and in
 * the state file. The algorithm:
 *
 * Assume the control file was changed and a new service (B) was added
 * so the monitrc file now contains the services: A B and C. The
 * running monit daemon only knows the services A and C. Upon restart
 * after a crash the monit daemon first read the monitrc file and
 * creates the service list structure with A B and C. We then read the
 * state file and update the service A and C since they are found in
 * the state file, B is not found in this file and therefore not
 * changed.
 *
 * The same strategy is used if a service was removed, e.g. if the
 * service A was removed from monitrc; when reading the state file,
 * service A is not found in the current service list (the list is
 * always generated from monitrc) and therefore A is simply discarded.
 *
 * Finally, after the monit service state is updated this function
 * writes the new state file.
 */
void state_update() {

  int i;
  int l= 0;
  State_T s;
  FILE *S= NULL;
  sigset_t ns, os;
  Service_T service;
  int has_error= FALSE;
  
  if(! (S= open_state("r")))
      return;
  
  set_signal_block(&ns, &os);

  errno= 0;
  if(fread(&l, 1, sizeof (int), S) != sizeof(int)) {
    log("%s: Unable to read monit state information from '%s'\n",
	prog, Run.statefile);
    has_error= TRUE;
    goto error;
  }

  if(l > 0) {
    for(i=0; i<l; i++) {
      if(fread(&s, 1, sizeof(State_T), S) != sizeof(State_T)) {
	log("%s: An error occured when updating monit state information\n",
	    prog);
	has_error= TRUE;
	goto error;
      }
      if((service= get_service(s.name))) {
	update_service_state(service, &s);
      }
    }
  }

  error:
  close_state(S);
  unset_signal_block(&os);

  if(!has_error)
      state_save();

}


/* ----------------------------------------------------------------- Private */


static FILE *open_state(const char *mode) {

  FILE *S= NULL;
  
  ASSERT(Run.statefile);

  umask(MYPIDMASK);
  
  if((S= fopen(Run.statefile, mode)) == NULL) {
    log("%s: Cannot open the monit state file '%s' -- %s\n",
	prog, Run.statefile, STRERROR);
    
    return NULL;
    
  }

  return S;
  
}


static void close_state(FILE *S) {

  ASSERT(fclose(S)==0);

}


static void clone_state(Service_T service, State_T *state) {
  
  memset(state, 0, sizeof(State_T));
  
  strncpy(state->name, service->name, STRLEN);
  state->mode= service->mode;
  state->nstart= service->nstart;
  state->ncycle= service->ncycle;
  state->do_monitor= service->do_monitor;
  state->has_permission_error= service->perm?service->perm->has_error:FALSE;
  state->has_uid_error= service->uid?service->uid->has_error:FALSE;
  state->has_gid_error= service->gid?service->gid->has_error:FALSE;

}


static void update_service_state(Service_T service, State_T *state) {

  service->mode= state->mode;
  service->nstart= state->nstart;
  service->ncycle= state->ncycle;
  service->do_monitor= state->do_monitor;
  if(service->perm)
    service->perm->has_error= state->has_permission_error;
  if(service->uid)
    service->uid->has_error= state->has_uid_error;
  if(service->gid)
    service->gid->has_error= state->has_gid_error;
  
}
