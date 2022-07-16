/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ptem.c	1.3"
/*
 * Description: The PTEM streams module is used as a pseudo
 *		driver emulator. Its purpose is to emulate
 *		the ioctl() functions of a terminal device driver 
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/termio.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/strtty.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/jioctl.h"
#include "sys/ptem.h"
#include "sys/debug.h"

extern struct ptem ptem[];
extern int ptem_cnt;
extern nulldev();

/*
 * stream data structure definitions
 */
STATIC int ptemopen(), ptemclose(), ptemrput(), ptemwput();

STATIC struct module_info ptem_info = { 0xabcd, "ptem", 0, 512, 0, 0};

STATIC struct qinit ptemrinit = { ptemrput, NULL, ptemopen, ptemclose, nulldev, &ptem_info, NULL};

STATIC struct qinit ptemwinit = { ptemwput, NULL, ptemopen, ptemclose, nulldev, &ptem_info, NULL};

struct streamtab pteminfo = { &ptemrinit, &ptemwinit, NULL, NULL};

STATIC void ptioc();

/*
 * ptemopen - open routine gets called when the
 *	       module gets pushed onto the stream.
 */

/*ARGSUSED*/
STATIC int
ptemopen( q, dev, oflag, sflag)
register queue_t *q;	/* pointer to the read side queue */
int dev;		/* the device number(major and minor) */
int sflag;		/* Open state flag */
int oflag;		/* The user open(2) supplied flags */
{
	register struct ptem *ntp;	/* Pointer to a ptem entry for this PTEM module */
	register mblk_t *mop;		/* Pointer to a setopts message block */


	if ( sflag != MODOPEN) {
		u.u_error = EINVAL;
		return( OPENFAIL);
	}

	if ( q->q_ptr != NULL) 		/* already attached */
		return( 1);

	/*
	 * Find a free ptem entry
	 */
	for ( ntp = ptem; ntp->state&INUSE; ntp++)
		if ( ntp >= &ptem[ptem_cnt-1]) {
			cmn_err( CE_WARN, "No ptem structures\n");
			u.u_error = ENODEV;
			return( OPENFAIL);
		}

	/*
	 * allocate a message block, used to pass the "zero length message
	 * for "stty 0"
	 * NOTE: its better to find out if such a message block can be
	 *	 allocated before its needed than to not be able to
	 *	 deliver(for possible lack of buffers) when a hang-up
	 *	 occurs.
	 */
	if ( ( ntp->dack_ptr = (mblk_t *)allocb( 4, BPRI_MED)) == NULL) {
		u.u_error = EAGAIN;
		return ( OPENFAIL);
	}

	/*
	 * set up hi/lo water marks on stream head read queue
	 * and add controlling tty if not set
	 */
	if ( mop = allocb( sizeof( struct stroptions), BPRI_MED)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext( q, mop);
	} else {
		freemsg( ntp->dack_ptr);
		ntp->dack_ptr = NULL;
		u.u_error = EAGAIN;
		return ( OPENFAIL);
	}

	q->q_ptr = (caddr_t)ntp;
	WR(q)->q_ptr = (caddr_t)ntp;
	/*
	 * Assign default cflag values cf termio(7)
	 * zero out the windowing paramters
	 */
	ntp->cflags = B300|CS8|CREAD|HUPCL;

	ntp->wsz.ws_row = 0;
	ntp->wsz.ws_col = 0;
	ntp->wsz.ws_xpixel = 0;
	ntp->wsz.ws_ypixel = 0;

	ntp->state = INUSE;
	return ( 0);
}


/*
 * ptemclose - This routine gets called when the module
 *              gets popped off of the stream.
 */

STATIC int
ptemclose( q)
register queue_t *q;	/* Pointer to the read queue */
{
	register struct ptem *ntp = (struct ptem *)q->q_ptr;


	ntp->state = 0;

	q->q_ptr = (caddr_t)NULL;
	WR(q)->q_ptr = (caddr_t)NULL;
	if ( ntp->dack_ptr) {
		freemsg( ntp->dack_ptr);
		ntp->dack_ptr = NULL;
	}
	return( 0);
}


/*
 * ptemrput - Module read queue put procedure.
 *             This is called from the module or
 *	       driver downstream.
 */

STATIC int
ptemrput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to the current message block */
{
	register struct iocblk *iocp;	/* Pointer to the M_IOCTL data */
	register struct copyresp *resp;	/* Pointer to the transparent ioctl response structure */
	register mblk_t *mctlp;		/* Pointer to a M_CTL message */
	register int	sig_number;	/* an ioctl(TIOCSIGNAL) setable signal number */


	switch ( mp->b_datap->db_type) {
	case M_DELAY:
	case M_READ:
		freemsg( mp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;

		switch ( iocp->ioc_cmd) {
		case TCSBRK:
			/*
			 * Send a break message upstream
			 */
			mp->b_datap->db_type = M_IOCACK; 
			if ( !(*(int *)mp->b_cont->b_rptr)) {
				if ( !putctl( q->q_next, M_BREAK)) {
					/*
					 * Send an NAK reply back
					 */
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK; 
				}
			}
			/*
			 * Send a reply back, default is a M_IOCACK
			 */
			iocp->ioc_count = 0;
			qreply( q, mp);
			break;

		case TIOCSWINSZ:
			if ( iocp->ioc_count == TRANSPARENT) {
				register struct copyreq *get_buf_p;

				get_buf_p = ( struct copyreq *)mp->b_rptr;
				get_buf_p->cq_private = NULL;
				get_buf_p->cq_flag = 0;
				get_buf_p->cq_size = sizeof( struct winsize);
				get_buf_p->cq_addr = (caddr_t) (*(long *)(mp->b_cont->b_rptr));
				freeb( mp->b_cont);
				mp->b_cont = NULL;
				mp->b_datap->db_type = M_COPYIN;
				mp->b_wptr = mp->b_rptr + sizeof( struct copyreq);
				qreply( q, mp);
			} else
				ptioc( q, mp, RDSIDE);
			break;

		case JWINSIZE:
		case TIOCGWINSZ:
			ptioc( q, mp, RDSIDE);
			break;

		case TIOCSIGNAL:
			/*
			 * This ioctl can emanate from the master side
			 * in remote mode only
			 */
			sig_number = *(int *)mp->b_cont->b_rptr;
			if ( sig_number < 1 || sig_number > NSIG) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EINVAL;
				iocp->ioc_count = 0;
				qreply( q, mp);
				return( 0);
			}

			if ( putctl1( q->q_next, M_PCSIG, sig_number) == 0) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				iocp->ioc_count = 0;
			} else {
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
			}
			qreply( q, mp);
			break;

		case TIOCREMOTE:
			/*
			 * Send M_CTL up using the iocblk format
			 */
			if (( mctlp = allocb( sizeof( struct iocblk), BPRI_MED)) == (mblk_t *)NULL) {
				iocp->ioc_count = 0;
				iocp->ioc_error = EAGAIN;
				mp->b_datap->db_type = M_IOCNAK;
				qreply( q, mp);
				break;
			}

			mctlp->b_wptr = mctlp->b_rptr + sizeof( struct iocblk);
			mctlp->b_datap->db_type = M_CTL;
			iocp = ( struct iocblk *)mctlp->b_rptr;
			if ( *(int *)mp->b_cont->b_rptr)
				iocp->ioc_cmd = MC_NO_CANON;
			else
				iocp->ioc_cmd = MC_DO_CANON;
			/*
			 * Send the M_CTL upstream to LDTERM
			 */
			putnext( q, mctlp);

			/*
			 * Format IOCACK message and pass back to
			 * the master
			 */
			iocp = (struct iocblk *)mp->b_rptr;
			iocp->ioc_count = 0;
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;

			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		default:
			putnext( q, mp);
			break;
		}
		break;

	case M_IOCDATA:
		resp = (struct copyresp *)mp->b_rptr;
		if ( resp->cp_rval) {
			/*
			 * Just free message on failure
			 */
			freemsg( mp);
			break;
		}

		/*
		 * Only need to copy data for the SET case
		 */
		switch ( resp->cp_cmd) {

		case  TIOCSWINSZ:
			ptioc( q, mp, RDSIDE);
			break;

		case JWINSIZE:
		case  TIOCGWINSZ:
			iocp = (struct iocblk *)mp->b_rptr;
			iocp->ioc_error = 0;
			iocp->ioc_count = 0;
			iocp->ioc_rval = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		default:
			freemsg( mp);
			break;
	}	
	break;

	case M_IOCNAK:
		/*
		 * If the PCKT module is not pushed on the master
		 * side all M_IOCTLs will be NAKed by the master
		 * stream head. Free them here.
		 */
		freemsg( mp);
		break;
	
	default:
		putnext( q, mp);
		break;
	}
	return( 0);
}


/*
 * ptemwput - Module write queue put procedure.
 *             This is called from the module or
 *	       stream head upstream.
 */

STATIC int
ptemwput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to current message block */
{
	register struct iocblk *iocp;	/* pointer to the out going ioctl structure */
	register struct termio *termiop;
	register struct termios *termiosp;
	register struct ptem *ntp;
	register struct copyresp *resp;

	mblk_t *dack_ptr;	/* pointer to the disconnect message ACK block */
	mblk_t *pckt_msgp;	/* pointer to a message sent to the PCKT module */


	ntp = (struct ptem *)q->q_ptr;

	switch ( mp->b_datap->db_type) {

	case M_IOCDATA:
		resp = (struct copyresp *)mp->b_rptr;
		if ( resp->cp_rval) {
			/*
			 * Just free message on failure
			 */
			freemsg( mp);
			break;
		}

		/*
		 * Only need to copy data for the SET case
		 */
		switch ( resp->cp_cmd) {

			case  TIOCSWINSZ:
				ptioc( q, mp, WRSIDE);
				break;

			case JWINSIZE:
			case  TIOCGWINSZ:
				iocp = (struct iocblk *)mp->b_rptr;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				iocp->ioc_rval = 0;
				mp->b_datap->db_type = M_IOCACK;
				qreply( q, mp);
				break;

			default:
				freemsg( mp);
		}	
		break;

	case M_IOCTL:
		/*
		 * Note: for each "set" type operation a copy of the M_IOCTL
		 * message is made and passed downstream. Eventually the
		 * PCKT module, if it has been pushed, should pick up this
		 * message. If the PCKT module has not been pushed the master
		 * side stream head will free it
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		switch ( iocp->ioc_cmd) {

		case TCSETAF:
			/*
			 * Flush the read queue
			 */
			if ( putctl1( q->q_next, M_FLUSH, FLUSHR) == 0) {
				iocp->ioc_count = 0;
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				qreply( q, mp);
				break;
			}

			termiop = (struct termio *)mp->b_cont->b_rptr;

			ntp->cflags = (ntp->cflags & 0xffff0000 | termiop->c_cflag);

			if (( termiop->c_cflag&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero length message
				 */
				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero length message downstream
					 */
					putnext( q, dack_ptr);
				}
			} else {
				/*
				 * Need a copy of this message to pass on to
				 * the PCKT module
				 */
				if (( pckt_msgp = copymsg( mp)) == NULL) {
					iocp->ioc_count = 0;
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK;
					qreply( q, mp);
					break;
				}

				/*
				 * Send a copy of the M_IOCTL to the PCKT module
				 */
				putnext( q, pckt_msgp);
			}
			/*
			 * Send ACK upstream
			 */
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		case TCSETA:
		case TCSETAW:
			termiop = (struct termio *)mp->b_cont->b_rptr;

			ntp->cflags = (ntp->cflags & 0xffff0000 | termiop->c_cflag);

			if (( termiop->c_cflag&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero length message
				 */
				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero length message downstream
					 */
					putnext( q, dack_ptr);
				}
			} else {
				/*
				 * Need a copy of this message to pass on to
				 * the PCKT module
				 */
				if (( pckt_msgp = copymsg( mp)) == NULL) {
					iocp->ioc_count = 0;
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK;
					qreply( q, mp);
					break;
				}

				/*
				 * Send a copy of the M_IOCTL to the PCKT module
				 */
				putnext( q, pckt_msgp);
			}
			/*
			 * Send ACK upstream
			 */
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		case TCSETSF:
			/*
			 * Flush the read queue
			 */
			if ( putctl1( q->q_next, M_FLUSH, FLUSHR) == 0) {
				iocp->ioc_count = 0;
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				qreply( q, mp);
				break;
			}

			termiosp = (struct termios *)mp->b_cont->b_rptr;
			ntp->cflags = termiosp->c_cflag;

			if (( termiosp->c_cflag&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero length message
				 */
				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero length message downstream
					 */
					putnext( q, dack_ptr);
				}
			} else {
				/*
				 * Need a copy of this message to pass on to
				 * the PCKT module
				 */
				if (( pckt_msgp = copymsg( mp)) == NULL) {
					iocp->ioc_count = 0;
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK;
					qreply( q, mp);
					break;
				}

				/*
				 * Send the orginal M_IOCTL to the PCKT module
				 */
				putnext( q, pckt_msgp);
			}
			/*
			 * Send ACK upstream
			 */
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		case TCSETS:
		case TCSETSW:
			termiosp = (struct termios *)mp->b_cont->b_rptr;
			ntp->cflags = termiosp->c_cflag;

			if (( termiosp->c_cflag&CBAUD) == B0) {
				/*
				 * hang-up: Send a zero length message
				 */
				dack_ptr = ntp->dack_ptr;

				if ( dack_ptr) {
					ntp->dack_ptr = NULL;	/* use only once */
					/*
					 * Send a zero length message downstream
					 */
					putnext( q, dack_ptr);
				}
			} else {
				/*
				 * Need a copy of this message to pass on to
				 * the PCKT module
				 */
				if (( pckt_msgp = copymsg( mp)) == NULL) {
					iocp->ioc_count = 0;
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK;
					qreply( q, mp);
					break;
				}

				/*
				 * Send the orginal M_IOCTL to the PCKT module
				 */
				putnext( q, pckt_msgp);
			}
			/*
			 * Send ACK upstream
			 */
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply( q, mp);
			break;

		case TCGETA:
			if ( mp->b_cont)
				freemsg( mp->b_cont);

			if (( mp->b_cont = allocb( sizeof(struct termio), BPRI_MED)) == NULL) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				iocp->ioc_count = 0;
				qreply( q, mp);
				break;
		}
			mp->b_cont->b_wptr =  mp->b_cont->b_rptr + sizeof(struct termio);
			termiop = (struct termio *)mp->b_cont->b_rptr;
			termiop->c_cflag = (ushort)ntp->cflags;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof( struct termio);
			qreply( q, mp);
			break;

		case TCGETS:
			if ( mp->b_cont)
				freemsg( mp->b_cont);

			if (( mp->b_cont = allocb( sizeof(struct termios), BPRI_MED)) == NULL) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				iocp->ioc_count = 0;
				qreply( q, mp);
				break;
		}

			mp->b_cont->b_wptr =  mp->b_cont->b_rptr + sizeof(struct termios);
			termiosp = (struct termios *)mp->b_cont->b_rptr;
			termiosp->c_cflag = ntp->cflags;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof( struct termios);
			qreply( q, mp);
			break;

		case TCSBRK:
			/*
			 * Need a copy of this message to pass it on to
			 * the PCKT module
			 */
			if (( pckt_msgp = copymsg( mp)) == NULL) {
				iocp->ioc_count = 0;
				iocp->ioc_error = EAGAIN;
				mp->b_datap->db_type = M_IOCNAK;
				qreply( q, mp);
				break;
			}
			/*
			 * Send a copy of the M_IOCTL to the PCKT module
			 */
			putnext( q, pckt_msgp);

			/*
			 * TCSBRK meaningful if data part of message is 0
			 * cf. termio(7)
			 */
			if ( !(*(int *)mp->b_cont->b_rptr))
				putctl( q->q_next, M_BREAK);
			/*
			 * ACK the ioctl
			 */
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply( q, mp);
			break;

		case JWINSIZE:
		case TIOCGWINSZ:
			ptioc( q, mp, WRSIDE);
			break;

		case TIOCSWINSZ:
			if ( iocp->ioc_count == TRANSPARENT) {
				register struct copyreq *get_buf_p;

				get_buf_p = ( struct copyreq *)mp->b_rptr;
				get_buf_p->cq_private = NULL;
				get_buf_p->cq_flag = 0;
				get_buf_p->cq_size = sizeof( struct winsize);
				get_buf_p->cq_addr = (caddr_t) (*(long*)(mp->b_cont->b_rptr));
				freeb( mp->b_cont);
				mp->b_cont = NULL;
				mp->b_datap->db_type = M_COPYIN;
				mp->b_wptr = mp->b_rptr + sizeof( struct copyreq);
				qreply( q, mp);
			} else
				ptioc( q, mp, WRSIDE);

			break;

		case TIOCSTI: { /* Simulate typing of a character at the terminal. */
			register mblk_t *bp;

			/*
			 * The permission checking has already been done at the stream
			 * head, since it has to be done in the context of the process
			 * doing the call.
			 */
			if ((bp = allocb(1, BPRI_MED)) != NULL) {
				if (!canput(RD(q)->q_next))
					freemsg(bp);
				else {
					*bp->b_wptr++ = *mp->b_cont->b_rptr;
					putnext(RD(q), bp);
				}
			}

			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			putnext( RD(q), mp);
			break;
		}

		default:
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			qreply( q, mp);
			break;
		}
		break;
	
	case M_READ:
	case M_DELAY: /* tty delays not supported */
		freemsg( mp);
		break;

	case M_STOP:
		/*
		 * Set the output flow control state
		 */
		ntp->state |= OFLOW_CTL;
		putnext( q, mp);
		break;

	case M_START:
		/*
		 * Relieve the output flow control state
		 */
		ntp->state &= ~OFLOW_CTL;
		putnext( q, mp);
		while (( mp = getq( q)) != NULL)
			putnext( q, mp);
		break;

	case M_DATA:
		if (( mp->b_wptr - mp->b_rptr) <= 0)
			/*
			 * Free all zero length messages
			 */
			freemsg( mp);
		else if ( ntp->state & OFLOW_CTL)
			/*
			 * Queue data messages in the flow control case
			 */
			putq( q, mp);
		else
			putnext( q, mp);
		break;

	default:
		putnext( q, mp);
		break;
	}
	return( 0);
}

/*
 * Message must be of type M_IOCTL or M_IOCDATA for this routine to be called
 */
STATIC void
ptioc( q, mp, qside)
register mblk_t *mp;
register queue_t *q;
int qside;
{
	register struct ptem *tp;
	register struct iocblk *iocp;
	register struct winsize *wb;
	register struct jwinsize *jwb;
	register struct copyreq *send_buf_p;

	register mblk_t *tmp;
	register mblk_t *pckt_msgp;	/* pointer to a message sent to the PCKT module */


	iocp = (struct iocblk *)mp->b_rptr;
	tp = (struct ptem *)q->q_ptr;

	switch ( iocp->ioc_cmd) {
			
	case JWINSIZE:
		/*
		 * For compatibility: If all zeros, NAK the message for dumb terminals
		 */
		if ( ( tp->wsz.ws_row == 0) && ( tp->wsz.ws_col == 0) && 
			( tp->wsz.ws_xpixel == 0) && ( tp->wsz.ws_ypixel == 0)) {
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_count = 0;
				iocp->ioc_error = EINVAL;
				qreply( q, mp);
				return;
		}

		if (( tmp = allocb( sizeof( struct jwinsize), BPRI_MED)) == NULL) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EAGAIN;
			qreply( q, mp);
			return;
		}
		
		if ( iocp->ioc_count == TRANSPARENT) {
			send_buf_p = ( struct copyreq *)mp->b_rptr;
			send_buf_p->cq_addr = (caddr_t) (*(long *)(mp->b_cont->b_rptr));
			freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct jwinsize);
			send_buf_p->cq_private = NULL;
			send_buf_p->cq_flag = 0;
			send_buf_p->cq_size = sizeof( struct jwinsize);
			mp->b_datap->db_type = M_COPYOUT;
			mp->b_wptr =  mp->b_rptr + sizeof( struct copyreq);
		} else {
			if ( mp->b_cont)
				freemsg( mp->b_cont);
			mp->b_cont = tmp;
			mp->b_datap->db_type = M_IOCACK;
			tmp->b_wptr += sizeof( struct jwinsize);
			iocp->ioc_count = sizeof( struct jwinsize);
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
		}
		jwb = ( struct jwinsize *)mp->b_cont->b_rptr;

		jwb->bytesx = tp->wsz.ws_col;
		jwb->bytesy = tp->wsz.ws_row;
		jwb->bitsx = tp->wsz.ws_xpixel;
		jwb->bitsy = tp->wsz.ws_ypixel;

		qreply( q, mp);
		return;

	case TIOCGWINSZ:
		/*
		 * If all zeros NAK the message for dumb terminals
		 */
		if ( ( tp->wsz.ws_row == 0) && ( tp->wsz.ws_col == 0) && 
		   ( tp->wsz.ws_xpixel == 0) && ( tp->wsz.ws_ypixel == 0)) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EINVAL;
			qreply( q, mp);
			return;
		}

		if (( tmp = allocb( sizeof( struct winsize), BPRI_MED)) == NULL) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			iocp->ioc_error = EAGAIN;
			qreply( q, mp);
			return;
		}

		if ( iocp->ioc_count == TRANSPARENT) {
			send_buf_p = ( struct copyreq *)mp->b_rptr;
			send_buf_p->cq_addr = (caddr_t)(*(long *)(mp->b_cont->b_rptr));
			freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct winsize);
			send_buf_p->cq_private = NULL;
			send_buf_p->cq_flag = 0;
			send_buf_p->cq_size = sizeof( struct winsize);
			mp->b_datap->db_type = M_COPYOUT;
		} else {
			if ( mp->b_cont)
				freemsg( mp->b_cont);
			mp->b_cont = tmp;
			tmp->b_wptr += sizeof( struct winsize);
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof( struct winsize);
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
		}

		wb = (struct winsize *)mp->b_cont->b_rptr;
		wb->ws_row = tp->wsz.ws_row;
		wb->ws_col = tp->wsz.ws_col;
		wb->ws_xpixel = tp->wsz.ws_xpixel;
		wb->ws_ypixel = tp->wsz.ws_ypixel;

		qreply( q, mp);
		return;

	case TIOCSWINSZ:
		wb = (struct winsize *)mp->b_cont->b_rptr;
	        /*
		 * Send a SIGWINCH signal if the row/col information
		 * has changed.
		 */
		if ( ( tp->wsz.ws_row != wb->ws_row) || 
		     ( tp->wsz.ws_col != wb->ws_col) ||
		     ( tp->wsz.ws_xpixel != wb->ws_xpixel) ||
		     ( tp->wsz.ws_ypixel != wb->ws_xpixel)) {

		        /*
			 * SIGWINCH is always sent upstream
			 */
			if ( qside == WRSIDE)
				putctl1( RD(q)->q_next, M_SIG, SIGWINCH);
			else if( qside == RDSIDE)
				putctl1( q->q_next, M_SIG, SIGWINCH);
			/*
			 * message may have come in as an M_IOCDATA
			 * pass it to the master side as an M_IOCTL
			 */
			mp->b_datap->db_type = M_IOCTL;
			if ( qside == WRSIDE) {
				/*
				 * Need a copy of this message to pass on to
				 * the PCKT module, only if the M_IOCTL orginated
				 * from the slave side.
				 */
				if (( pckt_msgp = copymsg( mp)) == NULL) {
					iocp->ioc_count = 0;
					iocp->ioc_error = EAGAIN;
					mp->b_datap->db_type = M_IOCNAK;
					qreply( q, mp);
					return;
				}
				putnext( q, pckt_msgp);
			}
			tp->wsz.ws_row = wb->ws_row;
			tp->wsz.ws_col = wb->ws_col;
			tp->wsz.ws_xpixel = wb->ws_xpixel;
			tp->wsz.ws_ypixel = wb->ws_ypixel;
		}

		iocp->ioc_count = 0;
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;

		mp->b_datap->db_type = M_IOCACK;
		qreply( q, mp);
		return;

	default:
		putnext( q, mp);
		return;

	}
}
