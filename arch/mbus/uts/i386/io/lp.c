/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:uts/i386/io/lp.c	1.1"
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
*/

/*
 * TITLE:       lp.c
 *
 * This driver implements centronix interface for a parallel printer. 
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/open.h>
#include <sys/cred.h>
#include <sys/cmn_err.h>
#include <sys/termio.h>
#include <sys/stream.h>
#include <sys/strtty.h>
#include <sys/inline.h>
#include <sys/iasy.h>

#ifdef HIINT
#include <sys/lp_hiint.h>
#endif
#ifdef I350
#include <sys/lp_i350.h>
#endif

#include <sys/ddi.h>

/*
 * This is a HW independent portion of parallel port driver. 
 * Hardware dependencies are defined as macros in proper header files.
 *
 * As this single file is used to generate drivers for different hardware, 
 * there are ifdefs so as to get different function and variable names.
 */

#ifdef HIINT
/* Functions */
#define	LPSTART		hlpstart
#define	LPPRINT		hlpprint
#define	LPPROC		hlpproc
#define	LPINTR		hlpintr
#define	LPTMOUT		hlptmout

/* Variables */
#define	LP_TTY		lp_hiint_tty
#define	LP_ALIVE	lp_hiint_alive
#define	LP_TMID		lp_hiint_tmid
#define	LP_TMRUN	lp_hiint_tmrun
#define	LP_TMTIME	lp_hiint_tmtime
#define	LP_CFG		lp_hiint_cfg
#endif

#ifdef I350
/* Functions */
#define	LPSTART		i350start
#define	LPPRINT		i350print
#define	LPPROC		i350proc
#define	LPINTR		i350intr
#define	LPTMOUT		i350tmout

/* Variables */
#define	LP_TTY		lp_i350_tty
#define	LP_ALIVE	lp_i350_alive
#define	LP_TMID		lp_i350_tmid
#define	LP_TMRUN	lp_i350_tmrun
#define	LP_TMTIME	lp_i350_tmtime
#define	LP_CFG		lp_i350_cfg
#endif

struct strtty *LP_TTY;				/* tty structure for this device */
int LP_ALIVE;						/* Is parallel port available ?? */
int	LP_TMID;	
int	LP_TMRUN		= 0; 			/* Is watchdog timer running ? */
time_t	LP_TMTIME   = 0x7fffffff; 	/* Don't time out. */
extern struct lp_cfg LP_CFG;		/* Defined in space.c */

#define	MAXTIME	2 					/* 2 sec. */

static int	lp_debug	= 0;

/*
 * On Multibus-1
 * -------------
 * Minor numbers 0-1 are reserved for 8251.
 * Minor number  4 is reserved for 350.
 * Minor numbers 6-29 are reserved for two 546/7/8 boards (12 ports per board).
 * On Multibus-2
 * -------------
 * Minor numbers 0-3 are reserved for 354 style serial ports.
 * Minor number  4 is reserved for 350.
 * Minor number  5 is reserved for parallel port on HiINT.
 * Minor numbers 6 onwards are for iSBC 410, iSBC 450 etc.
 */


/*
 * This procedure probes the host board for a parallel port. 
 * If found, it programmes the chip and does some initialisation. 
 */
LPSTART()
{
	register struct lp_cfg *pdev;
	extern	 int	LPPROC();
	extern	 int 	iasyhwdep();
	extern	 struct strtty  *iasy_register();

	pdev = &LP_CFG;

	LP_INITIALISE_CHIP(pdev);

	outb(LP_DATAPORT(pdev), LP_TEST_PATTERN);			/* test pattern */
	if ((inb(LP_DATAPORT(pdev)) != LP_TEST_PATTERN)) {	/* Chip not there */
		LP_ALIVE = 0;
	} else
		LP_ALIVE = 1;

	cmn_err(CE_CONT,"Parallel port (at 0x%x) level %d %s.\n", LP_DATAPORT(pdev),
		pdev->p_level,LP_ALIVE ? "found" : "NOT found");
	
	if (LP_ALIVE == 0)
		return;  /* obviously, we can't use it */

	/*
	 *	Register this driver with iasy driver. 
	 */
	LP_TTY = iasy_register(LP_MINOR, 1, LPPROC, iasyhwdep);
	/*
	 *	Move lp_tty to point to where the lp tty information 
	 *	actually lives, since unit computation is based on this.
	 *	The routine iasy_register actually returns a pointer to 
	 *	iasy_tty, which is not where the lp tty information begins.
	*/
	LP_TTY += LP_MINOR;

	LP_INITIALISE_PRINTER(pdev);

	LP_INTERRUPT_CLEAR(pdev);
}

/*
 * This routine actually xmits the character to the printer if 
 * things are in good shape.
 */
LPPRINT(tp)
register struct strtty *tp;
{
	register struct	t_buf	*tbuf;
	register struct lp_cfg	*pdev;
	int stat;
	int c;
	extern int LPTMOUT();

	if (lp_debug) 
		cmn_err(CE_CONT,"lpprint(%x)\n",tp);

	if ((tp->t_state & ISOPEN) == 0)
		return;

	pdev = &LP_CFG;

	/*
	 * check to see if we're ready to output. 
	 */
	if (tp->t_state & (BUSY|TIMEOUT|TTSTOP)) {

		if (lp_debug) 
			cmn_err(CE_CONT,"lpprint(): t_state BUSY\n");

		return;
	}

	/*
	 * We are ready to send another character to the printer.
	 */
	tbuf = (struct t_buf *)&tp->t_out;
	if (!(tbuf->bu_cnt)) {
		if (!(CPRES & iasy_output(tp))) {
			if (lp_debug) 
				cmn_err(CE_CONT,"lpprint: No output blocks\n");

			return;
		}
	}

	tp->t_state |= BUSY;

	/*
	 * Check the printer status.
	 */
	stat = LP_PRINTER_STATUS(pdev);

	if ( !(LP_PRINTER_READY(stat)) ) {
		drv_getparm(TIME, &LP_TMTIME);
		return;
	}

	c = *(tbuf->bu_ptr++); 
	--(tbuf->bu_cnt);

	if (lp_debug)
		cmn_err(CE_CONT,"lpprint '%c'\n",c);

	/*
	 * Clear the interrupt latch.
	 */
	LP_INTERRUPT_CLEAR(pdev);

	/*
	 * Output a character.
	 */
	LP_OUTPUT_CHAR(pdev, c);
	drv_usecwait (1);
	LP_TURNON_STROBE(pdev);
	drv_usecwait (1);
	LP_TURNOFF_STROBE(pdev);
	 
	drv_getparm(TIME, &LP_TMTIME); /* Remember the time for watchdog timer */

}

LPPROC(tp,cmd)
register struct strtty *tp;
register int cmd;
{
	int	retval=0;

	if (lp_debug)
		cmn_err(CE_CONT," lpproc(tp,cmd) = 0x%x  0x%x\n", tp, cmd);

	switch (cmd) {
		case T_SUSPEND:	/* Suspend output		*/
			tp->t_state |= TTSTOP;
		case T_RFLUSH:	/* Flush input queue		*/
		case T_BLOCK:	/* Block input via stop-char	*/
		case T_UNBLOCK:	/* Unblock input via start-char	*/
		case T_BREAK:	/* Send BREAK			*/
			return(retval);
		case T_TIME:	/* Time out is over		*/
			tp->t_state &= ~TIMEOUT;
			break;
		case T_WFLUSH:	/* Flush output queue		*/
			tp->t_out.bu_cnt = 0;
			break;
		case T_RESUME:	/* Resume output		*/
			tp->t_state &= ~TTSTOP;
			break;
		case T_OUTPUT:	/* Start output			*/
			break;
		case T_CONNECT: 
			if (!LP_TMRUN) {
				LP_TMTIME = 0x7fffffff; /* Dont timeout */
				LPTMOUT(tp);
			}

			tp->t_state |= CARR_ON;
			break;
		case T_DISCONNECT: 
			untimeout(LP_TMID);
			LP_TMRUN = 0;
			tp->t_state &= ~CARR_ON;
			break;
	}
	/* 
	 * call the print routine to get things rolling
	 */
	LPPRINT(tp);
	return (retval);
}

/*
 * Interrupt routine.
 */

LPINTR(level)
int level;
{
	int stat;
	int	counter;

	if (lp_debug)
		cmn_err(CE_CONT, "lpintr(level %d)\n",level);

	if (!LP_TTY) {
		/* 
		 * No HW present. Wonder where this interrupt came from.
		 */
		cmn_err(CE_NOTE, "lp : Unexpected interrupt(level 0x%x)", level);
		return;
	}

	/*
	 * Interrupt is produced at the falling edge of ACK pulse.
	 * If the ACK line is still low, we cannot clear the interrupt latch.
	 * ACK pulse is approximately 7 microsecs. So wait for 20 microsecs.
	 * Even then, if the signal is still low, we give up.
	 */
	stat = LP_PRINTER_STATUS(&LP_CFG);
	if (!(stat & LP_ACK)) {
		for (counter=0; counter<20; counter++) {
			drv_usecwait (1);
			stat = LP_PRINTER_STATUS(&LP_CFG);
			if (stat & LP_ACK)
				break;
		}
		if (counter==20) {
			cmn_err (CE_WARN, "lpintr(): ACK low for too long ..\n");
			return;
		}
	}

	LP_TMTIME = 0x7fffffff; /* Don't timeout */

	LP_TTY->t_state &= ~BUSY;

	LP_INTERRUPT_CLEAR(&LP_CFG); 	/* Clear the interrupt latch */
	LPPRINT (LP_TTY); 				/* Restart the output */
}

/*
 * Watchdog timer handler.
 */
LPTMOUT(tp)
struct	strtty	*tp; /* Not used at present */
{
	time_t diff, lptime;
	int	stat;
	int	dummy;

	if (lp_debug)
		cmn_err(CE_NOTE,"in lptmout");

	drv_getparm(TIME, &lptime);
	if ((diff = lptime - LP_TMTIME) > MAXTIME) {
		stat = LP_PRINTER_STATUS(&LP_CFG);
		if ( LP_PRINTER_READY(stat) ) {
			/*
			 * Everything is OK. Simulate the interrupt.
			 */
			LPINTR(dummy);
		}
		else {
			if (lp_debug)
				cmn_err(CE_CONT,"lptmout: Unable to start output\n");
		}
	}

	LP_TMRUN = 1;
	LP_TMID = timeout(LPTMOUT, tp, drv_usectohz(1000000));
}
