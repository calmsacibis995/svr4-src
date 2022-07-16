/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/rtc.c	1.3"

/* 
 * Driver for PC AT Calendar clock chip
 *	
 * PC-DOS compatibility requirements:
 *      1. must use local time, not GMT (sigh!)
 *      2. must use bcd mode, not binary
 *
 * To really use this device effectively there should be a program
 * called e.g. rtclock that works like date does.  i.e. "rtclock"
 * would return local time and "rtclock arg" would set local time arg
 * into the chip.  To set the system clock on boot /etc/rc should do
 * something like "date `rtclock`".
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/cmn_err.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/immu.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/rtc.h"

#include "sys/uio.h"
#include "sys/cred.h"
#include "sys/ddi.h"

static unsigned char	rtcbuf[RTC_NREG];
int rtcdevflag = 0;

/*
 * Initialize real time clock to interrupt us when the chip updates.
 * (approx. once a second)
 */
rtcinit()
{

	if (!rtcget(rtcbuf, 1)) {
/*
 * reinitialize the real time clock.
 */
		outb(RTC_ADDR, RTC_A);
		outb(RTC_DATA, RTC_DIV2 | RTC_RATE6);
		outb(RTC_ADDR, RTC_B);
		outb(RTC_DATA, RTC_HM);
		outb(RTC_ADDR, RTC_C);
		(void)inb(RTC_DATA);
		outb(RTC_ADDR, RTC_D);
		(void)inb(RTC_DATA);
	}
	return(0);
}

rtcopen(devp, flags, otyp, cred_p)
dev_t	*devp;
int	flags;
int	otyp;
struct cred *cred_p;
{
	int error;

	if ((flags & FWRITE) && (error = drv_priv(cred_p)))
		return(error);

	return(0);
}

rtcread(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	return(0);
}

rtcwrite(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	return(0);
}

/*

This is used to read and write the clock.

*/

rtcioctl(dev, cmd, addr, mode, cred_p, rval_p)
dev_t dev;
int cmd;
caddr_t addr;
int mode;
struct cred *cred_p;
int *rval_p;
{
	unsigned char	reg;
	int		oldpri;
	int		i;
	int ecode=0;

	oldpri = spl5();
	switch (cmd) {
	case RTCRTIME:
		if (rtcget(rtcbuf, 0)) {
			ecode = EIO;
			break;
		}
		copyout(rtcbuf, addr, RTC_NREG);
		break;
	case RTCSTIME:
		if (ecode = drv_priv(cred_p)) {
			break;
		}
		if (copyin(addr, rtcbuf, RTC_NREGP))
			break;
		rtcput(rtcbuf);
		break;
	default:
		ecode = EINVAL;
		break;
	}
	splx(oldpri);
	return(ecode);
}

rtcclose(dev, flags, otyp, cred_p)
dev_t	dev;
int	flags;
int	otyp;
struct cred *cred_p;
{
	return(0);
}

/*
 * routine to read contents of real time clock to the specified buffer.
 * returns -1 if clock not valid, else 0.
 * The routine will busy wait for the update in progress flag to clear
 * if the busywt flag is true, else it will sleep and wait for an update
 * ended interrupt.  It returns RTC_NREG (which is 15) bytes of data, as
 * given in the technical reference data.  This data includes both the time
 * and the status registers.
 */
rtcget(buf, busywt)
unsigned char	*buf;
int		busywt;
{
	register unsigned char	reg;
	register int		i;

	outb(RTC_ADDR, RTC_D); /* check if clock valid */
	reg = inb(RTC_DATA);
	if (reg & RTC_VRT == 0)
		return -1;
checkuip:
	outb(RTC_ADDR, RTC_A); /* check if update in progress */
	reg = inb(RTC_DATA);
	if (reg & RTC_UIP) {
		if (!busywt)
			tenmicrosec();
		else	
			sleep(rtcbuf, PWAIT);
		goto checkuip;
	}
	for (i = 0; i < RTC_NREG; i++) {
		outb(RTC_ADDR, i);
		*buf++ = inb(RTC_DATA);
	}
	return 0;
}

/*
 * This routine writes the contents of the given buffer to the real time
 * clock.  It is given RTC_NREGP bytes of data, which are the 10 bytes used
 * to write the time and set the alarm.  It should be called with the priority
 * raised to 5.
 */

rtcput (buf)
char	* buf;
{
	unsigned char	reg;
	int		i;

	outb(RTC_ADDR, RTC_B);
	reg = inb(RTC_DATA);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, reg | RTC_SET); /* allow time set now */
	for (i = 0; i < RTC_NREGP; i++) { /* set the time */
		outb(RTC_ADDR, i);
		outb(RTC_DATA, buf[i]);
	}
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, reg & ~RTC_SET); /* allow time update */
	wakeup(rtcbuf);
}

/*
 * Routine to handle interrupt from real time clock
 */
rtcintr(ivect)
	int	ivect;
{
	outb(RTC_ADDR, RTC_C); /* clear interrupt */
	(void)inb(RTC_DATA);
	return(0);
}
