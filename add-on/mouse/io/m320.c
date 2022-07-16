/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:io/m320.c	1.2.2.2"

/*
 * AT&T 320 Mouse Driver - Streams Version
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
#include "sys/cmn_err.h"
#include "sys/ws/chan.h"
#include "sys/ws/8042.h"
#include <sys/mouse.h>
#include "mse.h"

/* MSE_ANY used in 320 mouse command processing for case where we don't
 * care what return byte val is
 */

#define MSE_ANY		0xFE

#define	M_IN_DATA	0
#define	M_OUT_DATA	1
#define	SNDERR2		0xfc

int mse3open(), mse3close();
int mse3devflag = 0;
void mse3intr(), mse3ioc();
void ps2parse();
int mse3_wput();
unchar  rdmse3();

extern	void	mse_copyin(), mse_copyout();
extern	void	mse_iocnack(), mse_iocack();
extern	void	mseproc();

extern	int i8042_has_aux_port;

static struct	mcastat mcastat;

#define SEND8042(port, byte) { \
	int waitcnt = 200000; \
	while ((inb(MSE_STAT) & MSE_INBF) != 0 && waitcnt-- != 0) \
		; \
	outb(port, byte); \
	waitcnt = 200000; \
	while ((inb(MSE_STAT) & 0x02) != 0 && waitcnt-- != 0) \
		; \
}


struct module_info	mse3_info = { 22, "mse3", 0, INFPSZ, 256, 128};

static struct qinit mse3_winit = {
	 mse3_wput, NULL, NULL, NULL, NULL, &mse3_info, NULL};

static struct qinit mse3_rinit = {
	NULL, NULL, mse3open, mse3close, NULL, &mse3_info, NULL};


struct streamtab mse3info = { &mse3_rinit, &mse3_winit, NULL, NULL};


static struct strmseinfo *mse3ptr;

int	mse3closing = 0;
int	ps2_cont = 0;

void
mse3start()
{
	int	waitcnt = 200000;
	char tmp,ps2_cont;

	mcastat.present = 0;
	if( is_mca()){
#ifdef DEBUG
printf("msestart: thinks there's a 320\n");
#endif
		mcastat.present = 1;
		mcastat.mode = MSESTREAM;

		i8042_acquire(); /* obtain ownership of 8042 and disable
				 * interfaces
				 */

		/* the following only sets the software status
		 * variable for the 8042. If the aux interface was enabled on
		 * the 8042 prior to the call to i8042_acquire, it won't be
		 * disabled until the call to i8042_release()
		 */
		i8042_program(P8042_AUXDISAB); /* we'll enable in open */

		/* turn on interrupts for mouse */
		SEND8042(MSE_ICMD, MSE_RCB);
		while ((0x01 & inb(MSE_STAT)) == 0 && waitcnt-- != 0)
			;
		if (waitcnt == 0) {
			mcastat.present = 0;
			/* if this did not work, no support for 320 mouse */
			cmn_err(CE_WARN,"Unable to initialize auxiliary keyboard port.");
			i8042_release();
			return;
		}

		ps2_cont = inb(MSE_OUT); /* get cmd byte to change
					  * aux interrupt status
					  */

		SEND8042(MSE_ICMD, MSE_WCB);
		SEND8042(MSE_IDAT, (ps2_cont | 0x02)); /* turn interrupts on */
		i8042_release(); /* release ownership; aux should remain disabled */
	}
}


mse3open(q, devp, flag, sflag, cred_p)
queue_t *q;
dev_t *devp;
register flag;
register sflag;
struct cred *cred_p;
{
	register int oldpri;
	register int waitcnt = 200000;
	struct	cmd_320	mca;

	if(q->q_ptr != NULL)
		return(0);
	if (mcastat.present != 1)
		return(ENXIO);
	oldpri = splstr();
	while(mse3closing)
		sleep(&mse3info, PZERO + 1);
	i8042_program(P8042_AUXDISAB);
	mca.cmd = MSEON;		/* turn on 320 mouse */
	if(mcafunc(&mca) == FAILED){
		splx(oldpri);
		return(ENXIO);
	}
	mca.cmd = MSESETRES;
	mca.arg1 = 0x03;	
	mcafunc(&mca);

	mca.cmd = MSECHGMOD;
	mca.arg1 = 0x28;	
	mcafunc(&mca);

	splx(oldpri);
	/* allocate and initialize state structure */
	if ((mse3ptr = (struct strmseinfo *) kmem_zalloc( sizeof(struct strmseinfo), KM_SLEEP)) == NULL) {
		cmn_err(CE_WARN, "MSE320: open fails, can't allocate state structure");
		return (ENOMEM);
	}
	q->q_ptr = (caddr_t) mse3ptr;
	WR(q)->q_ptr = (caddr_t) mse3ptr;
	mse3ptr->rqp = q;
	mse3ptr->wqp = WR(q);
	mse3ptr->old_buttons = 0x07;	/* Initialize to all buttons up */

	/* enable the aux interface on the 8042 */
	i8042_acquire();
	i8042_program(P8042_AUXENAB);
	i8042_release();

	return(0);
}

mse3close(q, flag, cred_p)
queue_t *q;
register flag;
struct cred cred_p;
{
	register int oldpri;
	struct	cmd_320	mca;
	struct strmseinfo *tmpmse3ptr;

	oldpri = splstr();
	mse3closing = 1;
	mcastat.map_unit = -1;
	/* leave aux interface disabled after mcafunc() runs */
	i8042_program(P8042_AUXDISAB);
	mca.cmd = MSEOFF;
	mcafunc(&mca);
	q->q_ptr = (caddr_t) NULL;
	WR(q)->q_ptr = (caddr_t) NULL;
	mse3closing = 0;
	wakeup(&mse3info);
	tmpmse3ptr = mse3ptr;
	mse3ptr = (struct strmseinfo *) NULL;
	splx(oldpri);
	kmem_free(mse3ptr,sizeof(struct strmseinfo));
	return;
}

int
mse3_wput(q, mp)
queue_t *q;
mblk_t *mp;
{
	register mblk_t  *bp;
	register struct iocblk *iocbp;
	register int oldpri, tmp;

	switch (mp->b_datap->db_type){
		case M_IOCTL:
			iocbp = (struct iocblk *) mp->b_rptr;
			if(iocbp->ioc_count != TRANSPARENT){
				if(mp->b_cont){
					freemsg(mp->b_cont);
					mp->b_cont = NULL;
				}
				mse_iocnack(q, mp, iocbp, EINVAL, 0);
				return;
			}

			switch( iocbp->ioc_cmd ){
				case MOUSE320:{
					mse_copyin(q, mp, sizeof(struct cmd_320), M_IN_DATA);
					break;
				}
				default:
					mse_iocnack(q, mp, iocbp, EINVAL, 0);
					break;
			}
			break;
		case M_IOCDATA:
			mse3ioc(q, mp);
			break;
		default:
			putnext(q, mp);
			break;
	}
}



void
mse3ioc(q, mp)
queue_t *q;
mblk_t *mp;
{
	struct	iocblk *iocbp;
	struct	copyresp	*csp;
	struct cmd_320	*cmd320;
	struct msecopy	*stp;
	mblk_t	*bp;	
	int retval;

	csp = (struct copyresp *)mp->b_rptr;
	iocbp = (struct iocblk *)mp->b_rptr;

	if(csp->cp_rval){
		freemsg(mp);
		return;
	}
	switch(csp->cp_cmd){
		case MOUSE320:
			stp = (struct msecopy *)csp->cp_private;
			switch(stp->state){
				case M_IN_DATA:
					cmd320 = (struct cmd_320 *)mp->b_cont->b_rptr;
					if((retval = mcafunc(cmd320)) == -1){
						mse_iocnack(q, mp, iocbp, EINVAL, 0);
						break;
					}
					if(retval){
						bp = copyb(mp->b_cont);
						mse3ptr->copystate.state = M_OUT_DATA;
						mse_copyout(q, mp, bp, sizeof(struct cmd_320), 0);
					}else{
						mse_iocack(q, mp, iocbp, 0);
					}
					break;
				case M_OUT_DATA:
					mse_iocack(q, mp, iocbp, 0);
					break;
				default:
					mse_iocnack(q, mp, iocbp, EINVAL, 0);
					break;
			}
			break;
		default:
			freemsg(mp);
			break;
	}
}


void
mse3intr(vect)
unsigned vect;
{
	register char mdata;
	register int	unit;

			
	if(vect == 12){		/* vector 12 is AT&T 320 mouse */
		if (!mcastat.present)
			return;
		if (!mse3ptr)
			return;
		if(mcastat.mode == MSESTREAM ){
			if((mdata = rdmse3()) == FAILED) 
				return;
			ps2parse(mdata);
		}
		return;
	}
}

void
ps2parse(c)
register char	c;
{
	register char	tmp;
	register struct strmseinfo	*m = mse3ptr;

	/* Parse the next byte of input.  */

	switch (m->state)
	{

	case 0:	/*
		** Interpretation of the bits in byte 1:
		**
		**	Yo Xo Ys Xs 1 M R L
		**
		**	L:  left button state (1 == down)
		**	R:  right button state
		**	M:  middle button state
		**	1:  always 1, never used by mouse driver
		**	Xs: X delta (byte 2) sign bit (1 == Negative)
		**	Ys: Y delta (byte 3) sign bit
		**	Xo: X overflow bit, never used by mouse driver
		**	Yo: Y overflow bit, never used by mouse driver
		*/
		
		/*
		** Shift the buttons bits into the order required: LMR
		*/

		tmp = (c & 0x01)<<2;
		tmp |= (c & 0x06)>>1;	
		m->button = (tmp ^ 0x07);	/* Buttons */

		m->x = (c & 0x10);		/* X sign bit */
		m->y = (c & 0x20);		/* Y sign bit */
		m->state = 1;
		break;

	case 1:	/*
		** Second byte is X movement as a delta
		**
		**	This byte should be interpreted as a 9-bit
		**	2's complement number where the Most Significant
		**	Bit (i.e. the 9th bit) is the Xs bit from byte 1.
		**
		**	But since we store the value as a 2's complement
		**	8-bit number (i.e. signed char) we need to
		**	truncate to 127 or -128 as needed.
		*/

		if ( m->x )			/* Negative delta */
		{
			/*
			** The following blocks of code are of the form
			**
			**	statement1;
			**	if ( condition )
			**		statement2;
			**
			** rather than
			**
			**	if ( condition )
			**		statement2;
			**	else
			**		statement1;
			**
			** because it generates more efficent assembly code.
			*/

			m->x = -128;		/* Set to signed char min */

			if ( c & 0x80 )		/* NO truncate    */
				m->x = c;
		}
		else				/* Positive delta */
		{
			m->x = 127;		/* Set to signed char max */

			if ( !(c & 0x80 ))	/* Truncate       */
				m->x = c;
		}

		m->state = 2;
		break;

	case 2:	/*
		** Third byte is Y movement as a delta
		**
		**	See description of byte 2 above for how to
		**	interpret byte 3.
		**
		**	The driver assumes position (0,0) to be in the
		**	upper left hand corner, BUT the PS/2 mouse
		**	assumes (0,0) is in the lower left hand corner
		**	so the truncated delted also needs to be
		**	negated for the Y movement.
		**
		** The logic is a little contorted, however if you dig
		** through it, it should be correct.  Remember the part
		** about the 9-bit 2's complement number system
		** mentioned above.
		**
		** For complete details see "Logitech Technical
		** Reference & Programming Guide."
		*/
	
		if ( m->y )	/* Negative delta treated as Positive */
		{
			m->y = 127;		/* Set to signed char max */

			if ( (unsigned char)c > 128 )	/* Just negate */
				m->y = -c;
		}
		else		/* Positive delta treated as Negative */
		{
			m->y = -128;		/* Set to signed char min */

			if ( (unsigned char)c < 128 )	/* Just negate */
				m->y = -c;
		}

		m->state = 0;
		mseproc(m);
		break;
	}
}

/*
 * send command byte to the 320 mouse device. Expect bufsize bytes
 * in return from 320 mouse and store them in buf. Verify that the
 * first byte read back is ans. If command fails or first byte read
 * back is SNDERR or SNDERR2, retry. Give up after two attempts.
 */

int
snd_320_cmd(cmd, ans, bufsize,buf)
register unsigned char	cmd;
register unsigned char	ans;
unchar *buf;
int bufsize;
{
	register unsigned char data;
	register int wait, sndcnt = 0;
	int rv;

#ifdef DEBUG
printf("entered snd_320_cmd() cmd = %x\n",cmd);
#endif
	while (sndcnt < 2) {

	   /* send the command to the 8042. rv == 1 --> success
	    * The 2 implies send the cmd to the aux device
	    */
	   rv = i8042_send_cmd(cmd, 2, buf,bufsize);

	   if ( rv && ((ans == MSE_ANY) || (buf[0] == ans)) )
		return 0; /* command succeeded, and first byte matches */


	   if(buf[0] == SNDERR || buf[0] == SNDERR2){
#ifdef DEBUG
printf("snd_320_cmd() SNDERR\n");
#endif
			if(buf[0] == SNDERR2 || sndcnt++ > 1){
#ifdef DEBUG
printf("snd_320_cmd() FAILED two resends\n");
#endif
				return FAILED;
			}
	   }
	   else
	   	return FAILED;
	}
#ifdef DEBUG
printf("leaving snd_320_cmd() \n");
#endif
	return FAILED;
}


/* check if 8042 has aux device */
int
is_mca()
{
	int cnt= 200000;
	char    junk;

	/* Disable Keyboard */
	SEND8042(MSE_ICMD, 0xad);

	/* If there's anything in 8042 output buffer, get it out */
	if(inb(MSE_STAT) & 0x01)
	junk=inb(MSE_OUT);
	SEND8042(MSE_ICMD, MSE_DISAB);

	while((inb(MSE_STAT) & 0x01) == 0 && cnt-- != 0)
		;

	/* Re-enable KB */
	SEND8042(MSE_ICMD, 0xae);

	if (cnt <= 0) {
		i8042_has_aux_port=1;
		return(1);  /* command was understood */
	} 

	(void)inb(MSE_OUT);
	i8042_has_aux_port=0;
	return(0);
}


/* 320 mouse read data function */
unchar
rdmse3()
{
	register char data;
	register int wait;

#ifdef DEBUG
printf("rdmse3() called\n");
#endif
	/* wait until the 8042 output buffer is full */
	for(wait =0;wait < 60000;wait++){
		if ((MSE_OUTBF & inb(MSE_STAT)) == MSE_OUTBF)
			break; 
		tenmicrosec();
	}
	if(wait == 60000){
#ifdef DEBUG
printf("rdmse3() FAILED timeout\n");
#endif
		return FAILED;
	}
	data = inb(MSE_OUT); 	/* get data byte from controller */
	return (data);
}

/* 320 mouse command execution function */
int
mcafunc(mca)
struct cmd_320 *mca;
{
	register int	retflg = 0;
	unchar	data,buf[10];

#ifdef DEBUG
printf("entered mcafunc(): cmd = %x\n",mca->cmd);
#endif
	i8042_acquire(); /* take ownership of 8042 */

	/* must turn mouse off if streaming mode set */
	if(mcastat.mode == MSESTREAM){
		if(snd_320_cmd(MSEOFF, MSE_ACK,1, buf) == FAILED){
#ifdef DEBUG
printf("mcafunc(): MSEOFF failed\n");
#endif
			i8042_release();
			return FAILED;
		}
		if(mca->cmd == MSEOFF) { /* we just did requested cmd */
			i8042_release();
			return 0;
		}
	}

#ifdef DEBUG
printf("mcafunc: doing switch statement\n");
#endif
	switch(mca->cmd & 0xff){
		case MSESETDEF: /* these commands have no args */
		case MSEOFF:
		case MSEON:
		case MSESPROMPT:
		case MSEECHON:
		case MSEECHOFF:
		case MSESTREAM:
		case MSESCALE2:
		case MSESCALE1:
			if(snd_320_cmd(mca->cmd, MSE_ACK,1,buf) == FAILED) {
				retflg = FAILED;
				break;
			}
			if(mca->cmd == MSESTREAM || mca->cmd == MSESPROMPT)
				mcastat.mode = mca->cmd;
			break;

		case MSECHGMOD:
			if(snd_320_cmd(mca->cmd, MSE_ACK,1, buf) == FAILED){
				retflg = FAILED;
				break;
			}
			/* received ACK. Now send arg */
#ifdef DEBUG
printf("mcafunc: do arg1 of MSECHGMOD = %x\n",mca->arg1);
#endif
			if(snd_320_cmd(mca->arg1, MSE_ACK,1,buf) == FAILED){
				retflg = FAILED;
#ifdef DEBUG
printf("mcafunc(): MSECHGMOD failed\n");
#endif
			}
			break;

		case MSERESET: /* expecting ACK and 2 add'tl bytes */
			if(snd_320_cmd(mca->cmd, MSE_ACK,3,buf) == FAILED){
				retflg = FAILED;
				break;
			}
			/* command succeeded and got ACK as first byte 
			 * Now verify next two bytes.
			 */
			if(buf[1] != 0xaa || buf[2] != 0x00){
				retflg = FAILED;
#ifdef DEBUG
printf("mcafunc(): MSERESET failed\n");
#endif
			}
			break;

		case MSEREPORT: /* expect ACK and then 3 add'tl bytes */
		case MSESTATREQ:
			if(snd_320_cmd(mca->cmd, MSE_ACK,4,buf) == FAILED){
				retflg = FAILED;
				break;
			}
			mca->arg1 = buf[1];
			mca->arg2 = buf[2];
			mca->arg3 = buf[3];
			retflg = 1;
			break;

		case MSERESEND: /* expect 3 bytes back. Don't care what
				 * the first byte is */
			if(snd_320_cmd(mca->cmd, MSE_ANY,3,buf) == FAILED){
				retflg = FAILED;
				break;
			}
			mca->arg1 = buf[0];
			mca->arg2 = buf[1];
			mca->arg3 = buf[2];
			retflg = 1;
			break;

		case MSEGETDEV: /* expect 2 bytes back */
			if(snd_320_cmd(mca->cmd, MSE_ACK,2,buf) == FAILED) {
				retflg = FAILED;
				break;
			}
			/* got an ACK, the second byte is the return val */
			mca->arg1 = buf[1];
			retflg = 1;
			break;

		case MSESETRES: /* cmd has one arg */
			if(snd_320_cmd(mca->cmd, MSE_ACK,1,buf) == FAILED){
				retflg = FAILED;
				break;
			}
			/* sent cmd successfully. Now send arg. If
			 * return val is not arg echoed back, fail the cmd
			 */
			if(snd_320_cmd(mca->arg1, MSE_ACK, 1, buf) == FAILED)
				retflg = FAILED;
			break;
		default:
#ifdef DEBUG
printf("mcafunc:SWITCH default \n");
#endif
			retflg = FAILED;
			break;
	}

	/* turn the mouse back on if we were streaming and cmd was not
	 * MSEOFF
	 */
	if(mcastat.mode == MSESTREAM){ 
		if(mca->cmd != MSEOFF){
			if(snd_320_cmd(MSEON, MSE_ACK,1,buf) == FAILED){
				retflg = FAILED;
#ifdef DEBUG
printf("mcafunc(): MSEON failed\n");
#endif
			}
		}
	}

	/* finished cmd. Release ownership of 8042 */
	i8042_release();
	return (retflg);
}
