/*
 * Copyright (C), 1998 by Eric S. Raymond.
 * Copyright (C), 2000-2004 by the monit project group.
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include "monitor.h"


/**
 *  Memory allocation routines - Makes the program die "nicely" if
 *  request for more memory fails. Copied from the fetchmail code,
 *  extended and massaged a bit to suite the monit code and coding
 *  style.
 *
 *  @author Eric S. Raymond <esr@snark.thyrsus.com>
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: xmalloc.c,v 1.11 2004/01/29 17:52:11 martinp Exp $
 *
 *  @file
 */


/* ---------------------------------------------------------------- Public */


void *xmalloc(int n) {
  
    void *p;

    p= (void *)malloc(n);

/* Some malloc's don't return a valid pointer if you malloc(0), so check
   for that only if necessary. */

#if ! HAVE_MALLOC
    if ( n == 0) {

      log("%s: passed a broken malloc 0\n", prog);
      exit(1);

    }
#endif

    if ( p == NULL ) {
      
      log("%s: malloc failed -- %s\n", prog, STRERROR);
      exit(1);
      
    }
    
    return p;

}

void *xcalloc(long count, long nbytes) {
  
    void *p;

    p= (void *)calloc(count, nbytes);
    if ( p == NULL ) {
      
      log("%s: malloc failed -- %s\n", prog, STRERROR);
      exit(1);
      
    }
    
    return p;

}


char *xstrdup(const char *s) {
  
  char *p;

  ASSERT(s);
  
  p= (char *)xmalloc(strlen(s)+1);
  strcpy(p, s);
  
  return p;
  
}


char *xstrndup(const char *s, long l) {

  char *t;

  ASSERT(s);
  
  t= xmalloc(l + 1);
  strncpy(t, s, l);
  t[l]= 0;

  return t;
  
}


void *xresize(void *p, long nbytes) {

  if (p == 0) {
    
    return xmalloc(nbytes);

  }

  p= realloc(p, nbytes);
  if(p == NULL) {
    
    log("%s: realloc failed -- %s\n", prog, STRERROR);
    exit(1);
    
  }
  
  return p;
  
}
 

#if ! HAVE_MALLOC
#undef malloc

void *malloc (size_t);

void *rpl_malloc (size_t __size)
{   
  if (__size == 0) {
    __size++; 
  }
  return malloc(__size);
}
#endif
