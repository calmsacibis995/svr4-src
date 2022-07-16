/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kconnect.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kconnect.c 1.2 89/01/11 SMI"
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
 *	Kernel TLI-like function to allow a trasnport endpoint
 *	to initiate a connection to another transport endpoint.
 *	This function will wait for an ack and a T_CONN_CON
 *	before returning.
 *
 *	Returns:
 *		0 on success, and if rcvcall is non-NULL it shall be
 *		filled with the connection confirm data.
 *		Otherwise a positive error code.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>


int
t_kconnect(tiptr, sndcall, rcvcall)
	register TIUSER			*tiptr;
	register struct	t_call		*sndcall;
	register struct	t_call		*rcvcall;

{
	register int			len;
	register int			msgsz;
	register int			hdrsz;
	register struct T_conn_req	*creq;
	register union T_primitives	*pptr;
	register mblk_t			*nbp;
	register struct file		*fp;
	mblk_t				*bp;
	int				error;

	error = 0;

	fp = tiptr->fp;
	msgsz = TCONNREQSZ;
	while (!(bp = allocb(msgsz, BPRI_LO))) {
		if (strwaitbuf(msgsz, BPRI_LO)) {
			return ENOSR;
		}
	}

	/* LINTED pointer alignment */
	creq = (struct T_conn_req *)bp->b_wptr;
	creq->PRIM_type = T_CONN_REQ;
	creq->DEST_length = sndcall->addr.len;
	creq->OPT_length = sndcall->opt.len;
	if (sndcall->addr.len) {
		bcopy(sndcall->addr.buf, (char *)(bp->b_wptr+msgsz), sndcall->addr.len);
		creq->DEST_offset = msgsz;
		msgsz += sndcall->addr.len;
	}
	else	creq->DEST_offset = 0;
	if (sndcall->opt.len) {
		bcopy(sndcall->opt.buf, (char *)(bp->b_wptr+msgsz), sndcall->opt.len);
		creq->OPT_offset = msgsz;
		msgsz += sndcall->opt.len;
	}
	else	creq->OPT_offset = 0;

	bp->b_datap->db_type = M_PROTO;
	bp->b_wptr += msgsz;

	/* copy the users data, if any.
	 */
	if (sndcall->udata.len) {
		/* if CO then we would allocate a data block and
 		 * put the users connect data into it.
		 */
		KTLILOG(1, "Attempt to send connectionless data on T_CONN_REQ\n", 0);
		return EPROTO;
	}

	/* send it
	 */
	if ((error = tli_send(tiptr, bp, fp->f_flag)) != 0)
		return error;

	/* wait for acknowledgment
	 */
	if ((error = get_ok_ack(tiptr, T_CONN_REQ, fp->f_flag)) != 0)
		return error;

	/* wait for CONfirm
	 */
	if ((error = tli_recv(tiptr, &bp, fp->f_flag)) != 0)
		return error;

	if (bp->b_datap->db_type != M_PROTO)
		return EPROTO;

	/* LINTED pointer alignment */
	pptr = (union T_primitives *)bp->b_rptr;
	switch (pptr->type) {
		case T_CONN_CON:
			hdrsz = bp->b_wptr - bp->b_rptr;

			/* check everything for consistency
			 */
			if (hdrsz < TCONNCONSZ ||
		 	 hdrsz < (pptr->conn_con.OPT_length+
			 pptr->conn_con.OPT_offset) ||
			 hdrsz < (pptr->conn_con.RES_length+
			 pptr->conn_con.RES_offset) ) {
				error = EPROTO;
				break;
			}

			if (rcvcall != NULL) {
				/* okay, so now we copy them
				 */
				len = min(pptr->conn_con.RES_length,
					  rcvcall->addr.maxlen);
				bcopy(bp->b_rptr+pptr->conn_con.RES_offset,
						rcvcall->addr.buf, len);
				rcvcall->addr.len = len;
	
				len = min(pptr->conn_con.OPT_length,
					  rcvcall->opt.maxlen);
				bcopy(bp->b_rptr+pptr->conn_con.OPT_offset,
						rcvcall->opt.buf, len);
				rcvcall->opt.len = len;
	
				if (bp->b_cont) {
					nbp = bp;
					bp = bp->b_cont;
					msgsz = bp->b_wptr - bp->b_rptr;
					len = min(msgsz, rcvcall->udata.maxlen);
					bcopy(bp->b_rptr, 
						rcvcall->udata.buf, len);
					rcvcall->udata.len = len;
					freemsg(nbp);
				}
			}
			else	freemsg(bp);
			break;

		default:
			error = EPROTO;
			break;
	}
	return error;
}
/******************************************************************************/

