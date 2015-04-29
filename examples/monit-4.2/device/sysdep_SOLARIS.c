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
 *  @version \$Id: sysdep_SOLARIS.c,v 1.8 2004/02/29 22:24:44 martinp Exp $
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
# include <strings.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif

#include "monitor.h"
#include "device_sysdep.h"


/**
 * Solaris special block device mountpoint method. Filesystem must be mounted.
 * In the case of success, mountpoint is stored in device information
 * structure for later use.
 *
 * @param devinfo  Information structure where resulting data will be stored
 * @param blockdev Identifies block special device
 * @return         NULL in the case of failure otherwise mountpoint
 */
char *DeviceInfo_MountPoint_sysdep(DeviceInfo_T devinfo, char *blockdev) {

  struct mnttab mnt;
  FILE         *mntfd;

  ASSERT(devinfo);
  ASSERT(blockdev);


  if((mntfd= fopen("/etc/mnttab", "r")) == NULL) {
    log("%s: Cannot open /etc/mnttab file", prog);
    return NULL;
  }

  /* First match is significant */
  while(getmntent(mntfd, &mnt) == 0) {

    if(IS(blockdev, mnt.mnt_special)) {

      fclose(mntfd);
      return strncpy(devinfo->mntpath, mnt.mnt_mountp, sizeof(devinfo->mntpath));

    }

  }

  fclose(mntfd);

  return NULL;

}


/**
 * Solaris filesystem usage statistics. In the case of success result is stored in
 * given information structure.
 *
 * @param devinfo Information structure where resulting data will be stored
 * @return        TRUE if informations were succesfully read otherwise FALSE
 */
int DeviceInfo_Usage_sysdep(DeviceInfo_T devinfo) {

  struct statvfs usage;

  ASSERT(devinfo);

  if(statvfs(devinfo->mntpath, &usage) != 0) {
    log("%s: Error getting usage statistics for device '%s' -- %s\n",
        prog, devinfo->mntpath, STRERROR);
    return FALSE;
  }

  devinfo->f_bsize=           usage.f_bsize;
  devinfo->f_blocks=          usage.f_blocks/(usage.f_frsize?(usage.f_bsize/usage.f_frsize):1);
  devinfo->f_blocksfree=      usage.f_bavail/(usage.f_frsize?(usage.f_bsize/usage.f_frsize):1);
  devinfo->f_blocksfreetotal= usage.f_bfree/(usage.f_frsize?(usage.f_bsize/usage.f_frsize):1);
  devinfo->f_files=           usage.f_files;
  devinfo->f_filesfree=       usage.f_ffree;

  return TRUE;

}

