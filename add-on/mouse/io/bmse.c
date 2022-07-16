/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:io/bmse.c	1.3.1.1"

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
#include "sys/ddi.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/cmn_err.h"
#include "sys/ws/chan.h"
#include "sys/mouse.h"
#include "../io/mse.h"

int bmsedevflag = 0;
extern	void	mse_copyin(), mse_copyout();
extern	void	mse_iocnack(), mse_iocack();
extern	void	mseproc();
extern struct mouseconfig	mse_config;
extern int	mse_nbus;


static struct strmseinfo *bmseptr = 0;

static unsigned	BASE_IOA;	/* Set to base I/O addr of bus mouse */

int bmseopen(), bmseclose(), bmse_rsrv(), bmseintr();
int bmse_wput();

struct module_info	bmse_info = { 23, "bmse", 0, INFPSZ, 256, 128};

static struct qinit bmse_rinit = {
	NULL, NULL, bmseopen, bmseclose, NULL, &bmse_info, NULL};

static struct qinit bmse_winit = {
	bmse_wput, NULL, NULL, NULL, NULL, &bmse_info, NULL};

struct streamtab bmseinfo = { &bmse_rinit, &bmse_winit, NULL, NULL};

char	bmseclosing = 0;

void
bmseinit()
{
	register int	i;

	mse_config.present = 0;
	if(mse_nbus){
		BASE_IOA = mse_config.io_addr;
		/* Check if the mouse board exists */
		outb (CONFIGURATOR_PORT, 0x91);
		tenmicrosec();
		outb (SIGNATURE_PORT, 0xC);
		tenmicrosec();
		i = inb (SIGNATURE_PORT);
		tenmicrosec();
		outb (SIGNATURE_PORT, 0x50);
		tenmicrosec();
		if (i == 0xC && inb (SIGNATURE_PORT) == 0x50) {
			mse_config.present = 1;
			control_port(INTR_DISABLE);	/* Disable interrupts */
		}
	}
}

bmseopen(q, devp, flag, sflag, cred_p)
queue_t *q;
dev_t *devp;
register flag;
register sflag;
struct cred *cred_p;
{
	register int oldpri;
#ifdef DEBUG1
printf("bmseopen:entered\n");
#endif

	if(!mse_config.present)
		return(EIO);
	if (q->q_ptr != NULL){
#ifdef DEBUG1
printf("bmseopen:already open\n");
#endif
		return (0);		/* already attached */
	}
	oldpri = splstr();
	while(bmseclosing)
		sleep(&bmse_info, PZERO + 1);
	splx(oldpri);


	/* allocate and initialize state structure */
	if ((bmseptr = (struct strmseinfo *) kmem_zalloc( sizeof(struct strmseinfo), KM_SLEEP)) == NULL) {
		cmn_err(CE_WARN, "BMSE: open fails, can't allocate state structure");
		return (ENOMEM);
	}
	q->q_ptr = (caddr_t) bmseptr;
	WR(q)->q_ptr = (caddr_t) bmseptr;
	bmseptr->rqp = q;
	bmseptr->wqp = WR(q);
	control_port(0);	/* Enable interrupts */

#ifdef DEBUG1
printf("bmseopen:leaving\n");
#endif
	return(0);
}


bmseclose(q, flag, cred_p)
queue_t *q;
register flag;
struct cred cred_p;
{
	register int oldpri;

#ifdef DEBUG
printf("bmseclose:entered\n");
#endif
	control_port(INTR_DISABLE);	/* Disable interrupts */
	oldpri = splstr();
	bmseclosing = 1;
	q->q_ptr = (caddr_t) NULL;
	WR(q)->q_ptr = (caddr_t) NULL;
	kmem_free(bmseptr, sizeof(struct strmseinfo));
	bmseptr = (struct strmseinfo *) NULL;
	bmseclosing = 0;
	wakeup(&bmse_info);
	splx(oldpri);
#ifdef DEBUG
printf("bmseclose:leaving\n");
#endif
	return;
}

bmse_wput(q, mp)
queue_t *q;
mblk_t *mp;
{
	register struct iocblk *iocbp;
	register mblk_t *bp;
	register struct copyreq *cqp;
	register struct copyresp *csp;
	register int oldpri;

#ifdef DEBUG
printf("bmse_wput:entered\n");
#endif
	if(bmseptr == 0){
		freemsg(mp);
#ifdef DEBUG
printf("bmse_wput:bmseptr == NULL\n");
#endif
		return;
	}
	iocbp = (struct iocblk *) mp->b_rptr;
	switch (mp->b_datap->db_type){
		case M_FLUSH:
#ifdef DEBUG
printf("bmse_wput:M_FLUSH\n");
#endif
			if(*mp->b_rptr & FLUSHW)
				flushq(q, FLUSHDATA);
			qreply(q, mp);
			break;
		case M_IOCTL:
#ifdef DEBUG
printf("bmse_wput:M_IOCTL\n");
#endif
			switch( iocbp->ioc_cmd ){
				case MOUSEIOCREAD: {
#ifdef DEBUG
printf("bmse_wput:M_IOCTL-MOUSEIOCREAD\n");
#endif

					if((bp = allocb(sizeof(struct mouseinfo),BPRI_MED)) == NULL){ 
						mse_iocnack(q, mp, iocbp, EAGAIN, 0);
						break;
					}
					oldpri = splstr();
					bcopy(&bmseptr->mseinfo,bp->b_rptr,sizeof(struct mouseinfo));
					bmseptr->mseinfo.xmotion = bmseptr->mseinfo.ymotion = 0;
					bmseptr->mseinfo.status &= BUTSTATMASK;
					splx(oldpri);
					bp->b_wptr += sizeof(struct mouseinfo);
					if(iocbp->ioc_count == TRANSPARENT)
						mse_copyout(q, mp, bp, sizeof(struct mouseinfo), 0);
					else{
#ifdef DEBUG
printf("bmse_wput:M_IOCTL- not transparent\n");
#endif
						mp->b_datap->db_type = M_IOCACK;
						iocbp->ioc_count = sizeof(struct mouseinfo);
						if (mp->b_cont) freemsg(mp->b_cont);
						mp->b_cont = bp;
						qreply(q, mp);
					}
					break;
				}
				default:
#ifdef DEBUG
printf("bmse_wput:M_IOCTL-DEFAULT\n");
#endif
					mse_iocnack(q, mp, iocbp, EINVAL, 0);
					break;
			}
			break;
		case M_IOCDATA:
#ifdef DEBUG
printf("bmse_wput:M_IOCDATA\n");
#endif
			csp = (struct copyresp *)mp->b_rptr;
			if(csp->cp_cmd != MOUSEIOCREAD){
#ifdef DEBUG
printf("bmse_wput:M_IOCDATA - NACKing\n");
#endif
				mse_iocnack(q, mp, iocbp, EINVAL, 0);
				break;
			}
			if(csp->cp_rval){
#ifdef DEBUG
printf("bmse_wput:M_IOCDATA - freemsging\n");
#endif
				freemsg(mp);
				break;
			}
#ifdef DEBUG
printf("bmse_wput:M_IOCDATA - ACKing\n");
#endif
			mse_iocack(q, mp, iocbp, 0);
			break;
		default:
			freemsg(mp);
			break;
	}
#ifdef DEBUG
printf("bmse_wput:leaving\n");
#endif
}

bmseintr(vect)
unsigned vect;
{
	register unsigned BASE_IOA;
	register unchar d;
	register char mdata;
	register int	unit;

			
	if (mse_config.ivect != vect) {
		cmn_err(CE_WARN,
"Mouse interrupt on un-configured vector: %d", vect);
		return;
	}

	if (!bmseptr) {
#ifdef DEBUG
		cmn_err(CE_NOTE,"received interrupt before opened");
#endif
		control_port(0); /* reenable interrupts */
		return;
	}
	BASE_IOA = mse_config.io_addr;

/* Get the mouse's status and put it into the appropriate virtual structure */
	control_port(INTR_DISABLE | HC | HIGH_NIBBLE | X_COUNTER);
	bmseptr->x = (data_port & 0x0f) << 4;
	control_port(INTR_DISABLE | HC | LOW_NIBBLE | X_COUNTER);
	bmseptr->x |= (data_port & 0x0f);
	control_port(INTR_DISABLE | HC | HIGH_NIBBLE | Y_COUNTER);
	bmseptr->y = (data_port & 0x0f) << 4;
	control_port(INTR_DISABLE | HC | LOW_NIBBLE | Y_COUNTER);
	bmseptr->y |= ((d = data_port) & 0x0f);
	bmseptr->button = (d >> 5) & 0x07;

	mseproc(bmseptr);

/* Re-enable interrupts on the mouse and return */
	control_port(0);
}

