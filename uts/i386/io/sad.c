#ident	"@(#)sad.c	1.2	92/10/02	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:sad.c	1.3"

/*
 * STREAMS Administrative Driver
 *
 * Currently only handles autopush and module name verification.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/file.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/conf.h"
#include "sys/sad.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/ddi.h"

STATIC struct module_info sad_minfo = {
	0x7361, "sad", 0, INFPSZ, 0, 0
};

STATIC int sadopen(), sadclose(), sadwput();

STATIC struct qinit sad_rinit = {
	NULL, NULL, sadopen, sadclose, NULL, &sad_minfo, NULL
};

STATIC struct qinit sad_winit = {
	sadwput, NULL, NULL, NULL, NULL, &sad_minfo, NULL
};

struct streamtab sadinfo = {
	&sad_rinit, &sad_winit, NULL, NULL
};

queue_t *sadminqp;
struct autopush *strpfreep;
int saddevflag = 0;

STATIC struct autopush *ap_alloc(), *ap_hfind();
STATIC void ap_free(), ap_hadd(), ap_hrmv();
STATIC void apush_ioctl(), apush_iocdata(), nak_ioctl(), ack_ioctl();
STATIC void vml_ioctl(), vml_iocdata();
STATIC int valid_list();
void sadinit();

/*
 * sadinit() -
 * Initialize autopush freelist.
 */
void
sadinit()
{
	register struct autopush *ap;
	register int i;

	/*
	 * build the autpush freelist.
	 */
	strpfreep = autopush;
	ap = autopush;
	for (i = 1; i < nautopush; i++) {
		ap->ap_nextp = &autopush[i];
		ap->ap_flags = APFREE;
		ap = ap->ap_nextp;
	}
	ap->ap_nextp = NULL;
	ap->ap_flags = APFREE;
}

/*
 * sadopen() -
 * Allocate a sad device.  Only one
 * open at a time allowed per device.
 */
/* ARGSUSED */
STATIC int
sadopen(qp, devp, flag, sflag, credp)
	queue_t *qp;	/* pointer to read queue */
	dev_t *devp;	/* major/minor device of stream */
	int flag;	/* file open flags */
	int sflag;	/* stream open flags */
	cred_t *credp;	/* user credentials */
{
	register int i;

	if (sflag)		/* no longer called from clone driver */
		return (EINVAL);

	/*
	 * Both USRMIN and ADMMIN are clone interfaces.
	 */
	for (i = 0; i < sadcnt; i++)
		if (saddev[i].sa_qp == NULL)
			break;
	if (i >= sadcnt)		/* no such device */
		return (ENXIO);

	switch (getminor(*devp)) {
	case USRMIN:			/* mere mortal */
		saddev[i].sa_flags = 0;
		break;

	case ADMMIN:			/* priviledged user */
		saddev[i].sa_flags = SADPRIV;
		break;

	default:
		return (EINVAL);
	}

	saddev[i].sa_qp = qp;
	qp->q_ptr = (caddr_t)&saddev[i];
	WR(qp)->q_ptr = (caddr_t)&saddev[i];
	*devp = makedevice(getemajor(*devp), i);
	return (0);
}

/*
 * sadclose() -
 * Clean up the data structures.
 */
/* ARGSUSED */
STATIC int
sadclose(qp, flag, credp)
	queue_t *qp;	/* pointer to read queue */
	int flag;	/* file open flags */
	cred_t *credp;	/* user credentials */
{
	struct saddev *sadp;

	sadp = (struct saddev *)qp->q_ptr;
	sadp->sa_qp = NULL;
	sadp->sa_addr = NULL;
	qp->q_ptr = NULL;
	WR(qp)->q_ptr = NULL;
	return (0);
}

/*
 * sadwput() -
 * Write side put procedure.
 */
STATIC int
sadwput(qp, mp)
	queue_t *qp;	/* pointer to write queue */
	mblk_t *mp;	/* message pointer */
{
	struct iocblk *iocp;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(qp, mp);
		} else
			freemsg(mp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case SAD_SAP:
		case SAD_GAP:
			apush_ioctl(qp, mp);
			break;

		case SAD_VML:
			vml_ioctl(qp, mp);
			break;

		default:
			nak_ioctl(qp, mp, EINVAL);
			break;
		}
		break;

	case M_IOCDATA:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case SAD_SAP:
		case SAD_GAP:
			apush_iocdata(qp, mp);
			break;

		case SAD_VML:
			vml_iocdata(qp, mp);
			break;

		default:
			ASSERT(0);
			freemsg(mp);
			break;
		}
		break;

	default:
		freemsg(mp);
		break;
	} /* switch (db_type) */
	return (0);
}

/*
 * ack_ioctl() -
 * Send an M_IOCACK message in the opposite
 * direction from qp.
 */
STATIC void
ack_ioctl(qp, mp, count, rval, errno)
	queue_t *qp;	/* queue pointer */
	mblk_t *mp;	/* message block to use */
	int count;	/* number of bytes to copyout */
	int rval;	/* return value for icotl */
	int errno;	/* error number to return */
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_count = count;
	iocp->ioc_rval = rval;
	iocp->ioc_error = errno;
	mp->b_datap->db_type = M_IOCACK;
	qreply(qp, mp);
}

/*
 * nak_ioctl() -
 * Send an M_IOCNAK message in the opposite
 * direction from qp.
 */
STATIC void
nak_ioctl(qp, mp, errno)
	queue_t *qp;	/* queue pointer */
	mblk_t *mp;	/* message block to use */
	int errno;	/* error number to return */
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_error = errno;
	if (mp->b_cont) {
		freemsg(mp->b_cont);
		mp->b_cont = NULL;
	}
	mp->b_datap->db_type = M_IOCNAK;
	qreply(qp, mp);
}

/*
 * apush_ioctl() -
 * Handle the M_IOCTL messages associated with
 * the autopush feature.
 */
STATIC void
apush_ioctl(qp, mp)
	queue_t *qp;	/* pointer to write queue */
	mblk_t *mp;	/* message pointer */
{
	struct iocblk *iocp;
	struct copyreq *cqp;
	struct saddev *sadp;

	iocp = (struct iocblk *)mp->b_rptr;
	if (iocp->ioc_count != TRANSPARENT) {
		nak_ioctl(qp, mp, EINVAL);
		return;
	}
	sadp = (struct saddev *)qp->q_ptr;
	switch (iocp->ioc_cmd) {
	case SAD_SAP:
		if (!(sadp->sa_flags & SADPRIV)) {
			nak_ioctl(qp, mp, EPERM);
			break;
		}
		/* FALLTHRU */

	case SAD_GAP:
		cqp = (struct copyreq *)mp->b_rptr;
		cqp->cq_addr = (caddr_t)*(long *)mp->b_cont->b_rptr;
		sadp->sa_addr = cqp->cq_addr;
		freemsg(mp->b_cont);
		mp->b_cont = NULL;
		cqp->cq_size = sizeof(struct strapush);
		cqp->cq_flag = 0;
		cqp->cq_private = (mblk_t *)GETSTRUCT;
		mp->b_datap->db_type = M_COPYIN;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		qreply(qp, mp);
		break;

	default:
		ASSERT(0);
		nak_ioctl(qp, mp, EINVAL);
		break;
	} /* switch (ioc_cmd) */
}

/*
 * apush_iocdata() -
 * Handle the M_IOCDATA messages associated with
 * the autopush feature.
 */
STATIC void
apush_iocdata(qp, mp)
	queue_t *qp;	/* pointer to write queue */
	mblk_t *mp;	/* message pointer */
{
	int i;
	long idx;
	struct copyreq *cqp;
	struct copyresp *csp;
	struct strapush *sap;
	struct autopush *ap;
	struct saddev *sadp;

	csp = (struct copyresp *)mp->b_rptr;
	cqp = (struct copyreq *)mp->b_rptr;
	if (csp->cp_rval) {	/* if there was an error */
		freemsg(mp);
		return;
	}
	if (mp->b_cont)
		/* sap needed only if mp->b_cont is set */
		sap = (struct strapush *)mp->b_cont->b_rptr;
	switch (csp->cp_cmd) {
	case SAD_SAP:
		switch ((int)csp->cp_private) {
		case GETSTRUCT:
			switch (sap->sap_cmd) {
			case SAP_ONE:
			case SAP_RANGE:
			case SAP_ALL:
				if ((sap->sap_npush == 0) ||
				    (sap->sap_npush > MAXAPUSH) ||
				    (sap->sap_npush > nstrpush)) {

					/* invalid number of modules to push */

					nak_ioctl(qp, mp, EINVAL);
					break;
				}
				if ((idx = (long)etoimajor(sap->sap_major)) == -1) {

					/* invalid major device number */

					nak_ioctl(qp, mp, EINVAL);
					break;
				}
				if ((sap->sap_cmd == SAP_RANGE) &&
				    (sap->sap_lastminor < sap->sap_minor)) {

					/* bad range */

					nak_ioctl(qp, mp, ERANGE);
					break;
				}
				if (cdevsw[idx].d_str == NULL) {

					/* not a STREAMS driver */

					nak_ioctl(qp, mp, ENOSTR);
					break;
				}
				if (ap_hfind(sap->sap_major, sap->sap_minor, sap->sap_lastminor, sap->sap_cmd)) {

					/* already configured */

					nak_ioctl(qp, mp, EEXIST);
					break;
				}
				if (!valid_list(sap)) {

					/* bad module name */

					nak_ioctl(qp, mp, EINVAL);
					break;
				}
				if ((ap = ap_alloc()) == NULL) {

					/* no autopush structures - EAGAIN? */

					nak_ioctl(qp, mp, ENOSR);
					break;
				}
				ap->ap_common = sap->sap_common;
				for (i = 0; i < ap->ap_npush; i++)
					ap->ap_list[i] = findmod(sap->sap_list[i]);
				ap_hadd(ap);
				ack_ioctl(qp, mp, 0, 0, 0);
				break;

			case SAP_CLEAR:
				if ((idx = (long)etoimajor(sap->sap_major)) == -1) {

					/* invalid major device number */

					nak_ioctl(qp, mp, EINVAL);
					break;
				}
				if (cdevsw[idx].d_str == NULL) {

					/* not a STREAMS driver */

					nak_ioctl(qp, mp, ENOSTR);
					break;
				}
				if ((ap = ap_hfind(sap->sap_major, sap->sap_minor, sap->sap_lastminor, sap->sap_cmd)) == NULL) {

					/* not configured */

					nak_ioctl(qp, mp, ENODEV);
					break;
				}
				if ((ap->ap_type == SAP_RANGE) && (sap->sap_minor != ap->ap_minor)) {

					/* starting minors do not match */

					nak_ioctl(qp, mp, ERANGE);
					break;
				}
				if ((ap->ap_type == SAP_ALL) && (sap->sap_minor != 0)) {

					/* SAP_ALL must have minor == 0 */

					nak_ioctl(qp, mp, EINVAL);
					break;
				}
				ap_hrmv(ap);
				ap_free(ap);
				ack_ioctl(qp, mp, 0, 0, 0);
				break;

			default:
				nak_ioctl(qp, mp, EINVAL);
				break;
			} /* switch (sap_cmd) */
			break;

		default:
			ASSERT(0);
			freemsg(mp);
			break;
		} /* switch (cp_private) */
		break;

	case SAD_GAP:
		switch ((int)csp->cp_private) {
		case GETSTRUCT:
			if ((idx = (long)etoimajor(sap->sap_major)) == -1) {

				/* invalid major device number */

				nak_ioctl(qp, mp, EINVAL);
				break;
			}
			if (cdevsw[idx].d_str == NULL) {

				/* not a STREAMS driver */

				nak_ioctl(qp, mp, ENOSTR);
				break;
			}
			if ((ap = ap_hfind(sap->sap_major, sap->sap_minor, sap->sap_lastminor, SAP_ONE)) == NULL) {

				/* not configured */

				nak_ioctl(qp, mp, ENODEV);
				break;
			}
			sap->sap_common = ap->ap_common;
			for (i = 0; i < ap->ap_npush; i++)
				strcpy(sap->sap_list[i], fmodsw[ap->ap_list[i]].f_name);
			for ( ; i < MAXAPUSH; i++)
				bzero(sap->sap_list[i], FMNAMESZ + 1);
			mp->b_datap->db_type = M_COPYOUT;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
			cqp->cq_private = (mblk_t *)GETRESULT;
			sadp = (struct saddev *)qp->q_ptr;
			cqp->cq_addr = sadp->sa_addr;
			cqp->cq_size = sizeof(struct strapush);
			cqp->cq_flag = 0;
			qreply(qp, mp);
			break;

		case GETRESULT:
			ack_ioctl(qp, mp, 0, 0, 0);
			break;

		default:
			ASSERT(0);
			freemsg(mp);
			break;
		} /* switch (cp_private) */
		break;

	default:	/* can't happen */
		ASSERT(0);
		freemsg(mp);
		break;
	} /* switch (cp_cmd) */
}

/*
 * ap_alloc() -
 * Allocate an autopush structure.
 */
STATIC struct autopush *
ap_alloc()
{
	register struct autopush *ap;

	if (strpfreep == NULL)
		return(NULL);
	ap = strpfreep;
	ASSERT(ap->ap_flags == APFREE);
	strpfreep = strpfreep->ap_nextp;
	ap->ap_nextp = NULL;
	ap->ap_flags = APUSED;
	return (ap);
}

/*
 * ap_free() -
 * Give an autopush structure back to the freelist.
 */
STATIC void
ap_free(ap)
	struct autopush *ap;	/* the autopush structure */
{
	ASSERT(ap->ap_flags & APUSED);
	ASSERT(!(ap->ap_flags & APHASH));
	ap->ap_flags = APFREE;
	ap->ap_nextp = strpfreep;
	strpfreep = ap;
}

/*
 * ap_hadd() -
 * Add an autopush structure to the hash list.
 */
STATIC void
ap_hadd(ap)
	struct autopush *ap;	/* the autopush structure */
{
	ASSERT(ap->ap_flags & APUSED);
	ASSERT(!(ap->ap_flags & APHASH));
	ap->ap_nextp = strphash(ap->ap_major);
	strphash(ap->ap_major) = ap;
	ap->ap_flags |= APHASH;
}

/*
 * ap_hrmv() -
 * Remove an autopush structure from the hash list.
 */
STATIC void
ap_hrmv(ap)
	struct autopush *ap;	/* the autopush structure */
{
	struct autopush *hap;
	struct autopush *prevp = NULL;

	ASSERT(ap->ap_flags & APUSED);
	ASSERT(ap->ap_flags & APHASH);
	hap = strphash(ap->ap_major);
	while (hap) {
		if (ap == hap) {
			hap->ap_flags &= ~APHASH;
			if (prevp)
				prevp->ap_nextp = hap->ap_nextp;
			else
				strphash(ap->ap_major) = hap->ap_nextp;
			return;
		} /* if */
		prevp = hap;
		hap = hap->ap_nextp;
	} /* while */
}

/*
 * ap_hfind() -
 * Look for an autopush structure in the hash list
 * based on major, minor, lastminor, and command.
 */
STATIC struct autopush *
ap_hfind(maj, minor, last, cmd)
	long maj;	/* major device number */
	long minor;	/* minor device number */
	long last;	/* last minor device number (SAP_RANGE only) */
	uint cmd;	/* who is asking */
{
	register struct autopush *ap;

	ap = strphash(maj);
	while (ap) {
		if (ap->ap_major == maj) {
			if (cmd == SAP_ALL)
				break;
			switch (ap->ap_type) {
			case SAP_ALL:
				break;

			case SAP_ONE:
				if (ap->ap_minor == minor)
					break;
				if ((cmd == SAP_RANGE) &&
				    (ap->ap_minor >= minor) &&
				    (ap->ap_minor <= last))
					break;
				ap = ap->ap_nextp;
				continue;

			case SAP_RANGE:
				if ((cmd == SAP_RANGE) &&
				    (((minor >= ap->ap_minor) &&
				      (minor <= ap->ap_lastminor)) ||
				     ((ap->ap_minor >= minor) &&
				      (ap->ap_minor <= last))))
					break;
				if ((minor >= ap->ap_minor) &&
				    (minor <= ap->ap_lastminor))
					break;
				ap = ap->ap_nextp;
				continue;

			default:
				ASSERT(0);
				break;
			}
			break;
		}
		ap = ap->ap_nextp;
	}
	return (ap);
}

/*
 * valid_list() -
 * Step through the list of modules to autopush and
 * validate their names.  Return 1 if the list is
 * valid and 0 if it is not.
 */
STATIC int
valid_list(sap)
	struct strapush *sap;
{
	register int i;

	for (i = 0; i < sap->sap_npush; i++)
		if (findmod(sap->sap_list[i]) == -1)
			return (0);
	return (1);
}

/*
 * vml_ioctl() -
 * Handle the M_IOCTL message associated with a request
 * to validate a module list.
 */
STATIC void
vml_ioctl(qp, mp)
	queue_t *qp;	/* pointer to write queue */
	mblk_t *mp;	/* message pointer */
{
	struct iocblk *iocp;
	struct copyreq *cqp;

	iocp = (struct iocblk *)mp->b_rptr;
	if (iocp->ioc_count != TRANSPARENT) {
		nak_ioctl(qp, mp, EINVAL);
		return;
	}
	ASSERT (iocp->ioc_cmd == SAD_VML);
	cqp = (struct copyreq *)mp->b_rptr;
	cqp->cq_addr = (caddr_t)*(long *)mp->b_cont->b_rptr;
	freemsg(mp->b_cont);
	mp->b_cont = NULL;
	cqp->cq_size = sizeof(struct str_list);
	cqp->cq_flag = 0;
	cqp->cq_private = (mblk_t *)GETSTRUCT;
	mp->b_datap->db_type = M_COPYIN;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	qreply(qp, mp);
}

/*
 * vml_iocdata() -
 * Handle the M_IOCDATA messages associated with
 * a request to validate a module list.
 */
STATIC void
vml_iocdata(qp, mp)
	queue_t *qp;	/* pointer to write queue */
	mblk_t *mp;	/* message pointer */
{
	int i;
	struct copyreq *cqp;
	struct copyresp *csp;
	struct str_mlist *lp;
	struct str_list *slp;
	struct saddev *sadp;

	csp = (struct copyresp *)mp->b_rptr;
	cqp = (struct copyreq *)mp->b_rptr;
	if (csp->cp_rval) {	/* if there was an error */
		freemsg(mp);
		return;
	}
	ASSERT (csp->cp_cmd == SAD_VML);
	sadp = (struct saddev *)qp->q_ptr;
	switch ((int)csp->cp_private) {
	case GETSTRUCT:
		slp = (struct str_list *)mp->b_cont->b_rptr;
		if (slp->sl_nmods <= 0) {
			nak_ioctl(qp, mp, EINVAL);
			break;
		}
		sadp->sa_addr = (caddr_t)slp->sl_nmods;
		cqp->cq_addr = (caddr_t)slp->sl_modlist;
		freemsg(mp->b_cont);
		mp->b_cont = NULL;
		cqp->cq_size = (int)sadp->sa_addr * sizeof(struct str_mlist);
		cqp->cq_flag = 0;
		cqp->cq_private = (mblk_t *)GETLIST;
		mp->b_datap->db_type = M_COPYIN;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		qreply(qp, mp);
		break;

	case GETLIST:
		lp = (struct str_mlist *)mp->b_cont->b_rptr;
		for (i = 0; i < (int)sadp->sa_addr; i++, lp++)
			if (findmod(lp->l_name) == -1) {
				ack_ioctl(qp, mp, 0, 1, 0);
				return;
			}
		ack_ioctl(qp, mp, 0, 0, 0);
		break;

	default:
		ASSERT(0);
		freemsg(mp);
		break;
	} /* switch (cp_private) */
}

