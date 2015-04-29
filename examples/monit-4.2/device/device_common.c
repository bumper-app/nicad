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

/**
 *  System independent device methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: device_common.c,v 1.1 2004/02/29 22:24:44 martinp Exp $
 *
 *  @file
 */


#include <config.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "monitor.h"
#include "device.h"
#include "device_sysdep.h"


/**
 * This function validates whether given object is valid for filesystem
 * informations statistics and stores path suitable for it in given
 * device information structure for later use. Filesystem must be mounted.
 *
 * Valid objects are file or directory that are part of requested
 * filesystem, block special device or mountpoint.
 *
 * In the case of file, directory or mountpoint the result is original
 * object, in the case of block special device mountpoint is returned.
 *
 * @param devinfo Information structure where resulting data will be stored
 * @param object  Identifies appropriate device object
 * @return        NULL in the case of failure otherwise filesystem path
 */
char *DeviceInfo_Path(DeviceInfo_T devinfo, char *object) {

  struct stat buf;

  ASSERT(devinfo);
  ASSERT(object);

  if(stat(object, &buf) != 0) {
    log("%s: Cannot stat '%s' -- %s\n", prog, object, STRERROR);
    return NULL;
  }

  if(S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode)) {

    return strncpy(devinfo->mntpath, object, sizeof(devinfo->mntpath));

  } else if(S_ISBLK(buf.st_mode)) {

    return DeviceInfo_MountPoint_sysdep(devinfo, object);

  }

  log("%s: Not file, directory or block special device: '%s'", prog, object);

  return NULL;

}


/**
 * Filesystem usage statistics. In the case of success result is stored in
 * given information structure.
 *
 * @param devinfo Information structure where resulting data will be stored
 * @param object  Identifies requested device - either file, directory,
 *                block special device or mountpoint
 * @return        TRUE if informations were succesfully read otherwise FALSE
 */
int DeviceInfo_Usage(DeviceInfo_T devinfo, char *object) {

  ASSERT(devinfo);
  ASSERT(object);

  if(!DeviceInfo_Path(devinfo, object)) {
    return FALSE;
  }

  return DeviceInfo_Usage_sysdep(devinfo);

}

