/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:lib/libmb2/icslib.c	1.3"

#ifndef lint
static char icslibcopyright[] = "Copyright 1986, 1987, 1988 Intel Corporation 460953";
#endif /* lint */

/*****************************************************************************
 *
 * TITLE:	InterConnect Space Library
 *
 *		Contains some handy functions for dealing with Interconnect Space.
 *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/fs/s5dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/ics.h>
#include <stdio.h>

extern int errno;

/*****************************************************************************
 *
 * TITLE:	ics_find_rec
 *
 * ABSTRACT:	Given an open file descriptor for the interconnect driver,
 *				the slot, and register type, icsfrec returns the register
 *				offset in the interconnect space of the given slot.
 *				If a system error occurred, icsfrec returns -1.
 *				If the record cannot be found, icsfrec returns -2.
 *				NOTE: Let the kernel do most of the work.
 *
 ****************************************************************************/
int
ics_find_rec(devfd, slot, record)
int	devfd;
unsigned short slot;
unsigned char  record;
{
	struct ics_frec_args	ioc;

	ioc.ics_slot = slot;
	ioc.ics_recid = record;
	if(ioctl(devfd, ICS_FIND_REC, (caddr_t)&ioc) == -1)
		return(-1);
	if(ioc.ics_rval == 0xffff) {
		errno = EFAULT;
		return(-2);
	}
	return((int)ioc.ics_rval);
}

/*****************************************************************************
 *
 * TITLE:	ics_my_slot
 *
 * ABSTRACT:	Given an open file descriptor for the interconnect driver,
 *				get the slot id for the cpu board that we are on.
 *				If a system error occurred, icsmyslot returns -1.
 *				NOTE: Let the kernel do most of the work.
 *
 ****************************************************************************/
int
ics_my_slot(devfd)
int	devfd;
{
	unsigned char	slotid;

	if(ioctl(devfd, ICS_MY_SLOTID, (caddr_t)&slotid) == -1)
		return(-1);
	return((int)slotid);
}


/*****************************************************************************
 *
 * TITLE:	ics_my_cpu
 *
 * ABSTRACT:	Given an open file descriptor for the interconnect driver,
 *				get the number of the cpu board that we are on.
 *				If a system error occurred, icsmyslot returns -1.
 *				If the record cannot be found, icsmyslot returns -2.
 *				NOTE: Let the kernel do most of the work.
 *
 ****************************************************************************/
int
ics_my_cpu(devfd)
int	devfd;
{
	unsigned char	mycpunum;

	if(ioctl(devfd, ICS_MY_CPUNUM, (caddr_t)&mycpunum) == -1)
		return(-1);
	return((int)mycpunum);
}


/*****************************************************************************
 *
 * TITLE:	ics_read
 *
 * ABSTRACT:	Turn the ics_read ioctl into a regular read.
 *
 ****************************************************************************/
int
ics_read(fd, slot, reg, buf, count)
int fd;
unsigned short slot;
unsigned short reg;
caddr_t buf;
int count;
{
	struct ics_rdwr_args icotl;
	if((slot>ICS_MAX_SLOT && slot != ICS_MY_SLOT_ID) || reg>ICS_MAX_REG||
			buf == NULL || count>ICS_MAX_REG-(int)reg) {
		errno = EINVAL;
		return(-1);
	}
	icotl.ics_slot = slot;
	icotl.ics_reg = reg;
	icotl.ics_buf = buf;
	icotl.ics_count = count;
	return(ioctl(fd, ICS_READ_ICS, &icotl));
}

/*****************************************************************************
 *
 * TITLE:	ics_write
 *
 * ABSTRACT:	Turn the ics_write ioctl into a regular write.
 *
 ****************************************************************************/
int
ics_write(fd, slot, reg, buf, count)
int fd;
unsigned short slot;
unsigned short reg;
caddr_t buf;
int count;
{
	struct ics_rdwr_args icotl;
	if((slot>ICS_MAX_SLOT && slot != ICS_MY_SLOT_ID) || reg>ICS_MAX_REG||
			buf == NULL || count>ICS_MAX_REG-(int)reg) {
		errno = EINVAL;
		return(-1);
	}
	icotl.ics_slot = slot;
	icotl.ics_reg = reg;
	icotl.ics_buf = buf;
	icotl.ics_count = count;
	return(ioctl(fd, ICS_WRITE_ICS, &icotl));
}
