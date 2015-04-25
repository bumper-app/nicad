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
 *  System dependent device methods.
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @author Martin Pala, <martinp@tildeslash.com>
 *
 *  @version \$Id: sysdep_FREEBSD.c,v 1.9 2004/02/29 22:24:44 martinp Exp $
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#include "monitor.h"
#include "device_sysdep.h"

/**
 * FreeBSD special block device mountpoint method. Filesystem must be mounted.
 * In the case of success, mountpoint is stored in device information
 * structure for later use.
 *
 * @param devinfo  Information structure where resulting data will be stored
 * @param blockdev Identifies block special device
 * @return         NULL in the case of failure otherwise mountpoint
 */
char *DeviceInfo_MountPoint_sysdep(DeviceInfo_T devinfo, char *blockdev) {

  struct statfs usage;

  ASSERT(devinfo);
  ASSERT(blockdev);

  if(statfs(blockdev, &usage) != 0) {
    log("%s: Error getting mountpoint for device '%s' -- %s\n",
        prog, blockdev, STRERROR);
    return NULL;
  }

  return strncpy(devinfo->mntpath, usage.f_mntonname, sizeof(devinfo->mntpath));

}


/**
 * FreeBSD filesystem usage statistics. In the case of success result is stored in
 * given information structure.
 *
 * @param devinfo Information structure where resulting data will be stored
 * @return        TRUE if informations were succesfully read otherwise FALSE
 */
int DeviceInfo_Usage_sysdep(DeviceInfo_T devinfo) {

  struct statfs usage;

  ASSERT(devinfo);

  if(statfs(devinfo->mntpath, &usage) != 0) {
    log("%s: Error getting usage statistics for device '%s' -- %s\n",
        prog, devinfo->mntpath, STRERROR);
    return FALSE;
  }

  devinfo->f_bsize=           usage.f_bsize;
  devinfo->f_blocks=          usage.f_blocks;
  devinfo->f_blocksfree=      usage.f_bavail;
  devinfo->f_blocksfreetotal= usage.f_bfree;
  devinfo->f_files=           usage.f_files;
  devinfo->f_filesfree=       usage.f_ffree;

  return TRUE;

}

