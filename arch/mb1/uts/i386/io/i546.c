/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



/*	Copyright (c) 1983, 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ident	"@(#)mb1:uts/i386/io/i546.c	1.3.1.1"

#ifndef lint
static char i546_copyright[]
	= "Copyright 1983, 1986, 1987, 1988, 1989 Intel Corp. 462848";
#endif

/************************************************************************
 *
 *	Name:
 *		iSBC546 device driver for UNIX V.3 on 386
 *
 *	Purpose:
 *
 *		This is the set of routines that constitute the device
 *		for the iSBC 546 Intelligent Serial Interface Controller.
 *      It just happens to also work on the 188, 547, and 548
 *      async controllers.
 *
 *************************************************************************/
#include "sys/types.h"
#include "sys/param.h"		/* kernel global parameters */
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/conf.h"		/* device switch description */
#include "sys/termio.h"		/* terminal parameter definitions */
#include "sys/stream.h"
#include "sys/cmn_err.h"
#include "sys/strtty.h"
#include "sys/i546.h"		/* hardware structure and local commands */
#include "sys/iasy.h"
#include "sys/clockcal.h"	/* clock/calendar interface and ioctl */
#include "sys/ddi.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#ifndef lint
#include "sys/inline.h"
#endif

extern caddr_t sptalloc();
extern int copyin();
extern int copyout();
extern int getc();
extern int timeout();
extern int ttiocom();

extern void bcopy();
#ifdef lint
extern void copy_bytes();
extern void flushtlb();
extern void outb();
#endif
extern void signal();
extern void spinwait();
extern void splx();
extern void sptfree();
#undef wakeup
extern void wakeup();
extern void clk_log ();
extern struct strtty *iasy_register();
extern void iasy_input();
extern int iasy_output();
extern void iasyhwdep();
extern void iasy_hup();
extern int maxclick;		/* This should go away with v.4 */

extern struct streamtab iasyinfo;       /* pointer to iasy driver */

#define MAX_8251	2			/* Have to start minor # count here */
#ifdef DEBUG546
int	i546debug = 0;			/* debug output control */
int	trbufcount = 0;
char *i546cmds[] = {"ZERO", "INITIALIZE", "ENABLE", "DISABLE",
	"CONFIGURE", "TRANS BUFFER", "ABORT TRANS", "SUSPEND TRANS",
	"RESUME TRANS", "ASSERT DTR", "SET CTS/CD", "CLEAR CTS/CD",
	"SET DSR REP", "CLEAR DSR REP", "SET RI REP", "CLEAR RI REP",
	"CLEAR DTR", "SET BREAK", "CLEAR BREAK", "DOWNLOAD",
	"EXECUTE", "CLR REC BUF" };
char *i546msgs[] = {"ZERO", "TRANS COMP", "INPUT AVAIL", "DOWNLOAD COMP",
	"CARRIER DETECT", "CARRIER LOSS", "INIT RESP", "AUTOBAUD COMP",
	"SPEC CHAR REC", "DSR DETECT", "DSR LOST", "RI DETECT",
	"RI LOST" };
#define DDEBUG(x,y) if(i546debug&(x))cmn_err y
#define DINIT 1		/* initialization */
#define DCALL 2		/* driver call and start */
#define DINTR 4		/* interrupt time */
#define DIOCTL 8	/* IOCTL calls */
#define DMSG 16		/* messages as they go to and from the board */
#define DLOCAL 32	/* local, one time debugging stuff */
#else
#define DDEBUG(x,y)
#endif

/* set CARR_ON flag correctly */
/* "correctly" is on if CLOCAL and to true line state if not */
#define SETCARR(tx,lx)	if ((tx)->t_cflag & CLOCAL || (lx)->l_state&CARRF) \
				(tx)->t_state |= CARR_ON; \
			else \
				(tx)->t_state &= ~CARR_ON;
/* Routine for putting one chararacter into the input buffer */
#define PutInputChar(tx, c) if ((tx)->t_in.bu_ptr != NULL) { \
		*(tx)->t_in.bu_ptr = (c); \
		(tx)->t_in.bu_cnt--; \
		iasy_input((tx), L_BUF); \
	}
/*
 * These variables are defined in c546.c
 */
extern	int    i546_cnt;			/* number of 546s, this config */
extern	struct i546board i546board[];	/* board structure */
extern	struct i546cfg   i546cfg[];	/* board configuration addresses */
extern  int    i546baud[];		/* baud rate translation table */
extern	time_t	i546_time[];		/* output char watchdog timeout */

struct strtty     *i546_tty;
struct clock_buf {
	struct clkcal ck;	/* used to hold date information for clkread */
	unchar error; 	/* flag to hold error value when doing clock read */
} clock_buf;

/*
 * Variables local to the driver
 */
int		i546sleep;		/* semaphore of sleeping processes */
static char	i546tmrun = 0;		/* timer set up? */
#define MAXTIME	2			/* 2 seconds */

void i546tmout();			/* Catches interrupt snafus */
char *i546type[] = {
	"iSBC 546",
	"iSBC 188",
	"iSBC 547/548",
	"iSBC 546"
};

/*
 * the following driver specific routines are needed by the clock driver
 */
extern i546clkopen ();
extern i546clkclose ();
extern i546clkread ();
extern i546clkwrite ();
extern int i546proc();

/*
 * This procedure initializes the iSBC 546/48 when the call to dinit is
 * made. This procedure is done ONCE ONLY in the following sequence:
 * 	initialize the iSBC 546/48 structures to point at the board,
 *      calculate the addresses of the line structures' input buffer
 *	address, and output buffer address.
 *	Report the location and status of each board at the console.
 */

i546init()
{
register int  bnum;
register struct i546board *bd;
struct clkrtn clkrtn;

	DDEBUG(DINIT, (CE_CONT, "i546-init\n"));

	/*
	 * probe each board configured in the system
	 * if it exists sety up the driver's copy
	 * of firmware values and offsets.
	 */
	for (bnum=0; bnum < i546_cnt; bnum++) {
		bd = &i546board[bnum];
		i546check(bd, &i546cfg[bnum]);
		cmn_err(CE_CONT, "%s at 0x%x board %d ",
			i546type[bd->b_type],
			i546cfg[bnum].c_addr,
			bnum);
		if (bd->b_alive & ALIVE) {
			cmn_err(CE_CONT, "v%d.%d ", (bd->b_version>>4),
				(bd->b_version&0x0F));
			if (bd->b_alive & I546ERROR) {
				cmn_err(CE_CONT, "<<exception code %d>> Not initialized",
					(bd->b_addr->ConfidenceTestResult));
				bd->b_alive &= ~ALIVE;
			}
			cmn_err(CE_CONT, "Found\n");
			if ((bd->b_type == I546BOARD) && (bd->b_alive & ALIVE)) {
				/*
				 * initialize the clock switch
				 * Note that if there are more than one
				 * 546 boards then the last one is the one.
				 */
				clkrtn.c_open = i546clkopen;
				clkrtn.c_close = i546clkclose;
				clkrtn.c_read = i546clkread;
				clkrtn.c_write = i546clkwrite;
				/*
				 * call the clk name service to log this
				 */
				(void) clk_log (CLK_I546, makedev(15,5), &clkrtn);
			}
		} else {
			cmn_err(CE_CONT, "Not Found\n");
		}
	}
	/*
	 *	Register terminal server.  Each i546 board has 12 lines.
	*/
	i546_tty = iasy_register(MAX_8251, i546_cnt * 12, i546proc, iasyhwdep);
	/*
	 *	Move i546_tty to point to where the i546 tty information 
	 *	actually lives, since unit computation is based on this.
	 *	The routine iasy_register actually returns a pointer to 
	 *	iasy_tty, which is not where the i546 tty information begins.
	*/
	i546_tty += MAX_8251;
}


/*
 * This procedure verifies that a isbc546 board is presently
 * configured by sending a reset command to the board, if the
 * board is reset successfully (status byte has a value of 1,
 * indicating the command is accepted), the board is there; otherwise,
 * the board is considered not there.
 * Information on the aliveness of the board is returned as flags
 * in bd->b_alive.
 */

i546check(bd, cf)
	struct i546cfg   *cf;
	struct i546board *bd;
{

	DDEBUG(DINIT, ("CE_CONT, i546-check\n"));
	if (cf->c_addr == (long)0x0)
		return;
	bd->b_port = cf->c_port;
	/*
	 * Some 386 boards support more than 0xF80000 bytes of RAM.  For these
	 * processors, maxclick will be greater than 15.5 Mb, and Multibus
	 * addresses start at processor address 0xBF000000.
	*/
	if (ctob((ulong)(maxclick+1)) > 0xF80000)
		cf->c_addr |= 0xBF000000;
	/*
	 * if the board is configured in the system
	 * set up selector in kernels memory map for board
	 * reset board and see if it answers
	 * else assume its not there
	 */
	outb(bd->b_port, RESET);
	spinwait(5*1000);		/* wait for selftest to finish */
	/* the base address is different for some boards so we try
	 * to find it at two different addresses
	 */
	if (!tryboard(bd, cf->c_addr))
		(void) tryboard(bd, cf->c_addr+BDSTART);
}

tryboard(bd, baseaddr)
	register struct i546board *bd;
	long baseaddr;
{
	register struct i546line  *ln;
	unsigned output_base;
	unsigned output_line;
	int	alive;
	int	lines, count;

	alive = bd->b_alive = 0;
	if ((bd->b_addr=(struct i546control *)sptalloc(btoc(I546LIMIT),
			PG_RW|PG_V, baseaddr>>PNUMSHFT&PGFNMASK, 0)) == 0) {
		/* don't have the memory to allocate map */
		cmn_err(CE_CONT, "NO MAP\n");
		return(0);
	}
	/*
	 * if present, first determine it passed all tests
	 * then send initialize message and mark lines
	 * that are present.
	 */
	if ((bd->b_addr->BoardType != 0)
	&&  (bd->b_addr->BoardType < I546TYPES)) {
		bd->b_type = bd->b_addr->BoardType;
		bd->b_version = bd->b_addr->Version;
		if ((bd->b_addr->CompletionFlag&0xff) != 0xff
		||  (bd->b_addr->ConfidenceTestResult&0xff) != 0xff)
			alive |= (I546ERROR|ALIVE);
		else {
			bd->b_msg.m_type = INIT;
			bd->b_msg.m_cnt = iSBX354s;
			i546io_spin(bd, &bd->b_msg);
			spinwait(150);
			if (!i546get_cmd(bd, &bd->b_msg))
				bd->b_msg.m_type = 0;
			outb(bd->b_port, INTR_CLR);
			lines = bd->b_msg.m_cnt;

			/*
			 * mark the line structure
			 * for each configured port on board
			 * this configures in the optional
			 * iSBX 354 multimodules.
			 */
			if (bd->b_msg.m_type == INTCMP) {
				alive |= ALIVE;
				if (bd->b_type == I188BOARD) {
					output_base = OUTBASE_12LINES;
					output_line = OUTSIZE_12LINES;
				} else {
					output_base = OUTBASE_8LINES;
					output_line = OUTSIZE_8LINES;
				}
				lines = bd->b_msg.m_cnt;
				for (count=0; count<i546LINES; count++) {
					if (lines&(1<<count)) {
						ln = &bd->b_line[count];
						ln->l_state = ALIVE;
						ln->l_oba = output_base + (count*output_line);
						ln->l_obsiz = output_line;
						ln->l_iba = INBUFBASE + (count*INLINSIZ);
						ln->l_ibsiz = INLINSIZ;
					}
				}
#ifdef DEBUG546
				{
					/* this places the ASCII chars of the logical address
					 * into the memory of the 586.  This is useful for
					 * debugging.  The addresses are "backwards" but the
					 * code is simple.
					 */
					char *bbp; int i;
					bbp = (char *)&bd->b_addr->DSfiller[0];
					while ((int)bbp < (((int)bd->b_addr)+32000)) {
						for (i=(int)bbp; i!=0; bbp++) {
							*bbp = i%16>9 ? 'a'+i%16-10 : '0'+i%16;
							i /= 16;
						}
					}
				}
#endif
			}
		}
	}
	if (!alive) {
		sptfree(bd->b_addr, btoc(I546LIMIT), 0);
		bd->b_addr = NULL;
		/* the sptfree will leave stuff in the look aside buffer */
		flushtlb();
	}
	bd->b_alive = alive;
	return(alive);
}


/*
 * This procedure sets up the baud rate and the parameter of the device.
 * The code depends on having the tty structure filled out before a call is made
 * to i546param. This is the sequence of events;
 *	check for valid speed
 *	set up the baud rate and parameter
 * If successful, return 0; else return errno.
 */

int
i546param(unit)
int unit;
{
	register struct strtty	  *tp;
	register struct i546board *bd;
	struct i546line  *ln;
	int	bnum, param, l_num, s;
	int	speed;
	int	ret_val = 0;

	DDEBUG(DCALL, ("CE_CONT, i546-param unit %d ::",unit));
	bnum = unit / i546LINES;	/* calculate the board number */
	tp = &i546_tty[unit];		/* set up ptr. of the tty struct */
	bd = &i546board[bnum];
	l_num = unit % i546LINES;	/* get the line no. */
	ln = &bd->b_line[l_num];
	/*
	 * Critical region
	 */
	s = SPL();
	/* calling routine must wait for BUSY if they require */
	/*
	 * translate index into hardware dependent number
	 */
	speed = i546baud[tp->t_cflag&CBAUD];
	/*
	 * if speed is zero disable line else
	 * setup configuration message with current
	 * tty configuration and send to board
	 */
	if (speed == 0) {
		tp->t_cflag = (tp->t_cflag&~CBAUD)|B9600;
		speed = i546baud[B9600];
		/*
		 * only disable modem lines
		 */
		if (!(tp->t_cflag&CLOCAL)) {
			bd->b_msg.m_type = DTRCLR;
			bd->b_msg.m_line = l_num;
			i546io(bd);
		} else
			ret_val = EINVAL;
		/*
		 * Fall through to match hw state to sw state.
		*/
	}
	/*
	 * construct a complete line
	 * configure message
	 */
	bd->b_msg.m_type = CONFIG;
	bd->b_msg.m_line = l_num;
	bd->b_msg.m_cnt = speed;
	param = LNDISP;
	if (tp->t_cflag & CSTOPB)
		param |= STBITS2;
	else
		param |= STBITS1;
	SETCARR(tp, ln);
	/* set parity bits in "param"  */
	if (tp->t_cflag & PARENB) {
		/*
		 * support for 546 firmware that supports "better" input
		 * parity processing.  Previous versions of this driver knew
		 * that the board would not support PARMRK, IGNPAR, and INPCK
		 * so, if you specified PARENB, it would force PARMRK off and
		 * INPCK and IGNPAR on so that your flags would look like what
		 * the board was really going to do to you.  With the firmware
		 * update, these modes can now be supported so this odd driver
		 * behavior can be removed.
		*/
		/* tp->t_iflag &= ~PARMRK;		/* Can't do this	*/
		/* tp->t_iflag |= INPCK|IGNPAR;	/* Always do these	*/
		if (tp->t_cflag & PARODD)
			param |= PODD;
		else
			param |= PEVEN;
		if (tp->t_iflag & INPCK) {
			if (tp->t_iflag & IGNPAR) {
				param |= PEDEL;
			} else {
				if (tp->t_iflag & PARMRK) {
					param |= PEMARK;
				} else {
					/* the firmware does not support this mode so we
					 *  must mark the character and turn it into a null
					 *  later.
					 */
					param |= PEMARK;
				}
			}
		} else {
			param |= PEACCEPT;
		}
	} else
		param |= PNO;
	/* Set character size bits in "param" */
	switch (tp->t_cflag & CSIZE) {
		default:
			ret_val = EINVAL;	/* Rig to return error */
			tp->t_cflag = (tp->t_cflag&~CSIZE)|CS8;
			/*
			 * FALL THROUGH to align the hardware and software
			 * state of the line to some consistent (CS8) state.
			*/
		case CS8:
			param |= C8BITS;
			ln->l_mask = 0xff;
			break;
		case CS7:
			param |= C7BITS;
			ln->l_mask = 0x7f;
			break;
		case CS6:
			param |= C6BITS;
			ln->l_mask = 0x3f;
			break;
	}
	(void) i546proc(tp, T_WFLUSH);
	bd->b_msg.m_buf[0] = SPCHAR;
	bd->b_msg.m_buf[1] = SPHIWAT;
	bd->b_msg.m_buf[2] = NULL;
	if (tp->t_lflag & ISIG) {
		bd->b_msg.m_buf[3] = (unsigned)tp->t_cc[VINTR];
		bd->b_msg.m_buf[3] |= (((unsigned)tp->t_cc[VQUIT])<<8);
		bd->b_msg.m_buf[4] = (((unsigned)tp->t_cc[VQUIT])<<8);
	}
	if (tp->t_lflag & ICANON)
		bd->b_msg.m_buf[4] |= (unsigned)tp->t_cc[VKILL];
	bd->b_msg.m_ptr = param;
	i546io(bd);
	DDEBUG(DCALL, (CE_CONT, "i546-param: unit=%d, param=%x\n", unit, param));
	iasy_ctime(tp, 1);
	splx(s);
	return(ret_val);
}


/*
 * This routine is used to restart the input sequence of a
 * 546 line. We tell the 546 that we have read 0 chars
 * from it's buffer and that causes an input available interrupt.
 * May only be called by a task time procedure
 */
i546s_input(unit)
int unit;
{
	register struct i546board *bd;
	int l_num, s;

	bd = &i546board[unit/i546LINES];
	l_num = unit % i546LINES;

	/*
	 * send an INPUT COMMAND of 0 bytes to the 546
	 * This is to get interrupts started again
	 */
	s = SPL();
	bd->b_msg.m_type = INPUT;
	bd->b_msg.m_line = l_num;
	bd->b_msg.m_ptr = bd->b_line[l_num].l_ibp;
	bd->b_msg.m_cnt = 0;
	i546io(bd);
	splx(s);
}


/* i546s_output no longer needed */
/*
 * This procedure is called with interrupts off (spltty) when the
 * iSBC 546 interrupts.
 *	Warning:
 *		The interrupt routine must repeat its scan and read all
 *		messages from the board before it exits the interrupt
 *		routine. It is possible to get a spurious interrupt
 *		immediately after the message queue of a board has been
 *		purged.
 */
i546intr(level)
int	level;
{
	register struct strtty 		*tp;
	register struct i546line 	*ln;
	struct i546board	*bd;
	struct  i546msg fr_msg, to_msg;
	int 	bnum, nch, l_num;
	unsigned	char	c;
	unsigned int count, gotboard, gotmessage, max;

	DDEBUG(DINTR, (CE_CONT, "i546-intr\n"));
do_again:
	gotmessage = gotboard = 0;
	for (bnum=0, bd=0; bnum < i546_cnt; bnum++) {
		if(i546cfg[bnum].c_level != level)
			continue;
		if( ! (i546board[bnum].b_alive & ALIVE))
			continue;
		gotboard = 1;
		bd = &i546board[bnum];

		/*
		 * signal the 546/48 to clear its interrupt then
		 * get each message from the board's queue and
		 * service the command from the board
		 * repeat until no outstanding messages from
		 * the board are left in the queue
		 */
		outb(bd->b_port, INTR_CLR);
		(void) drv_getparm(TIME, &i546_time[bnum]);
		while ((i546get_cmd(bd, &fr_msg)) != CLEAR) {
			/* Allow interrupts once per iteration  */
			gotmessage = 1;
#ifdef NO_NOT_NOW
			s = spl6();
			splx(s);
#endif
			/* wakeup processes waiting on an event */
			if (i546sleep) {
				i546sleep = 0;
				wakeup((caddr_t)&i546sleep);
			}
			/*
			 * setup required pointers
			 */
			l_num = fr_msg.m_line;
			ln = &bd->b_line[l_num];
			tp = &i546_tty[((bnum*i546LINES) + l_num)];
			DDEBUG(DINTR, (CE_CONT, "board %d line %d\n",bnum, l_num));
			/*
			 * service status command on line
			 * according to type
			 */
			switch(fr_msg.m_type) {

			/*
			 * Input Available:
			 *	Set the input request indicator
			 *      Read in the input from the input buffer
			 *	Send the number of bytes input, line
			 *	number, and INPUT command to the board,
			 */
			case INAVIL:
				(void)drv_setparm(SYSRINT, 1);
				/* wakeup((caddr_t)&tp->t_rdqp); */

				if (ln->l_state & INFLUSH) {
					ln->l_state &= ~INFLUSH;
					nch = fr_msg.m_cnt;
				} else {
					ln->l_ibp = fr_msg.m_ptr - BDSTART;
					ln->l_ibc = fr_msg.m_cnt;
					if ((bd->b_type == I546BOARD) && (l_num == 5)) {
						/* Clock */
						clock_buf.error = 0;
						if (ln->l_ibc == CLKSIZE) {
							copy_bytes((caddr_t)bd->b_addr + ln->l_ibp,
										(caddr_t)&clock_buf.ck,
										CLKSIZE);
						} else
							clock_buf.error = 1;
						wakeup((caddr_t)&clock_buf.ck);
						nch = fr_msg.m_cnt;
					} else {
						/* Serial Line */
						if ((count = tp->t_in.bu_cnt) < 1) {
							ln->l_state |= INSTOP;
							tp->t_state |= TBLOCK;
							break;
						}
						ln->l_ibc = (ln->l_ibc > count ? count : ln->l_ibc);
						ln->l_state |= INBUSY;
						max = (ln->l_ibsiz - 1) + ln->l_iba;
						for (nch=0; ln->l_ibc != 0; ln->l_ibc--, nch++) {
							c = *(((unsigned char *)bd->b_addr) + ln->l_ibp);
							if ((++ln->l_ibp) > max)
								ln->l_ibp = ln->l_iba;
							ProcessInputChar(tp, ln, c);
						}
						ln->l_state &= ~INBUSY;
						(void) i546proc(tp, T_OUTPUT);
					}
				}
				to_msg.m_type = INPUT;
				to_msg.m_line = l_num;
				to_msg.m_cnt = nch;
if (!nch)
	cmn_err(CE_PANIC, "nch = %d ", nch);
				i546io_spin(bd, &to_msg);
				break;
			/*
			 * Output Complete:
			 */
			case OUTCMP:
				(void)drv_setparm(SYSXINT, 1);
				DDEBUG(DINTR, (CE_CONT, "i546intr: in OUTCMP case. count is %d.\n", fr_msg.m_cnt));
				tp->t_state &= ~BUSY;
				ln->l_obc -= fr_msg.m_cnt;
				(void) i546proc(tp, T_OUTPUT);
				break;
			/*
			 * Carrier Detect:
			 *	Wakeup the process
			 */
			case ONCARR:
				(void)drv_setparm(SYSMINT, 1);
				ln->l_state |= CARRF;
				SETCARR(tp, ln);
				wakeup((caddr_t)&tp->t_rdqp);
				break;
			/*
			 * Carrier Loss:
			 *	Signal the process
			 *	Reset the CARR_ON state of the line
			 */
			case OFCARR:
				(void)drv_setparm(SYSMINT, 1);
				ln->l_state &= ~CARRF;
				if ((tp->t_state&CARR_ON)  && !(tp->t_cflag&CLOCAL)) {
					iasy_hup(tp);
					/* Drop DTR in close, not here */
				}
				SETCARR(tp, ln);
				wakeup((caddr_t)&tp->t_oflag);
				break;
			/*
			 * Special condition character recognized.
			 * This only happens when the board's input buffer
			 * fills above a preset threshold.
			 * If the line is in cooked mode and quit or intr
			 * character is received it is handled immdeiately.
			 */
			case SPCOND:
				(void)drv_setparm(SYSRINT, 1);
				wakeup((caddr_t)&tp->t_rdqp);
				if (tp->t_lflag&ICANON) {
					c = fr_msg.m_cnt;
					PutInputChar(tp, c);
				}
				break;
			default:
				cmn_err(CE_NOTE, "Invalid iSBC 546/48 message: type %d",fr_msg.m_type);
				cmn_err(CE_NOTE, " board %d, line %d\n",bnum, l_num);
				break;
			}
		}
	}
	if(!gotboard) {
		cmn_err(CE_CONT, " Spurious iSBC 546/48 Interrupt\n");
		return;
	}
	if(gotmessage) {
		goto do_again;
	}
}

ProcessInputChar(tp, ln, c)
	register struct strtty *tp;
	register struct i546line *ln;
	register unsigned char c;
{
	switch (ln->l_state & (RECFF|RECFFNUL)) {
		/*
		 *  This code handles the parity marking case when INPCK is on
		 *  IGNPAR is off and PARMRK is off -- ie, input parity
		 *  errors are read as nulls.  The 546 will only mark
		 *  parity errors so we keep the state of the mark in RECFF and
		 *  RECFFNUL.  RECFF goes on when a mark (0xFF) is received.
		 *  RECFFNUL goes on when the next character is a NUL.  The
		 *  character after that will be the parity error char and will
		 *  get turned to NUL if the flags warrent that.  If a NUL does
		 *  not follow the MARK and marking is enabled the character is
		 *  passed through.
		 *  Note that RECFF is only set when marking is being turned to
		 *  NULs.
		 */
		case 0:
			/* no flags.  See if the char is the mark character */
			if (c == 0xFF) {
				if (tp->t_iflag&INPCK && !(tp->t_iflag&IGNPAR)) {
					ln->l_state |= RECFF;
					DDEBUG(DLOCAL, (CE_CONT, "ProcessChar: marking FF\n"));
					return;
				}
			}
			break;
		case RECFF:
			/* last character was a mark char */
			if (c == 0) {
				/* received the NUL after the MARK */
				ln->l_state |= RECFFNUL;
				DDEBUG(DLOCAL, (CE_CONT, "ProcessChar: NUL after mark\n"));
				return;
			} else {
				/* MARK not followed by NUL.  Another MARK means one MARK */
				if (c != 0xFF) {
					PutInputChar(tp, 0xFF); /* put in ignored MARK */
					/* process received char by falling through */
				}
				ln->l_state &= ~(RECFF|RECFFNUL);
				DDEBUG(DLOCAL, (CE_CONT, "ProcessChar: non-NUL after mark\n"));
			}
			break;
		case RECFF|RECFFNUL:
			/* received both MARK and NUL.  This is the bad char. */
			ln->l_state &= ~(RECFF|RECFFNUL);
			if (tp->t_iflag & PARMRK) {
				DDEBUG(DLOCAL, (CE_CONT, "ProcessChar: sending marked parity\n"));
				PutInputChar(tp, 0xFF);
				PutInputChar(tp, 0);
				/* Return char without ISTRIP */
				PutInputChar(tp, c&ln->l_mask);
				return;
			} else {
				DDEBUG(DLOCAL, (CE_CONT, "ProcessChar: changing marked to NUL\n"));
				c = 0;
				/* process NUL char by falling through */
			}
			break;
	}
	c &= ln->l_mask;	/* ignore non-data bits */
	if (tp->t_iflag & ISTRIP)
		c &= 0177;
	if (tp->t_iflag&IXON) {
		/* XON/XOFF processing for output control */
		if (tp->t_state & TTSTOP) {
			/* output stopped, see if time to startup */
			if (c==CSTART || tp->t_iflag&IXANY) {
				(void) i546proc(tp, T_RESUME);
				/* fall through for throw away check */
			}
		} else {
			/* see if we are to stop output */
			if (c == CSTOP) {
				(void) i546proc(tp, T_SUSPEND);
				/* fall through for throw away check */
			}
		}
		/* if IXON then start and stop chars are not read */
		if (c==CSTART || c==CSTOP)
			return;
	}
	/*
	 * process input character
	 * mask to return only data bits
	*/
	PutInputChar(tp, c);
}

/*
 * This procedure starts output on a line if needed.  i546startio gets chars
 * from the line discipline to tp->t_out, then outputs the characters to the
 * line and sets the BUSY flag.  The busy flag gets unset when the characters
 * have been transmitted [by i546intr()].  This procedure is called at task
 * time by the writing process, and asynchronously at interrupt time by the
 * interrupt service routine.
 */
i546startio(tp)
register struct strtty *tp;
{
	register struct i546board	*bd;
	register struct i546line	*ln;
	int		s, l_num, unit;
	static char start[] = { CSTART };
	static char stop[] = { CSTOP };
	int 	bytes;
	caddr_t	ptr;

	s = SPL();
	unit = tp - i546_tty;		/* get the unit number */
	DDEBUG(DCALL, (CE_CONT, "i546startio: called on unit %d\n",unit));
	if (tp->t_state & BUSY) {
		splx(s);
		return;
	}
	l_num = unit % i546LINES;	/* get the line number */
	bd = &i546board[unit/i546LINES];/* set pointer to board structure */
	ln = &bd->b_line[l_num];	/* set up ptr. to line struct */
	/*
	 * Check for characters indirectly through the
	 * linesw table.  If there are any, ship them out.
	 */
	if (tp->t_state & TTXON) {
		tp->t_state &= ~TTXON;
		ptr = start;
		bytes = 1;
	} else if (tp->t_state & TTXOFF) {
		tp->t_state &= ~TTXOFF;
		ptr = stop;
		bytes = 1;
	} else {
		if (!(tp->t_out.bu_cnt && tp->t_out.bu_ptr)) {
			if (!(CPRES & iasy_output(tp))) {
				splx(s);
				return;
			}
		}
		ptr = (caddr_t)tp->t_out.bu_ptr;
		bytes = min(tp->t_out.bu_cnt, ln->l_obsiz);
		tp->t_out.bu_ptr += bytes;
		tp->t_out.bu_cnt -= bytes;
	}
	/* Note: now ptr & bytes determine the output to do */
	DDEBUG(DCALL, (CE_CONT, "i546startio: %d on unit %d\n",bytes, unit));
	copy_bytes(ptr, ((char *)bd->b_addr)+ln->l_oba, bytes);
	bd->b_msg.m_type = OUTPUT;
	bd->b_msg.m_line = l_num;
	bd->b_msg.m_ptr = ln->l_oba + BDSTART;
	bd->b_msg.m_cnt = bytes;
	i546io_spin(bd, &bd->b_msg);
	tp->t_state |= BUSY;
	ln->l_obc += bytes;
	splx(s);
	return;
}


/*
 * This function called by the clock driver.  It is a replacement for the scheme
 * in System V/386 Rel. 3.x where the clock driver used to call i546open,
 * which no longer exists due to streams implementation.
 */

int
i546clkopen()
{
	return(0);
}


/*
 * This function called by the clock driver.  It is a replacement for the scheme
 * in System V/386 Rel. 3.x where the clock driver used to call i546close,
 * which no longer exists due to streams implementation.
 */

int
i546clkclose()
{
	return(0);
}


/*
 * This function called by the clock driver and from the ioctl
 * is used to read the device clock.
 */

i546clkread (dev, addr, flag)
dev_t dev;
caddr_t addr;
int flag;
{
	struct	i546board *bd = &i546board[BRDNO(dev)];
	int	unit, s, error;

	unit = minor(dev);
	if ((bd->b_type != I546BOARD) || (LINNO(dev) != 5))
		return(EINVAL);
	s = SPL();
	i546s_input(unit);		/* start input */
	(void) sleep((caddr_t)&clock_buf, TTIPRI);   /* wait for input */
	error = 0;
	if (!clock_buf.error) {
		if (flag == IN_KERNEL)
			bcopy((caddr_t)&clock_buf.ck, addr, (unsigned)CLKSIZE);
		else
			(void) copyout((caddr_t)&clock_buf.ck, addr, (unsigned)CLKSIZE);
	} else
		error = EIO;
	splx(s);
	return(error);
}

/*
 * This function called by the clock driver and from the ioctl
 * is used to write the device clock.
 */

int
i546clkwrite (dev, addr, flag)
dev_t dev;
caddr_t addr;
int flag;
{
	struct	clkcal ck;
	struct	i546board *bd = &i546board[BRDNO(dev)];
	struct	i546line *ln = &i546board[BRDNO(dev)].b_line[LINNO(dev)];
	int	s, error;

	if ((bd->b_type != I546BOARD) || (LINNO(dev) != 5))
		return(EINVAL);
	error = 0;
	if (flag == IN_KERNEL) {
		bcopy (addr, &ck, CLKSIZE);
	} else {
		if (copyin(addr, &ck, CLKSIZE) < 0) {
			return (EIO);
		}
	}
	s = SPL();
	ck.ck_reserv2 = ck.ck_month;
	copy_bytes(&ck, ((char *)bd->b_addr)+ln->l_oba, CLKSIZE);
	bd->b_msg.m_type = OUTPUT;
	bd->b_msg.m_line = LINNO(dev);
	bd->b_msg.m_ptr = ln->l_oba + BDSTART;
	bd->b_msg.m_cnt = CLKSIZE;
	i546io(bd);
	splx(s);
	return (error);
}


/*
 * This function is called by i546proc in the case of T_BREAK.  It clears the
 * break condition on line.
*/

void
i546_brk_clr(tp)
register struct strtty *tp;
{
	int s;
	int unit, l_num;
	struct i546board *bd;

	unit = tp - i546_tty;
	bd = &i546board[unit/i546LINES];
	l_num = unit % i546LINES;	/* get the line number */

	s = SPL();
	bd->b_msg.m_type = BRKCLR;
	bd->b_msg.m_line = l_num;
	i546io(bd);
	tp->t_state &= ~TTSTOP;
	(void) i546proc(tp, T_OUTPUT);
	splx(s);
}

/*
 * This function is the common interface for line discipline
 * routines to call the device driver. It performs device
 * specific I/O functions.
 */

int
i546proc(tp, comd)
register struct strtty *tp;
register int comd;
{
	int s;
	int unit, l_num;
	int ret_val = 0;
	struct i546board *bd;
	struct i546line  *ln;

	unit = tp - i546_tty;
	bd = &i546board[unit/i546LINES];
	l_num = unit % i546LINES;	/* get the line number */
	ln = &bd->b_line[l_num];

	DDEBUG(DCALL, (CE_CONT, "i546-proc unit %d, cmd %d ::", unit, comd));
	switch (comd) {
	/* flush output queue */
	case T_WFLUSH:
		s = SPL();
		if (tp->t_state & BUSY) {
			bd->b_msg.m_type = RESUME;
			bd->b_msg.m_line = l_num;
			i546io(bd);
			tp->t_state &= ~TTSTOP;
			bd->b_msg.m_type = ABORT;
			bd->b_msg.m_line = l_num;
			i546io(bd);
		}
		splx(s);
		break;
	/* resume output */
	case T_RESUME:
		s = SPL();
		bd->b_msg.m_type = RESUME;
		bd->b_msg.m_line = l_num;
		i546io(bd);
		tp->t_state &= ~TTSTOP;	/* Don't clear busy */
		splx(s);
		break;
	/* suspend output */
	case T_SUSPEND:
		s = SPL();
		tp->t_state |= TTSTOP;
		bd->b_msg.m_type = SUSPND;
		bd->b_msg.m_line = l_num;
		i546io(bd);
		splx(s);
		break;
	/* send stop character */
	case T_BLOCK:
		s = SPL();
		tp->t_state &= ~TTXON;
		tp->t_state |= (T_BLOCK|TTXOFF);
		(void) i546proc(tp, T_OUTPUT);
		splx(s);
		break;
	/* flush input queue */
	case T_RFLUSH:
		s = SPL();
		ln->l_state &= ~(RECFF|RECFFNUL);
		if (ln->l_state & INSTOP) {
			ln->l_state |= INFLUSH;
		}
		splx(s);
		/* FALL THROUGH */
	/* send start character */
	case T_UNBLOCK:
		s = SPL();
		tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;		/* send ctrl-Q */
		(void) i546proc(tp, T_OUTPUT);
		/* if blocked because of clist full, try starting */
		if (ln->l_state & INSTOP) {
			i546s_input(unit);
			ln->l_state &= ~INSTOP;
		}
		splx(s);
		break;
	/* remove timeout condition */
	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		break;
	/* output break condition on line */
	case T_BREAK:
		s = SPL();
		tp->t_state |= TTSTOP;
		bd->b_msg.m_type = BRKSET;
		bd->b_msg.m_line = l_num;
		i546io(bd);
		(void)timeout(i546_brk_clr, (caddr_t)tp, HZ/4);
		splx(s);
		break;
	/* start output */
	case T_OUTPUT:
		break;
	case T_CONNECT:
		/* send enable message to board */
		bd->b_msg.m_type = ENABLE;
		bd->b_msg.m_line = l_num;
		i546io(bd);

		/*
	 	* assert DTR signal and sleep waiting for a carrier detect
	 	* interrupt to wake the process up only if it is a modem
	 	* line. DTR must be asserted after the line is enabled (see
	 	* firmware EPS) and before a carrier detect can be sensed.
	 	*/
		bd->b_msg.m_type = DTRAST;
		bd->b_msg.m_line = l_num;
		i546io(bd);
		SETCARR(tp, ln);
		break;
	case T_PARM:
		ret_val = i546param(unit);
		break;
	case T_DISCONNECT:
		bd->b_msg.m_type = DTRCLR;
		bd->b_msg.m_line = l_num;
		i546io(bd);
		break;
	case T_SWTCH:
		break;
	default:
		break;
	}
	/* Let i546startio sweat BUSY */
	if (((tp->t_state&(TTSTOP|TIMEOUT)) == 0) && !(ln->l_state&INBUSY))
		i546startio(tp);
	return(ret_val);
}


i546io(bd)
register struct i546board *bd;
{
	/*
	 * if all message slots are in use
	 * wait on a interrupt to signal
	 * a free message slot
	 */
	while ((i546snd_cmd(bd, &bd->b_msg)) == 0) {
		i546sleep++;
		(void) sleep((caddr_t)&i546sleep, TTIPRI|PCATCH);
	}
}

i546io_spin(bd, buf)
	register struct i546board *bd;
	struct i546msg *buf;
{
	while ((i546snd_cmd(bd, buf)) == 0)
		;
}

/*
 *  Procedure to get put one command into the output queue.  If the
 *  queue is full, zero is returned.  Non-zero is returned if
 *  a command is succesfully placed.  A "copy_bytes" is used to place
 *  the command rather than a structure assignment because the
 *  546 board can not handle 32 bit MultiBus I data accesses.
 */
i546snd_cmd(brd, buf)
	struct i546board *brd;
	struct i546msg *buf;
{
	register struct i546control *ptr;

	DDEBUG(DMSG, (CE_CONT, "i546snd_cmd: cmd=%s, ln=%x, cnt=%d, ptr=%x, ",
		i546cmds[buf->m_type], buf->m_line, buf->m_cnt, buf->m_ptr));
	DDEBUG(DMSG, (CE_CONT, "b=%x,%x,%x,", buf->m_buf[0], buf->m_buf[1], buf->m_buf[2]));
	DDEBUG(DMSG, (CE_CONT, "b=%x,%x\n", buf->m_buf[3], buf->m_buf[4]));
	ptr = brd->b_addr;
	if (((ptr->InQTail + 1) % i546QSIZE) == ptr->InQHead)
		return(0);	/* return 0 if queue is full */
	copy_bytes(buf, &ptr->InQ[ptr->InQTail], sizeof(struct i546msg));
	ptr->InQTail = (ptr->InQTail + 1) % i546QSIZE;
	outb(brd->b_port, 0x02);
	return(1);		/* return 1 if success */
}

/*
 *  Procedure to get one message from the input queue.  If the
 *  queue is empty, zero is returned.  Non-zero is returned if
 *  a message is succesfully fetched.  A "copy_bytes" is used to get
 *  the message rather than a structure assignment because the
 *  546 board can not handle 32 bit MultiBus I data accesses.
 */
i546get_cmd(brd, buf)
	struct i546board *brd;
	struct i546msg *buf;
{
	register struct i546control *ptr;

	ptr = brd->b_addr;
	if (ptr->OutQTail == ptr->OutQHead)
		return(0);	/* return 0 if queue is empty */
	copy_bytes(&ptr->OutQ[ptr->OutQHead], buf, sizeof(struct i546msg));
	ptr->OutQHead = (ptr->OutQHead + 1) % i546QSIZE;
	DDEBUG(DMSG, (CE_CONT, "i546get_cmd: msg=%s, ln=%x, cnt=%d, ptr=%x\n",
		i546msgs[buf->m_type], buf->m_line, buf->m_cnt, buf->m_ptr));
	return(1);		/* return 1 if success */
}

/* ARGSUSED */
void
i546tmout(notused)
caddr_t notused; /* dummy variable that callout will supply */
{
	time_t diff;
	register int    dev;
	int		s;

	for (dev=0; dev < i546_cnt; dev++) {
		if( ! (i546board[dev].b_alive & ALIVE))
			continue;
		(void) drv_getparm(TIME, &diff);
		diff -= i546_time[dev];
		if (diff > MAXTIME)  {
			/*
			 * Assume we missed an interrupt:
			 * force output of next char
			 */
			s = SPL();    /* mask out other interrupts */
			i546intr(i546cfg[dev].c_level);
			splx(s);      /* restore interrupt to previous
					 priority level */
		}
	}
	i546tmrun = 1;
	(void)timeout(i546tmout, (caddr_t)0, HZ);
}

#ifdef lint
/*
 *	Reference each routine that the kernel does, to keep lint happy.
*/
main()
{
	i546init();
	return 0;
}
#endif /*lint*/
