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

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "monitor.h"

/**
 *  Signal handeling routines.
 *
 *  @author W. Richard Stevens
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *     
 *  @version \$Id: signal.c,v 1.10 2004/01/29 17:52:11 martinp Exp $
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


/**
 * Replace the standard signal() function, with a more reliable
 * using sigaction. From W. Richard Stevens' "Advanced Programming
 * in the UNIX Environment"
 */
Sigfunc *signal(int signo, Sigfunc *func) {
  
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef  SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;   /* SunOS */
#endif
  } else {
#ifdef  SA_RESTART
    act.sa_flags |= SA_RESTART;             /* SVR4, 44BSD */
#endif
  }
  if (sigaction(signo, &act, &oact) < 0)
      return(SIG_ERR);
  
  return(oact.sa_handler);
  
}


/**
 * Set a collective thread signal block for signals honored by monit
 * @param new The signal mask to use for the block
 * @param old The signal mask used to save the previous mask
 */
void set_signal_block(sigset_t *new, sigset_t *old) {

  sigemptyset(new);
  sigaddset(new, SIGHUP);
  sigaddset(new, SIGINT);
  sigaddset(new, SIGUSR1);
  sigaddset(new, SIGTERM);
  pthread_sigmask(SIG_BLOCK, new, old);

}


/**
 * Set the thread signal mask back to the old mask
 * @param old The signal mask to restore
 */
void unset_signal_block(sigset_t *old) {

  pthread_sigmask(SIG_SETMASK, old, NULL);
  
}
