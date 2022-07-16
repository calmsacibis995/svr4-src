/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_krcvudat.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_krcvudata.c 1.1 88/12/12 SMI"
#endif

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 *	Kernel TLI-like function to read a datagram off of a
 *	transport endpoints stream head.
 *
 *	Returns:
 *		0	On success or positive error code.
 *			On sucess, type is set to:
 *		T_DATA		If normal data has been received
 *		T_UDERR		If an error indication has been received,
 *				in which case uderr contains the unitdata
 *				error number.
 *		T_ERROR
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/vnode.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>


int
t_krcvudata(tiptr, unitdata, type, uderr)
	register TIUSER			*tiptr;
	register struct t_kunitdata	*unitdata;
	register int 			*type;
	register int			*uderr;

{
	register int			len;
	register int			hdrsz;
	register union T_primitives	*pptr;
	register struct file		*fp;
	mblk_t				*bp;
	register mblk_t			*nbp;
	register mblk_t			*mp;
	register mblk_t			*tmp;
	register int			error;

	fp = tiptr->fp;

	if (type == NULL || uderr == NULL)
		return EINVAL;

	error = 0;
	unitdata->udata.buf = (char *)NULL;

	if (unitdata->udata.udata_mp) {
		KTLILOG(2, "t_krcvudata: freeing existing message block\n", 0);
		freemsg(unitdata->udata.udata_mp);
		unitdata->udata.udata_mp = NULL;
	}

	if ((error = tli_recv(tiptr, &bp, fp->f_flag)) != 0)
		return error;

	/* Got something
	 */
	switch (bp->b_datap->db_type) {
	case M_PROTO:
		/* LINTED pointer alignment */
		pptr = (union T_primitives *)bp->b_rptr;
		switch (pptr->type) {
		case T_UNITDATA_IND:
			KTLILOG(2, "t_krcvudata: Got T_UNITDATA_IND\n", 0);
			hdrsz = bp->b_wptr - bp->b_rptr;

			/* check everything for consistency
			 */
			if (hdrsz < TUNITDATAINDSZ ||
		 	 	hdrsz < (pptr->unitdata_ind.OPT_length+
			 	pptr->unitdata_ind.OPT_offset) ||
			 	hdrsz < (pptr->unitdata_ind.SRC_length+
			 	pptr->unitdata_ind.SRC_offset) ) {
				error = EPROTO;
				freemsg(bp);
				break;
			}

			/* okay, so now we copy them
			 */
			len = min(pptr->unitdata_ind.SRC_length,
					unitdata->addr.maxlen);
			bcopy(bp->b_rptr+pptr->unitdata_ind.SRC_offset,
					unitdata->addr.buf, len);
			unitdata->addr.len = len;

			len = min(pptr->unitdata_ind.OPT_length,
					unitdata->opt.maxlen);
			bcopy(bp->b_rptr+pptr->unitdata_ind.OPT_offset,
					unitdata->opt.buf, len);
			unitdata->opt.len = len;

			bp->b_rptr += hdrsz;

			/* we assume that the client knows
			 * how to deal with a set of linked
			 * mblks, so all we do is make a pass
			 * and remove any that are zero length.
			 */
			nbp = NULL;
			mp = bp;
			while (mp) {
				if (!(bp->b_wptr-bp->b_rptr)){
					KTLILOG(2,
			 "t_krcvudata: zero length block\n", 0);
					tmp = mp->b_cont;
					if (nbp)
						nbp->b_cont = tmp;
					else	bp = tmp;

					freeb(mp);
					mp = tmp;
				}
				else	{
					nbp = mp;
					mp = mp->b_cont;
				}
			}
#ifdef DEBUG
{
	mblk_t *tp;

	tp = bp;
	while (tp) {
		struct datab *dmp;

		dmp = tp->b_datap;

		KTLILOG(2, "t_krcvudata: bp %x, ", tp);
		KTLILOG(2, "db_size %x, ", dmp->db_size);
		KTLILOG(2, "db_ref %x", dmp->db_ref);

		if (dmp->db_frtnp) {
			KTLILOG(2, ", func: %x", dmp->db_frtnp->free_func);
			KTLILOG(2, ", arg %x\n", dmp->db_frtnp->free_arg);
		} else
			KTLILOG(2, "\n", 0);
		tp = tp->b_cont;
	}
}
#endif

			/* now just point the users mblk
			 * pointer to what we received.
			 */
			if (bp == NULL) {
				KTLILOG(2, "t_krcvudata: No data\n", 0);
				error = EPROTO; 
				break;
			}
			if ((bp->b_wptr - bp->b_rptr) != 0) {
				if (!str_aligned(bp->b_rptr))
					if (!pullupmsg(bp, bp->b_wptr - bp->b_rptr)) {
						KTLILOG(1, 
					"t_krcvudata:  pullupmsg failed\n", 0);
						error = EIO;
						freemsg(bp);
						break;
					}
				unitdata->udata.buf = (char *)bp->b_rptr;
				unitdata->udata.len = bp->b_wptr-bp->b_rptr;

				KTLILOG(2,
 			"t_krcvudata: got %d bytes\n", unitdata->udata.len);
				unitdata->udata.udata_mp = bp;
			}
			else	{
				KTLILOG(2,
				"t_krcvudata: 0 length data message\n", 0);
				freemsg(bp);
				unitdata->udata.len = 0;
			}
			*type = T_DATA;
			break;

		case T_UDERROR_IND:
			KTLILOG(2, "t_krcvudata: Got T_UDERROR_IND\n", 0);
			hdrsz = bp->b_wptr - bp->b_rptr;

			/* check everything for consistency
			 */
			if (hdrsz < TUDERRORINDSZ ||
		 	 	hdrsz < (pptr->uderror_ind.OPT_length+
			 	pptr->uderror_ind.OPT_offset) ||
			 	hdrsz < (pptr->uderror_ind.DEST_length+
			 	pptr->uderror_ind.DEST_offset) ) {
				error = EPROTO;
				freemsg(bp);
				break;
			}

			if (pptr->uderror_ind.DEST_length >
						(int)unitdata->addr.maxlen ||
			    			pptr->uderror_ind.OPT_length >
						(int)unitdata->opt.maxlen) {
				error = EMSGSIZE;
				freemsg(bp);
				break;
			}

			/* okay, so now we copy them
			 */
			bcopy(bp->b_rptr+pptr->uderror_ind.DEST_offset,
							unitdata->addr.buf,
			(int)pptr->uderror_ind.DEST_length);
			unitdata->addr.len = pptr->uderror_ind.DEST_length;

			bcopy(bp->b_rptr+pptr->uderror_ind.OPT_offset,
							unitdata->opt.buf,
			(int)pptr->uderror_ind.OPT_length);
			unitdata->opt.len = pptr->uderror_ind.OPT_length;

			*uderr =  pptr->uderror_ind.ERROR_type;

			unitdata->udata.buf = NULL;
			unitdata->udata.udata_mp = NULL;
			unitdata->udata.len = 0;

			freemsg(bp);

			*type = T_UDERR;
			break;
		default:
			KTLILOG(1, 
		"t_krcvudata: Unknown transport primitive %d\n", pptr->type);
			error = EPROTO;
			freemsg(bp);
			break;
		}
		break;

	case M_FLUSH:
		KTLILOG(1, "t_krcvudata: tli_recv returned M_FLUSH\n", 0);
		freemsg(bp);
		*type = T_ERROR;
		break;

	default:
		KTLILOG(1, "t_krcvudata: unknown message type %x\n",
						bp->b_datap->db_type);
		freemsg(bp);
		*type = T_ERROR;
		break;
	}
	return error;
}

/******************************************************************************/
