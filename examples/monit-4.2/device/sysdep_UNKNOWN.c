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
 *  @version \$Id: sysdep_UNKNOWN.c,v 1.6 2004/02/29 22:24:45 martinp Exp $
 *
 *  @file
 */

#include <config.h>

#include "monitor.h"
#include "device_sysdep.h"

/**
 * Unknown OS special block device dummy mountpoint method.
 *
 * @param devinfo  Information structure
 * @param blockdev Identifies block special device
 * @return         NULL
 */
char *DeviceInfo_MountPoint_sysdep(DeviceInfo_T devinfo, char *blockdev) {

  log("%s: Unsupported mounted filesystem information method", prog);

  return NULL;

}


/**
 * Unknown OS filesystem dummy usage statistics.
 *
 * @param devinfo Information structure
 * @return        FALSE
 */
int DeviceInfo_Usage_sysdep(DeviceInfo_T devinfo) {

  log("%s: Unsupported filesystem informations gathering method\n", prog);

  return FALSE;

}

