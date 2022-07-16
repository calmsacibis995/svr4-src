/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_cirmgr.c	1.3.1.1"

#include "sys/list.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/file.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/var.h"
#include "sys/vnode.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/inline.h"
#include "sys/debug.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/tihdr.h"
#include "sys/hetero.h"
#include "sys/systm.h"
#include "rf_auth.h"
#include "sys/cmn_err.h"
#include "rf_canon.h"
#include "sys/strmdep.h"

#ifdef DEBUG
int	gdp_call_demon;
#endif

/*
 * Streamhead/virtual circuit manager for RFS.
 *
 * asm labels used only for testing.  Don't even THINK about using them
 * for anything else.
 */

#ifndef MIN
#define MIN(a, b)	((a) <= (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)	((a) >= (b) ? (a) : (b))
#endif

/*
 * Yield the size of the request or response header following *cop.
 * (Note that version 1 request and response headers are the same size,
 * as are version 1 responses and nacks and,  version 2 responses and nacks.)
 */
#define GDP_RHSZ(gp, cop)	((gp)->version < RFS2DOT0 ? \
  sizeof(rf_request_t) - sizeof(struct rqv2) : \
    (cop)->co_type == RF_REQ_MSG ? sizeof(rf_request_t) : sizeof(rf_response_t))

/*
 * Concatenate and align at least first len bytes of message under bp.
 * len == -1 means concat everything.  Yields 1 on success, 0 on failure.
 */
#define GDP_PULLUP(bp, len) \
	((len) != -1 && (bp)->b_wptr - (bp)->b_rptr >= (len) \
	  && str_aligned((bp)->b_rptr) || pullupmsg((bp), (len)))

/* imports */
extern int	nulldev();

STATIC void	gdp_rput();
STATIC void	gdp_rsrv();
STATIC void	gdp_wsrv();
STATIC void	gdp_dodiscon();
STATIC void	gdp_cleanhdr();
STATIC mblk_t	*gdp_splitmsg();
STATIC int	gdp_process();
STATIC int	gdp_init_circuit();
STATIC void	gdp_tokclear();
STATIC void	gdp_clean_circuit();
STATIC int	gdp_tokcmp();
STATIC void	gdp_setdsz();
STATIC size_t	gdp_msgsize();

STATIC struct module_info gdp_info = { 0, NULL, 0, INFPSZ, 4*1024, 1024 };

STATIC struct qinit gdprdata = {
	(int (*)())gdp_rput, (int (*)())gdp_rsrv, nulldev,
	nulldev, nulldev, &gdp_info, NULL
};

STATIC struct qinit gdpwdata = {
	nulldev, (int (*)())gdp_wsrv, nulldev, nulldev, nulldev, &gdp_info, NULL
};

/*
 * Get the queue pointer representing a remote machine.  Normally,
 * take the queue associated with fd (it had better be streams-based),
 * and associate with it the token, for future reference.  If the
 * fd is -1, then return the queue pointer previously associated
 * with the token.  gmp can be NULL iff fd is -1.
 */
int
gdp_get_circuit(fd, tokenp, qpp, gmp)
	int			fd;
	register rf_token_t	*tokenp;
	register queue_t	**qpp;
	register gdpmisc_t	*gmp;
{
	register gdp_t		*gp;
	register gdp_t		*endgp = gdp + maxgdp;

	for (gp = gdp; gp < endgp; gp++) {
		if (fd >= 0) {
			if (gp->constate == GDPFREE) {
				return gdp_init_circuit(fd, gp, tokenp, qpp,
				  gmp);
			}
		} else if (gp->constate == GDPCONNECT &&
		  gdp_tokcmp(&(gp->token), tokenp)) {
			*qpp = gp->queue;
			return 0;
		}
	}
	*qpp = NULL;
	return ENOLINK;
}

void
gdp_put_circuit(qpp)
	queue_t **qpp;
{
	if (*qpp) {
		register gdp_t *gdpp = QPTOGP(*qpp);

		if (!gdpp->mntcnt) {
			gdp_clean_circuit(*qpp);
		}
		*qpp = NULL;
	}
}

int
gdp_init()
{
	register gdp_t *gp;
	register int	i;

	for (gp = gdp, i = 0; i < maxgdp; gp++) {

		/*
		 * set the local half of sysid -- the remote half is set
		 * when the first message arrives
		 */

		gp->queue = NULL;
		gp->sysid = ++i;
		gp->constate = GDPFREE;
		gp->token.t_id = 0;
		gp->token.t_uname[0] = '\0';
		gp->idmap[0] = 0;
		gp->idmap[1] = 0;
		gp->input.oneshot = 0;
		gp->timeout = 0;
		gdp_cleanhdr(gp);
		gp->inseq = gp->outseq = 0;
	}
	return 0;
}

/* Return the stream queue to remote with sysid. */
queue_t *
gdp_sysidtoq(sysid)
	sysid_t sysid;
{
	register struct gdp *gdpp;
	register struct gdp *gdpend = gdp + maxgdp;

	for (gdpp = gdp; gdpp < gdpend; gdpp++) {
		if (gdpp->sysid == sysid && gdpp->constate != GDPFREE) {
			return gdpp->queue;
		}
	}
	return NULL;
}

void
gdp_discon(message, gp)
	char	*message;
	gdp_t	*gp;
{
	cmn_err(CE_NOTE, "%s: disconnecting on gdp %x, remote %s",
	  message, (int)gp, gp->token.t_uname);
	gdp_dodiscon(gp);
}

void
gdp_j_accuse(message, gp)
	char	*message;
	gdp_t	*gp;
{
	cmn_err(CE_NOTE, "%s: gdp %x, remote %s",
	  message, (int)gp, gp->token.t_uname);
}

void
gdp_kill()
{
	register gdp_t *tmp;
	register gdp_t *endgdp = gdp + maxgdp;

	for (tmp = gdp; tmp < endgdp; tmp++) {
		if (tmp->constate == GDPCONNECT) {
			gdp_clean_circuit(tmp->queue);
		}
	}
}

STATIC size_t
gdp_msgsize(mp)
	register mblk_t	*mp;
{
	register size_t	n;

	for (n = 0; mp; mp = mp->b_cont) {
		n += mp->b_wptr - mp->b_rptr;
	}
	return n;
}

STATIC void
gdp_rput(q, bp)
	register queue_t	*q;
	register mblk_t		*bp;
{
	register union T_primitives *Tp;
	register mblk_t		*contbp;
	register gdp_t		*gp;
	size_t			size;

	gp = QPTOGP(q);
	switch (bp->b_datap->db_type) {
	case M_PROTO:
	case M_PCPROTO:

		/* Assume the 1st block M_PROTO/M_PCPROTO is not fragmented. */

		Tp = (union T_primitives *)bp->b_rptr;
		switch ((int)Tp->type) {
		case T_DATA_IND:
			ASSERT((bp->b_wptr - bp->b_rptr) ==
			    sizeof(struct T_data_ind ));
			if (((struct T_data_ind *)Tp)->MORE_flag) {
				gp->input.oneshot = 0;
			}
			contbp = bp->b_cont;
			freeb(bp);
			if (contbp->b_datap->db_type != M_DATA) {
asm(".globl .gdpbadproto");
asm(".gdpbadproto:");
				freemsg(contbp);
				return;
			}

			putq(q, contbp);
			break;

		case T_DISCON_IND:
		case T_ORDREL_IND:

#ifdef NPACK_BUG_FIXED
			ASSERT((bp->b_wptr-bp->b_rptr) ==
			  sizeof(struct T_discon_ind));
#endif
			if ((size = (bp->b_wptr - bp->b_rptr)) !=
			   sizeof(struct T_discon_ind)
			  || size != sizeof(struct T_ordrel_ind)) {
asm(".globl .gdpbaddiscon");
asm(".gdpbaddiscon:");
			}
			gdp_dodiscon(gp);
			goto free;

		case T_INFO_ACK:
			 {
				long	tsdusz =
					  ((struct T_info_ack *)Tp)->TSDU_size;
				size_t	maxmsg;

				gp->maxpsz =
				  ((struct T_info_ack *)Tp)->TIDU_size;
				gdp_setdsz(gp);
				maxmsg = RF_MAXHEAD(gp->version) + gp->datasz;
				if (tsdusz != -2 && (size_t)tsdusz >= maxmsg &&
				  gp->maxpsz >= maxmsg) {

					/*
					 * Provider supports data messages,
					 * appears able to accomodate any RFS
					 * message in a single protocol
					 * transmission.
					 */

					gp->input.oneshot = 1;
				}
				goto free;
			}

		default:
			goto free;
		} /*end switch */
		break;

	case M_DATA:
		putq(q, bp);
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHR) {
			flushq(q, FLUSHALL);
		}
		if (*bp->b_rptr & FLUSHW) {
			*bp->b_rptr &= ~FLUSHR;
			putnext(WR(q), bp);
			break;
		}
		goto free;

	case M_ERROR:
	case M_HANGUP:
		gdp_dodiscon(gp);
		goto free;

	default:
free:
		freemsg(bp);
		break;
	}
}

STATIC void
gdp_rsrv(qp)
	register queue_t	*qp;
{
	register mblk_t		*bp;
	register mblk_t		*splitbp;
	register gdp_t		*gp;

	gp = QPTOGP(qp);
	gp->timeout = 0;

	if (gp->constate != GDPCONNECT) {
		while (bp = getq(qp)) {
			freeb(bp);
		}
		return;
	}

	if (gp->input.oneshot) {
		while (bp = getq(qp)) {
			if (!gdp_process(qp, bp)) {
				goto loop;
			}
		}
		return;
	}

	/*
	 * If getq fails while we are gathering messages, we want just to
	 * return and wait until more arrive.  We never call getq when we
	 * already have a meaningful bp in hand.  If we are in either
	 * GDPSTPMC or GDPSTPR, and we call getq, it is because we previously
	 * did a putbq.
	 */

	while ((bp = getq(qp)) != NULL ||
	    gp->input.istate == GDPSTPMC ||
	    gp->input.istate == GDPSTPR) {
		rf_common_t	*cop;
		int		type;

loop:
		splitbp = NULL;
		switch (gp->input.istate) {

		case GDPSTGMC:

asm(".globl .GDPGMC");
asm(".GDPGMC:");

			/* gathering rf_message_t and rf_common_t headers */

			ASSERT(bp && bp != (mblk_t * )1 && gp->hlen > 0);

			splitbp = gdp_splitmsg(bp, (long)gp->hlen);
			if (splitbp == (mblk_t *)-1) {
				putbq(qp, bp);
				timeout(qenable, (char *)qp, HZ);
				return;
			}
			if (gp->hdr) {
				linkb(gp->hdr, bp);
			} else {
				gp->hdr = bp;
			}
			if (!splitbp) {

				/* not enough data for header */

				gp->hlen -= gdp_msgsize(bp);
				continue;
			}
			gp->input.istate = GDPSTPMC;
			bp = splitbp;

			/* FALLTHROUGH */

		case GDPSTPMC:

asm(".globl .GDPPMC");
asm(".GDPPMC:");

			/* pulling up rf_message_t and rf_common_t headers */

			ASSERT(gp->hdr);

			if (!GDP_PULLUP(gp->hdr, RF_MCSZ)) {

				/*
				 * Headers not all aligned in first block
				 * and can't pull them up.
				 */

				if (bp && bp != (mblk_t *)1) {

					/*
					 * Message and common headers are in
					 * gp->hdr, but unaligned, and we're
					 * short on resources.  Put this back
					 * for later.
					 */

					putbq(qp, bp);
				}
				if (!bufcall(RF_MCSZ, BPRI_MED, qenable,
				     (long)qp)) {
					timeout(qenable, (char *)qp, HZ);
				}
				return;
			}
			if (gp->hetero == ALL_CONV && !gp->input.mcdecan) {
				if (!rf_mcfcanon(gp->hdr)) {
					putbq(qp, gp->hdr);
					gp->hdr = NULL;
					gp->idata = NULL;
					gdp_cleanhdr(gp);
					gdp_discon("gdp_rsrv: bad header data",
					  gp);
					return;
				}
				gp->input.mcdecan = 1;
			}
			cop = RF_COM(gp->hdr);

			/*
			 * The following check could fail if the remote
			 * implementation is defective.  E.g., the length of
			 * the preceeding message could have been incorrectly
			 * encoded.  (For providers that don't preserve message
			 * boundaries, this check is about as good as we can
			 * do.)
			 */

			if ((type = cop->co_type) != RF_REQ_MSG
			  && type != RF_RESP_MSG && type != RF_NACK_MSG) {
				putbq(qp, gp->hdr);
				gp->hdr = NULL;
				gp->idata = NULL;
				gdp_cleanhdr(gp);
				gdp_discon("gdp_rsrv bad RFS header type", gp);
				return;
			}

			gp->rhlen = gp->hlen = GDP_RHSZ(gp, cop);
			gp->input.istate = GDPSTGR;
			if (!bp || bp == (mblk_t *)1) {
				continue;
			}

			/* FALLTHROUGH */

		case GDPSTGR:

asm(".globl .GDPGR");
asm(".GDPGR:");
			/* gathering rf_request_t or rf_response_t header */

			ASSERT(bp && bp != (mblk_t *)1 && gp->hlen > 0);
			ASSERT(gp->hdr);

			splitbp = gdp_splitmsg(bp, (long)gp->hlen);
			if (splitbp == (mblk_t *)-1) {
				putbq(qp, bp);
				timeout(qenable, (char *)qp, HZ);
				return;
			}
			linkb(gp->hdr, bp);
			if (!splitbp) {

				/* need more data for header */

				gp->hlen -= gdp_msgsize(bp);
				continue;
			}
			gp->hlen = 0;
			bp = splitbp;
			splitbp = NULL;
			gp->input.istate = GDPSTPR;

			/* FALLTHROUGH */

		case GDPSTPR:

asm(".globl .GDPPR");
asm(".GDPPR:");
			/*
			 * Pulling up rf_request_t or rf_response_t header
			 * with rf_message_t and rf_common_t header
			 */

			ASSERT(gp->hdr);

			if (!GDP_PULLUP(gp->hdr, RF_MCSZ + gp->rhlen)) {
				if (bp && bp != (mblk_t *)1) {

					/*
					 * All headers are in gp->hdr, but
					 * unaligned, and we're short on
					 * resources.  Put this back for later.
					 */

					putbq(qp, bp);
				}
				if (!bufcall(RF_MCSZ + gp->rhlen, BPRI_MED,
				  qenable, (long)qp)) {
					timeout(qenable, (char *)qp, HZ);
				}
				return;
			}
			if (gp->hetero == ALL_CONV && !gp->input.rhdecan) {
				if (!rf_rhfcanon(gp->hdr, gp)) {
					if (bp && bp != (mblk_t *)1) {
						putbq(qp, bp);
					}
					gdp_discon("gdp_rsrv bad header data",
					  gp);
					return;
				}
				gp->input.rhdecan = 1;
			}
			gp->input.istate = GDPSTGD;
			gp->dlen =
			  RF_MSG(gp->hdr)->m_size - (RF_MCSZ + gp->rhlen);
			if (!gp->dlen) {

				/* no data part in this RFS message */

				if (bp && bp != (mblk_t *)1) {

					/* Data left in streams message. */

					putbq(qp, bp);
				}
				goto eom;
			}
			if (!bp || bp == (mblk_t *)1) {
				continue;
			}

			/* FALLTHROUGH */

		case GDPSTGD:

asm(".globl .GDPGD");
asm(".GDPGD:");

			/* Gathering data portion of RFS message. */

			ASSERT(bp && bp != (mblk_t *)1 && gp->dlen >= 0);
			ASSERT(!splitbp);

			splitbp = gdp_splitmsg(bp, (long)gp->dlen);
			if (splitbp == (mblk_t *)-1) {
				putbq(qp, bp);
				timeout(qenable, (char *)qp, HZ);
				return;
			}
			if (gp->idata) {
				linkb(gp->idata, bp);
			} else {
				gp->idata = bp;
			}
			if (!splitbp) {

				/* data is not yet complete */

				gp->dlen -= gdp_msgsize(bp);
				continue;
			}
eom:
			/* Data portion gathered. */

			(bp = gp->hdr)->b_cont = gp->idata;

			/* bp now points to a well-formed RFS message */

			gp->hdr = NULL;
			gp->idata = NULL;
			gdp_cleanhdr(gp);

			RF_MSG(bp)->m_queue = (long)qp;
#ifdef DEBUG
			{
				rf_message_t	*msg;

				/*
				 * By checking filler0 for a nonzero value, we
				 * let non-participants slip by.
				 */

				msg = RF_MSG(bp);
				if (gp->version > RFS1DOT0 &&
				  msg->filler0 && msg->filler0 != gp->inseq) {
					cmn_err(CE_NOTE,
					  "msg sequence %d gdp sequence %d\n",
					msg->filler0, gp->inseq);
					if (gdp_call_demon) {
						call_demon();
					}
				}
				gp->inseq = msg->filler0 + 1;
			}
#else
			gp->inseq++;
#endif

			ASSERT(bp->b_wptr - bp->b_rptr >=
			  RF_MCSZ + GDP_RHSZ(gp, RF_COM(bp)));

			rf_deliver(bp);

			if (!splitbp || (bp = splitbp) == (mblk_t *)1) {
				continue;
			} else {
				goto loop;
			}

		default:
			cmn_err(CE_PANIC, "gdp_rsrv: bad istate %x\n",
				gp->input.istate);
			break;
		}
	} /* while */
}

/*
 * If canput(next), send msg along and fragment it if needed.
 * Assumes that the msg has been canonized if necessary.
 */
STATIC void
gdp_wsrv(qp)
	register queue_t *qp;
{
	register mblk_t *bp;
	register gdp_t *gp;
	register union T_primitives *Tp;
	register int dbtype;

	gp = QPTOGP(qp);

	while (bp = getq(qp)) {

		dbtype = bp->b_datap->db_type;

		switch (dbtype) {
		case M_DATA:
	frag:
		if (!canput(qp->q_next)) {
			putbq(qp, bp);
				goto endwhile;
		}
		if (gdp_msgsize(bp) > gp->maxpsz) {
			register mblk_t *bp1;

			bp1 = gdp_splitmsg(bp, (long)gp->maxpsz);
			if (bp1 == (mblk_t *)-1) {
				putbq(qp, bp);
					goto endwhile;
			}
			putnext(qp, bp);
			bp = bp1;
			goto frag;
		} else {
			putnext(qp, bp);
		}
			continue;
		case M_PROTO:
		case M_PCPROTO:
			Tp = (union T_primitives *)bp->b_rptr;
			if (Tp->type == T_DISCON_REQ) {
				putnext(qp, bp);
				flushq(qp, FLUSHALL);
				flushq(RD(qp), FLUSHALL);
				return;
	}
			/* FALL THROUGH */
		default:
			putnext(qp, bp);
			continue;
		}
	}
endwhile:
	if (canput(qp)) {
		wakeprocs(qp->q_ptr, PRMPT);
	}
}

/*
 * Gather all RFS headers into one aligned streams message.  Make sure
 * that balance of message is present, too.  For failure, timeout
 * and try again later.  If message is not all present, also reset oneshot.
 *
 * Pulled-up headers are decanonized if necessary, and mcdecan and
 * rhdecan updated to reflect this.
 *
 * If succesful, calls rf_deliver, and returns 1.
 * Otherwise, returns 0; the  caller decides whether to put bp back
 * on the queue.
 */
STATIC int
gdp_process(qp, bp)
	queue_t			*qp;
	register mblk_t		*bp;
{
	if (GDP_PULLUP(bp, RF_MCSZ)) {
		register gdp_t		*gp = QPTOGP(qp);
		register size_t		msgsize = gdp_msgsize(bp);
		register rf_message_t	*msg = RF_MSG(bp);
		rf_common_t		*cop = RF_COM(bp);

		/* rf_message_t and rf_common_t headers pulled up. */

		if (gp->hetero == ALL_CONV && !gp->input.mcdecan) {
			if (!rf_mcfcanon(bp)) {
				gdp_discon("gdp_process bad header data", gp);
				freeb(bp);
				return 1;
			}
			gp->input.mcdecan = 1;
		}
		if (GDP_PULLUP(bp, RF_MCSZ + GDP_RHSZ(gp, cop))) {

			/* rf_request_t or rf_response_t header pulled up. */

			if (gp->hetero == ALL_CONV && !gp->input.rhdecan) {
				if (!rf_rhfcanon(bp, gp)) {
					gdp_discon(
					  "gdp_process bad header type", gp);
					freeb(bp);
					return 1;
				}
				gp->input.rhdecan = 1;
			}
			if (msg->m_size == msgsize) {

				/* All headers and data present. */

#ifdef DEBUG
			{
				rf_message_t	*msg;

				/*
				 * By checking filler0 for a nonzero value, we
				 * let non-participants slip by.
				 */

				msg = RF_MSG(bp);
				if (gp->version > RFS1DOT0 &&
				  msg->filler0 && msg->filler0 != gp->inseq) {
					cmn_err(CE_NOTE,
					  "msg sequence %d gdp sequence %d\n",
					msg->filler0, gp->inseq);
					if (gdp_call_demon) {
						call_demon();
					}
				}
				gp->inseq = msg->filler0 + 1;
			}
#else
				gp->inseq++;
#endif
				ASSERT(bp->b_wptr - bp->b_rptr >=
				  RF_MCSZ + GDP_RHSZ(gp, cop));

				gdp_cleanhdr(gp);
				RF_MSG(bp)->m_queue = (long)qp;
				rf_deliver(bp);
				return 1;

			} else if (msg->m_size < msgsize) {
				gdp_discon("gdp_process bad size", gp);
				freeb(bp);
				return 1;
			}
		}
		if (gp->input.oneshot && RF_MSG(bp)->m_size != msgsize) {

			/* Less than entire RFS message is present */

			gp->input.oneshot = 0;
		}
	}
	timeout(qenable, (caddr_t)qp, HZ);
	return 0;
}

/* attaches the stream described by fd to gdp struct pointed at by gdpp */
STATIC int
gdp_init_circuit(fd, gdpp, tokenp, qpp, gmp)
	register int		fd;
	register gdp_t		*gdpp;
	register rf_token_t	*tokenp;
	queue_t			**qpp;
	register gdpmisc_t	*gmp;
{
	register queue_t *qp;
	register struct stdata *stp;
	file_t *fp;
	register mblk_t *bp, *mp;
	int	s;

	if (fd < 0 || fd >= u.u_nofiles || getf(fd, &fp) ||
	    fp->f_vnode->v_count != 1 || !(stp = fp->f_vnode->v_stream)) {
		*qpp = NULL;
		return EBADF;
	}

	if ((bp = allocb(sizeof(struct T_info_req ), BPRI_HI)) == NULL) {
		*qpp = NULL;
		return ENOMEM;
	}

	s = splstr();

	qp = RD(stp->sd_wrq); 		/* steal the stream head's queues */
	stp->sd_flag |= STPLEX;		/* in case someone else can open late */
	fp->f_count++;			/* for consistency */

	gdpp->queue = qp;
	gdpp->mntcnt = 0;
	gdpp->timeskew_sec = 0;
	gdpp->inseq = gdpp->outseq = 0;

	/* point q at gdp structure  and intialize q */

	qp->q_ptr = WR(qp)->q_ptr = (caddr_t)gdpp;
	setq(qp, &gdprdata, &gdpwdata);
	qp->q_flag |= QWANTR;
	WR(qp)->q_flag |= QWANTR;

	gdpp->file = fp;
	/*
	 * strip M_PROTO:T_GR_IND
	 */

	for (mp = qp->q_first; mp; ) {
		mblk_t * emp, *tmp;

		switch (mp->b_datap->db_type) {
		case M_PROTO:
			if (((union T_primitives *)mp->b_rptr)->type !=
			  T_DATA_IND)
				goto stripbad;
			emp = mp->b_next;
			rmvq(qp, mp);
			tmp = (mblk_t * )unlinkb(mp);
			freeb(mp);
			insq(qp, emp, tmp);
			mp = emp;
			break;

		case M_DATA:
			mp = mp->b_next;
			break;

		default:
stripbad:
			freeb(bp);
			*qpp = NULL;
			return EPROTO;
		}
	}

	if (qp->q_first) {
		qenable(qp);
	}
	splx(s);

	/* request TSDU_size from the provider */
	gdpp->input.oneshot = 0;
	gdpp->input.mcdecan = 0;
	gdpp->input.rhdecan = 0;
	gdpp->token = *tokenp;
	gdpp->hetero = gmp->hetero;
	gdpp->version = gmp->version;
	gdpp->ngroups_max = gmp->ngroups_max;
	gdpp->idmap[0] = 0;
	gdpp->idmap[1] = 0;
	gdpp->constate = GDPCONNECT;

	bp->b_datap->db_type = M_PCPROTO;
	((struct T_info_req *)(bp->b_wptr))->PRIM_type = T_INFO_REQ;
	bp->b_wptr += sizeof(struct T_info_req);
	putnext(WR(qp), bp);
	*qpp = qp;
	return 0;
}

STATIC void
gdp_clean_circuit(qp)
	register queue_t *qp;
{
	extern struct qinit strdata, stwdata;
	register gdp_t *gdpp;
	register struct stdata *stp;
	register int	s;

	gdpp = QPTOGP(qp);
	if (gdpp->constate == GDPFREE) {
		return;
	}

	/* restore queues to their rightful owner, the stream head */

	stp = gdpp->file->f_vnode->v_stream;
	s = splstr();
	setq(qp, &strdata, &stwdata);
	qp->q_ptr = WR(qp)->q_ptr = (char *)stp;
	splx(s);

	stp->sd_flag &= ~STPLEX;
	closef(gdpp->file);
	gdpp->file = NULL;
	gdpp->queue = NULL;
	gdpp->constate = GDPFREE;
	gdpp->sysid = gdpp - gdp + 1;
	gdpp->hetero = 0;
	gdpp->version = 0;
	gdpp->timeskew_sec = 0;
	gdpp->mntcnt = 0;
	gdpp->timeout = 0;
	gdp_tokclear (gdpp);
	if (gdpp->idmap[0]) {
		rf_freeidmap(gdpp->idmap[0]);
	}
	if (gdpp->idmap[1]) {
		rf_freeidmap(gdpp->idmap[1]);
	}
	gdpp->idmap[0] = 0;
	gdpp->idmap[1] = 0;
	gdpp->input.oneshot = 0;
	gdpp->sysid = gdpp - gdp + 1;
	gdpp->inseq = gdpp->outseq = 0;
	gdp_cleanhdr(gdpp);
}

STATIC void
gdp_tokclear(gp)
	register gdp_t *gp;
{
	gp->token.t_id = 0;
	gp->token.t_uname[0] = '\0';
}

STATIC int
gdp_tokcmp(n1, n2)
	rf_token_t *n1, *n2;
{
	register char	*p1 = n1->t_uname, *p2 = n2->t_uname;

	if (n1->t_id != n2->t_id) {
		return 0;
	}
	if (strncmp(p1, p2, MAXDNAME) == 0) {
		return 1;
	}
	return 0;
}

#define NBS (sizeof(size_t) * NBBY)	/* n bits in a size_t */

STATIC void
gdp_setdsz(gp)
	register gdp_t	*gp;
{
	register queue_t *gwq, *tpwq;

	gwq = WR(gp->queue);
	tpwq = gwq->q_next;
	if (gp->version < RFS2DOT0) {
		gp->datasz = DU_DATASIZE;
	} else {

		/*
		 * This implementation picks
		 * MAX(PAGESIZE, greatest power of two <= transport->hiwat).
		 * We assume hiwat approximates the transport windowsize,
		 * and that, by not exceeding that value, we will be less
		 * likely to block on flow control and handshaking.
		 */

		register int	shift;
		register size_t	pow2;
		register size_t	hiwat = tpwq->q_hiwat;

		for (shift = 1; shift < NBS; shift++ ) {
			if (hiwat <= (pow2 = 1 << shift)) {
				break;
			}
		}
		if (shift == NBS || pow2 > hiwat) {
			pow2 = 1 << --shift;
		}
		gp->datasz = MAX(PAGESIZE, pow2);
	}
	if (gp->datasz < 64 * 1024) {
		RD(gwq)->q_hiwat = gp->datasz * 4;
		RD(gwq)->q_lowat = gp->datasz;
		gwq->q_hiwat = gp->datasz * 4;
		gwq->q_lowat = gp->datasz;
	} else {
		RD(gwq)->q_hiwat = gp->datasz;
		RD(gwq)->q_lowat = gp->datasz / 4;
		gwq->q_hiwat = gp->datasz;
		gwq->q_lowat = gp->datasz / 4;
	}
}

STATIC void
gdp_dodiscon(gp)
	gdp_t	*gp;
{
	int	s = splstr();

	if (gp->constate == GDPCONNECT) {
		gp->constate = GDPDISCONN;
		rf_daemon_flag |= RFDDISCON;
		splx(s);
		wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);
	} else {
		splx(s);
	}
	return;
}

STATIC void
gdp_cleanhdr(gdpp)
	register gdp_t *gdpp;
{
	freemsg(gdpp->hdr);
	gdpp->hdr = NULL;
	freemsg(gdpp->idata);
	gdpp->idata = NULL;
	gdpp->hlen = RF_MCSZ;
	gdpp->rhlen = 0;
	gdpp->dlen = 0;
	gdpp->input.mcdecan = 0;
	gdpp->input.rhdecan = 0;
	gdpp->input.istate = GDPSTGMC;
}

/*
 * Leave the 1st len bytes in mp and return the remaining message.
 * case (len == gdp_msgsize(mp)): mp unchanged, return 1;
 * case (len > gdp_msgsize(mp)):  mp unchanged, return NULL;
 * case (len < gdp_msgsize(mp)):  reduce mp to len bytes, return remainder
 * in result.
 * failure:  return (mblk_t *)-1.
 *
 * TO DO:  replace this rotten overloading with substates in gdp_rsrv.
 */
STATIC mblk_t *
gdp_splitmsg(mp, len)
	mblk_t		*mp;
	register long	len;
{
	register mblk_t	*bp;
	register mblk_t	*bp2;

	for (bp = mp; bp; bp = bp->b_cont) {
		len -= bp->b_wptr - bp->b_rptr;
		if (len <= 0) {
			break;
		}
	}
	if (!len) {
		if (!bp->b_cont) {
			return (mblk_t *)1;
		} else {
			bp2 = bp->b_cont;
			bp->b_cont = NULL;
			return bp2;
		}
	}
	if (len > 0) {
		return NULL;
	}
	if ((bp2 = dupb(bp)) == NULL) {
		return (mblk_t *)-1;
	}
	/* len < 0 */
	bp2->b_rptr = (bp->b_wptr += len);
	if (bp->b_cont) {
		bp2->b_cont = bp->b_cont;
		bp->b_cont = NULL;
	}
	return bp2;
}

void
gdp_put_discon(wq)
	queue_t	*wq;
{
	struct T_discon_req	*drp;
	mblk_t			*dbp;

	(void)rf_allocb(sizeof(char), BPRI_HI, FALSE,
	  NULLCADDR, NULLFRP, &dbp);
	dbp->b_datap->db_type = M_FLUSH;
	*dbp->b_rptr = FLUSHRW;
	dbp->b_wptr = dbp->b_rptr + sizeof(char);
	putq(wq, dbp);
	(void)rf_allocb(sizeof(struct T_discon_req), BPRI_HI,
	  FALSE, NULLCADDR, NULLFRP, &dbp);
	drp = (struct T_discon_req *)dbp->b_rptr;
	drp->PRIM_type = T_DISCON_REQ;
	drp->SEQ_number = -1;
	dbp->b_datap->db_type = M_PCPROTO;
	dbp->b_wptr = dbp->b_rptr + sizeof(struct T_discon_req);
	putq(wq, dbp);
}
