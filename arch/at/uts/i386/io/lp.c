/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/lp.c	1.3"

/*
 *      LP (Line Printer) Driver        EUC handling version
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/lp.h"
#include "sys/dma.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/termio.h"
#include "sys/termios.h"
#include "sys/cmn_err.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/debug.h"
#include "sys/eucioctl.h"
#include "sys/ddi.h"
#include "sys/cred.h"
#include "sys/uio.h"


extern  struct strtty lp_tty[];   /* tty structs for each device */
extern  time_t  last_time[];     /* output char watchdog timeout */
extern  struct lpcfg lpcfg[]; 
extern  int     NUMLP;
                                

#define MAXTIME         2               /* 2 sec */
int             lpdevflag=0;
extern time_t           last_time[];    /* output char watchdog timeout */
static char             lptmrun=0;      /* timer set up? */
static int              dummy;          /* placeholder for callout timer */

#ifdef MERGE386

extern  int     merge386enable;

#endif /* MERGE386 */

/* extern  debugger();  -- comment in to use kernel debug on lpinit() */



int lpflag=0;
int timeflag=0;
int lpopen(), lpclose(), lpoput(), lpgetoblk();
int lpisrv(), lpproc(), lprstrt(), lpdelay();

struct module_info lp_info = { 
/* id, name, min pkt siz, max pkt siz, hi water, low water */
        42, "lp", 0, INFPSZ, 256, 128};
static struct qinit lp_rint = { 
        putq, NULL, lpopen, lpclose, NULL, &lp_info, NULL};
static struct qinit lp_wint = { 
        lpoput, NULL, lpopen, lpclose, NULL, &lp_info, NULL};
struct streamtab lpinfo = { 
        &lp_rint, &lp_wint, NULL, NULL};


lpopen(q, devp, flag, sflag, cred_p) 
queue_t *q;
dev_t *devp;
register flag;
register sflag;
struct cred *cred_p;

{
        register struct strtty *tp;
        register int    oldpri;
        mblk_t *mop;
        minor_t dev;
        int tmp;

#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPOPEN");
#endif
        dev = getminor(*devp);

        if ((lpcfg[dev].flag & LPPRES) == 0)
                return(ENXIO);

#ifdef MERGE386  /* Provide exclusive access to device for MERGE */

        if(merge386enable) {
                oldpri = SPL();
                if (( lpcfg[dev].flag & OPEN) == 0 )
                        if( !portalloc(lpcfg[dev].data, lpcfg[dev].control)) {
                                splx(oldpri);
                                return(EBUSY);
                        }
                splx(oldpri); 
        }

#endif /* MERGE386 */

        if (lpcfg[dev].flag & OPEN)
                return(EBUSY);
        else
                lpcfg[dev].flag |= OPEN;

        tp = &lp_tty[dev];
        q->q_ptr = (caddr_t) tp;
        WR(q)->q_ptr = (caddr_t) tp;
        tp->t_rdqp = q;
        tp->t_dev = dev;

        if (mop = allocb(sizeof(struct stroptions), BPRI_MED)) {
                register struct stroptions *sop;

                mop->b_datap->db_type = M_SETOPTS;
                mop->b_wptr += sizeof(struct stroptions);
                sop = (struct stroptions *)mop->b_rptr;
                sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
                sop->so_hiwat = 512;
                sop->so_lowat = 256;
                putnext(q, mop);
        } else 
                return(EAGAIN);

        if ((tp->t_state & (ISOPEN | WOPEN)) == 0) {
                tp->t_iflag = IGNPAR;
                tp->t_cflag = B300|CS8|CLOCAL;
                oldpri = SPL();
                if (!lptmrun) {
                        last_time[dev] = 0x7fffffffL;
                        lptmout(dummy);
                }
                splx(oldpri); 
        }

        oldpri = SPL();

        tp->t_state |=  CARR_ON;

        tp->t_state &= ~WOPEN;
        tp->t_state |= ISOPEN;

        splx(oldpri);
        return(0);
}


lpclose(q, flag, cred_p)
queue_t *q;
register flag;
cred_t *cred_p;
{
        register struct strtty *tp;
        register int    oldpri;
        int dev;

#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPCLOSE");
#endif
        tp = (struct strtty *)q->q_ptr;
        dev = tp - lp_tty;


        if(!(tp->t_state & ISOPEN)) {  /* See if it's closed already */
                return(0);
        }

        if ( !( flag & (FNDELAY|FNONBLOCK))) {
                /* Drain queued output to the printer. */
                oldpri = spltty();
                while (( tp->t_state & CARR_ON) ){
                        if((tp->t_out.bu_bp == 0 ) && (WR(q)->q_first == NULL)) 
                                break;
                        tp->t_state |= TTIOW;
                        if( sleep((caddr_t) &tp->t_oflag,TTIPRI|PCATCH)){
                                tp->t_state &= ~(TTIOW|CARR_ON);
                                break;
                        }
                }
                splx(oldpri);
        }

        /* do not >>> outb(lpcfg[tp->t_dev].control, 0) -- because 
        close() gets called before all characters are sent, therefore, 
        the last chars do not get output with the interrupt turned off */ 

        untimeout(timeflag);  
        lptmrun = 0;
        lpcfg[dev].flag &= ~OPEN;
        tp->t_state &= ~(ISOPEN|CARR_ON);
#ifdef MERGE386

        if(merge386enable)
                portfree(lpcfg[dev].data, lpcfg[dev].control);

#endif /* MERGE386 */

        tp->t_rdqp = NULL;
        q->q_ptr = WR(q)->q_ptr = NULL;
}


lpoput(q, bp)
queue_t *q;
mblk_t *bp;
{
        register struct msgb *bp1;
        register struct strtty *tp;
        int s;

#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPOPUT");
#endif
        tp = (struct strtty *)q->q_ptr;

        switch(bp->b_datap->db_type) {

        case M_DATA:
                s = spltty();
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
                        lpgetoblk(tp);
                }
                break;

        case M_IOCTL:
                lpputioc(q, bp);
                if (q->q_first != NULL) {
                        lpgetoblk(tp);
                }
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
                timeout ( lpdelay, tp, ((int)*(bp->b_rptr))*HZ/60);
                splx (s);
                freemsg (bp);
                break;

        case M_FLUSH:
                s = SPL();
                switch( *(bp->b_rptr) ) {

                case FLUSHRW:
                        lpflush(tp, (FREAD|FWRITE));
                        *(bp->b_rptr) = FLUSHR;
                        qreply(q, bp);
                        break;

                case FLUSHR:
                        lpflush(tp, FREAD);
                        qreply(q, bp);
                        break;

                case FLUSHW:
                        lpflush(tp, FWRITE);
                        freemsg(bp);
                        break;

                default:
                        break;
                }
                splx(s);
                break;

        case M_START:
                s = SPL();
                lpproc(tp, T_RESUME);
                splx(s);
                freemsg(bp);
                break;

        case M_STOP:
                s = SPL();
                lpproc(tp, T_SUSPEND);
                splx(s);
                freemsg(bp);
                break;

        default:
                freemsg(bp);
                break;
        }
}

lpgetoblk(tp)
register struct strtty *tp;
{
        register int s;
        register struct queue *q;
        unsigned char chan, sr;
        unsigned int addr;
        register struct msgb    *bp;

#ifdef DEBUG
cmn_err(CE_NOTE,"in lpgetoblk");
#endif
        if (tp->t_rdqp == NULL) {
                return(0);
        }
        q = WR(tp->t_rdqp);

        s = SPL();

        while (!(tp->t_state & BUSY) && (bp = getq(q)) != NULL) {
                
                switch (bp->b_datap->db_type) {

                case M_DATA:
                        if (tp->t_state & (TTSTOP | TIMEOUT)) {
                                putbq(q, bp);
                                splx(s);
                                return(0);
                        }

                        /* start output processing for bp */
                        tp->t_out.bu_bp = bp;
                        lpproc(tp, T_OUTPUT);
                        break;

                case M_DELAY:
                        if(tp->t_state & TIMEOUT ) {
                                putbq( q, bp );
                                splx(s);
                                return;
                        }
                        tp->t_state |= TIMEOUT;
                        timeout ( lpdelay, tp, ((int)*(bp->b_rptr))*HZ/60);
                        freemsg (bp);
                        break;

                case M_IOCTL:
                        lpsrvioc(q, bp);
                        break;

                default:
                        freemsg(bp);
                        break;
                }
        }
        /* Wakeup any process sleeping waiting for drain to complete */
        if (( tp->t_out.bu_bp == 0 ) && (tp->t_state & TTIOW)) {
                tp->t_state &= ~(TTIOW);
                wakeup((caddr_t) &tp->t_oflag);
        }  
        splx(s);
        return(0);
}

/*
 * ioctl handler for output PUT procedure
 */
lpputioc(q, bp)
queue_t *q;
mblk_t *bp;
{
        struct strtty *tp;
        struct iocblk *iocbp;
        struct termio  *cb;
        struct termios *scb;
        mblk_t *bpr;
        mblk_t *bp1;
        int s, arg;

        iocbp = (struct iocblk *)bp->b_rptr;
        tp = (struct strtty *)q->q_ptr;

        /* Only called for M_IOCTL messages. */
#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPPUTIOC ioctl=%x", iocbp->ioc_cmd);
#endif

        switch( iocbp->ioc_cmd ) {

        case TCSBRK:
        case TCSETAW:
        case TCSETSW:
        case TCSETSF:
        case TCSETAF:/* run these now, if possible */

                if( q->q_first != NULL || (tp->t_state & (BUSY|TIMEOUT) ) 
                        || (tp->t_out.bu_bp != NULL)) 
                {
                        putq(q, bp);
                        break;
                }
                lpsrvioc(q, bp);
                break;

        case TCSETS:
        case TCSETA:    /* immediate parm set   */

                if ( tp->t_state & BUSY) {
                        putbq( q, bp); /* queue these for later */
                        break;
                }

                lpsrvioc(q, bp);
                break;

        case TCGETS:
        case TCGETA:    /* immediate parm retrieve */
                lpsrvioc(q, bp);
                break;

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
                if ((iocbp->ioc_cmd&IOCTYPE) == LDIOC) {
                        bp->b_datap->db_type = M_IOCACK; /* ignore LDIOC cmds */
                        bp1 = unlinkb(bp);
                        if (bp1) {
                                freeb(bp1);
                        }
                        iocbp->ioc_count = 0;
                } else {
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
lpsrvioc(q, bp)
queue_t *q;
mblk_t *bp;
{
        struct strtty *tp;
        struct iocblk *iocbp;
        struct termio  *cb;
        struct termios *scb;
        int arg;
        mblk_t *bpr;
        mblk_t *bp1;

        iocbp = (struct iocblk *)bp->b_rptr;
        tp = (struct strtty *)q->q_ptr;
#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPSRVIOC ioctl=%x", iocbp->ioc_cmd); 
#endif
        switch( iocbp->ioc_cmd ) {

        case TCSETSF: /* The output has drained now. */
                lpflush(tp, FREAD); /* fall thru .. */

        /* (couldn't get block before...) */

        case TCSETS: 
        case TCSETSW:
                if ( !bp->b_cont) {
                        iocbp->ioc_error = EINVAL;
                        bp->b_datap->db_type = M_IOCNAK;
                        iocbp->ioc_count = 0;
                        qreply(q, bp);
                        break;
                }

                scb = (struct termios *)bp->b_cont->b_rptr;
                tp->t_cflag = scb->c_cflag;
                tp->t_iflag = scb->c_iflag;
                bp->b_datap->db_type = M_IOCACK;
                bp1 = unlinkb(bp);
                if (bp1) {
                        freeb(bp1);
                }
                iocbp->ioc_count = 0;
                qreply(q, bp);
                break;

        case TCSETAF: /* The output has drained now. */
                lpflush(tp, FREAD); /* fall thru .. */

        case TCSETA: 
        case TCSETAW:
                if ( !bp->b_cont) {
                        iocbp->ioc_error = EINVAL;
                        bp->b_datap->db_type = M_IOCNAK;
                        iocbp->ioc_count = 0;
                        qreply(q, bp);
                        break;
                }

                cb = (struct termio *)bp->b_cont->b_rptr;
                tp->t_cflag = cb->c_cflag;
                tp->t_iflag = cb->c_iflag;
                bp->b_datap->db_type = M_IOCACK;
                bp1 = unlinkb(bp);
                if (bp1) {
                        freeb(bp1);
                }
                iocbp->ioc_count = 0;
                qreply(q, bp);
                break;

        case TCGETS:    /* immediate parm retrieve */
                if ( bp->b_cont )
                        freemsg(bp->b_cont);

                if( (bpr = allocb(sizeof(struct termios),BPRI_MED)) == NULL ) {
                        ASSERT(bp->b_next == NULL);
                        putbq(q, bp);
                        bufcall((ushort)sizeof(struct termios), BPRI_MED,
                                        lpgetoblk,tp);
                        return(0);
                }
                bp->b_cont = bpr;

                scb = (struct termios *)bp->b_cont->b_rptr;

                scb->c_iflag = tp->t_iflag;
                scb->c_cflag = tp->t_cflag;

                bp->b_cont->b_wptr += sizeof(struct termios);
                bp->b_datap->db_type = M_IOCACK;
                iocbp->ioc_count = sizeof(struct termios);
                qreply(q, bp);
                break;

        case TCGETA:    /* immediate parm retrieve */
                if ( bp->b_cont )
                        freemsg(bp); /* bad user supplied parameter */

                if( (bpr = allocb(sizeof(struct termio),BPRI_MED)) == NULL ) {
                        ASSERT(bp->b_next == NULL);
                        putbq(q, bp);
                        bufcall((ushort)sizeof(struct termio), BPRI_MED,
                                        lpgetoblk,tp);
                        return(0);
                }
                bp->b_cont = bpr;
                cb = (struct termio *)bp->b_cont->b_rptr;

                cb->c_iflag = tp->t_iflag;
                cb->c_cflag = tp->t_cflag;

                bp->b_cont->b_wptr += sizeof(struct termio);
                bp->b_datap->db_type = M_IOCACK;
                iocbp->ioc_count = sizeof(struct termio);
                qreply(q, bp);
                break;

        case TCSBRK:
                /* Skip the break since it's a parallel port. */
                arg = *(int *)bp->b_cont->b_rptr;
                bp->b_datap->db_type = M_IOCACK;
                bp1 = unlinkb(bp);
                if (bp1) {
                        freeb(bp1);
                }
                iocbp->ioc_count = 0;
                qreply(q, bp);
                break;

        case EUC_MSAVE: /* put these here just in case... */
        case EUC_MREST:
        case EUC_IXLOFF:
        case EUC_IXLON:
        case EUC_OXLOFF:
        case EUC_OXLON: 
                bp->b_datap->db_type = M_IOCACK;
                iocbp->ioc_count = 0;
                qreply(q, bp);
                break;

        default: /* unexpected ioctl type */
                if( canput(RD(q)->q_next) == 1 ) {
                        bp->b_datap->db_type = M_IOCNAK;
                	iocbp->ioc_count = 0;
                        qreply(q, bp);
                } else {
                        putbq(q, bp);
                }
                break;
        }
        return(0);
}

lpflush(tp, cmd)
struct strtty *tp;
{
        queue_t *q;
        int s;

        s = SPL();
#ifdef DEBUG
        cmn_err(CE_NOTE,"IN LPFLUSH");
#endif
        if (cmd&FWRITE) {
                q = WR(tp->t_rdqp);
                /* Discard all messages on the output queue. */
                flushq(q,FLUSHDATA);
                tp->t_state &= ~(BUSY);
                tp->t_state &= ~(TBLOCK);
                if (tp->t_state & TTIOW) {
                        tp->t_state &= ~(TTIOW);
                        wakeup((caddr_t) &tp->t_oflag);
                }

        }
        if (cmd&FREAD) {
                tp->t_state &= ~(BUSY);
        }
        splx(s);
        lpgetoblk(tp);

}


/*
 * lpintr is the entry point for all interrupts. 
 */
lpintr(vec)
unsigned int    vec;
{
        register        mdev;
        register unsigned char  status;

#ifdef DEBUG
cmn_err(CE_NOTE,"in lpintr");
#endif
        for (mdev=0; mdev <= NUMLP; mdev++) 
                if( (lpcfg[mdev].vect == vec) && lpcfg[mdev].flag) 
                        break;
                
        if (mdev > NUMLP)
                return(ENXIO);

        status = inb(lpcfg[mdev].status);

        if ( status & NOPAPER )
                return(1);

        if ( status & UNBUSY  ) 
                lpxintr(mdev);
}

/*
 * This is logically a part of lpintr.  This code
 * handles transmit buffer empty interrupts, 
 * It works in  conjunction with lptmout() to insure that lost 
 * interrupts don't hang  the driver:  
 * if a char is xmitted and we go more than 2s (MAXTIME) without
 * an interrupt, lptmout will supply it.
 */
lpxintr(mdev)
register int    mdev;
{
        struct strtty      *tp;
        char            c;

#ifdef DEBUG
cmn_err(CE_NOTE,"in lpxintr");
#endif
        last_time[mdev] = 0x7fffffffL;  /* don't time out */
        tp = &lp_tty[mdev];

        if (tp->t_state & BUSY) {
                tp->t_state &= ~BUSY;
                lpproc( tp, T_OUTPUT );

                /* if output didn't start get a new message */
                if (!(tp->t_state & BUSY)) {
                        if (tp->t_out.bu_bp) {
                                freemsg(tp->t_out.bu_bp);
                        }
                        tp->t_out.bu_bp = 0;
                        lpgetoblk(tp);
                }
        }
}

/*
 *      General command routine that performs device specific operations for
 * generic i/o commands.  All commands are performed with tty level interrupts
 * disabled.
 */
lpproc(tp, cmd)
struct strtty   *tp;
int             cmd;
{
        char            c;      /* char to process      */
        int             dev, i;
        register int    oldpri;
        register struct msgb *bp;

        
        oldpri = SPL();
        /*
         * get device number and control port
         */
        dev = tp - lp_tty;

#ifdef DEBUG
        cmn_err(CE_NOTE,"in lpproc cmd=%x", cmd);
#endif
        /*
         * based on cmd, do various things to the device
         */

        switch (cmd) {

        case T_TIME:            /* stop sending a break -- disabled for LP */
                goto start;

        case T_RESUME:          /* enable output        */

                tp->t_state &= ~TTSTOP;
                /* fall through */

        case T_OUTPUT:          /* do some output       */
start:

                /* If we are busy, do nothing */
                if ( tp->t_state & ( BUSY|TTSTOP|TIMEOUT ) ) break;

                /*
                 * Check for characters ready to be output.
                 * If there are any, ship one out.
                 */
                bp = tp->t_out.bu_bp;
                if (bp == NULL || bp->b_wptr <= bp->b_rptr) {
                        if (tp->t_out.bu_bp) {
                                freemsg(tp->t_out.bu_bp);
                        }
                        tp->t_out.bu_bp = 0;
                        lpgetoblk(tp);
                        break;
                }

                /* specify busy, and output a char */
                tp->t_state |= BUSY;
                /* reset the time so we can catch a missed interrupt */
                outb(lpcfg[dev].data, *bp->b_rptr++ );
                outb(lpcfg[dev].control, SEL|RESET);
                outb(lpcfg[dev].control, SEL|RESET|STROBE);
                drv_usecwait(10);
                outb(lpcfg[dev].control, SEL|RESET|INTR_ON);
                drv_getparm(TIME, &last_time[dev]);

                break;

        case T_SUSPEND:         /* block on output      */

                tp->t_state |= TTSTOP;
                break;

        case T_BREAK:           /* send a break -- disabled for LP    */
                break;
        }
        splx(oldpri);
}

/*
 * Initialization code that the kernel runs when coming up.  Detect
 * any LPs we see hanging around.
 */
lpinit()
{
        register ushort testval;
        int                 dev;
        char            b_type;
        int             dummyarg, x;

        for (dev=0; dev < NUMLP; dev++) {
                /* Probe for the board. */
                outb(lpcfg[dev].data, 0x55);
                testval = ((short)inb(lpcfg[dev].data) & 0xFF) ;

                if (testval != 0x55)
                        continue;
                        
                /* We found a live one 
                 * Program the device to be benign.
                 */
                outb(lpcfg[dev].control, SEL);
                drv_usecwait(750);
                outb(lpcfg[dev].control, SEL|RESET);    
                lpcfg[dev].flag = LPPRES; /* controller present */

        }
}

/*
 * Watchdog timer handler.
 */
lptmout(notused)
int notused; /* dummy variable that callout will supply */
{
        time_t diff, lptime;
        register int    dev;
        register int    oldpri;
        unsigned char flush_reg;

#ifdef DEBUG
cmn_err(CE_NOTE,"in lptmout");
#endif

        for (dev=0; dev <= NUMLP; dev++) {
                drv_getparm(TIME, &lptime);
                if ((diff = lptime-last_time[dev]) > MAXTIME &&
                    diff <= MAXTIME+1)  {
                        oldpri = SPL();
                        lpxintr(dev);
                        splx(oldpri);
                }
        }
        lptmrun = 1;
        timeflag = timeout(lptmout, &dummy, drv_usectohz(1000000));
}


lpdelay(tp)
register struct strtty *tp;

{
        int s;

        s=spltty();
        tp->t_state &= ~TIMEOUT;
        lpproc(tp, T_OUTPUT);
        splx(s);
}
