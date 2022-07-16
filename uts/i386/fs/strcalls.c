/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:strcalls.c	1.2"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strsubr.h"

/*
 * STREAMS system calls.
 */

struct msgnp {		/* non-priority version retained for compatibility */
	int fdes;
	struct strbuf *ctl;
	struct strbuf *data;
	int flags;
};

struct msgp {		/* priority version */
	int fdes;
	struct strbuf *ctl;
	struct strbuf *data;
	int pri;
	int flags;
};

#if defined(__STDC__)
int	msgio(struct msgp *, rval_t *, int, unsigned char *, int *);
#else
int	msgio();
#endif

int
getmsg(uap, rvp)
	struct msgnp *uap;
	rval_t *rvp;
{
	struct msgp ua;
	int error;
	int localflags;
	int realflags = 0;
	unsigned char pri = 0;

	/*
	 * Convert between old flags (localflags) and new flags (realflags).
	 */
	if (copyin((caddr_t)uap->flags, (caddr_t)&localflags, sizeof(int)))
		return (EFAULT);
	switch (localflags) {
	case 0:
		realflags = MSG_ANY;
		break;

	case RS_HIPRI:
		realflags = MSG_HIPRI;
		break;

	default:
		return (EINVAL);
	}

	ua.fdes = uap->fdes;
	ua.ctl = uap->ctl;
	ua.data = uap->data;
	if ((error = msgio(&ua, rvp, FREAD, &pri, &realflags)) == 0) {
		/*
		 * massage realflags based on localflags.
		 */
		if (realflags == MSG_HIPRI)
			localflags = RS_HIPRI;
		else
			localflags = 0;
		if (copyout((caddr_t)&localflags, (caddr_t)uap->flags,
		  sizeof(int)))
			error = EFAULT;
	}
	return (error);
}

int
putmsg(uap, rvp)
	struct msgnp *uap;
	rval_t *rvp;
{
	unsigned char pri = 0;

	switch (uap->flags) {
	case RS_HIPRI:
		uap->flags = MSG_HIPRI;
		break;

	case 0:
		uap->flags = MSG_BAND;
		break;

	default:
		return (EINVAL);
	}
	return (msgio((struct msgp *)uap, rvp, FWRITE, &pri, &uap->flags));
}

int
getpmsg(uap, rvp)
	struct msgp *uap;
	rval_t *rvp;
{
	int error;
	int flags;
	int intpri;
	unsigned char pri;

	if (copyin((caddr_t)uap->flags, (caddr_t)&flags, sizeof(int)))
		return (EFAULT);
	if (copyin((caddr_t)uap->pri, (caddr_t)&intpri, sizeof(int)))
		return (EFAULT);
	if ((intpri > 255) || (intpri < 0))
		return (EINVAL);
	pri = (unsigned char)intpri;
	if ((error = msgio(uap, rvp, FREAD, &pri, &flags)) == 0) {
		if (copyout((caddr_t)&flags, (caddr_t)uap->flags, sizeof(int)))
			return (EFAULT);
		intpri = (int)pri;
		if (copyout((caddr_t)&intpri, (caddr_t)uap->pri, sizeof(int)))
			error = EFAULT;
	}
	return (error);
}

int
putpmsg(uap, rvp)
	struct msgp *uap;
	rval_t *rvp;
{
	unsigned char pri;

	if ((uap->pri > 255) || (uap->pri < 0))
		return (EINVAL);
	pri = (unsigned char)uap->pri;
	return (msgio(uap, rvp, FWRITE, &pri, &uap->flags));
}

/*
 * Common code for getmsg and putmsg calls: check permissions,
 * copy in args, do preliminary setup, and switch to
 * appropriate stream routine.
 */
msgio(uap, rvp, mode, prip, flagsp)
	register struct msgp *uap;
	rval_t *rvp;
	register int mode;
	unsigned char *prip;
	int *flagsp;
{
	file_t *fp;
	register vnode_t *vp;
	struct strbuf msgctl, msgdata;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return (error);
	if ((fp->f_flag & mode) == 0)
		return (EBADF);
	vp = fp->f_vnode;
	if ((vp->v_type != VFIFO && vp->v_type != VCHR) || vp->v_stream == NULL)
		return (ENOSTR);
	if (uap->ctl && copyin((caddr_t)uap->ctl, (caddr_t)&msgctl,
	    sizeof(struct strbuf)))
		return (EFAULT);
	if (uap->data && copyin((caddr_t)uap->data, (caddr_t)&msgdata,
	    sizeof(struct strbuf)))
		return (EFAULT);
	if (mode == FREAD) {
		if (!uap->ctl)
			msgctl.maxlen = -1;
		if (!uap->data)
			msgdata.maxlen = -1;
		if (error = strgetmsg(vp, &msgctl, &msgdata, prip,
		    flagsp, fp->f_flag, rvp))
			return (error);
		if ((uap->ctl && copyout((caddr_t)&msgctl, (caddr_t)uap->ctl,
		    sizeof(struct strbuf))) || (uap->data &&
		    copyout((caddr_t)&msgdata, (caddr_t)uap->data,
		    sizeof(struct strbuf))))
			return (EFAULT);
		return (0);
	}
	/*
	 * FWRITE case 
	 */
	if (!uap->ctl)
		msgctl.len = -1;
	if (!uap->data)
		msgdata.len = -1;
	return (strputmsg(vp, &msgctl, &msgdata, *prip, *flagsp, fp->f_flag));
}
