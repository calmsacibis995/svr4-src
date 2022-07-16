/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kutil.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kutil.c 1.2 89/01/11 SMI"
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
 *	Contains the following utility functions:
 *		t_stropen:
 *		tli_send:
 *		tli_recv:
 *		get_ok_ack:
 *
 *	Returns:
 *		See individual functions.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/strsubr.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>

extern char qrunflag;	/* should be in stream.h */

int
tli_send(tiptr, bp, fmode)
	register TIUSER		*tiptr;
	register mblk_t	 	*bp;
	register int		fmode;

{
	int	 		retval;
	register int		s;
	register struct file	*fp;
	register struct vnode	*vp;
	register struct stdata	*stp;
	int			error;

	retval = 0;
	error = 0;
	fp = tiptr->fp;
	vp = fp->f_vnode;
	stp = vp->v_stream;

	s = splstr();
	while (!canput(stp->sd_wrq->q_next)) {
		if ((error = strwaitq(stp, WRITEWAIT, (off_t)0, fmode, 
						&retval)) || retval) {
			return error;
		}
	}
	(void)splx(s);

	putnext(stp->sd_wrq, bp);

	return 0;
}

int 
tli_recv(tiptr, bp, fmode)
	register TIUSER	 	*tiptr;
	register mblk_t	 	**bp;
	register int		fmode;

{
	int	 		retval;
	register int		s;
	register struct file	*fp;
	register struct vnode	*vp;
	register struct stdata	*stp;
	int			error;

	retval = 0;
	error = 0;
	fp = tiptr->fp;
	vp = fp->f_vnode;
	stp = vp->v_stream;

	s = splstr();
	if (stp->sd_flag & (STRDERR|STPLEX)) {
		error = (stp->sd_flag&STPLEX) ? EINVAL : stp->sd_rerror;
		(void)splx(s);
		return error;
	}

	while ( !(*bp = getq(RD(stp->sd_wrq)))) {
		if ((error = strwaitq(stp, READWAIT, (off_t)0, fmode,
						 &retval)) || retval) {
			(void)splx(s);
			return error;
		}
	}
	if (stp->sd_flag)
		stp->sd_flag &= ~STRPRI;
	(void)splx(s);

	return 0;
}

int
get_ok_ack(tiptr, type, fmode)
	register TIUSER			*tiptr;
	register int 			type;
	register int 			fmode;

{
	register int			msgsz;
	register union T_primitives	*pptr;
	mblk_t				*bp;
	int				error;

	error = 0;

	/* wait for ack
	 */
	if ((error = tli_recv(tiptr, &bp, fmode)) != 0)
		return error;

	if ((msgsz = (bp->b_wptr - bp->b_rptr)) < sizeof(long))
		return EPROTO;

	/* LINTED pointer alignment */
	pptr = (union T_primitives *)bp->b_rptr;
	switch(pptr->type) {
		case T_OK_ACK:
			if (msgsz < TOKACKSZ ||
					pptr->ok_ack.CORRECT_prim != type) 
				error = EPROTO;
			break;

		case T_ERROR_ACK:
			if (msgsz < TERRORACKSZ) {
				error = EPROTO;
				break;
			}

                        if (pptr->error_ack.TLI_error == TSYSERR)
                                error = pptr->error_ack.UNIX_error;
                        else    error = t_tlitosyserr(pptr->error_ack.TLI_error);

			break;

		default:
			error = EPROTO;
			break;
	}
	return error;
}

/*
 * Translate a TLI error into a system error as best we can.
 */
static ushort tli_errs[] = {
	      0,		/* no error	 */
	      EADDRNOTAVAIL,    /* TBADADDR      */
	      ENOPROTOOPT,      /* TBADOPT       */
	      EACCES,		/* TACCES	 */
	      EBADF,		/* TBADF	 */
	      EADDRNOTAVAIL,	/* TNOADDR       */
	      EPROTO,		/* TOUTSTATE     */
	      EPROTO,		/* TBADSEQ       */
	      0,		/* TSYSERR - will never get */
	      EPROTO,		/* TLOOK - should never be sent by transport */
	      EMSGSIZE,		/* TBADDATA      */
	      EMSGSIZE,		/* TBUFOVFLW     */
	      EPROTO,		/* TFLOW	 */
	      EWOULDBLOCK,      /* TNODATA       */
	      EPROTO,		/* TNODIS	 */
	      EPROTO,		/* TNOUDERR      */
	      EINVAL,		/* TBADFLAG      */
	      EPROTO,		/* TNOREL	 */
	      EOPNOTSUPP,       /* TNOTSUPPORT   */
	      EPROTO,		/* TSTATECHNG    */
};
 
int
t_tlitosyserr(terr)
	register int	terr;
{
	if (terr > (sizeof(tli_errs) / sizeof(ushort)))
		return EPROTO;
	else	return tli_errs[terr];
}

#ifdef DEBUG
int ktlilog = 0;

/*
 * Kernel level debugging aid. The global variable "ktlilog" is a bit
 * mask which allows various types of debugging messages to be printed
 * out.
 * 
 *	ktlilog & 1 	will cause actual failures to be printed.
 *	ktlilog & 2	will cause informational messages to be
 *			printed.
 */
int
ktli_log(level, str, a1)
	register int	level;
	register char	*str;
	register int	a1;

{
        if (level & ktlilog)
                printf(str, a1);
}
#endif

