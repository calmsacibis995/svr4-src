/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Driver for PC AT CMOS battery backed up ram.
 *
 * PC-DOS compatibility requirements:
 *      Nearly all locations have defined values, see
 *      the PC AT Hardware reference manual for details.
 *
 */

#ident	"@(#)at:uts/i386/io/cram.c	1.3"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/cram.h"
#include "sys/uio.h"
#include "sys/cred.h"
#include "sys/ddi.h"

static unsigned char    cmosbuf[2]; /* byte 0 is addr, byte 1 is data */
int cmosdevflag = 0;

cmosopen(devp, flag, otyp, cred_p)
dev_t	*devp;
int	flag;
int	otyp;
struct cred *cred_p;
{
	int error;
	if ((flag & FWRITE) && (error = drv_priv(cred_p))) {
		return(error);
	}
	return(0);
}

cmosread(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	return(0);
}

cmoswrite(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	return(0);
}

cmosioctl(dev, cmd, addr, mode, cred_p, rval_p)
dev_t dev;
int cmd;
register caddr_t addr;
int mode;
struct cred *cred_p;
int *rval_p;
{
	unsigned char   reg;
	int             oldpri;
	int             i;
	int ecode=0;

	oldpri = spl5();
	switch (cmd) {
	case CMOSREAD:
		if (copyin(addr, (caddr_t)&cmosbuf[0], 1)) {
			ecode = EFAULT;
			break;
		}
		if (cmosbuf[0] < DSB || cmosbuf[0] > 0x3f) {
			ecode = ENXIO;
			break;
		}
		cmosbuf[1] = CMOSread(cmosbuf[0]);
		copyout((caddr_t)&cmosbuf[1], addr + 1, 1);
		break;
	case CMOSWRITE:
		if (ecode = drv_priv(cred_p)) {
			break;
		}
		if (copyin(addr, (caddr_t)cmosbuf, 2)) {
			ecode = EFAULT;
			break;
		}
		if (cmosbuf[0] < DSB || cmosbuf[0] > 0x3f) {
			ecode = ENXIO;
			break;
		}
		outb(CMOS_ADDR, cmosbuf[0]);
		outb(CMOS_DATA, cmosbuf[1]);
		break;
	default:
		ecode = EINVAL;
		break;
	}
	splx(oldpri);
	return(ecode);
}

cmosclose(dev, flags, otyp, cred_p)
dev_t	dev;
int	flags;
int	otyp;
struct cred *cred_p;
{
	return(0);
}

/*
 * routine to read contents of a location in the PC AT CMOS ram.
 * The value read is returned.
 */
unsigned char
CMOSread(addr)
	unsigned char   addr;
{
	outb(CMOS_ADDR, addr); /* address to read from in CMOS ram */
	return inb(CMOS_DATA); /* return the value from the ram */
}
