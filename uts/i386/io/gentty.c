/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:gentty.c	1.3.1.1"
/*
 * Indirect driver for controlling tty.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/tty.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/session.h"
#include "sys/ddi.h"
#include "sys/vnode.h"
#include "sys/debug.h"

int sydevflag = 0;

extern struct vnode *common_specvp();

/* ARGSUSED */
int
syopen(devp, flag, otyp, cr)
	dev_t *devp;
	int flag;
	int otyp;
	struct cred *cr;
{
	dev_t ttyd;
	register vnode_t *ttyvp;
	register vnode_t *ttycvp;

	if ((ttyd = u.u_procp->p_sessp->s_dev) == NODEV)
		return ENXIO;
	if ((ttyvp = u.u_procp->p_sessp->s_vp) == NULL)
		return EIO;
	ttycvp = common_specvp(ttyvp);
	if (cdevsw[getmajor(ttyd)].d_str) {
		int error;

		VN_HOLD(ttyvp);
		error = stropen(ttycvp, &ttyd, flag, cr);
		VN_RELE(ttyvp);
		return (error);
	}
	return (*cdevsw[getmajor(ttyd)].d_open) (&ttyd, flag, otyp, cr);
}

/* ARGSUSED */
int
syclose(dev, flag, otyp, cr)
	dev_t dev;
	int flag;
	int otyp;
	struct cred *cr;
{
	return 0;
}

/* ARGSUSED */
int
syread(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	register dev_t ttyd;
	register vnode_t *ttyvp;
	register vnode_t *ttycvp;

	if ((ttyd = u.u_procp->p_sessp->s_dev) == NODEV)
		return ENXIO;
	if ((ttyvp = u.u_procp->p_sessp->s_vp) == NULL)
		return EIO;
	ttycvp = common_specvp(ttyvp);
	if (cdevsw[getmajor(ttyd)].d_str)
		return strread(ttycvp, uiop, cr);
	return (*cdevsw[getmajor(ttyd)].d_read) (ttyd, uiop, cr);
}

/* ARGSUSED */
int
sywrite(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	register dev_t ttyd;
	register vnode_t *ttyvp;
	register vnode_t *ttycvp;

	if ((ttyd = u.u_procp->p_sessp->s_dev) == NODEV)
		return ENXIO;
	if ((ttyvp = u.u_procp->p_sessp->s_vp) == NULL)
		return EIO;
	ttycvp = common_specvp(ttyvp);
	if (cdevsw[getmajor(ttyd)].d_str)
		return strwrite(ttycvp, uiop, cr);
	return (*cdevsw[getmajor(ttyd)].d_write) (ttyd, uiop, cr);
}


/* ARGSUSED */
int
syioctl(dev, cmd, arg, mode, cr, rvalp)
	dev_t dev;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	register dev_t ttyd;
	register vnode_t *ttyvp;
	register vnode_t *ttycvp;

	if ((ttyd = u.u_procp->p_sessp->s_dev) == NODEV)
		return ENXIO;
	if ((ttyvp = u.u_procp->p_sessp->s_vp) == NULL)
		return EIO;
	ttycvp = common_specvp(ttyvp);
	if (cdevsw[getmajor(ttyd)].d_str)
		return strioctl(ttycvp, cmd, arg, mode, U_TO_K, cr, rvalp);
	return (*cdevsw[getmajor(ttyd)].d_ioctl)
		  (ttyd, cmd, arg, mode, cr, rvalp);
}
