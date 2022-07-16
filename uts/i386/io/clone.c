/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:clone.c	1.3"
/*
 * Clone Driver.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/user.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/errno.h"
#include "sys/sysinfo.h"
#include "sys/systm.h"
#include "sys/ddi.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/mkdev.h"

struct vnode *makespecvp();	/* Device file system */

int clndevflag = 0;

int clnopen();
static struct module_info clnm_info = { 0, "CLONE", 0, 0, 0, 0 };
static struct qinit clnrinit = { NULL, NULL, clnopen, NULL, NULL, &clnm_info, NULL };
static struct qinit clnwinit = { NULL, NULL, NULL, NULL, NULL, &clnm_info, NULL };
struct streamtab clninfo = { &clnrinit, &clnwinit };

/*
 * Clone open.  Maj is the major device number of the streams
 * device to open.  Look up the device in the cdevsw[].  Attach
 * its qinit structures to the read and write queues and call its
 * open with the sflag set to CLONEOPEN.  Swap in a new vnode with
 * the real device number constructed from either
 *	a) for old-style drivers:
 *		maj and the minor returned by the device open, or
 *	b) for new-style drivers:
 *		the whole dev passed back as a reference parameter
 *		from the device open.
 */
int
clnopen(qp, devp, flag, sflag, crp)
	register queue_t *qp;
	dev_t *devp;
	int flag;
	int sflag;
	cred_t *crp;
{
	register struct streamtab *stp;
	dev_t newdev;
	int error;
	major_t maj;
	minor_t emaj;

	if (sflag)
		return (ENXIO);

	/*
	 * Get the device to open.
	 */
	emaj = getminor(*devp); /* minor is major for a cloned drivers */
	maj = etoimajor(emaj);	/* get internal major of cloned driver */

	if ((maj >= cdevcnt) || !(stp = cdevsw[maj].d_str))
		return (ENXIO);

	/*
	 * Substitute the real qinit values for the current ones.
	 */
	setq(qp, stp->st_rdinit, stp->st_wrinit);

	/*
	 * Call the device open with the stream flag CLONEOPEN.  The device
	 * will either fail this or return a minor device number (for old-
	 * style drivers) or the whole device number (for new-style drivers).
	 */
	newdev = makedevice(emaj, 0);	/* create new style device number  */

	if (*cdevsw[maj].d_flag & D_OLD) {
		int oldev;

		qp->q_flag |= QOLD;
		WR(qp)->q_flag |= QOLD;

		/*
		 * newdev is minor for pre-SVR4 drivers.
		 * old style drivers get the old device format 
		 * so make sure it fits.
		 */
		if ((oldev = (o_dev_t)cmpdev(newdev)) == NODEV)
			return ENXIO;
		if ((newdev = (*qp->q_qinfo->qi_qopen)(qp, oldev, flag, CLONEOPEN)) == OPENFAIL)
			return (u.u_error == 0 ? ENXIO : u.u_error);	/* XXX */

			/* return new style dev to caller */

		*devp = makedevice(emaj, (newdev & OMAXMIN));
	} else {
		qp->q_flag &= ~QOLD;
		WR(qp)->q_flag &= ~QOLD;
		if (error = (*qp->q_qinfo->qi_qopen)(qp, &newdev, flag, CLONEOPEN, crp))
			return (error);
		if ((getmajor(newdev) > cdevcnt) || !(stp = cdevsw[getmajor(newdev)].d_str)) {
			(*qp->q_qinfo->qi_qclose)(qp, flag, crp);
			return (ENXIO);
		}
		setq(qp, stp->st_rdinit, stp->st_wrinit);
		*devp = newdev;
	}
	return (0);
}	
