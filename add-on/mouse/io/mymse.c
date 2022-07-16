/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:io/smse.c	1.3.3.1"
/*
 * Serial Mouse Module - Streams
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/kmem.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/termio.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/cmn_err.h"
#include "sys/ws/chan.h"
#include "sys/mouse.h"
#include "mse.h"
#include "sys/ddi.h"

int smsedevflag = 0;
extern int wakeup();
extern	void mse_iocnack(), mse_iocack();
extern	void mse_copyout(), mse_copyin();
extern	void mseproc();

void smseparse();

int smseopen(), smseclose(), smse_rput(), smse_wput(), smse_srvp();

struct module_info	smse_info = { 24, "smse", 0, INFPSZ, 256, 128};

static struct qinit smse_rinit = {
	smse_rput, NULL, smseopen, smseclose, NULL, &smse_info, NULL};

static struct qinit smse_winit = {
	smse_wput, NULL, NULL, NULL, NULL, &smse_info, NULL};

struct streamtab smseinfo = { &smse_rinit, &smse_winit, NULL, NULL};

/*
 * Set the baud rate of the asy port to 1200 baud.
 */

int
smse_baud(msp)
struct strmseinfo	*msp;
{
	mblk_t	*bp;
	struct iocblk *iocbp;
	struct termio	*cb;

#ifdef DEBUG1
printf("smsebaud:entered\n");
#endif
	if (!(bp = allocb(sizeof(struct iocblk), 0)))
		return;
	bp->b_datap->db_type = M_IOCTL;
	iocbp = (struct iocblk *)bp->b_rptr;
	bp->b_wptr += sizeof(struct iocblk);
	iocbp->ioc_cmd = TCSETAF;
	iocbp->ioc_count = sizeof(struct termio);
	if (!(bp->b_cont = allocb(sizeof(struct termio), 0))) {
		freemsg(bp);
		return;
	}
	cb = (struct termio *)bp->b_cont->b_rptr;
	bp->b_cont->b_wptr += sizeof(struct termio);
	cb->c_iflag = IGNBRK|IGNPAR;
	cb->c_oflag = 0;
	cb->c_cflag = B1200|CS8|CREAD|CLOCAL;
	cb->c_lflag = 0;
	cb->c_line = 0;
	cb->c_cc[VMIN] = 1;
	cb->c_cc[VTIME] = 0;
	putnext(msp->wqp, bp);

#ifdef DEBUG1
printf("smsebaud:exited\n");
#endif
}

int
smseopen(q, devp, flag, sflag, cred_p)
queue_t *q;
dev_t *devp;
register flag;
register sflag;
struct cred *cred_p;
{
	mblk_t	*bp;
	struct strmseinfo *msp;
	int i =0;

	if(q->q_ptr != NULL)
		return;
#ifdef DEBUG1
printf("smseopen:entered\n");
#endif

	/* allocate and initialize state structure */
	if ((q->q_ptr = (caddr_t) kmem_zalloc( sizeof(struct strmseinfo), KM_SLEEP)) == NULL) {
		cmn_err(CE_WARN, "SMSE: open fails, can't allocate state structure\n");
		return (ENOMEM);
	}

	msp = q->q_ptr;
	msp->rqp = q;
	msp->wqp = WR(q);
	msp->wqp->q_ptr = q->q_ptr;
	msp->old_buttons = 0x07;	/* Initialize to all buttone up */

	smse_baud(msp);
#ifdef DEBUG1
printf("smseopen:leaving\n");
#endif
	return(0);

}


smseclose(q, flag, cred_p)
queue_t *q;
register flag;
struct cred cred_p;
{
	register int oldpri;
	struct strmseinfo *msp;
	mblk_t *bp;

#ifdef DEBUG1
printf("smseclose:entered\n");
#endif
	msp = (struct strmseinfo *) q->q_ptr;

	oldpri = splstr();
	kmem_free((caddr_t) msp, sizeof(struct strmseinfo));
	q->q_ptr = (caddr_t) NULL;
	WR(q)->q_ptr = (caddr_t) NULL;
	splx(oldpri);
#ifdef DEBUG1
printf("smseclose:leaving\n");
#endif
	return;
}

smse_rput(q, mp)
queue_t *q;
mblk_t *mp;
{
#ifdef DEBUG1
printf("smse_rput:entered\n");
#endif
	switch (mp->b_datap->db_type){
		case M_DATA:
			smseparse( q, mp );
			break;
		case M_IOCACK: {
			struct iocblk *iocp;
			iocp = (struct iocblk *)mp->b_rptr;
			if (iocp->ioc_cmd == TCSETAF)
				freemsg(mp);
			else
				putnext(q,mp);
			break;
		}
		case M_FLUSH:
			if(*mp->b_rptr & FLUSHR)
				flushq(q, FLUSHDATA);
			putnext(q, mp);
			break;
		default:
			putnext(q, mp);
			break;
	}
#ifdef DEBUG1
printf("smse_rput:leaving\n");
#endif
	return;
}



smse_wput(q, mp)
queue_t *q;
mblk_t *mp;
{
	struct iocblk *iocbp;
	register struct strmseinfo *mseptr;
	register mblk_t *bp;
	register struct copyreq *cqp;
	register struct copyresp *csp;
	int oldpri;

#ifdef DEBUG1
printf("smse_wput:entered\n");
#endif
	mseptr = (struct strmseinfo *)q->q_ptr;
	iocbp = (struct iocblk *) mp->b_rptr;
	switch (mp->b_datap->db_type){
		case M_FLUSH:
#ifdef DEBUG
printf("smse_wput:M_FLUSH\n");
#endif
			if(*mp->b_rptr & FLUSHW)
				flushq(q, FLUSHDATA);
			putnext(q, mp);
			break;
		case M_IOCTL:
#ifdef DEBUG1
printf("smse_wput:M_IOCTL\n");
#endif
			switch( iocbp->ioc_cmd ){
				case MOUSEIOCREAD: {
					if((bp = allocb(sizeof(struct mouseinfo),BPRI_MED)) == NULL){ 
						mse_iocnack(q, mp, iocbp, EAGAIN, 0);
						break;
					}
					oldpri = spltty();
					bcopy(&mseptr->mseinfo,bp->b_rptr,sizeof(struct mouseinfo));
					mseptr->mseinfo.xmotion = mseptr->mseinfo.ymotion = 0;
					mseptr->mseinfo.status &= BUTSTATMASK;
					bp->b_wptr += sizeof(struct mouseinfo);
					splx(oldpri);
					if(iocbp->ioc_count == TRANSPARENT)
						mse_copyout(q, mp, bp, sizeof(struct mouseinfo), 0);
					else{
						mp->b_datap->db_type = M_IOCACK;
						iocbp->ioc_count = sizeof(struct mouseinfo);
						qreply(q, mp);
					}
					break;
				}
				default:
					mse_iocnack(q, mp, iocbp, EINVAL, 0);
					break;
			}
			break;
		case M_IOCDATA:
#ifdef DEBUG1
printf("smse_wput:M_IOCDATA\n");
#endif
			csp = (struct copyresp *)mp->b_rptr;
			if(csp->cp_cmd != MOUSEIOCREAD){
				putnext(q, mp);
				break;
			}
			if(csp->cp_rval){
				freemsg(mp);
				return;
			}
			mse_iocack(q, mp, iocbp, 0);
			break;
		default:
			putnext(q, mp);
			break;
	}
#ifdef DEBUG1
printf("smse_wput:leaving\n");
#endif
}

void
smseparse(q, mp)
queue_t *q;
mblk_t *mp;
{

	register mblk_t	*bp;
	register struct strmseinfo *mseptr;
	register unchar c;

	/* Parse the next byte of serial input.
	 * This assumes the input is in MM Series format.
	 */
	mseptr = (struct strmseinfo *)q->q_ptr;

	for(bp = mp; bp != (mblk_t *)NULL; bp = bp->b_cont){
		while(bp->b_wptr - bp->b_rptr){
			c = *bp->b_rptr++;
			switch (mseptr->state) {
			case 0:	/* First byte contains buttons */
				/* Bit seven set always means the first byte */
				if ((c & 0xf8) == 0x80) {
					mseptr->button = (~c & 0x07);
					mseptr->state = 1;
				}
				break;
			case 1:	/* Second byte is X movement */
				mseptr->x = c;
				mseptr->state = 2;
				break;
			case 2:	/* Third byte is Y movement */
				mseptr->y = -c;
				mseptr->state = 3;
				break;
			case 3:	/* Fourth byte is X movement */
				mseptr->x += c;
				mseptr->state = 4;
				break;
			case 4:	/* Fifth byte is Y movement */
				mseptr->y -= c;
				mseptr->state = 0;
				mseproc(mseptr);
				break;
			}
   		}
	}
	freemsg(mp);
}
