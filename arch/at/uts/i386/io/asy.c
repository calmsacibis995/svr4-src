#ident	"@(#)asy.c	1.4	92/10/19	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/asy.c	1.3.3.1"

/*
 *	Asychronous Console Driver	EUC handling version
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/termio.h"
#include "sys/asy.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/cmn_err.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/debug.h"
#include "sys/eucioctl.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#include "sys/fcntl.h"
#include "sys/tty.h"
#include "sys/inline.h"
#ifdef VPIX
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/v86.h"
#endif
#include "sys/sysmsg.h"

extern 	struct	asy asytab[];   /* asy structs for each port */
extern 	struct	strtty asy_tty[];   /* tty structs for each device */
extern	int	p_asy0;
extern	int	num_asy;
extern	time_t  asylast_time[];     /* output char watchdog timeout */
int 	asyinitialized = 0;
extern	int	asyrflag[];
extern	int	asyiflag[];
extern	int	asyoflag[];
extern	int	space[];  /* count for global bp */

/* Values for asy_opened */
#define ASY_NOT_OPEN    0
#define ASY_OPEN        1
#define ASY_EXCL_OPEN   2

#ifdef VPIX
extern v86_t *asystash[];	/* save proc.p_v86 here for psuedorupts */
extern int asyintmask[];	/* which pseudorupt to give */
extern struct termss asyss[];	/* start/stop characters */
extern int   asy_closing[];
extern char asy_opened[];
extern int asy_v86_prp_pd[];
extern  int     validproc();


/* In DOSMODE and PARMRK we store MSR and LSR in the ring when they
 * change.  Borrow existing FRERROR and PERROR bits for these. */
#define MSRWORD FRERROR
#define LSRWORD PERROR
#endif

#ifdef MERGE386          /*  Extern structures for Merge */

extern  int     asy_is_assigned();
extern  int     com_ppiioctl();
extern  int     merge386enable;

#endif   /* MERGE386 */

#define TRUE 	1
#define FALSE 	0
#define CL_TIME	8*HZ	/* Timeout to remove block state while closing */

#define MAXTIME         2		/* 2 sec */
int asydevflag = 0;
extern time_t           asylast_time[];    /* output char watchdog timeout */
static char             asytmrun=0;	/* timer set up? */
static int              dummy;          /* placeholder for callout timer */

/* extern  debugger();  -- comment in to use kernel debug on asyinit() */
#define	RINGSIZ 256 	/* Size of the receive side character hold buffer */
#define	MSGBUFSZ 64 	/* Size of the message to be sent upstream  */
short	ringbuf[RINGSIZ];
short	*ringget = ringbuf;
short	*ringput = ringbuf;
short	ringcnt = 0;
char	asyrbsy = FALSE;
long	asydchars = 0;

/*
 * Baud rate table. Indexed by #defines found in sys/termios.h
 */

#define MAXBAUDS 17
ushort asyspdtab[] = {
	0,	/* 0 baud rate */
	0x900,	/* 50 baud rate */
	0x600,	/* 75 baud rate */
	0x417,	/* 110 baud rate ( %0.026 ) */
	0x359,	/* 134.5 baud rate ( %0.058 ) */
	0x300,	/* 150 baud rate */
	0x240,	/* 200 baud rate */
	0x180,	/* 300 baud rate */
	0x0c0,	/* 600 baud rate */
	0x060,	/* 1200 baud rate */
	0x040,	/* 1800 baud rate */
	0x030,	/* 2400 baud rate */
	0x018,	/* 4800 baud rate */
	0x00c,	/* 9600 baud rate */
	0x006,	/* 19200 baud rate */
	0x003	/* 38400 baud rate */
};


int asyopen(), asyclose(), asyoput(), asygetoblk();
int asyisrv(), asyproc(), asyrstrt(), asy_drain();
int asyosrv(), asydelay(), asyrtmout(), asygettim(), asyhangup() ; 

struct module_info asy_info = { 
	42, "asy", 0, INFPSZ, 256, 128};
static struct qinit asy_rint = { 
	putq, asyisrv, asyopen, asyclose, NULL, &asy_info, NULL};
static struct qinit asy_wint = { 
	asyoput, asyosrv, asyopen, asyclose, NULL, &asy_info, NULL};
struct streamtab asyinfo = { 
	&asy_rint, &asy_wint, NULL, NULL};


/*
 * Wakeup sleep function calls sleeping for a STREAMS buffer
 * to become available
 */
STATIC void
asybufwake( tp)
register struct strtty *tp;
{
 	wakeup( (caddr_t)&tp->t_cc[3]);
}

asyopen(q, devp, flag, sflag, crp)
queue_t *q;
dev_t *devp;
register flag, sflag;
cred_t *crp;
{
	register struct strtty *tp;
	struct asy *asyp;
	int	mcr, oldpri;
	int speed, initial_baud;
	mblk_t *mop;
	minor_t dev, ldev;
	struct	stroptions *sop;
	extern struct smsg_flags smsg_flags;

	ldev = getminor(*devp);
	dev = ldev & 0x0F ;

	if (dev >= num_asy || !( asytab[dev].asy_flags & ASYHERE ) ) {
		return(ENXIO);
	}
#ifdef VPIX
	/* enforce exclusive access */
	if (asy_opened[asychan(dev)] == ASY_EXCL_OPEN ||
	   ((flag & O_EXCL) && asy_opened[asychan(dev)] != ASY_NOT_OPEN))
		return(EBUSY);
#endif
	asyp = &asytab[dev];
	tp = &asy_tty[dev];

#ifdef MERGE386
/*
 *      Initialize the state keyboard state structure for this unit
 */
        if (asyp->asy_ppi_func && asy_is_assigned(asyp) ) {
                return(EACCES);
        }

#endif /* MERGE386 */

	if ((tp->t_state & (ISOPEN | WOPEN)) == 0) {

		/*set process group on first tty open*/
		
		oldpri = spltty();

		q->q_ptr = (caddr_t) tp;
		WR(q)->q_ptr = (caddr_t) tp;
		tp->t_rdqp = q;
		tp->t_dev = dev;

		while (( mop = allocb( sizeof( struct stroptions), BPRI_MED)) == NULL) {
			if ( flag & (FNDELAY | FNONBLOCK)) {
                                splx( oldpri);
                                return( EAGAIN);
			}
			bufcall( (uint)sizeof( struct stroptions), BPRI_MED, asybufwake, tp);
			if ( sleep( (caddr_t)&tp->t_cc[3], TTIPRI | PCATCH)) {
                                splx( oldpri);
                                return( EINTR);
			}
		}
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof( struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
                splx( oldpri);
		putnext( q, mop);

/*      Override the default baud rate of 300 set in ttinit().
 *      if COM1 and the RCEF is on, used the COM1 CMOS baud rate.
 *      If COM2 and the ACEF is on, use the COM2 CMOS baud rate.
 *      Otherwise use 1200 baud.
 */
                if(asyp->asy_flags & ASY82510)
                        outb(asyp->asy_isr, 0x00);      /* set bank 0 */

                initial_baud = smsg_check_cmos_baud(dev, B1200);
                speed = asyspdtab[initial_baud];

		/* Set Default line Settings */
                tp->t_cflag = (tp->t_cflag & ~CBAUD) | initial_baud | CREAD | HUPCL | CS8;
                tp->t_lflag = 0 ;
                tp->t_iflag = 0 ;
                tp->t_oflag = 0 ;
                tp->t_line = 0 ;

                outb(asyp->asy_lcr, DLAB);
                outb(asyp->asy_dat+DLL, speed & 0xff);
                outb(asyp->asy_dat+DLH, (speed >> 8) & 0xff);
                outb(asyp->asy_lcr, 0);
                outb(asyp->asy_lcr, STOP1|BITS8);
		if( ldev & 0x80 ) {
 			asyp->asy_flags |= HWDEV;
 			asyp->asy_flags &= ~(HWFLWO);
 		}
#ifdef VPIX
		asy_opened[asychan(dev)] = (flag & O_EXCL) ? ASY_EXCL_OPEN : ASY_OPEN;
		asyss[asychan(dev)].ss_start = CSTART;
		asyss[asychan(dev)].ss_stop = CSTOP;
		asy_v86_prp_pd[2*asychan(tp->t_dev)] = 0;
		asy_v86_prp_pd[(2*asychan(tp->t_dev)) + 1] = 0;
#else
		tp->t_cc[VSTART] = CSTART;
		tp->t_cc[VSTOP] = CSTOP;
#endif

		if ((asyp->asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) 
			cmn_err(CE_NOTE, "asy open: No message blocks.");
		space[dev]=MSGBUFSZ;

		oldpri = SPL();
		if (!asytmrun) {
			asylast_time[dev] = 0x7fffffffL;
			asytmout(dummy);
		}
		splx(oldpri);
		asyU_reset(dev);
		asyparam(dev);
	}
	else {
		if(( (asyp->asy_flags & HWDEV) == HWDEV ) ) {
			if( (ldev & 0x80 ) == 0 )
				return(EBUSY);	
		}
		else
			if( (ldev & 0x80 ) == 0x80 )
				return(EBUSY);	
	}
	
	oldpri = SPL();

	/* On every OPEN raise DTR */
	mcr = inb(asyp->asy_mcr);	
	outb(asyp->asy_mcr, (mcr|DTR));	

	if(tp->t_cflag & CLOCAL)
		tp->t_state |= CARR_ON;
	else {
		if  ( inb(asyp->asy_msr) & DCD  ) {
			tp->t_state |= CARR_ON;
			wakeup((caddr_t) &tp->t_rdqp);
                } else {
                        if ((tp->t_state & CARR_ON) &&
                              (tp->t_state & ISOPEN))
                                {      
                                       	asyflush(tp, FREAD|FWRITE);
					putctl(tp->t_rdqp->q_next, M_HANGUP);
                                }
                                tp->t_state &= ~CARR_ON;
               	}
	}     


	if ( !(flag & ( O_NDELAY | O_NONBLOCK)) && !(smsg_flags.acef && dev == 1 ))
		while ((tp->t_state & CARR_ON) == 0) {
			tp->t_state |= WOPEN;
			if (sleep((caddr_t) &tp->t_rdqp, TTIPRI|PCATCH)) {
				if ((tp->t_state & ISOPEN) == 0) {
					q->q_ptr = NULL;
					WR(q)->q_ptr = NULL;
					tp->t_rdqp = NULL;
					outb (asytab[dev].asy_mcr, OUT2);
 					asyp->asy_flags &= ~(HWDEV|HWFLWO);
#ifdef VPIX
					asy_opened[asychan(dev)] = ASY_NOT_OPEN;
#endif
					splx(oldpri);
					mop = asyp->asyrbp;
					asyp->asyrbp = NULL;
					freemsg(mop);
					space[dev]=0;
				}
				else
					splx(oldpri);

				tp->t_state &= ~(WOPEN);
				return(EINTR);
			}
		}

	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;


	splx(oldpri);
	return(0);
}

asyclose(q, flag, cred_p)
queue_t *q;
register flag;
cred_t *cred_p;
{
	register struct strtty *tp;
	register int	oldpri;
 	struct asy *asyp;
	dev_t dev;
	register struct msgb	*bp;
	extern asy_drain();


	tp = (struct strtty *)q->q_ptr;
	dev = tp->t_dev;
 	asyp = &asytab[dev];

	if(!(tp->t_state & ISOPEN)) {  /* See if it's closed already */
		return;
	}
#ifdef VPIX
	asy_closing[asychan(dev)] = 1;
	asy_opened[asychan(dev)] = ASY_NOT_OPEN;
	asyintmask[asychan(dev)] = 0;
	asystash[asychan(dev)] = 0;
	tp->t_iflag &= ~DOSMODE;
	asy_v86_prp_pd[2*asychan(tp->t_dev)] = 0;
	asy_v86_prp_pd[(2*asychan(tp->t_dev)) + 1] = 0;
#endif


	if ( !( flag & (FNDELAY|FNONBLOCK))) {
		/* Drain queued output to the user's terminal. */
		oldpri = spltty();
		while (( tp->t_state & CARR_ON) ){
			tp->t_dstat = 0;
			if((tp->t_out.bu_bp == 0 ) && (WR(q)->q_first == NULL)) 
				break;
			tp->t_state |= TTIOW;
			tp->t_dstat = timeout( asy_drain, (caddr_t) tp, CL_TIME);
			if( sleep((caddr_t) &tp->t_oflag,TTIPRI|PCATCH)){
				if( tp->t_dstat ) {
					untimeout(tp->t_dstat);
					tp->t_dstat = 0;
				}
				tp->t_state &= ~TTIOW;
				break;
			}
		}
		splx(oldpri);
	}

	/* do not >>> outb(asytab[dev].asy_icr, 0) -- because 
	close() gets called before all characters are sent, therefore, 
	the last chars do not get output with the interrupt turned on */ 

	if (tp->t_cflag & HUPCL) {
		/* turn off DTR, RTS but NOT interrupt to 386 */
		outb (asytab[dev].asy_mcr, OUT2);
	}

#ifdef VPIX
	asy_closing[asychan(dev)] = 0;
#endif

/* Make sure that the  Not in interrupt routine receive and  asyrproc  */
	oldpri = spltty();
	tp->t_state &= ~(ISOPEN | BUSY | TIMEOUT | TTSTOP);
	if (asyrflag[dev]) {
		untimeout(asyrflag[dev]);
		asyrflag[dev] = 0;
	}
		if ( asytab[dev].asyrbp )
			freeb(asytab[dev].asyrbp);

	tp->t_cflag = 0;
	splx(oldpri);

 	asyp->asy_flags &= ~(HWDEV|HWFLWO);
	tp->t_rdqp = NULL;
	q->q_ptr = WR(q)->q_ptr = NULL;
}


asyoput(q, bp)
queue_t *q;
mblk_t *bp;
{
	register struct msgb *bp1, *tmp;
	register struct strtty *tp;
	int s;

	tp = (struct strtty *)q->q_ptr;


	switch(bp->b_datap->db_type) {

	case M_DATA:
		if ( !(tp->t_state & CARR_ON )) {
			putq(q, bp);
			return;
		}

		s=spltty(); 
		while (bp) {
			bp->b_datap->db_type = M_DATA;
			bp1 = unlinkb(bp);
			bp->b_cont = NULL;
			if ((bp->b_wptr - bp->b_rptr) <= 0) {
				freeb(bp);
			} else {
				putq(q,bp);
			}
			bp = bp1;
		}
		splx(s);
		if (q->q_first != NULL) {
			asygetoblk(tp);
		}
		break;

	case M_IOCTL:
		asyputioc(q, bp);
		if (q->q_first != NULL) {
			asygetoblk(tp);
		}
		break;

	case M_FLUSH:
		s = splstr();
		if ( *bp->b_rptr & FLUSHW) {
			asyflush( tp, FWRITE);
			*bp->b_rptr &= ~FLUSHW;
		}
		if ( *bp->b_rptr & FLUSHR) {
			asyflush( tp, FREAD);
			putnext( RD(q), bp);
		} else
			freemsg( bp);
		splx( s);
		break;

	case M_START:
		s = SPL();
		asyproc(tp, T_RESUME);
		splx(s);
		freemsg(bp);
		asygetoblk(tp);
		break;

	case M_STOP:
		s = SPL();
		asyproc(tp, T_SUSPEND);
		splx(s);
		freemsg(bp);
		break;

	case M_DELAY:
		s = spltty();
		if( (tp->t_state & TIMEOUT ) || (q->q_first != NULL ) 
			|| (tp->t_out.bu_bp != NULL)) {
			putq( q, bp );
			splx(s);
			break;
		}
		tp->t_state |= TIMEOUT;
		timeout ( asydelay, tp, ((int)*(bp->b_rptr))*HZ/60);
		splx (s);
		freemsg (bp);
		return;

	case M_STARTI:
		s = spltty();
		asyproc(tp, T_UNBLOCK);
		splx(s);
		freemsg(bp);
		break;

	case M_STOPI:
		s = spltty();
		asyproc(tp, T_BLOCK);
		splx(s);
		freemsg(bp);
		break;
		
	case M_CTL: 
		{
			struct termios *termp;
			struct iocblk *iocbp;
			
			if (( bp->b_wptr - bp->b_rptr) != sizeof(struct iocblk) ) {
				freeb(bp);
				break;
			}
			iocbp = (struct iocblk *)bp->b_rptr;
			if ( iocbp->ioc_cmd  == MC_CANONQUERY ) {
				if ((bp1 = allocb(sizeof(struct termios),BPRI_HI)) == (mblk_t *) NULL) {
					/* Couldn't get message */	
					freeb(bp);
					break;
				}
				bp->b_datap->db_type = M_CTL;
				iocbp->ioc_cmd = MC_PART_CANON;
				bp->b_cont = bp1;
				bp1->b_wptr += sizeof(struct termios);
				termp = (struct termios *)bp1->b_rptr;
				termp->c_iflag = ISTRIP|IXON|IXANY;
				termp->c_cflag = 0;
				termp->c_oflag = 0;
				termp->c_lflag = 0;
		
				qreply(q, bp);
		
			} else
				freemsg(bp);
		}
		break;
	default:
		freemsg(bp);
		break;
	}
	return(0);
}

asygetoblk(tp)
register struct strtty *tp;
{
	register int s;
	register struct queue *q;
	register struct msgb	*bp;

	if (tp->t_rdqp == NULL) {
		return;
	}

	if (tp->t_state & WIOC ) {   /* Using the WIOC flag to prevent
					Multiple calls to routine from
					asygettim() and asyoput()
				      */
		return;
	}

	q = WR(tp->t_rdqp);

	s = SPL();

	while (!(tp->t_state & BUSY) && (bp = getq(q)) != NULL) {
		/* wakeup close write queue drain */

		switch (bp->b_datap->db_type) {

		case M_DATA:
			if ((tp->t_state & (TTSTOP | TIMEOUT)) != 0 ) {
				putbq(q, bp);
				splx(s);
				return;
			}
			/* start output processing for bp */
			tp->t_out.bu_bp = bp;
			asyproc(tp, T_OUTPUT);
			break;

		case M_IOCTL:
			if ((tp->t_state & (TTSTOP | TIMEOUT)) != 0 ) {
				putbq(q, bp);
				splx(s);
				return;
			}
			asysrvioc(q, bp);
			break;

		case M_DELAY:
			if ((tp->t_state & (TTSTOP | TIMEOUT)) != 0 ) {
				putbq(q, bp);
				splx(s);
				return;
			}
			tp->t_state |= TIMEOUT;
			timeout ( asydelay, tp, ((int)*(bp->b_rptr))*HZ/60);
			freemsg (bp);
			break;

		default:
			freemsg(bp);
			break;
		}
	}
	/* Wakeup any process sleeping waiting for drain to complete */
	if (( tp->t_out.bu_bp == 0 ) && (tp->t_state & TTIOW)) {
		if( tp->t_dstat ) {
			untimeout(tp->t_dstat);
			tp->t_dstat  = 0;
		}
		tp->t_state &= ~(TTIOW);
		wakeup((caddr_t) &tp->t_oflag);
	}  
	splx(s);

}

/*
 * ioctl handler for output PUT procedure
 */
asyputioc(q, bp)
queue_t *q;
mblk_t *bp;
{
	struct strtty *tp;
	struct iocblk *iocbp;
	mblk_t *bpr;
	mblk_t *bp1;
	int s, arg;

	iocbp = (struct iocblk *)bp->b_rptr;
	tp = (struct strtty *)q->q_ptr;



	switch( iocbp->ioc_cmd )
	{
	case RTS_TOG:
	case TCSETSW:
	case TCSETSF:
	case TCSETAW:
	case TIOCSTI:
	case TCSETAF:
	case TCSBRK: /* run these now, if possible */
		
		if( q->q_first != NULL || (tp->t_state & (TTSTOP | BUSY | TIMEOUT) ) 
			|| (tp->t_out.bu_bp != NULL)) 
		{
			putq(q, bp);
			break;
		}
		asysrvioc(q, bp);
		break;

	case TCSETA:	/* immediate parm set   */
	case TCSETS:

		if ( tp->t_state & BUSY) {
			putbq( q, bp); /* queue these for later */
			break;
		}
		asysrvioc (q, bp);
		break;

	case TCGETA:
	case TCGETS:	/* immediate parm retrieve */

		asysrvioc (q, bp);
		break;
#ifdef VPIX
	case AIOCDOSMODE:
	case AIOCNONDOSMODE:
	case AIOCINFO:
	case AIOCSERIALOUT:
	case AIOCSERIALIN:
	case AIOCINTTYPE:
	case AIOCSETSS:
		if( q->q_first != NULL || (tp->t_state & (BUSY|TIMEOUT) ) 
			|| (tp->t_out.bu_bp != NULL)) 
		{
			putq(q, bp);
			break;
		}
		asysrvioc(q, bp);
		break;;
#endif
	case EUC_MSAVE:
	case EUC_MREST:
	case EUC_IXLOFF:
	case EUC_IXLON:
	case EUC_OXLOFF:
	case EUC_OXLON: 
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		break;

	default:
		if ((iocbp->ioc_cmd & IOCTYPE) == LDIOC) { /* An IOCTYPE ? */
			bp->b_datap->db_type = M_IOCACK; /* ignore LDIOC cmds */
			bp1 = unlinkb(bp);
			if (bp1) {
				freeb(bp1);
			}
			iocbp->ioc_count = 0;
		} else {
#ifdef MERGE386
			if (merge386enable) {
				if( q->q_first != NULL )
				{
					putq(q, bp);
					return;
				}
				asysrvioc(q, bp);
				return;
			}
#endif /* MERGE386 */
/*
 * Unknown IOCTLs aren't errors, they just may have been intended for an
 * upper module that isn't present.  NAK them...
 */
			iocbp->ioc_error = EINVAL;
			iocbp->ioc_rval = (-1);
			bp->b_datap->db_type = M_IOCNAK;
		}
		qreply(q, bp);
		break;
	}
	return(0);
}

/*
 * Ioctl processor for queued ioctl messages.
 *
 */
asysrvioc(q, bp)
queue_t *q;
mblk_t *bp;
{
	struct strtty *tp;
	struct iocblk *iocbp, *iocp;
	struct asy *asyp;
	struct copyreq *cqp;
	char *argp, val, val2;
	int arg, rq, cmd;
	register int mcr;
	mblk_t *bpr;
	mblk_t *bp1, *bp2;

	iocbp = (struct iocblk *)bp->b_rptr;
	tp = (struct strtty *)q->q_ptr;
	asyp = &asytab[tp->t_dev];

	cmd = iocbp->ioc_cmd ;
	
	switch( cmd ) {

#ifdef VPIX
	case AIOCDOSMODE:
	{
		struct v86blk *p_v86;
		if ((tp->t_iflag & DOSMODE) == 0)
		{       
			p_v86 = (struct v86blk *)bp->b_cont->b_rptr;
			asystash[asychan(tp->t_dev)] = p_v86->v86_p_v86;
			asy_v86_prp_pd[2*asychan(tp->t_dev)] = (int)p_v86->v86_u_procp;
			asy_v86_prp_pd[(2*asychan(tp->t_dev)) + 1] = p_v86->v86_p_pid;
			tp->t_iflag |= DOSMODE;

			/*
			 * DOSMODE should be equal to 
			 *			CLOCAL TRUE
			 *			MODEM INTS ENABLED
			 *			CARR_ON TRUE
			 *	And MIEN should not be allowed off while
			 *      in DOSMODE (see standard ioctl stuff below)
			 *
			 */
			tp->t_cflag |= CLOCAL;	/* No hang up stuff */
			tp->t_state |= CARR_ON;	/* Let data flow */ 
			val = inb(asyp->asy_icr ) | MIEN;
			outb(asyp->asy_icr , val);

			/* Program should already have done AIOCINTTYPE.
			 * Assume SERIAL0 if has not.
			 */
			if (asyintmask[asychan(tp->t_dev)] == 0) {
				asyintmask[asychan(tp->t_dev)] = V86VI_SERIAL0;
			}
		}
                iocbp->ioc_rval = 0;
                bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;
	}
	case AIOCNONDOSMODE:
		if (tp->t_iflag & DOSMODE)
		{       
			asystash[asychan(tp->t_dev)] = 0;
			tp->t_iflag &= ~DOSMODE;
			asyintmask[asychan(tp->t_dev)] = 0;
			asy_v86_prp_pd[2*asychan(tp->t_dev)] = 0;
			asy_v86_prp_pd[(2*asychan(tp->t_dev)) + 1] = 0;
			tp->t_cflag &= ~CLOCAL;	
		}
                iocbp->ioc_rval = 0;
                bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;

	case AIOCINFO:
		iocbp->ioc_rval = ('a' << 8) | (tp - asy_tty);
                bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;


	case AIOCSERIALOUT:
		if ((tp->t_iflag & DOSMODE) && asystash[asychan(tp->t_dev)]) {
                        /* The first byte in bp->b_cont->b_cont indicates */
                        /* the argument for this command which is	  */
                        /* the string we want to copy                     */
			argp = (char *)bp->b_cont->b_rptr;
			rq = fubyte(argp);
			if (rq & SIO_MASK(SO_DIVLLSB)) {
				val = fubyte(argp+SO_DIVLLSB);
				intr_disable();
				val2 = inb(asyp->asy_lcr);
				outb(asyp->asy_lcr, val2 | DLAB);
				outb(asyp->asy_dat, val);
				outb(asyp->asy_lcr, val2 & ~DLAB);
				intr_restore();
			}
			if (rq & SIO_MASK(SO_DIVLMSB)) {
				val = fubyte(argp+SO_DIVLMSB);
				intr_disable();
				val2 = inb(asyp->asy_lcr);
				outb(asyp->asy_lcr, val2 | DLAB);
				outb(asyp->asy_icr, val);
				outb(asyp->asy_lcr, val2 & ~DLAB);
				intr_restore();
			}
			if (rq & SIO_MASK(SO_LCR)) {
				val = fubyte(argp+SO_LCR);
				outb(asyp->asy_lcr, val);
			}
			if (rq & SIO_MASK(SO_MCR)) {
				val = fubyte(argp+SO_MCR);
				/* force OUT2 on to preserve interrupts */
				outb(asyp->asy_mcr, val | OUT2);
			}
		}
                iocbp->ioc_rval = 0;
                bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;

	case AIOCSERIALIN:
		if ((tp->t_iflag & DOSMODE) && asystash[asychan(tp->t_dev)]) {
			argp = (char *)bp->b_cont->b_rptr;
			rq = fubyte(argp);
			if (rq & SIO_MASK(SI_MSR)) {
				subyte(argp+SI_MSR,inb(asyp->asy_mcr));
                		bp->b_datap->db_type = M_IOCACK;
                		iocbp->ioc_count = SI_MSR;
                		qreply(q, bp);
			}
		}
                iocbp->ioc_rval = 0;
                bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;

	case AIOCINTTYPE:
		switch (* (int *)bp->b_cont->b_cont->b_rptr)
		{ 
		case V86VI_KBD:
			asyintmask[asychan(tp->t_dev)] = V86VI_KBD;
			break;
		case V86VI_SERIAL1:
			asyintmask[asychan(tp->t_dev)] = V86VI_SERIAL1;
			break;
		case V86VI_SERIAL0:
		default:
			asyintmask[asychan(tp->t_dev)] = V86VI_SERIAL0;
			break;
		}
                iocbp->ioc_rval = 0;
		bp->b_datap->db_type = M_IOCACK;
       		iocbp->ioc_count = 0;
       		qreply(q, bp);
		return;

	case AIOCSETSS:
	{
		struct termios *termp;

		asyss[asychan(tp->t_dev)] = *(struct termss *)bp->b_cont->b_rptr;
		bp->b_datap->db_type = M_IOCACK;
       		iocbp->ioc_count = 0;
       		qreply(q, bp);
		return;
	}
#endif

	/* The output has drained now. */
	case TCSETAF:

		asyflush( tp, FREAD);
		putctl1( RD(q)->q_next, M_FLUSH, FLUSHR);

	case TCSETA:
	case TCSETAW:{

		register struct termio *cb;

		if ( !bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			qreply(q, bp);
			break;
		}
		cb = (struct termio *)bp->b_cont->b_rptr;

		tp->t_cflag = (tp->t_cflag & 0xffff0000 | cb->c_cflag);
		tp->t_iflag = (tp->t_iflag & 0xffff8000 | cb->c_iflag);

		bcopy ((caddr_t)cb->c_cc, (caddr_t)tp->t_cc, NCC);
		asyparam(tp->t_dev);
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		qreply(q, bp);
		break;

	}
	case TCSETSF:

		asyflush( tp, FREAD);
		putctl1( RD(q)->q_next, M_FLUSH, FLUSHR);

	case TCSETS:
	case TCSETSW:{

		register struct termios *cb;

		if ( !bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			qreply(q, bp);
			break;
		}
		cb = (struct termios *)bp->b_cont->b_rptr;

		tp->t_cflag = cb->c_cflag;
		tp->t_iflag = cb->c_iflag;
		bcopy ((caddr_t)cb->c_cc, (caddr_t)tp->t_cc, NCCS);

#ifdef VPIX
		asyss[asychan(tp->t_dev)].ss_start = tp->t_cc[VSTART];
		asyss[asychan(tp->t_dev)].ss_stop = tp->t_cc[VSTOP];
#endif
		asyparam(tp->t_dev);
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		qreply(q, bp);
		break;

	}
	case TCGETA:{	/* immediate parm retrieve */
		register struct termio *cb;

		if ( bp->b_cont)	/* Bad user supplied parameter */
			freemsg( bp->b_cont);

		if( (bpr = allocb(sizeof(struct termio),BPRI_MED)) == NULL )
		{
			ASSERT(bp->b_next == NULL);
			putbq(q, bp);
			bufcall((ushort)sizeof(struct termio), BPRI_MED, asygetoblk,tp); 
			return;
		}

		bp->b_cont = bpr;
		cb = (struct termio *)bp->b_cont->b_rptr;

		cb->c_iflag = (ushort)tp->t_iflag;
		cb->c_cflag = (ushort)tp->t_cflag;
		bcopy ((caddr_t)tp->t_cc, (caddr_t)cb->c_cc, NCC);

		bp->b_cont->b_wptr += sizeof(struct termio);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = sizeof(struct termio);
		qreply(q, bp);
		break;

	}
	case TCGETS:{	/* immediate parm retrieve */

		register struct termios *cb;

		if ( bp->b_cont)	/* Bad user supplied parameter */
			freemsg( bp->b_cont);

		if( (bpr = allocb(sizeof(struct termios),BPRI_MED)) == NULL ) {
			ASSERT(bp->b_next == NULL);
			putbq(q, bp);
			bufcall((ushort)sizeof(struct termios), BPRI_MED, asygetoblk,tp); 
			return;
		}
		bp->b_cont = bpr;
		cb = (struct termios *)bp->b_cont->b_rptr;

		cb->c_iflag = tp->t_iflag;
		cb->c_cflag = tp->t_cflag;
		bcopy ((caddr_t)tp->t_cc, (caddr_t)cb->c_cc, NCCS);

		bp->b_cont->b_wptr += sizeof(struct termios);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = sizeof(struct termios);
		qreply(q, bp);
		break;

	}
 	case TCSBRK:
		if ( !bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			qreply(q, bp);
			break;
		}
		arg = *(int *)bp->b_cont->b_rptr;
		if (arg == 0)
			asyproc(tp, T_BREAK);
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		qreply(q, bp);
		break;

	case TIOCSTI: { /* Simulate typing of a character at the terminal. */
		register mblk_t *mp;

		/*
		 * The permission checking has already been done at the stream
		 * head, since it has to be done in the context of the process
		 * doing the call.
		 */
		if ((mp = allocb(1, BPRI_MED)) != NULL) {
			if (!canput(RD(q)->q_next))
				freemsg(mp);
			else {
				arg = *(int *)bp->b_cont->b_rptr;
				*mp->b_wptr++ = (unsigned char) arg;
				putq( tp->t_rdqp, mp);
			}
		}

		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		putnext( RD(q), bp);
		break;
	}
	case EUC_MSAVE:	/* put these here just in case... */
	case EUC_MREST:
	case EUC_IXLOFF:
	case EUC_IXLON:
	case EUC_OXLOFF:
	case EUC_OXLON: 
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		break;

        case RTS_TOG:
                mcr = inb(asyp->asy_lcr);
                arg = *(int *)bp->b_cont->b_rptr;
                if( (int) arg == 0x01 && !(mcr & RTS)){
                                outb(asyp->asy_mcr, mcr|RTS);
                                asyproc(tp,T_RESUME);

                }else if((int)arg == 0x00 && (mcr & RTS)){
                        outb(asyp->asy_mcr, mcr & ~RTS);
                        asyproc(tp,T_SUSPEND);
                }
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
                break;


	default: /* unexpected ioctl type */
		if (( iocbp->ioc_cmd&IOCTYPE) == LDIOC) {
			/*
			 * ACK LDIOC ioctls
			 */
			bp->b_datap->db_type = M_IOCACK;
			bp1 = unlinkb( bp);
			if ( bp1)
				freeb( bp1);
                                iocbp->ioc_count = 0;
		} else {
#ifdef MERGE386
			if(merge386enable) {
				if (com_ppiioctl( q, bp, asyp, cmd))
						return;
			}

#endif /* MERGE386  */
			/*
			 * Unknown ioctls. Just NAK them.
			 */
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
		}
		putnext( RD(q), bp);
		break;
	}
	return;
}

asyflush(tp, cmd)
struct strtty *tp;
register cmd;
{
	queue_t *q;
	mblk_t *bp;
	int s;

	s = SPL();
	if (cmd&FWRITE) {
		q = WR(tp->t_rdqp);
		flushq(q,FLUSHDATA);
		tp->t_state &= ~(BUSY);
		if ( tp->t_out.bu_bp != NULL ) {
			bp = tp->t_out.bu_bp;
			tp->t_out.bu_bp = NULL ;
			freeb(bp);
		}
		if ( tp->t_state & TTIOW) {
			tp->t_state &= ~(TTIOW);
			if( tp->t_dstat ) {
				untimeout(tp->t_dstat);
				tp->t_dstat = 0;
			}
			wakeup((caddr_t) &tp->t_oflag);
		}
	}
	if (cmd&FREAD) {
		struct msgb	*bp;

		q = tp->t_rdqp;
		flushq(q,FLUSHDATA);
		bp = asytab[tp->t_dev].asyrbp;
		bp->b_wptr = bp->b_datap->db_base;
		space[tp->t_dev]=MSGBUFSZ;
		tp->t_state &= ~(TBLOCK);
	}
	splx(s);
	asygetoblk(tp);

}

/*
/* Param detects modifications to the line characteristics and reprograms 
 */
asyparam(dev)
int dev;
{
	struct asy *asy;
	struct strtty *tp;
	unsigned int flags;  /* To ease access to c_flags    */
	int s;      /* Index to conversion table for COM baud rate */
	int x, mcr, oldpri;

	asy = &asytab[getminor(dev)];
	tp = &asy_tty[dev];
	flags = tp->t_cflag;
	x = (TIEN | SIEN | MIEN | RIEN);

	if (tp->t_cflag & CLOCAL)
		tp->t_state |= CARR_ON;
	else
		asymodem(dev);

	outb(asy->asy_icr, x);

	x = inb(asytab[dev].asy_lcr);
	x &= ~(WLS0|WLS1|STB|PEN|EPS);
	if ((tp->t_cflag & CSTOPB) == CSTOPB )
		x |= STB;  

	if ((tp->t_cflag & CS6) == CS6 )
		x |= BITS6;
	if ((tp->t_cflag & CS7) == CS7 )
		x |= BITS7;
	if ((tp->t_cflag & CS8) == CS8 )
		x |= BITS8;
	if ((tp->t_cflag & PARENB) == PARENB )
		x |= PEN;
	if ((tp->t_cflag & PARODD) == 0)
		x |= EPS;
	outb(asy->asy_lcr, x);

	/*
	 * Set the baud rate:
	 * Bounds check baud rate index and then verify that the chip supports
	 * that speed.  Then program it. Default to 1200 baud.
	 */
	s = flags & CBAUD;
	if (s > MAXBAUDS || s < 0)
		s = B1200;

	oldpri = SPL();
	asyT_prog(dev, s);
	splx(oldpri);

	/* if baud rate is zero turn off DTR */
	if  ( (flags & CBAUD) == 0 ) {
		mcr = inb(asy->asy_mcr);	
		outb(asy->asy_mcr, (mcr & ~DTR));	
	}
}

/*
 * asyintr is the entry point for all interrupts.  There are four different
 *	interrupt types indexed by asy_isr register values:
 *		0: modem 
 *		1: Tx holding register is empty, ready for next char
 *		2: Rx register now holds a char to be picked up
 *		3: error or break on line
 */

/* In order to limit the amount of time we spend at spl7(), the
 * interrupt routine is closely connected to asyrproc and asyrtmout.
 * You should not change one without looking at the affects upon
 * the other two routines.
 * IMPORTANT NOTE:  This driver runs at an spl of 7.  splstr() is set to at 6.
 * As a result, NO STREAMS operations can be done at interrupt level.  They
 * must be done in the asyrproc routine which is called by timeout.
 */
asyintr(vec)
unsigned int	vec;
{
	register		mdev;
	struct asy		*asy;

	asy = &asytab[0];
	for (mdev = 0; mdev < num_asy; ++mdev) {
	 	if ((asy->asy_vect == vec) && (asy->asy_flags & ASYHERE))
			if (asydointr(mdev))
				break;
		++asy;
	}
}

static
asydointr(mdev)
	register		mdev;
{
	register mblk_t		*bp;
	struct strtty		*tp;
	struct asy		*asy;
	register short		c;
	unsigned char   interrupt_id, line_status, modem_status;
	int intr = 0;

	extern asyrproc();

	asy = &asytab[mdev];
	tp = &asy_tty[mdev];
#ifdef MERGE386
	/* needed for com port attachment */
	if (merge386enable)
		if (asy->asy_ppi_func && (*asy->asy_ppi_func) (asy, -1))
			return(0);

#endif /* MERGE386 */
loop:
	interrupt_id = inb(asy->asy_isr) & 0x0f; 
	line_status = inb(asy->asy_lsr);
	modem_status = inb(asy->asy_msr);

	if (asy->asy_flags & ASY82510)
		outb(asy->asy_isr, 0x00); /* set bank 0 */
	if ((interrupt_id == RxRDY) ||
	    (interrupt_id == FFTMOUT) ||
	    (line_status == RCA) ||
	    (interrupt_id == RSTATUS)) {

		if (line_status & RCA) 
			c = inb(asy->asy_dat) & 0xff;
		
		if ((tp->t_cflag & CREAD) != CREAD)
			goto loop;

#ifdef MERGE386
		/* Needed for direct attachment of the serial port */
		if (merge386enable) {
			if (asy->asy_ppi_func && (*asy->asy_ppi_func) (asy, c))
				goto loop;
		}
#endif /* MERGE386 */
		if (line_status & (PARERR|FRMERR|BRKDET|OVRRUN)) {
#ifdef VPIX
			if ((tp->t_iflag & (DOSMODE | PARMRK)) == 
					   (DOSMODE | PARMRK)) {
				c = (line_status | (mdev << 8) | LSRWORD);
				if (asyrflag[mdev] && space[mdev]) {
					untimeout(asyrflag[mdev]);
					asyrflag[mdev] = 0;
				}
				if (space[mdev])
					asyrflag[mdev] = timeout(asyrtmout,
								 mdev, HZ/30);
				if (ringcnt != RINGSIZ) {
					*ringput++ = c;
					if (ringput >= &ringbuf[RINGSIZ])
						ringput = ringbuf;
					ringcnt++;
				}
				else	asydchars++;
				if (!asyiflag[mdev]) {
					asyiflag[mdev] = timeout(asyrproc,
								 mdev, HZ/60);
				}

				line_status = 0;
			}
#endif

			if (line_status & PARERR)
				c |= PERROR;
			else if (line_status & FRMERR|BRKDET)
				c |= FRERROR;
			else if (line_status & OVRRUN)
				c |= OVERRUN;
		}

		c |= mdev << 8;
		if (asyrflag[mdev] && space[mdev]) {
			untimeout(asyrflag[mdev]);
			asyrflag[mdev] = 0;
		}
		if (space[mdev]) 
			asyrflag[mdev] = timeout(asyrtmout, mdev, HZ/30);

		if (ringcnt != RINGSIZ) {
			*ringput++ = c;
			if (ringput >= &ringbuf[RINGSIZ])
				ringput = ringbuf;
			ringcnt++;
		}
		else	asydchars++; /* just dropeed a character */
		if (!asyiflag[mdev]) {
			asyiflag[mdev]=timeout(asyrproc, mdev, HZ/60);
		}

	}
	else if ((interrupt_id == TxRDY) ||
		 (line_status & XHRE) && (tp->t_state & BUSY)) {
		/*
		 * Transmitted Character
		 * Check for a transmit interrupt even if we just got a recieve
		 * interrupt because that could mask the transmit; if we
		 * lose it,
		 * we will have to wait for tmout to pick us up again.
		 */
		asyxintr(mdev);

	}
	else if (interrupt_id == MSTATUS)  
		asymodem(mdev);
	else
		return(intr);
	++intr;
	goto loop;
}

asyrtmout(dev)
register dev;
{
	struct asy      *asy;
	struct	strtty      *tp;
	register mblk_t *bp;
	extern  int     validproc();

	asyrflag[dev] = 0;
	tp = &asy_tty[dev];

	if (tp->t_rdqp == NULL) {
		return;
	}

	if (asytab[dev].asyrbp == NULL) 
		return;

	bp = asytab[dev].asyrbp;

	if (bp->b_wptr != bp->b_rptr) {
#ifdef VPIX
		if ( (tp->t_iflag & DOSMODE) && asystash[dev]) {
			if( validproc(asy_v86_prp_pd[2*asychan(dev)], asy_v86_prp_pd[(2*asychan(dev)) + 1]))
				v86setint(asystash[dev], asyintmask[dev]);
			else {
				tp->t_iflag &= ~DOSMODE;
				asystash[asychan(dev)] = 0;
				asyintmask[asychan(dev)] = 0;
				asy_v86_prp_pd[2*asychan(dev)] = 0;
				asy_v86_prp_pd[(2*asychan(dev)) + 1] = 0;
			}
		}
#endif
		putq(tp->t_rdqp, bp);
		if (tp->t_rdqp->q_count > tp->t_rdqp->q_hiwat) {
			if ((tp->t_iflag & IXOFF) && !(tp->t_state &
						TBLOCK))
				asyproc (tp, T_BLOCK);
			if (tp->t_rdqp->q_count > MAX_INPUT)
				asyflush (tp,FREAD);
		}
		if ((asytab[dev].asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) {
			cmn_err(CE_WARN, "asyrtmout: no msg blocks");
			return;
		}
		space[dev]=MSGBUFSZ;
	}
}


/* This routine is called from a timeout set by the asyintr code.
 * This routine runs mostly at spl5().  asyrproc
 * will remain active as long as there are characters on the
 * ring buffer.  If the buffer associated with the devices
 * runs out of space, it will be put on the queue to 
 * wait for action by the service routine (asyisrv).
 * The timout routine is called every HZ/30 ticks.
 * this time is pushed out by the asyintr routine if
 * characters are being received in a continuous stream.
 * Process character.  Argument is the device number.
 */
asyrproc(mdev)
register short mdev;
{
	struct	strtty      *tp;
	int 	oldpri;
	struct	asy	*asy;
	unsigned char ctmp;
	int	dev;
	mblk_t	*bp;
	short int c;
	extern  int     validproc();

	for(;;){
		if ( ringcnt ) {
			--ringcnt;
			c = *ringget++;
			if ( ringget >= &ringbuf[RINGSIZ])
				ringget = ringbuf;
		} else 
			break;

		oldpri = spl5();
		dev = (c>>8)&017;
		tp = &asy_tty[dev];


                if (!(tp->t_state & (ISOPEN | WOPEN)))
                        goto rloop;
		if ( asytab[dev].asyrbp  == NULL) {
			if ((asytab[dev].asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) {
				cmn_err(CE_WARN, "asyrtmout: no msg blocks");
				goto rloop;
			}
			space[dev]=MSGBUFSZ;
			asyrflag[dev] = timeout(asyrtmout, dev, HZ/30);
		}

		bp = asytab[dev].asyrbp;

		if (c & PERROR && !(tp->t_iflag & INPCK)) {
			if( (tp->t_cflag & (PARODD|PARENB)) != (PARODD|PARENB)
				&& (( c & 0377 ) != 0 ))
					c &= ~PERROR;
		}
		if (c & (FRERROR|PERROR|OVERRUN)) {
#ifdef VPIX
                        if ((tp->t_iflag & (DOSMODE | PARMRK)) ==
                                        (DOSMODE | PARMRK))
			{
				if (space[dev] < 2 ) {
					untimeout(asyrflag[dev]);
						if ( (tp->t_iflag & DOSMODE) && asystash[dev])
							if( validproc(asy_v86_prp_pd[2*asychan(dev)], asy_v86_prp_pd[(2*asychan(dev)) + 1]))
								v86setint(asystash[dev], asyintmask[dev]);
							else {
								tp->t_iflag &= ~DOSMODE;
								asystash[asychan(dev)] = 0;
								asyintmask[asychan(dev)] = 0;
								asy_v86_prp_pd[2*asychan(dev)] = 0;
								asy_v86_prp_pd[(2*asychan(dev)) + 1] = 0;
							}
						putq(tp->t_rdqp, bp);
						if ((asytab[dev].asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) {
							cmn_err(CE_WARN, "asyrtmout: no msg blocks");
							goto rloop;
						}
					space[dev]=MSGBUFSZ;
					asyrflag[dev] = timeout(asyrtmout, dev, HZ/30);
				}
                                *bp->b_wptr++ = 0377;
                                if (c & MSRWORD)
                                        *bp->b_wptr++ = 2;
                                else if (c & LSRWORD)
                                        *bp->b_wptr++ = 1;
                                *bp->b_wptr++ = (char) c;
                                space[dev] -= 3;
                                goto output;
                        }
#endif 

			if ((c & 0377) == 0) {
				if (tp->t_iflag & IGNBRK) {
					goto rloop;
				}
				putctl(tp->t_rdqp->q_next, M_BREAK);
				goto rloop;
			} else 
				if ((tp->t_iflag & IGNPAR)) 
					goto rloop;
			if (tp->t_iflag & PARMRK) {
				if (space[dev] < 2 ) {
					untimeout(asyrflag[dev]);
#ifdef VPIX
						if ( (tp->t_iflag & DOSMODE) && asystash[dev])
							if( validproc(asy_v86_prp_pd[2*asychan(dev)], asy_v86_prp_pd[(2*asychan(dev)) + 1]))
								v86setint(asystash[dev], asyintmask[dev]);
							else {
								tp->t_iflag &= ~DOSMODE;
								asystash[asychan(dev)] = 0;
								asyintmask[asychan(dev)] = 0;
								asy_v86_prp_pd[2*asychan(dev)] = 0;
								asy_v86_prp_pd[(2*asychan(dev)) + 1] = 0;
							}
#endif 
						putq(tp->t_rdqp, bp);
						if ((asytab[dev].asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) {
							cmn_err(CE_WARN, "asyrtmout: no msg blocks");
							goto rloop;
						}
					space[dev]=MSGBUFSZ;
					asyrflag[dev] = timeout(asyrtmout, dev, HZ/30);
				}
                                *bp->b_wptr++ = 0377;
				*bp->b_wptr++ = 0;
				*bp->b_wptr++ = (char) c;
				space[dev] -= 3;
				goto output;
			} else 
				/* Send up a '\0' character  */
				*bp->b_wptr++ = 0 ;
				space[dev]--;
				goto output; 
		} 

		if (tp->t_iflag & ISTRIP)
			ctmp = c & 0177;
		 else 
			ctmp = c & 0377;

                if ((c == 0377) && (tp->t_iflag & PARMRK)) {
			*bp->b_wptr++ = 0377;
			space[dev]--;
                }

		if (tp->t_iflag & IXON) {

			/*
			 * If we are stopped, and if the char is a xon,
			 * or if we should restart on any char, resume
			 */
			if (tp->t_state & TTSTOP)  {
				/* got a ctl-q or resume char */
#ifdef VPIX
				if (ctmp == asyss[dev].ss_start  ||
#else
				if (ctmp == tp->t_cc[VSTART]  ||
#endif 
				    tp->t_iflag & IXANY) {
					asyproc(tp, T_RESUME);
				}
			} else {
				/* not stopped by ctl-s */
#ifdef VPIX
				if (ctmp == asyss[dev].ss_stop && (ctmp != 0 )) {
#else
				if (ctmp == tp->t_cc[VSTOP] && (ctmp != 0 )) {
#endif 
					asyproc(tp, T_SUSPEND);
				}
			}

			/* If char not NULL, throw it away */
			if ( ( ctmp & 0377 ) != 0 ) {
#ifdef VPIX
				if ( ctmp == asyss[dev].ss_start ||
					ctmp == asyss[dev].ss_stop ){
#else
				if ( ctmp == tp->t_cc[VSTART] || 
					ctmp == tp->t_cc[VSTOP] ){
#endif 
					goto rloop;
				}
			}

		}  /* end if IXON */

		*bp->b_wptr++ = ctmp;
		space[dev]--;
output:		
		if (!space[dev]) {
			untimeout(asyrflag[dev]);
#ifdef VPIX
			if ( (tp->t_iflag & DOSMODE) && asystash[dev])
				if( validproc(asy_v86_prp_pd[2*asychan(dev)], asy_v86_prp_pd[(2*asychan(dev)) + 1]))
					v86setint(asystash[dev], asyintmask[dev]);
				else {
					tp->t_iflag &= ~DOSMODE;
					asystash[asychan(dev)] = 0;
					asyintmask[asychan(dev)] = 0;
					asy_v86_prp_pd[2*asychan(dev)] = 0;
					asy_v86_prp_pd[(2*asychan(dev)) + 1] = 0;
				}
#endif
			putq(tp->t_rdqp, bp);
			if ((asytab[dev].asyrbp = allocb(MSGBUFSZ, BPRI_MED)) == NULL) {
				cmn_err(CE_WARN, "asyrtmout: no msg blocks");
				goto rloop;
			}
			space[dev]=MSGBUFSZ;
			asyrflag[dev] = timeout(asyrtmout, dev, HZ/30);
		}
rloop:	
		/* back to high priority */
		splx(oldpri);
	}
	asyiflag[mdev] = 0;;
}

/*
 * This is logically a part of asyintr, but that's too big.  This code
 * handles transmit buffer empty interrupts, dealing with current xon/xoff
 * state and availability of more data to transmit.  It also works in
 * conjunction with asytmout() to insure that lost interrupts don't hang
 * the driver:  if a char is xmitted and we go more than 2s (MAXTIME) without
 * an interrupt, asytmout will supply it.
 */
asyxintr(mdev)
register int    mdev;
{
	struct strtty      *tp;
	struct asy      *asy;
	char            c, mcr;
	int		oldpri;
	extern asygettim();

	asylast_time[mdev] = 0x7fffffffL;  /* don't time out */
	tp = &asy_tty[mdev];
	asy = &asytab[mdev];

	if(!(tp->t_state & ISOPEN))   
		return;


	/*
	 * If we are supposed to send an xoff or xon, do it now
	 */
	if (tp->t_state & TTXON) {
		tp->t_state &= ~TTXON;
		tp->t_state |= BUSY;
 		if ( asy->asy_flags & HWDEV ) {
 			mcr = inb(asy->asy_mcr);
                        outb(asy->asy_mcr, mcr | RTS);
 		} 
#ifdef VPIX
 		outb(asy->asy_dat, CSTART);
#else
 		outb(asy->asy_dat, tp->t_cc[VSTART]);
#endif
	} 

	if (tp->t_state & TTXOFF) {
		tp->t_state &= ~TTXOFF;
		tp->t_state |= BUSY;
#ifdef VPIX
 		outb(asy->asy_dat, CSTOP);
#else
 		outb(asy->asy_dat, tp->t_cc[VSTOP]);
#endif

 		if ( asy->asy_flags & HWDEV ) {
 			mcr = inb(asy->asy_mcr);
                	outb(asy->asy_mcr, mcr & ~RTS);
 		}
	} 

	/* try to initiate more output */
	tp->t_state &= ~BUSY;
	asyproc( tp, T_OUTPUT );

	/* if output didn't start get a new message */
	if ((!(tp->t_state & BUSY)) && !(tp->t_state & TTSTOP)) {
		if(!(asyoflag[tp->t_dev])) {
			tp->t_state |= WIOC; /*Using the WIOC flag to prevent*/
			asyoflag[mdev]=timeout(asygettim, tp, 2);
		}
	}
}

/*
 *	General command routine that performs device specific operations for
 * generic i/o commands.  All commands are performed with tty level interrupts
 * disabled.
 */
asyproc(tp, cmd)
struct strtty	*tp;
int		cmd;
{
	char            c, mcr;      /* char to process      */
	int             dev, oldpri;
	unsigned	char		line_ctl;
	register struct msgb *bp;
	struct asy *asy;
	extern asygettim();

	oldpri = spltty();
	/*
	 * get device number and control port
	 */
	dev = tp - asy_tty;
	asy = &asytab[dev];

	/*
	 * based on cmd, do various things to the device
	 */
	switch (cmd) {

	case T_TIME:            /* stop sending a break */

		tp->t_state &= ~TIMEOUT;
		line_ctl = inb(asy->asy_lcr);
		outb( asy->asy_lcr, line_ctl & ~SETBREAK ); 
		goto start;

	case T_RESUME:          /* enable output        */

 		if ( (asy->asy_flags & (HWDEV | HWFLWO) == (HWDEV | HWFLWO) ) ){
			break;
		}

		tp->t_state &= ~TTSTOP;

		/* fall through */

	case T_OUTPUT:          /* do some output       */
start:

		/* If we are stopped, doing a break, or busy, do nothing */
		if ( tp->t_state & ( TIMEOUT | TTSTOP | BUSY ) ) {
			break;
		}

		/*
		 * Check for characters ready to be output.
		 * If there are any, ship one out.
		 */
		bp = tp->t_out.bu_bp;
		if (bp == NULL || bp->b_wptr <= bp->b_rptr) {
			if(!(asyoflag[tp->t_dev])) {
				tp->t_state |= WIOC; /*Using the WIOC flag to prevent*/
				asyoflag[tp->t_dev]=timeout(asygettim, tp, 2);
			}	

			break;
		}

		/* specify busy, and output a char */
		tp->t_state |= BUSY;
		outb( asy->asy_dat, *bp->b_rptr++ );

		/* reset the time so we can catch a missed interrupt */
		drv_getparm(TIME,&asylast_time[dev]);
		break;

	case T_SUSPEND:         /* block on output      */

		tp->t_state |= TTSTOP;
		break;

	case T_BREAK:           /* send a break         */
		
		tp->t_state |= TIMEOUT;   
		line_ctl = inb(asy->asy_lcr);
		outb(asy->asy_lcr, line_ctl | SETBREAK); 
		timeout(asyrstrt, tp, HZ/5);
		break;

	case T_BLOCK:

		tp->t_state &= ~(TTXON);
		tp->t_state |= TBLOCK;
		if (tp->t_state & BUSY)
			tp->t_state |= TTXOFF;
		else {
 			tp->t_state |= BUSY; 
#ifdef VPIX
 			outb(asy->asy_dat, CSTOP);
#else
 			outb(asy->asy_dat, tp->t_cc[VSTOP]);
#endif
 			if ( asy->asy_flags & HWDEV ) {
 				mcr = inb(asy->asy_mcr);
                		outb(asy->asy_mcr, mcr & ~RTS);
			}
		}
		break;

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF | TBLOCK);
		if (tp->t_state & BUSY) {
			tp->t_state |= TTXON;
		}
		else {
 			if ( asy->asy_flags & HWDEV ) {
 				mcr = inb(asy->asy_mcr);
                         	outb(asy->asy_mcr, mcr | RTS);
			}
 			tp->t_state |= BUSY; 
#ifdef VPIX
 			outb(asy->asy_dat, CSTART);
#else
 			outb(asy->asy_dat, tp->t_cc[VSTART]);
#endif
		}
		break;

	}
	splx(oldpri);
}

asy_drain(tp)
struct strtty *tp;
{
	asyproc(tp, T_RESUME);
	tp->t_state &= ~(TTIOW);
	wakeup((caddr_t) &tp->t_oflag);
	tp->t_dstat = 0;
}

asyrstrt(tp)
struct strtty *tp;
{
	asyproc(tp, T_TIME);
}

/*
 * New service procedure.  Pass everything upstream.
 */
asyisrv(q)
queue_t *q;
{
	register mblk_t *mp;
	register struct strtty *tp;
	int s;

	tp = (struct strtty *)q->q_ptr;

	s = spltty();
	while ((mp = getq(q)) != NULL) {
		/*
		 * If we can't put, then put it back if it's not
		 * a priority message.  Priority messages go up
		 * whether the queue is "full" or not.  This should
		 * allow an interrupt in, even if the queue is hopelessly
		 * backed up.
		 */
		if (canput(q->q_next) == 0) {
			putbq(q, mp);
			splx(s);
			return;
		}
		putnext(q, mp);
		if ( tp->t_state & TBLOCK) {
			if (q->q_count < q->q_lowat) 
				asyproc(tp, T_UNBLOCK);
		}
	}
	splx(s);
	return(0);
}

/*
 * Initialization code that the kernel runs when coming up.  Detect and
 * program any COMs we see hanging around.
 */
asyinit()
{
	register ushort	testval;
	int                 dev;
	char		b_type;
	int		dummyarg;
	struct	asy	*asy;

	if ( asyinitialized )
		return;
	asyinitialized = 1;

	for (dev=0; dev < num_asy; dev++) {
		asy = &asytab[dev];
                /* ser. int. uses bits 0,1,2; FIFO uses 3,6,7; 4,5 wired low.
                   If bit 4 or 5 appears on inb() ISR, board is not there. */
		if ((inb(asy->asy_isr) & 0x30)) {
			asy->asy_flags |= ~ASYHERE;
                        continue; /* no serial adapter here */
		}
		asy->asy_flags |= ASYHERE;
                outb(asy->asy_isr,0x20);
                if ((inb(asy->asy_isr) & 0x20)){
/* Since most of the gerneal operation of the 82510 chip can be done from */
/* BANK 0 (8250A/16450 compadable mode) we'll deafult to BANK 0.           */
                        asy->asy_flags |= ASY82510; /* 82510 chip present */
                        outb((asy->asy_dat + 0x7),0x04);   /* Status clear */
                        outb(asy->asy_isr,0x40);       /* set to bank 2 */
                                outb(asy->asy_mcr,0x08); /*  IMD */
                                outb(asy->asy_dat,0x21); /*  FMD */
                        outb(asy->asy_isr,0x00);       /* set to bank 0 */
                }
/* Set each UART in FIFO mode */
                if( (asy->asy_flags & ASY82510) == 0 ) {
                        outb(asy->asy_isr, 0x00);   /* clear fifo registers */
                        outb(asy->asy_isr,FIFOEN);     /* fifo trigger set */
                }
	}
}

/*
 * Watchdog timer handler.
 */
asytmout(notused)
int notused; /* dummy variable that callout will supply */
{
	time_t diff, asytime;
	register int    dev;
	int oldpri;

	for (dev=0; dev < num_asy; dev++) {
		drv_getparm(TIME,&asytime);
		if ((diff = asytime-asylast_time[dev]) > MAXTIME &&
		    diff <= MAXTIME+1)  {
			/* assume interrupt was missed & COM has crashed */	
				asyparam(dev); 
				asyxintr(dev);
		}
	}
	asytmrun = 1;
	timeout(asytmout, &dummy, HZ);
}

/*
 * This procedure programs the baud rate generator.
 */
asyT_prog(dev, speed)
int	dev;
ushort	speed;

{
	ushort y;
	unsigned char  x;
	register struct     asy *asy = &asytab[dev];

	x = inb(asy->asy_lcr);
	outb(asy->asy_lcr, x|DLAB);
	y = asyspdtab[speed];
	outb(asy->asy_dat, y & 0xff);
	outb(asy->asy_icr, y >>8);
	outb(asy->asy_lcr, x);	

}

/*
 * This procedure does the initial reset on an COM USART.
 */
asyU_reset(dev)
int	dev;

{
	register struct     asy *asy = &asytab[dev];
	unsigned char	flush_regs;
	struct strtty *tp;

	flush_regs = inb(asy->asy_isr);
	flush_regs = inb(asy->asy_lsr);
	flush_regs = inb(asy->asy_msr);
	flush_regs = inb(asy->asy_dat);
	
	outb(asy->asy_lcr, DLAB);
	outb(asy->asy_dat, asyspdtab[B1200] & 0xff);		
	outb(asy->asy_icr, asyspdtab[B1200]  >> 8);		
	outb(asy->asy_lcr, BITS8); 
	outb(asy->asy_mcr, DTR|RTS|OUT2);	
	tp = &asy_tty[dev];
	tp->t_state &= ~BUSY;
}

int	asynrerr;	/* number of receive errors seen */

/*
 * called for Modem Status register changes while in DOSMODE.
 */
asymodem(dev)
int dev;
{
	register struct strtty *tp;
	register int msr;
	register struct msgb	*bp;
	struct asy *asyp = &asytab[asychan(dev)];
	extern asyhangup();
	extern  int     validproc();

 	msr = inb(asyp->asy_msr);    /* this resets the interrupt */
	tp = &asy_tty[dev];

	if (asyp->asy_flags & ASY82510 ) {
		outb(asyp->asy_msr,msr & 0xF0);
	}

	if(!(tp->t_state & (ISOPEN|WOPEN))) 
		return;

 	if ( ( asyp->asy_flags & HWDEV) ) {
 		if( ( msr & CTS ) ) {
			asyproc(tp, T_RESUME);
 			asyp->asy_flags |= HWFLWO ;
 		} else {
			asyproc(tp, T_SUSPEND);
 			asyp->asy_flags &= ~HWFLWO ;
 
		}
	}

#ifdef VPIX
	if ((tp->t_iflag & DOSMODE) && (asyintmask[dev] != V86VI_KBD))
	{
		if (tp->t_iflag & PARMRK) {
			short c = (msr | (asyp->asy_dev<<8) | MSRWORD);
			if (asyrflag[dev] && space[dev]) {
				untimeout(asyrflag[dev]);
				asyrflag[dev] = 0;
			}
			if (space[dev]) 
				asyrflag[dev] = timeout(asyrtmout, dev, HZ/30);

			if (ringcnt != RINGSIZ) {
				*ringput++ = c;
				if (ringput >= &ringbuf[RINGSIZ])
					ringput = ringbuf;
				ringcnt++;
			} else
				asydchars++;

			if (!asyiflag[dev])
				asyiflag[dev]=timeout(asyrproc, dev, HZ/60);
		}
		if (asystash[tp->t_dev])
			if( validproc(asy_v86_prp_pd[2*asychan(dev)], asy_v86_prp_pd[(2*asychan(dev)) + 1]))
				v86setint(asystash[dev], asyintmask[dev]);
			else {
				tp->t_iflag &= ~DOSMODE;
				asystash[asychan(dev)] = 0;
				asyintmask[asychan(dev)] = 0;
				asy_v86_prp_pd[2*asychan(dev)] = 0;
				asy_v86_prp_pd[(2*asychan(dev)) + 1] = 0;
			}
	}
	if (((tp->t_iflag & DOSMODE) && (asyintmask[tp->t_dev] == V86VI_KBD)) ||
						 !(tp->t_cflag & CLOCAL))
	{
#else
        if (!(tp->t_cflag & CLOCAL))
        {
#endif
		if (msr & DCD)  {
                        if (!(tp->t_state & CARR_ON))  {
                                wakeup((caddr_t)&tp->t_rdqp);
                                tp->t_state |= CARR_ON;
                        }
                } else {
                        if ((tp->t_state & CARR_ON) &&
                              (tp->t_state & ISOPEN))
                                {      
					if((asyoflag[tp->t_dev])) { 
					/* Clear the WIOC flag to prevent 
					  messaging processing HANGS */
						tp->t_state &= ~WIOC;
						untimeout(asyoflag[dev]);
					}
					asyoflag[tp->t_dev]=timeout(asyhangup, tp, 1);
                                }
                                tp->t_state &= ~CARR_ON;
               	}
	}
}



/*
 * debugger/console support routines.
 */

/*
 * put a character out the first serial port.
 * Do not use interrupts.  If char is LF, put out LF, CR.
 */
asyputchar(c)
unsigned char	c;
{
	if (! asyinitialized)
		asyinit();
	if (inb(p_asy0+ISR) & 0x38)
		return;
	while((inb(p_asy0+LSR) & XHRE) == 0) /* wait for xmit to finish */
	{
		if ((inb(p_asy0+MSR) & DCD ) == 0)
			return;
		drv_usecwait(10);
	}
	outb(p_asy0+DAT, c); /* put the character out */
	if (c == '\n')
		asyputchar(0x0d);
}

/*
 * get a character from the first serial port.
 *
 * If no character is available, return -1.
 * Run in polled mode, no interrupts.
 */

int
asygetchar()
{
	if ((inb(p_asy0+ISR) & 0x38) || (inb(p_asy0+LSR) & RCA) == 0) {
		drv_usecwait(10);
		return -1;
	}
	return inb(p_asy0+DAT);
}

int
asydbginit()
{
}

asyosrv(q)

	queue_t *q;
{
	return;
}

asydelay(tp)
register struct strtty *tp;

{
	int s;

	s=spltty();
	tp->t_state &= ~TIMEOUT;
	asyproc(tp, T_OUTPUT);
	splx(s);
}

asygettim(tp)
register struct strtty *tp;
{
	mblk_t *bp;

	if (tp->t_out.bu_bp) {
		bp = tp->t_out.bu_bp ;
		tp->t_out.bu_bp = 0;
		freemsg(bp);
	}
	tp->t_state &= ~WIOC; 	/*Using the WIOC flag to prevent*/
	asygetoblk(tp); 
	asyoflag[tp->t_dev] = 0;
}

asyhangup(tp)
register struct strtty *tp;
{
	if( tp->t_rdqp != NULL ) {
               	asyflush(tp, FREAD|FWRITE);
		putctl(tp->t_rdqp->q_next, M_HANGUP);
	}
	asyoflag[tp->t_dev] = 0;
}
