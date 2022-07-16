/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1989  Intel Corporation
*/
#ident	"@(#)mbus:uts/i386/io/i350.c	1.1"
static char	i350_copyright[] = "Copyright 1989 Intel Corp. 463989-010";

/*
 *
 * TITLE:       i350.c
 *
 * ABSTRACT:	This procedure implements a centronix interface
 *		for a parallel printer on the 386/2x using iSBX350. 
 *		The back-panel centronix connector does not support
 *		the printers busy signal being returned so we have the
 *		following scenerio, its ugly and wastes interupts but...
 *		in start and print we set ~ACK hi by writing ENABLE_INT_350
 *		this also issues an interrupt.  intr then checks the status
 *		of the printer and if it has already set ~ack then print
 *		another char else let the daemon start things again 
 *		whenever the status says the printer is ready.
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
#include <sys/ddi.h>
#include <sys/i350.h>		/* structures & defines */


extern struct i350_cfg i350_cfg;	/* in space.c -> 8255 hardware structure */
extern int lp_busy_delay;	/* in space.c -> tunable busy error delay */
extern int lp_error_msg;	/* in space.c -> error message on/off flag */
extern int lp_daemon_lim;	/* in space.c -> how long to wait before
				 			 * reporting printer is down */

static struct strtty *i350_tty;
static int lp_state;			/* driver state flag */
static int lp_daemon_count;
static int lp_wait_for_ack;
static int p_control_sig_port;

/*
 * On Multibus-1
 * -------------
 * Minor number 0-1 are reserved for 8251.
 * Minor number 4 is reserved for 350.
 * Minor number 6-29 are reserved for two 546/7/8 boards (12 ports per board).
 * On Multibus-2
 * -------------
 * Minor number 0-3 are reserved for 354 style serial ports.
 * Minor number 4 is reserved for 350.
 * Minor number 6 onwards is for iSBC 410, iSBC 450 etc.
 */

#define MINOR_i350 4

/*
 * TITLE:       i350start
 *
 * CALL:        i350start()
 *
 * CALLS:	i350daemon
 *
 * ABSTRACT:	This procedure probes the iSBX350 board for the
 *		8255 parallel port. Initializes the interface and 
 *		finally figures out whether the port is valid.
 *
 *
 * History:
 *
*/
i350start()
{
	register struct strtty *tp;
	register struct i350_cfg *pdev;
	extern	 int	i350proc();
	extern	 int 	iasyhwdep();
	extern	 struct strtty  *iasy_register();
	pdev = &i350_cfg;

	/*
	 *	Register this driver with iasy driver. 
	 */
	i350_tty = iasy_register(MINOR_i350, 1, i350proc, iasyhwdep);
	/*
	 *	Move i350_tty to point to where the i350 tty information 
	 *	actually lives, since unit computation is based on this.
	 *	The routine iasy_register actually returns a pointer to 
	 *	iasy_tty, which is not where the i350 tty information begins.
	*/
	i350_tty += MINOR_i350;

	p_control_sig_port = pdev->p_portc;
	
	/*
	 * Initialize 8255
	 * mode word 0xA8 for 8255 to set ports.
	 * group A
	 * d7=mode set flag active
	 * d6/5=mode 1
	 * d4=port A=output
	 * d3=port C hi nibble = input
	 * group B
	 * d2=mode 0
	 * d1=port B=output
	 * d0=port C lo nibble = output	
	 */
	outb(pdev->control, PRINT_INIT_350);

	outb(pdev->p_porta, TEST);			/* test pattern */
	if((inb(pdev->p_porta) != TEST)) {	/* 8255 not there */
		lp_state = 0;
	} else
		lp_state = LP_ALIVE;

	cmn_err(CE_CONT, "iSBX 350 at (port %x) level %d %s.\n", pdev->p_porta,
		pdev->p_level,lp_state ? "found" : "NOT found");
	
	if (lp_state == 0)
		return;  /* obviously, we can't use it */

	/* enable the interrupt flip/flop */
	outb(pdev->control,ENABLE_INT_350);

	/* 
	 * now start up the daemon. It's used to wake up the hardware if
	 * it's been busy too long, or if we have missed an ACK coming 
	 * back from the printer.
	*/
	i350daemon();		
}

/*
 *
 * TITLE:	i350ready
 *
 * CALL:	i350ready(tp)
 *
 * INTERFACES:	i350daemon, i350print
 *
 * CALLS:	i350print
 *
 * ABSTRACT:	This procedure is called if i350daemon found the 
 *		printer in a not running state. If the printer still isn't
 *		ready it leaves flags to allow the daemon to call again 
 * 		at a later time, else it clears the flags and tries
 *		i350print.
 *		Note: this expects to be called with interrupts
 *		already turned off (by SPL) and the flags set to
 *		cause the i350daemon to call later.  
 *
*/

static 
i350ready(tp)
register struct strtty *tp;
{
	int stat;
	
#ifdef DEBUG
	cmn_err(CE_CONT, "i350ready(%x)\n",tp);
#endif
	stat = inb(p_control_sig_port);
	if (!(stat & PR_SELECT_350)){
		stat |= PR_ERROR_350;
	}
	/*
	 * Check to see if the printer still has ACK low,
	 * the character has not been acknowledged, or
	 * an error condition still exists.
	 */
	if ((~stat & PR_ACK_BAR_350) ||
	    (lp_wait_for_ack) ||
	    (stat & (tp->t_state|PR_ERROR_350))){
		/* not ready yet, let the daemon call us again later*/
		return;
	}
	/*
	 * everything is ok so turn off daemon and busy flags
	 */
	else{
		lp_state &= ~LP_DAEMON;
		tp->t_state &= ~BUSY;
		i350print(tp);
	}
}

/*
 *
 * TITLE:	i350print
 *
 * CALL:	i350print(tp)
 *
 * INTERFACES:	i350intr, i350write i350ready
 *
 * CALLS:	
 *
 * ABSTRACT:	This procedure does the actual output to the parallel port.
 *
*/
i350print(tp)
register struct strtty *tp;
{
	register struct	t_buf	*tbuf;
	struct i350_cfg *pdev;
	int spl,stat;
	int c;

#ifdef DEBUG
	cmn_err(CE_CONT,"i350print(%x)\n",tp);
#endif

	pdev =&i350_cfg;
	spl = SPL();
	/*
	 * check to see if we're ready to output. BUSY is set by
	 * the driver, TIMEOUT and TTSTOP are only set by the line
	 * discipline routines.
	 */
	if (tp->t_state & (BUSY|TIMEOUT|TTSTOP)) {
		splx(spl);
		return;
	}
	stat = inb(p_control_sig_port);
	if (!(stat & PR_SELECT_350)){
		stat |= PR_ERROR_350;
	}
	/* 
	 * if the hardware isn't ready, let the daemon 
	 * wake us up at a later time
	 */
	if(stat & (PR_ERROR_350)){
		splx(spl);
		return;
	}
	tbuf = &tp->t_out;
	if (!(tbuf->bu_ptr && tbuf->bu_cnt)) {
		if (!(CPRES & iasy_output(tp))) {
			splx(spl);
			return;
		}
	}
	/* 
	 * set BUSY so we don't run again until we get another interrupt
	 */
	tp->t_state |= BUSY;
	lp_daemon_count = 0;
	lp_state |= LP_DAEMON;

	c = *(tbuf->bu_ptr++); 
	--(tbuf->bu_cnt);

#ifdef DEBUG
	cmn_err(CE_CONT,"i350print %c\n",c);
#endif

	lp_wait_for_ack = 1;
	c = ~c;  /* iSBX350 uses inverters!!! */

	outb(pdev->control,ENABLE_INT_350);
	outb(pdev->p_porta,c);		 /* output a character */
	outb(p_control_sig_port,ONSTROBE);
	outb(p_control_sig_port,OFFSTROBE);

	splx(spl);
}

/*
 *
 * TITLE:	i350proc
 *
 * CALL:	i350proc(tp,cmd)
 *
 * INTERFACES:	UNIX (line discipline routines)
 *
 * CALLS:	i350print
 *
 * ABSTRACT: This function is the common interface for line discipline
 * routines to call the device driver. It performs device specific I/O functions.
 *
*/
static 
i350proc(tp,cmd)
register struct strtty *tp;
register int cmd;
{
		int	retval=0;

#ifdef DEBUG
	cmn_err(CE_CONT," i350proc(tp,cmd) = %x	%s\n",tp,cmd);
#endif
	switch (cmd) {
		case T_RFLUSH:	/* Flush input queue		*/
		case T_BLOCK:	/* Block input via stop-char	*/
		case T_UNBLOCK:	/* Unblock input via start-char	*/
		case T_BREAK:	/* Send BREAK			*/
			return;
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
		case T_SUSPEND:	/* Suspend output		*/
			tp->t_state |= TTSTOP;
			break;
		case T_CONNECT: 
			lp_wait_for_ack = 0;
			tp->t_state |= CARR_ON;
			break;
		case T_DISCONNECT: 
			lp_wait_for_ack = 0;
			tp->t_state &= ~CARR_ON;
			break;
	}
	/* 
	 * call the print routine to get things rolling
	 */
	i350print(tp);
	return (retval);
}

/*
 *
 *
 * TITLE:       i350intr
 *
 * CALL:        i350intr(level)
 *
 * INTERFACES:  device interrupt
 *
 * CALLS:	i350print
 *
 * ABSTRACT:	This procedure is called on each interrupt,
 *		clears the interrupt flip/flop if ready and 
 *		calls the start routine to output characters.
 * 		depending on the status  of the hardware.
 *
*/
i350intr(level)
int level;
{
	register struct strtty *tp = i350_tty;
	int stat;

#ifdef DEBUG
	cmn_err(CE_CONT, "i350intr(%d)\n",level);
#endif

	lp_wait_for_ack = 0;
	stat = inb(p_control_sig_port);
	if (!(stat & PR_SELECT_350)){
		stat |= PR_ERROR_350;
	}
	/*
 	* NOTE: If the acknowledge line is low, the interrupt
	* flip-flop can not be  reset(r/s flipflop).
	* so we set flags to do it later.
	*/
	if(stat & PR_ACK_BAR_350){
		lp_state &= ~LP_DAEMON;
		tp->t_state &= ~BUSY;
		/* 
		 * Restart output 
		 */
		i350print(tp);		
	}
}
/*
 *
 * TITLE:	i350daemon
 *
 * CALL:	i350daemon()
 *
 * INTERFACES:	i350start
 *
 * CALLS:	i350ready
 *
 * ABSTRACT:	This routine is started at start time and runs
 *		every lp_busy_delay ticks. Its job is to 
 *		restart the output process if we either lost
 *		an interrupt, or the hardware has been busy
 *		for longer than lp_busy_delay. 
 *
*/
static 
i350daemon()
{
	register struct strtty *tp = i350_tty;
	int spl;

	spl = SPL();
	/*
	 * if the line is open and in trouble (busy too long
	 * or missed an interrupt) we call i350ready to get
	 * things rolling again
	 */
	if((lp_state & LP_DAEMON) &&
	   (tp->t_state & BUSY) &&
	   (tp->t_state & ISOPEN)){
		if (lp_daemon_count <= lp_daemon_lim)
			lp_daemon_count++;
		if (lp_daemon_count == lp_daemon_lim){
			/* don't wait any longer for the acknowledge */
			lp_wait_for_ack = 0;
			if (lp_error_msg)
			   cmn_err(CE_NOTE, "Error on printer or printer off line");
		}
		i350ready(tp);
	}

	splx (spl);

	/* after all that work, take a little snooze */
	/* lp_busy_delay is in space.c so the user can reconfig if needed */

	timeout(i350daemon,(caddr_t)0,lp_busy_delay);	/* 1 tick = ~10ms */
}
