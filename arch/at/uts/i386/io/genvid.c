/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/genvid.c	1.3"

/*
 *	indirect driver for /dev/video.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/kmem.h"
#include "sys/tty.h"
#include "sys/stream.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/vnode.h"
#include "sys/genvid.h"
#include "sys/cmn_err.h"
#include "sys/ddi.h"

int gviddevflag = 0;
gvid_t Gvid = {0};
int gvidflg = 0;



int
gviddev(ttydev,devp)
dev_t ttydev,*devp;
{
	int i;
	major_t majnum;
	minor_t minnum;

	majnum = getmajor(ttydev);
	if (majnum != Gvid.gvid_maj)
		return (EINVAL);
	minnum = getminor(ttydev);

	if (minnum >= Gvid.gvid_num)
		return (EINVAL);
	*devp = *(Gvid.gvid_buf + minnum);
	if (*devp == NODEV) return (ENODEV);
	return (0);
}


int
gvidopen(devp, flag, otyp, cr)
	dev_t *devp;
	int flag;
	int otyp;
	struct cred *cr;
{
	dev_t gdev,cttydev;
	minor_t gen_minor;
	int error,oldpri;

	gen_minor = getminor(*devp);

	if (gen_minor == 1)
		return (0); /* success if administrative open */

	if (error = ws_getctty(&cttydev)) 
		return error;

	oldpri = splhi(); /* Enforce mutual exclusion */

	if (! (gvidflg & GVID_SET)) {
		splx(oldpri);
		return (EBUSY); /* fail opens until table is loaded */
	}

	while (gvidflg & GVID_ACCESS) 
		sleep(&gvidflg, STIPRI);

	gvidflg |= GVID_ACCESS;
	splx(oldpri);


	if (error = gviddev(cttydev,&gdev)) {
		gvidflg &= ~GVID_ACCESS;
		wakeup(&gvidflg);
		return error;
	}
	/* release access to the flag -- we could sleep in open */
	gvidflg &= ~GVID_ACCESS;
	wakeup(&gvidflg);
	error = (*cdevsw[getmajor(gdev)].d_open)(&gdev, flag, otyp, cr);

	if (!error) *devp = gdev; /* clone!! */
	return error;
}


int
gvidclose(dev, flag, otyp, cr)
	dev_t dev;
	int flag;
	int otyp;
	struct cred *cr;
{
	return 0;
}


int
gvidread(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	return (ENXIO);
}


int
gvidwrite(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	return (ENXIO);
}


/*
 * The only way we can get in here is when /dev/vidadm is opened.
 * Ioctls on /dev/video go the the underlying video driver
 */

int
gvidioctl(dev, cmd, arg, mode, crp, rvalp)
	dev_t dev;
	int cmd;
	int arg;
	int mode;
	struct cred *crp;
	int *rvalp;
{
	minor_t gdev;
	int oldpri;

	gdev = getminor(dev);

	switch (cmd) {

	case GVID_SETTABLE: {
		gvid_t tmpmap;
		dev_t *devbufp,*tmpbuf;
		int tmpnum;


		oldpri = splhi(); /* Protect against stream head access */
		while (gvidflg & GVID_ACCESS) 
			sleep(&gvidflg, STIPRI);

		gvidflg |= GVID_ACCESS;
		splx(oldpri);

		if (copyin(arg, (caddr_t) &tmpmap, sizeof(gvid_t)) == -1) {
#ifdef DEBUG
			cmn_err(CE_WARN,"gvid: could not copy to tmpmap");
#endif
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
			return (EFAULT);
		}

		devbufp = (dev_t *)kmem_alloc(tmpmap.gvid_num*sizeof(dev_t),KM_SLEEP);
		if (devbufp == (dev_t *) NULL) {
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
			return (ENOMEM);
		}

		if (copyin(tmpmap.gvid_buf, devbufp, tmpmap.gvid_num*sizeof(dev_t)) == -1) {
			kmem_free(devbufp,sizeof(dev_t)*tmpmap.gvid_num);
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
#ifdef DEBUG
			cmn_err(CE_WARN,"gvid: could not copy to tmpbuf");
#endif
			return (EFAULT);
		}

		tmpbuf = Gvid.gvid_buf;
		tmpnum = Gvid.gvid_num;

		Gvid.gvid_buf = devbufp;
		Gvid.gvid_num = tmpmap.gvid_num;
		Gvid.gvid_maj = tmpmap.gvid_maj;
		gvidflg &= ~GVID_ACCESS;
		gvidflg |= GVID_SET;
		wakeup(&gvidflg);

		if (tmpbuf)
			kmem_free(tmpbuf,sizeof(dev_t)*tmpnum);
		return(0);
	}


	case GVID_GETTABLE: {
		gvid_t tmpmap;
		int oldpri;

		oldpri = splhi(); /* Protect against stream head access */
		if (! (gvidflg & GVID_SET)) {
			splx(oldpri);
			return EBUSY;
		}

		while (gvidflg & GVID_ACCESS) 
			sleep(&gvidflg, STIPRI);

		gvidflg |= GVID_ACCESS;
		splx(oldpri);

		if (copyin(arg, (caddr_t) &tmpmap, sizeof(gvid_t)) == -1) {
#ifdef DEBUG
			cmn_err(CE_WARN,"gvid: could not copy to tmpmap");
#endif
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
			return (EFAULT);
		}

		tmpmap.gvid_num = min(tmpmap.gvid_num,Gvid.gvid_num);
		tmpmap.gvid_maj = Gvid.gvid_maj;

		if (copyout(Gvid.gvid_buf,tmpmap.gvid_buf, tmpmap.gvid_num*sizeof(dev_t)) == -1) {
#ifdef DEBUG
			cmn_err(CE_WARN,"gvid: could not copy to tmpbuf");
#endif
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
			return(EFAULT);
		}

		if (copyout(&tmpmap, arg, sizeof(gvid_t)) == -1) {
#ifdef DEBUG
			cmn_err(CE_WARN,"gvid: could not copy to tmpbuf");
#endif
			gvidflg &= ~GVID_ACCESS;
			wakeup(&gvidflg);
			return(EFAULT);
		}

		gvidflg &= ~GVID_ACCESS;
		wakeup(&gvidflg);
		return (0);
	 }

	default:
		return EINVAL;

	}
}
