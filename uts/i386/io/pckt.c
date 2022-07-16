/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:pckt.c	1.3"
/*
 * Description: The PCKT module packitizes meesages on
 *		its read queue by pre-fixing an M_PROTO
 *		message type to certain incomming messages.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"

extern nulldev();

/*
 * stream data structure definitions
 */
STATIC int pcktopen(), pcktclose(), pcktrput(), pcktwput();

STATIC struct module_info pckt_info = { 0x9898, "pckt", 0, 512, 0, 0};

STATIC struct qinit pcktrinit = { pcktrput, NULL, pcktopen, pcktclose, nulldev, &pckt_info, NULL};

STATIC struct qinit pcktwinit = { pcktwput, NULL, NULL, NULL, nulldev, &pckt_info, NULL};

struct streamtab pcktinfo = { &pcktrinit, &pcktwinit, NULL, NULL };

STATIC mblk_t *add_ctl_info();

/*
 * pcktopen - open routine gets called when the
 *	       module gets pushed onto the stream.
 */
/*ARGSUSED*/
STATIC int
pcktopen( q, dev, oflag, sflag)
register queue_t *q;	/* pointer to the read side queue */
int dev;		/* the device number(major and minor) */
int sflag;		/* Open state flag */
int oflag;		/* The user open(2) supplied flags */
{
	register mblk_t *mop;		/* Pointer to a setopts message block */


	if ( sflag != MODOPEN) {
		u.u_error = EINVAL;
		return( OPENFAIL);
	}

	/*
	 * set up hi/lo water marks on stream head read queue
	 */
	if ( mop = allocb( sizeof( struct stroptions), BPRI_MED)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext( q, mop);
	} else {
		u.u_error = EAGAIN;
		return ( OPENFAIL);
	}

	return ( 0);
}


/*
 * pcktclose - This routine gets called when the module
 *              gets popped off of the stream.
 */

/*ARGSUSED*/
STATIC int
pcktclose( q)
register queue_t *q;	/* Pointer to the read queue */
{
	return( 0);
}

/*
 * pcktrput - Module read queue put procedure.
 *             This is called from the module or
 *	       driver downstream.
 */
STATIC int
pcktrput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to the current message block */
{
	register mblk_t *pckt_msgp;


	switch( mp->b_datap->db_type) {
	/*
	 * Some messages are prefixed with an M_PROTO
	 */
	case M_FLUSH:
		/*
		 * The PTS driver swaps the FLUSHR and FLUSHW flags
		 * we need to swap them back to reflect the actual
		 * slave side FLUSH mode
		 */
		if ( ( *mp->b_rptr & FLUSHRW) != FLUSHRW)
			if ( ( *mp->b_rptr & FLUSHRW) == FLUSHR)
				*mp->b_rptr = FLUSHW;
			else if ( ( *mp->b_rptr & FLUSHRW) == FLUSHW)
				*mp->b_rptr = FLUSHR;

		pckt_msgp = copymsg( mp);
		if ( *mp->b_rptr & FLUSHW) {
			/*
			 * In the packet model we are not allowing
			 * flushes of the master's stream head read
			 * side queue. This is because all packet
			 * state information is stored there and
			 * a flush could destroy this data before
			 * it is read
			 */
			*mp->b_rptr = FLUSHW;
			putnext( q, mp);
		} else
			/*
			 * Free messages that only flush the
			 * master's read queue
			 */
			freemsg( mp);

		if ( pckt_msgp != NULL) {
			/*
			 * Prefix an M_PROTO header to message and pass upstream
			 */
			pckt_msgp = add_ctl_info( pckt_msgp);
			putnext( q, pckt_msgp);
		}
		break;

	case M_PROTO:
	case M_PCPROTO:
	case M_STOP:
	case M_START:
	case M_IOCTL:
	case M_DATA:
	case M_READ:
	case M_STARTI:
	case M_STOPI:
		/*
		 * Prefix an M_PROTO header to message and pass upstream
		 */
		mp = add_ctl_info( mp);
		putnext( q, mp);
		break;

	default:
		putnext( q, mp);
		break;
	}
	return( 0);
}
/*
 * pcktwput - Module write queue put procedure.
 *             All messages are send downstream unchanged
 */

STATIC int
pcktwput( q, mp)
register queue_t *q;	/* Pointer to the read queue */
register mblk_t *mp;	/* Pointer to current message block */
{
	putnext( q, mp);
	return( 0);
}
/*
 * Description: add message control information to in coming
 * message
 */
STATIC mblk_t *
add_ctl_info( mp)
mblk_t	*mp;	/* pointer to the raw data input message */
{
	register mblk_t	*bp;	/* pointer to the unmodified message block */


	/*
	 * Need to add the message block header as
	 * an M_PROTO type message
	 */
	if (( bp = allocb( sizeof( char), BPRI_MED)) == (mblk_t *)NULL) {
		(void)bufcall( sizeof( char), BPRI_MED, (int (*)())add_ctl_info, (long)mp);
		return( NULL);
	}

	/*
	 * Copy the message type information to this message
	 */
	bp->b_datap->db_type = M_PROTO;
	*(unsigned char *)bp->b_rptr = mp->b_datap->db_type;
	bp->b_wptr++;

	/*
	 * Now change the orginal message type to M_DATA
	 */
	mp->b_datap->db_type = M_DATA;

	/* 
	 * Tie the messages together
	 */
	bp->b_cont = mp;

	return( bp);
}
