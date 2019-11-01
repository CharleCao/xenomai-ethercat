/******************************************************************************
 *
 *  $Id$
 *
 *  Copyright (C) 2006-2012  Florian Pose, Ingenieurgemeinschaft IgH
 *
 *  This file is part of the IgH EtherCAT master userspace library.
 *
 *  The IgH EtherCAT master userspace library is free software; you can
 *  redistribute it and/or modify it under the terms of the GNU Lesser General
 *  Public License as published by the Free Software Foundation; version 2.1
 *  of the License.
 *
 *  The IgH EtherCAT master userspace library is distributed in the hope that
 *  it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the IgH EtherCAT master userspace library. If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 *****************************************************************************/

#ifndef __EC_LIB_IOCTL_H__
#define __EC_LIB_IOCTL_H__

/*****************************************************************************/

#include <sys/ioctl.h>

/*****************************************************************************/

#include "master/ioctl.h"

/*****************************************************************************/

/* FIXME : historical code inherited from difference
           between RTDM ioctl an normal ioctl
           -> should ioctl.h be removed ? */

/* libc's ioctl() always returns -1 on error and sets errno */
#define EC_IOCTL_IS_ERROR(X) ((X) == -1)
#define EC_IOCTL_ERRNO(X) (errno)

#include <errno.h>

/*****************************************************************************/

#endif /* __EC_LIB_IOCTL_H__ */

/*****************************************************************************/

