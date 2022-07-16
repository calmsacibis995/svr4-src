#ident	"@(#)driver.c	1.6	92/06/28	JPB"
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)qt:io/driver.c	1.4.2.2"

#include <sys/param.h>
#include <sys/sysmacros.h>
#include "qt.h"
#include <sys/dma.h>
#include <sys/i8237A.h>
#include "globals.h"

#ifdef DEBUG
#include "debug.h"
#else
#define msinb(x)	inb (x)
#define msoutb(x,y)	outb(x,y)
#define RETURN(x,y)	return (y)
#endif

struct dma_cb	*qtcb;

qt_breakup(bp)
struct buf	*bp;
{
	void qt_strategy();
	dma_breakup(qt_strategy, bp);
}
/*
NAME
	qtinit

DESCRIPTION
	Qtinit sets necessary constants and sets pointers so that 
	internal buffers do not contain 64K boundaries since the DMA
	chip won't work across them.
*/

qtinit ()
{
	register  short	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e01");
#endif

	for (i = 0; i < SPACEWORDS; space[i++] = (ALIGN) 0)	/* Clear the static buffers */
		;
	
	if (isboundary(first3rd())) {
		iqt = (struct iqt *) first3rd();
		iqt->vbuffer = (caddr_t)second3rd();
	} else if (isboundary(second3rd())) {
		iqt = (struct iqt *) second3rd();
		iqt->vbuffer = (caddr_t)first3rd();
	} else {
		iqt = (struct iqt *) third3rd();
		iqt->vbuffer = (caddr_t)first3rd();
	}
	iqt->pbuffer = kvtophys ((paddr_t)iqt->vbuffer);
	iqt->vbufferp = &iqt->vbuffer[BLOCKSIZE];
	zeroq();
	board_type = qt_tryctlr(qt_0_sioa);
	if (board_type == BT_UNKNOWN)
		printf("\nWARNING: Cartridge controller was not found.\n");
	else {
		printf("at addr %xH intr %d dma %d.\n",
		       qt_0_sioa, qt_0_vect, qt_chan);
	}
	dma_init();
	qtcb = dma_get_cb(DMA_NOSLEEP);
	qtcb->targbufs = dma_get_buf(DMA_NOSLEEP);
	qtcb->targ_step = DMA_STEP_INC;
	qtcb->targ_path = DMA_PATH_8;
	qtcb->trans_type = DMA_TRANS_SNGL;
	qtcb->targ_type = DMA_TYPE_IO;
	qtcb->bufprocess = DMA_BUF_SNGL;
}

qt_tryctlr(qt_sioa)
	int qt_sioa;
{
	/* Test for which controller is installed */
	/*								*/
	/*	Makers			60MB		150MB		*/
	/*	---------------		-------		-------		*/
	/*	Wangtek/Everex		PC-36		EV-811 		*/
	/*	Archive			SC499-R		SC402		*/

	sel_wangtek(qt_sioa);	/* Try Wangtek PC-36/EV-811 first	*/
	if (qt_check ()) {
		printf("Wangtek PC-36/EV-811 cartridge tape controller ");
		return(BT_WANGTEK);
	}
	sel_archive(qt_sioa);/* No PC-36 or equiv, try Archive SC499R*/
	if (qt_check ()) {
		msoutb(RSTDMA,01);
		printf("Archive SC499-R/VP402 cartridge tape controller ");
		return(BT_ARCHIVE);
	}
	return(BT_UNKNOWN);
}

static int	
qt_check ()
{
	char	stat_buf[6];
	register short	spl, i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e01");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	clrflags(F_PROGRESS | F_ACTIVE | F_LCMDR | F_LCMDW);
	if (board_type == BT_WANGTEK)
		deassert(CTL_ONLINE);
	assert(CTL_RESET);
	waitloop(TDELTA25);
	deassert(CTL_RESET);
	
	waitloop(100000);	/* Wait one second	*/
	if (board_type == BT_ARCHIVE) {   /* Archive SC499-R POR takes 5 sec */
		waitloop(400000);	/* Wait four more seconds	*/
	}
	if (!qt_rdstatus (stat_buf) || !ispor(stat_buf)) 
		RETURN(1, (FAIL));
	RETURN(1, (SUCCESS));
}


/*
NAME
	qt_timeout

SYNOPSIS
	static  short
	qt_timeout ()

DESCRIPTION
	This routine's purpose is to kill a pending request if it gets hung.
*/

static  short	
qt_timeout ()
{
	register unsigned  short	kill_it;
	short spl;
	register time_t elapsed;
	time_t our_lbolt, our_start_time;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e02");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	our_start_time = iqt->start_time;
	our_lbolt = lbolt;
	if (isset(F_PROGRESS)) {
		elapsed = our_lbolt - our_start_time;
		kill_it = FALSE;
		/* Determine whether to kill the current operation or set a new reseed time */
		switch (topq()) {
		case A_RDFILEM:
			kill_check(10 * TM_1T);
			break;
		case A_RETENSION:
		case A_ERASE:
			kill_check(3 * TM_1T);
			break;
		case A_REWIND:
			kill_check(TM_1T);
			break;
		case A_WRDATA:
		case A_RDDATA:
		case A_WRFILEM:
		case A_FLUSH:
			kill_check(TM_1T + TM_1B);
			break;
		case A_SBF:
			kill_check(TM_1T + TM_1B * iqt->sfarg);
			break;
		}
		if (kill_it) {	/* Abort the current operation */
			if (topq() == A_RDDATA ||
			    topq() == A_WRDATA ||
			    topq() == A_FLUSH ||
			    topq() == A_SBF)
				d37A_dma_disable(qt_chan);
			deassert(CTL_INTRENA);
			iqt->uexit = I_ENACINT;
			error_proc (EIO);
			if (isset(F_USLEEP)) {
				clrflags(F_USLEEP);
				wakeup (iqt->sleep);
			} else if (isset(F_OPENED))
				assert(CTL_INTRENA);
		}
	}
	if (isset(F_PROGRESS | F_OPENED))	/* Issue reseed call if necessary */
		timeout (qt_timeout, (caddr_t) 2, TMOPERIOD);
	else
		clrflags(F_TMOON);
	return;
}


/*
NAME
	qtopen

DESCRIPTION
	Qtopen opens the minor device dev for mode oflag.
*/

qtopen (dev, oflag, otyp)
register  short	dev;
register  short	oflag;
int	otyp;
{
	char	stat_buf[6];
	register  short	spl, max_retry = 10;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e03");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	if (board_type == BT_UNKNOWN) {	 /* Quit immediately if no controller */
		u.u_error = ENXIO;
		return;
	}
	if (minor(dev) & ~(MD_RET | MD_REW)) {		/* Validate minor device */
		u.u_error = ENXIO;
		return;
	}
	if (oflag & FAPPEND) {			/* Ensure append not requested */
		u.u_error = EINVAL;
		return;
	}
	splon();	/* Ensure this is the only open on the device */
	if (isset(F_OPENED)) {
		sploff();
		u.u_error = EBUSY;
		return;
	}
	setflags(F_OPENED | F_EXCLUSIVE);
	sploff();

	qt_inprog (F_OPENED | F_EXCLUSIVE);	/* Sleep if operation in progress */
	if (isclr(F_TMOON)) {
		setflags(F_TMOON);
		timeout (qt_timeout, (caddr_t) 1, TMOPERIOD);
	}
	/* Clear tape status register */
	while (!qt_rdstatus(stat_buf)) {
		if (!max_retry--) {
			clrflags(F_OPENED | F_EXCLUSIVE);
			u.u_error = EIO;
			return;
		}
		delay(2 * HZ);
	}

	if( stat_buf[ 0 ] & 0x40 ) {	/* Cartridge not in place! */
		qt_rdstatus(stat_buf);	/* for Archive 2150L drive */
	}

	if( stat_buf[ 0 ] & 0x40 ) {	/* Cartridge not in place */
		clrflags(F_OPENED | F_EXCLUSIVE);
		u.u_error = EIO;
		return;
	}
 
 	if((oflag & FWRITE ) && stat_buf[ 0 ] & 0x10) {	/* Write Protected */
 		clrflags(F_OPENED | F_EXCLUSIVE);
 		u.u_error = EACCES;
 		return;
 	}

	clrflags(F_ERROCC | F_ACTIVE | F_FFMR);
	zeroq();
	if (dev & MD_RET) {	/* Issue retension if requested */
		nq(A_RETENSION)
		iqt->sleep = (caddr_t) (oflag & FNDELAY ? 0 : iqt);
		qt_start ();
	}
	if (!ctl_exit ()) {
		deassert(CTL_INTRENA);
		u.u_error = iqt->error;
		clrflags(F_OPENED);
	}
	clrflags(F_EXCLUSIVE);
	return;
}


/*
NAME
	qtclose

SYNOPSIS
	qtclose (dev, oflag, otyp)
	short dev;
	int	oflag, otyp;

DESCRIPTION
	Qtclose closes the minor device dev by flushing the buffer, 
	writing tape marks, rewinding, or reading a tape mark as necessary.
*/

qtclose (dev)
register  short	dev;
{
	register  short	spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e04");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	/* Queue flush, write file marks, rewind, and read file mark as appropriate */
	splon();
	if (isclr(F_PROGRESS)) {
		zeroq();
		clrflags(F_ERROCC);
	}
	if (isset(F_LOPW)) {
		flush_check();
		ffmw_check();
		if (dev & MD_REW) {
			sfmw_check();
			nq(A_REWIND)
		}
	} else if (dev & MD_REW) {
		nq(A_REWIND)
	} else if (isset(F_READ) && isclr(F_EOM))  {
		ffmr_check();
	}
	/* Reset internal flags */
	clrflags(F_READ | F_FFMR | F_LOPW | F_FFMW | F_SFMW | F_EOFR | F_EOM);
	/* Start actions if necessary */
	iqt->sleep = (caddr_t) 0;
	if (isset(F_PROGRESS))
		setflags(F_INTERRUPT);
	else {
		deassert(CTL_INTRENA);
		if (topq()) {
			iqt->uexit = I_ENACINT;
			qt_start ();
			if (!ctl_exit ()) {
				u.u_error = iqt->error;
			}
		}
	}
	clrflags(F_OPENED | F_EXCLUSIVE);
	sploff();
	return;
}


qtioctl (dev, cmd, arg, mode)
int	dev;
register short	cmd;
char	*arg;
int	mode; 
{ 
	char	stat_buf[6];
	register short	spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e05");
	if (debug_flag & DB_STRUCT)
		prtstr ();
	if (cmd == 2) {
		debug_flag = (int) arg;
		return;
	}
#endif
	excl_check();
	qt_inprog (F_EXCLUSIVE);
	zeroq();
	clrflags(F_ERROCC);
	switch (cmd) {
	case T_RST:
		clrflags(F_FFMR | F_FFMW | F_SFMW | F_LOPW | F_EOFR | F_EOM);
		if (!qt_reset ())
			u.u_error = ENXIO;
		goto done;
	case T_RDSTAT:
		splon();
		iqt->start_time = lbolt;
		sploff();
		if (qt_rdstatus (stat_buf)) {
			if (copyout (stat_buf, arg, 6))
				u.u_error = EFAULT;
		} else
			u.u_error = EIO;
		goto done;
	case T_ERASE:
		clrflags(F_FFMR | F_FFMW | F_SFMW | F_LOPW | F_EOFR | F_ACTIVE | F_EOM);
		nq(A_ERASE)
		break;
	case T_RETENSION:
	case T_LOAD:
	case T_UNLOAD:
	case T_RWD:
		if (isset(F_LOPW)) {
			flush_check();
			ffmw_check();
			sfmw_check();
		}
		clrflags(F_FFMR | F_LOPW | F_FFMW | F_SFMW | F_EOFR | F_EOM);
		nq(cmd == T_RETENSION ? A_RETENSION : A_REWIND)
		break;
	case T_WRFILEM:
		setflags(F_LOPW);
		clrflags(F_FFMR | F_EOFR);
		flush_check();
		nq(A_WRFILEM)
		if (isset(F_FFMW))
			setflags(F_SFMW);
		else
			setflags(F_FFMW);
		break;
	case T_SFF:
	case T_SBF:
		seq_check();
		if ((int) arg < 0) {
			iqt->error = EINVAL;
			setflags(F_ERROCC);
			goto done;
		}
		if (!(iqt->sfarg = (int) arg))
			goto done;
		if (isset(F_EOM)) {
			iqt->error = EIO;
			setflags(F_ERROCC);
			goto done;
		}
		if (iqt->sfarg > 0 && isset(F_EOFR)) {
			clrflags(F_EOFR | F_ACTIVE);
			if (!--iqt->sfarg)
				goto done;
		}
		clrflags(F_ACTIVE);
		nq(cmd == T_SFF ? A_RDFILEM : A_SBF)
		break;
	default:
		u.u_error = EINVAL;
		goto done;
	}
	iqt->sleep = (caddr_t) iqt;
	qt_start ();
done:
	if (!ctl_exit ())
		u.u_error = iqt->error;
	splon();
	clrflags(F_EXCLUSIVE);
	sploff();
	return;
}


static void
qt_strategy (bp)
struct buf *bp;
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e06");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	zeroq();
	if ((iqt->b = bp)->b_flags & B_READ) {	/* Queue the action */
		if (isset(F_EOFR | F_EOM)) {
			if (isset(F_FFMR)) 
				clrflags(F_READ | F_EOFR | F_EOM | F_FFMR);
			else
				setflags(F_FFMR);
			bp->b_resid = bp->b_bcount;
			goto done;
		}
		nq(A_RDDATA)
		setflags(F_READ);
	} else {
		clrflags(F_EOFR);
		nq(A_WRDATA)
		setflags(F_LOPW);
	}
	iqt->sleep = (caddr_t) iqt;
	iqt->io_count =  bp->b_bcount;
	iqt->pio_addr = kvtophys((paddr_t)(iqt->vio_addr = bp->b_un.b_addr));
	bp->b_resid = 0;
	qt_start ();
done:
	bp->b_flags |= B_DONE;
	return;
}


qtread (dev)
register short	dev;
{
	register short	spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e07");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	excl_check();
	seq_check();
	qt_inprog (F_EXCLUSIVE);
	clrflags(F_ERROCC);
	physio (qt_breakup, (struct buf *) 0, dev, B_READ);
	clrflags(F_EXCLUSIVE);
	if (!ctl_exit ())
		u.u_error = iqt->error;
	return;
}


qtwrite (dev)
register short	dev;
{
	register short	spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e08");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	excl_check();
	if (u.u_count) {
		qt_inprog (F_EXCLUSIVE);
		if (isset(F_EOM))
			u.u_error = ENOSPC;
		else {
			clrflags(F_FFMR | F_FFMW | F_SFMW | F_ERROCC);
			physio (qt_breakup, (struct buf *) 0, dev, B_WRITE);
			if (!ctl_exit ())
				u.u_error = iqt->error;
		}
	}
	splon();
	clrflags(F_EXCLUSIVE);
	sploff();
	return;
}


static int	
qt_inprog (flags)
register short	flags;
{
	register short	spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e09");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	splon();
	if (isset(F_PROGRESS)) {
		iqt->sleep = (caddr_t) iqt;
		setflags(F_USLEEP);
		while (isset(F_USLEEP))
			if (sleep (iqt->sleep, ENASIGPRI)) {
				clrflags(flags | F_USLEEP);
				iqt->sleep = (caddr_t) 0;
				sploff();
				longjmp (&u.u_qsav, 1);
			}
		iqt->sleep = (caddr_t) 0;
	}
	sploff();
	deassert(CTL_INTRENA);
	iqt->uexit = I_ENACINT;
}


static int	
ctl_exit ()
{
	register short	ret;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e10");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	switch (iqt->uexit) {
	case I_SNDCMD:
		if (isset(F_ERROCC))
			deassert(CTL_REQUEST);
		else if (rdy_wait()) {
			assert(CTL_INTRENA);
			deassert(CTL_REQUEST);
			RETURN(10, (SUCCESS));
		} else {
			setflags(F_ERROCC);
			iqt->error = EIO;
			deassert(CTL_REQUEST);
		}
		clrflags(F_PROGRESS);
		assert(CTL_INTRENA);
		RETURN(10, (FAIL));
	case I_ENACINT:
		if (!(ret = (isset(F_ERROCC) ? FAIL : SUCCESS)))
			clrflags(F_PROGRESS);
		assert(CTL_INTRENA);
		RETURN(10, ret);
	case I_ENADMA:
		if (isset(F_ERROCC)) {
			clrflags(F_PROGRESS);
			assert(CTL_INTRENA);
			RETURN(10, FAIL);
		}
		assert(CTL_INTRENA);
		tenmicrosec();
		if (board_type == BT_ARCHIVE)
			msoutb (DMAGO, 01);
		d37A_dma_enable(qt_chan);
		RETURN(10, SUCCESS);
	}
}


static int	
qt_start ()
{
	register short	priority, spl;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e11");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	priority = ENASIGPRI;
	switch (*(iqt->paction = iqt->action)) {
	case A_WRDATA:
	case A_RDDATA:
		priority = DISSIGPRI;
		qt_buffer ();
		if (!iqt->io_count )
			goto done;
	case A_FLUSH:
	case A_SBF:
		setflags(F_ISSRDWR);
	}
	execq();
	if (isclr(F_ERROCC) && isset(F_PROGRESS) && iqt->sleep) {
		splon();
		if (ctl_exit ()) {
			setflags(F_USLEEP);
			while (isset(F_USLEEP))
				if (sleep (iqt->sleep, priority)) {
					clrflags(F_EXCLUSIVE | F_USLEEP);
					iqt->sleep = (caddr_t) 0;
					sploff();
					longjmp (&u.u_qsav, 1);
				}
			sploff();
			iqt->uexit = I_ENACINT;
		}
		else {
			sploff();
		}
	}
	iqt->sleep = (caddr_t) 0;
	if (isclr(F_ERROCC)) {
		if ((*iqt->action == A_RDDATA || *iqt->action == A_WRDATA) && iqt->io_count)
			qt_buffer ();
	}
done:
	return;
}

static int	
qt_buffer ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e12");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	if (firstq() == A_WRDATA) {
		if (iqt->io_count < BLOCKSIZE && (isclr(F_ACTIVE) || isclr(F_WRMODE))) {
			setflags(F_ACTIVE | F_WRMODE);
			for (iqt->vbufferp = iqt->vbuffer; iqt->vbufferp < &iqt->vbuffer[BLOCKSIZE]; *iqt->vbufferp++ = '\0')
				;
			iqt->vbufferp = iqt->vbuffer;
		}
		if (isset(F_ACTIVE) && isset(F_WRMODE))
			for ( ; iqt->io_count && iqt->vbufferp < &iqt->vbuffer[BLOCKSIZE]; ) {
				if (KADDR((unsigned)iqt->vio_addr))
					*iqt->vbufferp++ = *iqt->vio_addr++;
				else
					*iqt->vbufferp++ = fubyte (iqt->vio_addr++); 
				iqt->io_count--;
				iqt->pio_addr++;
			}
	} else if (firstq() == A_RDDATA && isset(F_ACTIVE) && isclr(F_WRMODE)) {
		for ( ; iqt->io_count && iqt->vbufferp < &iqt->vbuffer[BLOCKSIZE]; ) {
			if (KADDR((unsigned)iqt->vio_addr))
				*iqt->vio_addr++ = *iqt->vbufferp++; 
			else
				subyte(iqt->vio_addr++, *iqt->vbufferp++);
			iqt->io_count--;
			iqt->pio_addr++;
			if (iqt->b->b_resid)
				iqt->b->b_resid--;
		}
		if (iqt->vbufferp >= &iqt->vbuffer[BLOCKSIZE])
			iqt->flags &= ~F_ACTIVE;
	}
	return;
}

qtintr (dev)
register short	dev;
{
	register char	status;
	char	stat_buf[6];

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e14");
	if (debug_flag & DB_STRUCT)
		prtstr ();
	/* debug_flag = 0; */
#endif
	status = msinb (STATPORT);
	if (topq() == A_RDDATA ||
	    topq() == A_WRDATA ||
	    topq() == A_FLUSH ||
	    topq() == A_SBF)
		d37A_dma_disable(qt_chan);
	deassert(CTL_INTRENA);
	iqt->uexit = I_ENACINT;
	if (isexception(status))
		qt_rdstatus (stat_buf);
	if (isclr(F_PROGRESS)) {	/* Stray interrupts */
		if (isexception(status) && stat_buf[0] & 010)
			setflags(F_EOM);
		assert(CTL_INTRENA);
		return;
	}
	clrflags(F_PROGRESS);
	if (isset(F_INTERRUPT)) {
		clrflags(F_INTERRUPT);
		setflags(F_ISSRDWR);
		dq();
	} else if (isexception(status))
		if (iseom(stat_buf))
			eom_proc ();
		else if (isreadfm(stat_buf))
			readfm_proc ();
		else
			error_proc (qt_geterrno (stat_buf));
	else if (topq() == A_RETENSION || topq() == A_REWIND ||
		topq() == A_ERASE || topq() == A_WRFILEM)
		dq();
	if (isclr(F_ERROCC)) {
		execq();
		if (isset(F_PROGRESS) && ctl_exit ())
			return;
	}
	if (isset(F_USLEEP)) {
		clrflags(F_USLEEP);
 		iqt->sleep = (caddr_t) iqt;
		wakeup (iqt->sleep);
	} else if (isset(F_OPENED))
		assert(CTL_INTRENA);
	return;
}


static int	
error_proc (n)
register short	n;
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e15");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	iqt->error = n;
	if (topq() == A_RDDATA || topq() == A_WRDATA) {
		iqt->b->b_flags |= (B_DONE | B_ERROR | B_STALE | B_AGE);
		iqt->b->b_error = iqt->error;
		iqt->b->b_resid = iqt->io_count;
	}
	setflags(F_ERROCC);
	zeroq();
	clrflags(F_PROGRESS | F_DMAWAIT);
}


static int	
readfm_proc ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e16");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	switch (topq()) {
	case A_RDDATA:
		if (isset(F_DMAWAIT)) {
			iqt->b->b_resid = iqt->io_count + BLOCKSIZE;
			clrflags(F_DMAWAIT);
			setflags(F_FFMR);
		} else {
			iqt->b->b_resid = iqt->io_count;
			setflags(F_EOFR);
		}
		dq();
		break;
	case A_SBF:
		setflags(F_ISSRDWR);
	case A_RDFILEM:
		break;
	default:
		error_proc (EIO);
	}
}


static int	
eom_proc ()
{
#ifdef DEBUG
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	setflags(F_EOM);
	switch (topq()) {
	case A_RDDATA:
		if (isset(F_DMAWAIT)) {
			iqt->b->b_resid = iqt->io_count + BLOCKSIZE;
			clrflags(F_DMAWAIT);
		} else {
			iqt->b->b_resid = iqt->io_count;
 		}
 		dq();
 		break;
 	case A_WRDATA:
 		if (isset(F_DMAWAIT)) {
 			iqt->b->b_resid = iqt->io_count - BLOCKSIZE;
 			clrflags(F_DMAWAIT);
 		} else {
 			iqt->b->b_resid = iqt->io_count;
 		}
 		dq();
 		break;
	case A_SBF:
	case A_RDFILEM:
	case A_WRFILEM:
		dq();
	case A_FLUSH:
		break;
	default:
		error_proc (EIO);
	}
}


static int	
qt_geterrno (s)
register char	*s;
{
	register unsigned int	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e17");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	switch (i = (unsigned int) s[0] << 8 | (unsigned int) s[1]) {
	case 0170000:		/* No drive */
		RETURN(17, (ENXIO));
	case 0104000:		/* End of media */
		RETURN(17, (ENOSPC));
	}
	if ((i & 0177567) == 0110000)	/* Write protected */
		RETURN(17, (EROFS));
	if ((i & 016777) == 0140000)	/* No cartridge */
		RETURN(17, (ENXIO));
	RETURN(17, (EIO));
}


static int	
qt_exec ()
{
	register short	spl, ret;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e18");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	setflags(F_PROGRESS);
	iqt->start_time = lbolt;
	switch (topq()) {
	case A_ERASE:		/* Erase tape */
		ret = qt_sndcmd (C_ERASE);
		break;
	case A_REWIND:		/* Rewind to BOT */
		ret = qt_sndcmd (C_REWIND);
		break;
	case A_RETENSION:	/* Retension */
		ret = qt_sndcmd (C_RETENSION);
		break;
	case A_WRFILEM:		/* Write file mark */
		ret = qt_sndcmd (C_WRFILEM);
		break;
	case A_RDFILEM:		/* Read file mark */
		ret = qt_rdfilem ();
		break;
	case A_RDDATA:		/* Read data */
		ret = qt_rddata ();
		break;
	case A_WRDATA:		/* Write data */
		ret = qt_wrdata ();
		break;
	case A_FLUSH:		/* Flush internal buffer */
		ret = qt_flush ();
		break;
	case A_SBF:		/* Skip block forward */
		ret = qt_sbf ();
		break;
	}
	if (ret)
		RETURN(18, (SUCCESS));
	iqt->flags &= ~F_PROGRESS;
	iqt->flags |= F_ERROCC;
	iqt->error = EIO;
	RETURN(18, (FAIL));
}


static int	
qt_rddata ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e19");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	iqt->b->b_start = iqt->start_time;
	if (isset(F_DMAWAIT))
		RETURN(19, (dmawait ()));
	if (!iqt->io_count || (isset(F_ACTIVE) && isclr(F_WRMODE) && iqt->io_count < BLOCKSIZE)) {
		clrflags(F_PROGRESS | F_ISSRDWR);
		RETURN(19, (SUCCESS));
	}
	if (isset(F_ISSRDWR) && !qt_sndcmd (C_RDDATA))
		RETURN(19, (FAIL));
	if (iqt->io_count < BLOCKSIZE) {
		setflags(F_ACTIVE);
		clrflags(F_WRMODE);
		iqt->vbufferp = iqt->vbuffer;
		RETURN(19, (qt_setdma (iqt->pbuffer, DMA_CMD_READ)));
	}
	if (qt_setdma (iqt->pio_addr, DMA_CMD_READ)) {
		iqt->io_count -= BLOCKSIZE;
		iqt->pio_addr += BLOCKSIZE;
		iqt->vio_addr += BLOCKSIZE;
		RETURN(19, (SUCCESS));
	}
	RETURN(19, (FAIL));
}


static int	
qt_wrdata ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e20");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	iqt->b->b_start = iqt->start_time;
	if (isset(F_DMAWAIT))
		RETURN(20, (dmawait ()));
	if (!iqt->io_count || (isclr(F_ACTIVE) && iqt->io_count < BLOCKSIZE)) {
		clrflags(F_PROGRESS | F_ISSRDWR);
		RETURN(20, (SUCCESS));
	}
	if (isset(F_ISSRDWR) && !qt_sndcmd (C_WRDATA))
		RETURN(20, (FAIL));
	if (isset(F_ACTIVE) && isset(F_WRMODE)) {
		clrflags(F_ACTIVE | F_WRMODE);
		RETURN(20, (qt_setdma (iqt->pbuffer, DMA_CMD_WRITE)));
	}
	if (qt_setdma (iqt->pio_addr, DMA_CMD_WRITE)) {
		iqt->io_count -= BLOCKSIZE;
		iqt->pio_addr += BLOCKSIZE;
		iqt->vio_addr += BLOCKSIZE;
		RETURN(20, (SUCCESS));
	}
	RETURN(20, (FAIL));
}


static int	
qt_flush ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e21");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	if (isset(F_DMAWAIT))
		RETURN(21, (dmawait ()));
	if (isclr(F_ACTIVE) || isclr(F_WRMODE)) {
		clrflags(F_PROGRESS | F_ISSRDWR);
		RETURN(21, (SUCCESS));
	}
	iqt->flags &= ~F_ACTIVE;
	RETURN(21, (qt_sndcmd (C_WRDATA) && qt_setdma (iqt->pbuffer, DMA_CMD_WRITE)));
}


static int	
dmawait ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e22");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	clrflags(F_DMAWAIT);
	iqt->uexit = I_ENADMA;
	RETURN(22, (SUCCESS));
}


static int	
qt_rdfilem ()
{
	register short	ret;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e23");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	ret = SUCCESS;
	if (isset(F_EOM))
		clrflags(F_PROGRESS);
	else {
		if (iqt->sfarg--) {
			ret = qt_sndcmd (C_RDFILEM);
			setflags(F_FFMR);
		} else
			clrflags(F_PROGRESS);
	}
	RETURN(23, (ret));
}


static int	
qt_sbf ()
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e24");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	if (isset(F_DMAWAIT))
		RETURN(24, (dmawait ()));
	if (iqt->sfarg--) {
		if (isset(F_ISSRDWR) && !qt_sndcmd (C_RDDATA))
			RETURN(24, (FAIL));
		RETURN(24, (qt_setdma (iqt->pbuffer, DMA_CMD_READ)));
	} else {
		iqt->sfarg = 0;
		clrflags(F_PROGRESS);
	}
	RETURN(24, (SUCCESS));
}


static int	
qt_reset ()
{
	char	stat_buf[6];
	register short	spl, i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e25");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	splon();
	iqt->start_time = lbolt;
	sploff();
	clrflags(F_PROGRESS | F_ACTIVE | F_LCMDR | F_LCMDW);
	if (board_type == BT_WANGTEK)
		deassert(CTL_ONLINE);
	assert(CTL_RESET);
	waitloop(TDELTA25);
	deassert(CTL_RESET);
	w1sec();
	if (board_type == BT_ARCHIVE) {   /* Archive SC499-R POR takes 5 sec */
		w1sec(); w1sec(); w1sec(); w1sec();
	}
	if (!qt_rdstatus (stat_buf) || !ispor(stat_buf)) 
		RETURN(25, (FAIL));
	RETURN(25, (SUCCESS));
}


static int	/* Read controller status */
qt_rdstatus (stat_buf)
register char	*stat_buf;
{
	register char	*p;
	register short	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e26");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	for (p = stat_buf; p < stat_buf + 6; *p++ = '\0') /* Clear status buffer */
		;
	clrflags(F_LCMDW | F_LCMDR);
	msoutb (CMDPORT, C_STATUS);
	assert(CTL_REQUEST);

/* ----- */
	if (!nexc_wait())		/* Wait for reset of exception */
		RETURN(26, (FAIL));
	waitloop(TDELTA20);		/* Wait at least 20 usec */
	if (!rdy_wait()) {		/* Wait for READY to set */
		deassert(CTL_REQUEST);
		RETURN(26, (FAIL));
	}
	deassert(CTL_REQUEST);	/* deassert REQUEST as soon as READY is set */
	if(!nrdy_wait())	/* Wait for reset of READY before go on */
		RETURN(26, (FAIL));
/* ------ */

	for (p = stat_buf; p < stat_buf + 6; p++) /* Get six status bytes */
		if (rdy_wait()) {			/* Wait for ready */
			waitloop(TDELTA25);
			waitloop(TDELTA25);
			*p = msinb (DATAPORT);		/* Read dataport */
			assert(CTL_REQUEST);		/* Set REQUEST */
			if (nrdy_wait()) {	/* Wait for not ready */
				waitloop(TDELTA20);	/* Wait 20 Usec */
				deassert(CTL_REQUEST);	/* Reset REQUEST */
			} else {
				deassert(CTL_REQUEST);
				break;
			}
		} else
			break;

	rdy_wait();	/* Must wait for Archive 2150L drive */

	if ( p >= stat_buf + 6 ) 
		return(SUCCESS);
	else 
		return(FAIL);
}

static int	/* Issue command cmd to the controller port */
qt_sndcmd (cmd)
register unsigned char	cmd;
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e27");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	if (cmd == C_RDDATA)
		if (isset(F_LCMDR)) {
			clrflags(F_ISSRDWR);
			RETURN(27, SUCCESS);
		} else {
			setflags(F_LCMDR);
			clrflags(F_LCMDW);
		}
	else if (cmd == C_WRDATA)
		if (isset(F_LCMDW)) {
			clrflags(F_ISSRDWR);
			RETURN(27, SUCCESS);
		} else {
			setflags(F_LCMDW);
			clrflags(F_LCMDR);
		}
	else
		clrflags(F_LCMDR | F_LCMDW);
	/* Make sure ready is asserted */
	if (!rdy_wait ())
		RETURN(27, (FAIL));
	if (board_type == BT_WANGTEK) {
		if (cmd == C_RDDATA || cmd == C_WRDATA || cmd == C_RDFILEM || cmd == C_WRFILEM)
		assert(CTL_ONLINE);
	}
	msoutb (CMDPORT, cmd);				/* Issue the command */
	assert(CTL_REQUEST);				/* Assert REQUEST */
	iqt->uexit = I_SNDCMD;
	RETURN(27, (SUCCESS));
}


static int	
qt_setdma (addr, cmd)
paddr_t addr;
unsigned char cmd;
{
#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e28");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	qtcb->command = cmd;
	qtcb->targbufs->address = addr;
	qtcb->targbufs->count = 512;
	if (!d37A_prog_chan(qtcb, qt_chan)) {
		printf("Burp...\n");
	}
	if (isset(F_ISSRDWR)) {
		clrflags(F_ISSRDWR);
		setflags(F_DMAWAIT);
	} else
		iqt->uexit = I_ENADMA;
	RETURN(28, (SUCCESS));
}

static int	
rdy_wait ()
{
	register char	j;
	register int	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e29");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	for (i = 0; isnotready(j = inb (STATPORT)) && i < WAITLIMIT; i++)
		tenmicrosec();
#ifdef DEBUG
	if (debug_flag & DB_BYTES)
		printf ("i0%o:", j);
	if (debug_flag & DB_WAIT)
		printf ("w%u", i);
#endif
	RETURN(29, (isnotready(j) ? FAIL : SUCCESS));
}


static int	
nrdy_wait ()
{
	register char	j;
	register int	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
	printf ("e30");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	for (i = 0; !isnotready(j = inb (STATPORT)) && i < WAITLIMIT; i++)
		tenmicrosec();
#ifdef DEBUG
	if (debug_flag & DB_BYTES)
		printf ("i0%o:", j);
	if (debug_flag & DB_WAIT)
		printf ("w%u", i);
#endif
	RETURN(30, (isnotready(j) ? SUCCESS : FAIL));
}


static int	
nexc_wait ()
{
	register char	j;
	register int	i;

#ifdef DEBUG
	if (debug_flag & DB_ENTER)
		printf ("e31");
	if (debug_flag & DB_STRUCT)
		prtstr ();
#endif
	for (i = 0; isexception(j = inb (STATPORT)) && i < WAITLIMIT; i++)
		tenmicrosec();
#ifdef DEBUG
	if (debug_flag & DB_BYTES)
		printf ("i0%d:", j);
	if (debug_flag & DB_WAIT)
		printf ("w%u", i);
#endif
	RETURN(31, (isexception(j) ? FAIL : SUCCESS));
}

sel_wangtek (qt_sioa)                                                  
	int qt_sioa;
{									
	/* Set constant addresses and flags */				
	DATAPORT = CMDPORT = (CTLPORT = STATPORT = qt_sioa) + 1;	
									
	/* Hardware control port bits */				
	CTL_INTRENA	= 0010;						
	CTL_ONLINE	= 0001;						
	CTL_RESET	= 0002;						
	CTL_REQUEST	= 0004;						
									
	/* Hardware status port bits */						
	STS_NRDY	= 0001;						
	STS_NEXC	= 0002;						
}

sel_archive (qt_sioa) 							
	int qt_sioa;
{									
	/* Set constant addresses and flags */				
	CTLPORT = STATPORT = (CMDPORT = DATAPORT = qt_sioa) + 1;	
	RSTDMA = (DMAGO = qt_sioa + 2) + 1;				
									
	/* Hardware control port bits */				
	CTL_INTRENA	= 0040;						
	CTL_DONEN	= 0020;						
	CTL_REQUEST	= 0100;							
	CTL_RESET	= 0200;						
									
	/* Hardware status port bits */					
	STS_DONE	= 0020;						
	STS_NEXC	= 0040;							
	STS_NRDY	= 0100;							
	STS_NINT	= 0200;						
}
