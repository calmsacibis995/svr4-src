/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-io:emap.c	1.3"

/*
 *	@(#) emap.c 1.2 88/09/06 kern-io:emap.c
 *
 *	Copyright (C) The Santa Cruz Operation, 1985, 1986.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 *
 */

/*
 * Eight-bit or European character mapping
 * for line discipline 0.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/dir.h"
#include "sys/buf.h"
#include "sys/signal.h"
#include "sys/seg.h"
#include "sys/conf.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/emap.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/stream.h"
#include "sys/kmem.h"
#include "sys/ddi.h"

struct emap *emaddmap();


/*
 * Enable emapping with a new emap on a line.
 * Called from ttioctl().
 */
emsetmap(tp, arg)
register struct tty *tp;
caddr_t arg;
{
	register struct emap *mp;
	struct buf *bp;

	bp = ngeteblk(E_TABSZ);			/* get an outboard buf to hold emap */
	/*
	 * Copy the new user map table into
	 * the allocated buffer
	 */
	if (copyin(arg, (caddr_t)paddr(bp), E_TABSZ) ) {
		u.u_error = EFAULT;
		goto out;
	}
	if (emchkmap(bp)) {		/* validate emap */
		u.u_error = EINVAL;
		goto out;
	}

	/*
	 * If this line is the only user of an emap,
	 * free it now to ensure success in emaddmap().
	 */
	if ((tp->t_mstate != ES_NULL) && (tp->t_mp->e_count == 1))
		emunmap(tp);

	mp = emaddmap(bp);
	if (mp == (struct emap *)NULL) {	/* can't add emap */
		u.u_error = ENAVAIL;
		goto out;
	}
	if (mp->e_count == 1)			/* if this is a new emap, */
		bp = (struct buf *)NULL;	/*   don't free the buf   */
	if (tp->t_mstate != ES_NULL)		/* free old emap	  */
		emunmap(tp);			/*   if we still have one */
						/* to attach new mapping  */

	tp->t_mstate = ES_START;		/* emapping enabled */
	tp->t_mp = mp;				/*   using new map  */

  out:
	if (bp)
		brelse(bp);
}


/*
 * Return the current emap in effect on a line.
 * Called from ttioctl().
 */
emgetmap(tp, arg)
register struct tty *tp;
faddr_t arg;
{
	register struct buf *bp;

	if (tp->t_mstate == ES_NULL)
		u.u_error = ENAVAIL;
	else {
		bp = tp->t_mp->e_bp;	/* get map buffer for this tty */

		/* copy the channel mapping table into user buffer */
		if (copyout((caddr_t)paddr(bp), arg, E_TABSZ))
			u.u_error = EFAULT;
	}
}


/*
 * Disable emapping on a line.
 * Decrement the use count of the emap,
 * and free it if the count becomes zero.
 * Called from ttioctl() and emsetmap().
 */
emunmap(tp)
register struct tty *tp;
{
	register struct emap *mp;

	if (tp->t_mstate == ES_NULL)
		return;

	spl5();
	(*tp->t_proc)(tp, T_RESUME);
	spl0();
	ttywait(tp);
	ttyflush(tp, (FREAD|FWRITE));

	mp = tp->t_mp;			/* pointr to map buf for this tty */
	tp->t_mp = (struct emap *)NULL;	/* null out pointer to map buffer */
	tp->t_mstate = ES_NULL;		/* mapng is disabled for this tty */

	/*
	 * If no other channel uses this
	 * map buffer for mapping, release
	 * the buffer back to freelist.
	 */
	if (--mp->e_count == 0) {
		brelse(mp->e_bp);
		mp->e_bp = (struct buf *)NULL;
	}
}


/*
 * Duplicate the mapping of a given channel for a new channel.
 * For sanity's sake, check to see if the new channel is
 * already mapped.  Currently, only called from the shell layers driver.
 */
emdupmap(tp, ntp)
struct tty *tp, *ntp;
{
	register struct emap *mp;

	if (tp == ntp)			/* just in case; sigh */
		return;
	if (tp->t_mstate == ES_NULL) {
		emunmap(ntp);
		return;
	}
	mp = tp->t_mp;
	/*
	 * make sure we don't lose the source map
	 * while we get rid of the target's map.
	 */
	mp->e_count++;
	emunmap(ntp);
	ntp->t_mstate = ES_START;	/* emapping enabled   */
	ntp->t_mp = mp;			/*   using source map */
}


/*
 * Add a new emap to the system.
 * Check for a match with existing emaps,
 * and increment the use count if a match is found.
 * Otherwise, grab an unused emap slot,
 * and setup info in the emap structure.
 */
struct emap *
emaddmap(bp)
struct buf *bp;
{
	register struct emap *mp, *fmp;
	emp_t ep;
	int i;

	fmp = (struct emap *)NULL;
	mp = &emap[0];
	i = v.v_emap;
	while (--i >= 0) {
		if (mp->e_count == 0) {
			if (fmp == (struct emap *)NULL)
				fmp = mp;
		} else {
			if (emcmpmap(bp, mp->e_bp) == 0) {
				++mp->e_count;
				return(mp);
			}
		}
		++mp;
	}

	/*
	 * New map entry found, adding new 
	 * mapping to the system map table.
	 */
	if (fmp) {
		ep = (emp_t)bimap(bp);
		fmp->e_count = 1;
		fmp->e_bp    = bp;
		fmp->e_p     = ep;
		fmp->e_ndind =
		    (ep->e_cind - E_DIND) / sizeof(struct emind);
		fmp->e_ncind =
		    ((ep->e_dctab - ep->e_cind) / sizeof(struct emind)) - 1;
		fmp->e_nsind =
		    (ep->e_stab - ep->e_sind) / sizeof(struct emind);
	}

	return(fmp);
}


/*
 * Compare two emaps; return 0 if identical.
 */
emcmpmap(bp1, bp2)
register struct buf *bp1, *bp2;
{
	register int i;

	for (i = 0; i < E_TABSZ; i += sizeof(long))
		if (bigetl(bp1, i) != bigetl(bp2, i))
			return(1);
	return(0);
}


/*
 * Check the validity of an emap; return 0 if ok.
 * A completely consistent emap is a user/utility responsibility.
 * We just check for indices and offsets that would cause us to
 * stray outside the emap.
 */
emchkmap(bp)
struct buf *bp;
{
	register int n;
	int ndind, ncind, ndcout, nsind, nschar;
	emp_t ep;
	emip_t eip;

	ep = (emp_t)bimap(bp);		/* Point to structured map buffer */

	/* check table offsets */

	n = ep->e_cind - E_DIND;
	ndind = n / sizeof(struct emind);
	if ((n < 0) || (n % sizeof(struct emind)) ||
	    (ep->e_cind > (E_TABSZ - 2 * sizeof(struct emind))))
		return(1);

	n = ep->e_dctab - ep->e_cind;
	ncind = n / sizeof(struct emind);
	if ((n < 0) || (n % sizeof(struct emind)) ||
	    (ep->e_dctab > (E_TABSZ - sizeof(struct emind))))
		return(1);

	n = ep->e_sind - ep->e_dctab;
	ndcout = n / sizeof(struct emout);
	if ((n < 0) || (n % sizeof(struct emout)) || (ep->e_sind > E_TABSZ))
		return(1);

	n = ep->e_stab - ep->e_sind;
	nsind = n / sizeof(struct emind);
	nschar = E_TABSZ - ep->e_stab;
	if ((n < 0) || (n % sizeof(struct emind)) || (nschar < 0))
		return(1);

	/* check dead/compose indices */
	eip = ep->e_dind;
	n = ndind + ncind;
	while (--n > 0) {
		if (eip[1].e_ind < eip[0].e_ind)
			return(1);
		++eip;
	}
	if ((n == 0) && ((int)eip->e_ind > ndcout))
		return(1);

	/* check string indices */
	eip = (emip_t)((emcp_t)ep + ep->e_sind);
	n = nsind;
	while (--n > 0) {
		if (eip[1].e_ind < eip[0].e_ind)
			return(1);
		++eip;
	}
	if ((n == 0) && ((int)eip->e_ind > nschar))
		return(1);

	/* looks like a usable map */
	return(0);
}



/*
 * Do input emapping; called by ttin().
 * Given a pointer to and length of a string of characters,
 * map the string in place and return its new length.
 * The string will not expand, but may contract.
 * Tty structure emapping fields are affected.
 */
emmapin(tp, cp, nc)
struct tty *tp;
unsigned char *cp;
int nc;
{
	register int i;
	register unsigned char c;
	unsigned char mc;
	unsigned char *ocp = cp;
	unsigned char *icp = cp;
	int err = 0;
	struct emap *mp;
	emp_t ep;
	emip_t eip;
	emop_t eop;

	mp = tp->t_mp;
	ep = mp->e_p;
	while (--nc >= 0) {
	    c = *icp++;			/* Grab a char from string */
	    mc = ep->e_imap[c];		/* Index down to the imap table */

	    switch (tp->t_mstate) {

	    case ES_START:
		if ((mc != E_ESC) || (c == E_ESC)) {
			*ocp++ = mc;	/* Substitute char w/ its map char */
			continue;
		}
		if (c == ep->e_comp) {
			tp->t_mstate = ES_COMP1;
		} else {
			tp->t_mchar = c;
			tp->t_mstate = ES_DEAD;
		}
		break;

	    case ES_COMP1:
		if (mc == E_ESC) {
			if (c == ep->e_comp) {
				++err;
			} else {
				tp->t_mchar = c;
				tp->t_mstate = ES_COMP2;
			}
		} else {
			tp->t_mchar = mc;
			tp->t_mstate = ES_COMP2;
		}
		break;

	    case ES_DEAD:
		if (mc == E_ESC) {
			++err;
			if (c == ep->e_comp)
				tp->t_mstate = ES_COMP1;
			else
				tp->t_mstate = ES_START;
			break;
		}
		eip = ep->e_dind;
		i = mp->e_ndind;
		goto dcsearch;

	    case ES_COMP2:
		if (mc == E_ESC) {
			if (c == ep->e_comp) {
				++err;
				tp->t_mstate = ES_COMP1;
				break;
			}
			mc = c;
		}
		eip = (emip_t)((emcp_t)ep + ep->e_cind);
		i = mp->e_ncind;

  dcsearch:
		c = tp->t_mchar;
		while (--i >= 0) {
			if (eip->e_key == c)
				break;
			++eip;
		}
		if (i >= 0) {
			i = eip->e_ind;
			eop = (emop_t)((emcp_t)ep + ep->e_dctab) + i;
			i = (++eip)->e_ind - i;
			c = mc;
			while (--i >= 0) {
				if (eop->e_key == c) {
					*ocp++ = eop->e_out;
					break;
				}
				++eop;
			}
		}
		if (i < 0)
			++err;
		tp->t_mstate = ES_START;
		break;

	    } /* end switch */

	} /* end while */

	tp->t_merr = err && ep->e_beep;
	return(ocp - cp);
}


/*
 * Do output emapping; called by ttxput().
 * Given a character, return a pointer to and the length of
 * the string of characters to which it maps.
 */
emcp_t
emmapout(tp, c, pnc)
struct tty *tp;
unsigned char c;
int *pnc;
{
	register int i;
	struct emap *mp;
	emp_t ep;
	emcp_t ecp, esp;
	emip_t eip;

	mp = tp->t_mp;
	ep = mp->e_p;
	ecp = &ep->e_omap[c];
	if (*ecp == E_ESC) {
		eip = (emip_t)((emcp_t)ep + ep->e_sind);
		esp = (emcp_t)ep + ep->e_stab;
		i = mp->e_nsind;
		while (--i >= 0) {
			if (eip->e_key == c) {
				i = eip->e_ind;
				*pnc = (++eip)->e_ind - i;
				return(esp + i);
			}
			++eip;
		}
	}
	*pnc = 1;
	return(ecp);
}



str_emsetmap(q, str_bp, emp_tp)
register queue_t *q;
register mblk_t *str_bp;
register struct emp_tty *emp_tp;
{
	register struct emap *mp;
	struct buf *bp;
	int error = 0;

	bp = getrbuf(KM_NOSLEEP);
	if (bp == (struct buf *) NULL)
		return (ENOMEM);
	bp->b_un.b_addr = kmem_alloc(E_TABSZ,KM_NOSLEEP);
	if (paddr(bp) == (paddr_t) NULL) {
		freerbuf(bp);
		return (ENOMEM);
	}

	/*
	 * Copy the new user map table into
	 * the allocated buffer
	 */
	bcopy((caddr_t)str_bp->b_rptr, (caddr_t)paddr(bp), E_TABSZ);
	if (emchkmap(bp)) {		/* validate emap */
		error = EINVAL;
		goto out;
	}

	/*
	 * If this line is the only user of an emap,
	 * free it now to ensure success in emaddmap().
	 */
	if ((emp_tp->t_mstate != ES_NULL) && (emp_tp->t_mp->e_count == 1))
		str_emunmap(q, emp_tp);

	mp = (struct emap *) emaddmap(bp);
	if (mp == (struct emap *)NULL) {	/* can't add emap */
		error = ENAVAIL;
		goto out;
	}
	if (mp->e_count == 1)			/* if this is a new emap, */
		bp = (struct buf *)NULL;	/*   don't free the buf   */
	if (emp_tp->t_mstate != ES_NULL)	/* free old emap	  */
		str_emunmap(q, emp_tp);		/*   if we still have one */
						/* to attach new mapping  */

	emp_tp->t_mstate = ES_START;		/* emapping enabled */
	emp_tp->t_mp = mp;				/*   using new map  */

  out:
	if (bp) {
		kmem_free((caddr_t)paddr(bp),E_TABSZ);
		freerbuf(bp);
	}
	return(error);
}



/*
 * Disable emapping on a line.
 * Decrement the use count of the emap,
 * and free it if the count becomes zero.
 */
str_emunmap(q, emp_tp)
register queue_t *q;
register struct emp_tty *emp_tp;
{
	register struct emap *mp;

	if (emp_tp->t_mstate == ES_NULL)
		return;

	if (q->q_next)
		(void) putctl(q->q_next, M_START);
	flushq(q,FLUSHDATA);
	flushq(RD(q), FLUSHDATA);

	mp = emp_tp->t_mp;		/* pointr to map buf for this tty */
	emp_tp->t_mp = (struct emap *)NULL;	/* null out pointer to map buffer */
	emp_tp->t_mstate = ES_NULL;	/* mapng is disabled for this tty */

	/*
	 * If no other channel uses this
	 * map buffer for mapping, release
	 * the buffer back to freelist.
	 */
	if (--mp->e_count == 0) {
		kmem_free(paddr(mp->e_bp),E_TABSZ);
		freerbuf(mp->e_bp);
		mp->e_bp = (struct buf *)NULL;
	}
}


/*
 * Return the current emap in effect on a line. Store the data in bpp 
 */
str_emgetmap(emp_tp,bpp)
register mblk_t *bpp;
register struct emp_tty *emp_tp;
{
	register struct buf *bp;
	int error = 0;

	if (emp_tp->t_mstate == ES_NULL)
		error = ENAVAIL;
	else {
		bp = emp_tp->t_mp->e_bp; /* get map buffer for this tty */
		/* copy the channel mapping table into user buffer */
		bcopy((caddr_t)paddr(bp), bpp->b_rptr, E_TABSZ);
		bpp->b_wptr += E_TABSZ;
	}
	return(error);
}


/*
 * Given a pointer to and length of a string of characters,
 * map the string in place and return its new length.
 * The string will not expand, but may contract.
 * Tty structure emapping fields are affected.
 */
str_emmapin(bp, emp_tp)
mblk_t *bp;
struct emp_tty *emp_tp;
{
	register int i;
	register unsigned char c;
	unsigned char mc;
	unsigned char *cp = (unsigned char *)bp->b_rptr;
	unsigned char *ocp = cp;
	int err = 0;
	struct emap *mp;
	emp_t ep;
	emip_t eip;
	emop_t eop;

	mp = emp_tp->t_mp;
	ep = mp->e_p;
	while ( cp < bp->b_wptr ) {
	    c = *cp++;			/* Grab a char from string */
	    mc = ep->e_imap[c];			/* Index down to the imap table */

	    switch (emp_tp->t_mstate) {

	    case ES_START:
		if ((mc != E_ESC) || (c == E_ESC)) {
			*ocp++ = mc;	/* Substitute char w/ its map char */
			continue;
		}
		if (c == ep->e_comp) {
			emp_tp->t_mstate = ES_COMP1;
		} else {
			emp_tp->t_mchar = c;
			emp_tp->t_mstate = ES_DEAD;
		}
		break;

	    case ES_COMP1:
		if (mc == E_ESC) {
			if (c == ep->e_comp) {
				++err;
			} else {
				emp_tp->t_mchar = c;
				emp_tp->t_mstate = ES_COMP2;
			}
		} else {
			emp_tp->t_mchar = mc;
			emp_tp->t_mstate = ES_COMP2;
		}
		break;

	    case ES_DEAD:
		if (mc == E_ESC) {
			++err;
			if (c == ep->e_comp)
				emp_tp->t_mstate = ES_COMP1;
			else
				emp_tp->t_mstate = ES_START;
			break;
		}
		eip = ep->e_dind;
		i = mp->e_ndind;
		goto dcsearch;

	    case ES_COMP2:
		if (mc == E_ESC) {
			if (c == ep->e_comp) {
				++err;
				emp_tp->t_mstate = ES_COMP1;
				break;
			}
			mc = c;
		}
		eip = (emip_t)((emcp_t)ep + ep->e_cind);
		i = mp->e_ncind;

  dcsearch:
		c = emp_tp->t_mchar;
		while (--i >= 0) {
			if (eip->e_key == c)
				break;
			++eip;
		}
		if (i >= 0) {
			i = eip->e_ind;
			eop = (emop_t)((emcp_t)ep + ep->e_dctab) + i;
			i = (++eip)->e_ind - i;
			c = mc;
			while (--i >= 0) {
				if (eop->e_key == c) {
					*ocp++ = eop->e_out;
					break;
				}
				++eop;
			}
		}
		if (i < 0)
			++err;
		emp_tp->t_mstate = ES_START;
		break;

	    } /* end switch */

	} /* end while */

	emp_tp->t_merr = err && ep->e_beep;
	bp->b_wptr = ocp;
	return(ocp - (char *)bp->b_rptr);
}


/*
 * Do output emapping; called by ttxput().
 * Given a character, return a pointer to and the length of
 * the string of characters to which it maps.
 */
emcp_t
str_emmapout(emp_tp, c, pnc)
struct emp_tty *emp_tp;
unsigned char c;
int *pnc;
{
	register int i;
	struct emap *mp;
	emp_t ep;
	emcp_t ecp, esp;
	emip_t eip;

	mp = emp_tp->t_mp;
	ep = mp->e_p;
	ecp = &ep->e_omap[c];
	if (*ecp == E_ESC) {
		eip = (emip_t)((emcp_t)ep + ep->e_sind);
		esp = (emcp_t)ep + ep->e_stab;
		i = mp->e_nsind;
		while (--i >= 0) {
			if (eip->e_key == c) {
				i = eip->e_ind;
				*pnc = (++eip)->e_ind - i;
				return(esp + i);
			}
			++eip;
		}
	}
	*pnc = 1;
	return(ecp);
}
