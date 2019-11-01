/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright (C) 2009-2010  Moehwald GmbH B. Benner
 *                     2011  IgH Andreas Stewering-Bone
 *                     2012  Florian Pose <fp@igh-essen.com>
 *
 *  This file is part of the IgH EtherCAT master.
 *
 *  The IgH EtherCAT master is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; version 2 of the License.
 *
 *  The IgH EtherCAT master is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the IgH EtherCAT master. If not, see <http://www.gnu.org/licenses/>.
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 ****************************************************************************/

/** \file
 * RTDM interface.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mman.h>

#include <rtdm/driver.h>

#include "master.h"
#include "ioctl.h"
#include "rtdm.h"

/** Set to 1 to enable device operations debugging.
 */
#define DEBUG 0


/****************************************************************************/

int ec_rtdm_open(struct rtdm_fd *, int);
void ec_rtdm_close(struct rtdm_fd *);
int ec_rtdm_rt_ioctl(struct rtdm_fd *,
        unsigned int, void __user *);
int ec_rtdm_ioctl(struct rtdm_fd *,
        unsigned int, void __user *);
int ec_rtdm_mmap(struct rtdm_fd *fd, struct vm_area_struct *vma);

/****************************************************************************/

static struct rtdm_driver ec_rtdm_driver = {
	.profile_info		=	RTDM_PROFILE_INFO(foo,
							  RTDM_CLASS_EXPERIMENTAL,
							  222,
							  1),
	.device_flags		=	RTDM_NAMED_DEVICE,
	.device_count		=	1,
	.context_size		=	sizeof(ec_ioctl_context_t),
	.ops = {
        .open      = ec_rtdm_open,
        .close     = ec_rtdm_close,
        .ioctl_rt  = ec_rtdm_rt_ioctl,
        .ioctl_nrt = ec_rtdm_ioctl,
        .mmap      = ec_rtdm_mmap
	},
};

/** Initialize an RTDM device.
 *
 * \return Zero on success, otherwise a negative error code.
 */
int ec_rtdm_dev_init(
        ec_rtdm_dev_t *rtdm_dev, /**< EtherCAT RTDM device. */
        ec_master_t *master /**< EtherCAT master. */
        )
{
    int ret;
    char* devlabel;

    rtdm_dev->master = master;

    rtdm_dev->dev = kzalloc(sizeof(struct rtdm_device), GFP_KERNEL);
    if (!rtdm_dev->dev) {
        EC_MASTER_ERR(master, "Failed to reserve memory for RTDM device.\n");
        return -ENOMEM;
    }

    devlabel = kzalloc(RTDM_MAX_DEVNAME_LEN+1, GFP_KERNEL);
    if (!devlabel) {
        EC_MASTER_ERR(master, "Failed to reserve memory for RTDM device name.\n");
        return -ENOMEM;
        kfree(rtdm_dev->dev);
    }
    snprintf(devlabel, RTDM_MAX_DEVNAME_LEN,
            "EtherCAT%u", master->index);

    rtdm_dev->dev->label = devlabel;
    rtdm_dev->dev->driver = &ec_rtdm_driver;
    rtdm_dev->dev->device_data = rtdm_dev; /* pointer to parent */


    EC_MASTER_INFO(master, "Registering RTDM device %s.\n",
            devlabel);
    ret = rtdm_dev_register(rtdm_dev->dev);
    if (ret) {
        EC_MASTER_ERR(master, "Initialization of RTDM interface failed"
                " (return value %i).\n", ret);
        kfree(rtdm_dev->dev->label);
        kfree(rtdm_dev->dev);
    }

    return ret;
}

/****************************************************************************/

/** Clear an RTDM device.
 */
void ec_rtdm_dev_clear(
        ec_rtdm_dev_t *rtdm_dev /**< EtherCAT RTDM device. */
        )
{
    EC_MASTER_INFO(rtdm_dev->master, "Unregistering RTDM device %s.\n",
            rtdm_dev->dev->label);
    rtdm_dev_unregister(rtdm_dev->dev);

    kfree(rtdm_dev->dev->label);
    kfree(rtdm_dev->dev);
}

/****************************************************************************/

/** Driver open.
 *
 * \return Always zero (success).
 */
int ec_rtdm_open(
        struct rtdm_fd *fd, /**< User data. */
        int oflags /**< Open flags. */
        )
{
    ec_ioctl_context_t *ctx = (ec_ioctl_context_t *) rtdm_fd_to_private(fd);
#if DEBUG
    ec_rtdm_dev_t *rtdm_dev = (ec_rtdm_dev_t *) rtdm_fd_device(fd)->device_data;
#endif

    ctx->writable = oflags & O_WRONLY || oflags & O_RDWR;
    ctx->requested = 0;
    ctx->process_data = NULL;
    ctx->process_data_size = 0;

#if DEBUG
    EC_MASTER_INFO(rtdm_dev->master, "RTDM device %s opened.\n",
            context->device->device_name);
#endif
    return 0;
}

/****************************************************************************/

/** Driver close.
 *
 * \return Always zero (success).
 */
void ec_rtdm_close(
        struct rtdm_fd *fd /**< User data. */
        )
{
    ec_ioctl_context_t *ctx = (ec_ioctl_context_t *) rtdm_fd_to_private(fd);
    ec_rtdm_dev_t *rtdm_dev = (ec_rtdm_dev_t *) rtdm_fd_device(fd)->device_data;

    if (ctx->requested) {
        ecrt_release_master(rtdm_dev->master);
	}

#if DEBUG
    EC_MASTER_INFO(rtdm_dev->master, "RTDM device %s closed.\n",
            context->device->device_name);
#endif
}

/****************************************************************************/

/** Driver ioctl.
 *
 * \return ioctl() return code.
 */
int ec_rtdm_rt_ioctl(
        struct rtdm_fd *fd, /**< User data. */
        unsigned int request, /**< Request. */
        void __user *arg /**< Argument. */
        )
{
    switch (request) {
        /*
            Requests to be handled directly in primary domain
            Note: list was made by selecting calls in ioctl.c
                  that seems not to make calls forbiden in primary mode
        */
        case EC_IOCTL_MASTER_RESCAN:
        case EC_IOCTL_SEND:
        case EC_IOCTL_RECEIVE:
        case EC_IOCTL_MASTER_STATE:
        case EC_IOCTL_MASTER_LINK_STATE:
        case EC_IOCTL_APP_TIME:
        case EC_IOCTL_SYNC_REF:
        case EC_IOCTL_SYNC_SLAVES:
        case EC_IOCTL_REF_CLOCK_TIME:
        case EC_IOCTL_SYNC_MON_QUEUE:
        case EC_IOCTL_SYNC_MON_PROCESS:
        case EC_IOCTL_SC_EMERG_POP:
        case EC_IOCTL_SC_EMERG_CLEAR:
        case EC_IOCTL_SC_EMERG_OVERRUNS:
        case EC_IOCTL_SC_STATE:
        case EC_IOCTL_DOMAIN_STATE:
        case EC_IOCTL_DOMAIN_PROCESS:
        case EC_IOCTL_DOMAIN_QUEUE:
        case EC_IOCTL_SDO_REQUEST_INDEX:
        case EC_IOCTL_SDO_REQUEST_TIMEOUT:
        case EC_IOCTL_SDO_REQUEST_STATE:
        case EC_IOCTL_SDO_REQUEST_READ:
        case EC_IOCTL_SDO_REQUEST_WRITE:
        case EC_IOCTL_SDO_REQUEST_DATA:
        case EC_IOCTL_REG_REQUEST_DATA:
        case EC_IOCTL_REG_REQUEST_STATE:
        case EC_IOCTL_REG_REQUEST_WRITE:
        case EC_IOCTL_REG_REQUEST_READ:
        case EC_IOCTL_VOE_SEND_HEADER:
        case EC_IOCTL_VOE_REC_HEADER:
        case EC_IOCTL_VOE_READ:
        case EC_IOCTL_VOE_READ_NOSYNC:
        case EC_IOCTL_VOE_WRITE:
        case EC_IOCTL_VOE_EXEC:
        case EC_IOCTL_VOE_DATA:
            return ec_rtdm_ioctl(fd, request, arg);
        default:
            break;
    }
    /* When a call is not supposed to happen in primary domain,
       syscall catches -ENOSYS, then switch to secondary mode,
       and calls .ioctl_nrt */
    return -ENOSYS;
}

int ec_rtdm_ioctl(
        struct rtdm_fd *fd, /**< User data. */
        unsigned int request, /**< Request. */
        void __user *arg /**< Argument. */
        )
{
    ec_ioctl_context_t *ctx = (ec_ioctl_context_t *) rtdm_fd_to_private(fd);
    ec_rtdm_dev_t *rtdm_dev = (ec_rtdm_dev_t *) rtdm_fd_device(fd)->device_data;

#if DEBUG
    EC_MASTER_INFO(rtdm_dev->master, "ioctl(request = %u, ctl = %02x)"
            " on RTDM device %s.\n", request, _IOC_NR(request),
            context->device->device_name);
#endif
    return ec_ioctl_rtdm(rtdm_dev->master, ctx, request, arg);
}

/****************************************************************************/

/** Memory-map process data to user space.
 *
 */
int ec_rtdm_mmap(struct rtdm_fd *fd, struct vm_area_struct *vma)
{
	size_t len;
    ec_ioctl_context_t *ctx = (ec_ioctl_context_t *) rtdm_fd_to_private(fd);
	len = vma->vm_end - vma->vm_start;
	return rtdm_mmap_kmem(vma, (void *)ctx->process_data);
}

/****************************************************************************/
