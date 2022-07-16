/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:kperf.c	1.3"

#include "sys/types.h" 
#ifdef	KPERF
#include "sys/param.h"
#include "sys/reg.h" 
#include "sys/tss.h" 
#include "sys/sysmacros.h" 
#include "sys/immu.h" 
#include "sys/systm.h" 
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h" 
#include "sys/mount.h" 
#include "sys/fstyp.h" 
#include "sys/var.h" 
#include "sys/buf.h" 
#include "sys/utsname.h" 
#include "sys/sysinfo.h" 
#include "sys/proc.h" 
#include "sys/map.h" 
#include "sys/swap.h" 
#include "sys/debug.h" 
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/inline.h"
#include "sys/pit.h"
#include "sys/disp.h"

#define COUNT	0x2000
#define MICROCOUNT	0x2000

kernperf_t kpft[NUMPHASE*NUMRC+5];
extern int hr_lbolt;
int putphase = 0;
int takephase = 0;
/*	putphase = takephase ===> no buffer awaiting being 
**	copied out.
*/
int KPtime = 0;		/* variable to store previous time */
int pre_trace = 0;
int Kpc;
int numrccount = 0;
int numrc;
/* int out_of_tbuf = 0;  */
int outbuf = 0;
int KPF_opsw;			/* to store value of old psw */


int pitctl_port  = PITCTL_PORT;		/* For 386/20 Board */
int pitctr0_port = PITCTR0_PORT;	/* For 386/20 Board */
int pitctr1_port = PITCTR1_PORT;	/* For 386/20 Board */
int pitctr2_port = PITCTR2_PORT;	/* For 386/20 Board */
/* We want PIT 0 in square wave mode */

int pit0_mode = PIT_C0|PIT_SQUAREMODE|PIT_READMODE ;

unsigned long microdata=50;		/* loop count for 10 microsecond wait.
					   MUST be initialized for those who
					   insist on calling "tenmicrosec"
					   it before the clock has been
					   initialized.
					 */
/*	This function was added to gather statistics for the kernel
**	performance statistics gatherer.
*/


void
kperf_write(record_type,kpc,kproc)
int record_type;
int kpc;
proc_t *kproc;

{ 
	register int	counter = 0;	/* counter to record time */
	register kernperf_t  *kpfptr;
	register kspl; 		/* to gather current processor 
				** priority level
				*/

	unsigned int flags;
	unsigned char byte;
	unsigned short leftover;

	counter = 0;	
	/* get current processor level, before critical section */
	kspl = get_spl(); 

	intr_disable();                 /* disable interrupts */
	/* asm(" ORW2 &0x1e000,%psw"); */ 	/* enter critical section */
	kpfptr = &kpft[putphase*NUMRC+numrccount];

	kpfptr->kp_type	= (char) record_type;
	kpfptr->kp_pid	= kproc->p_pid;
	kpfptr->kp_level = (char) kspl; 
	kpfptr->kp_pc = kpc;
	/* time recorded  in 10's of usec's */
	/* asm("	MOVB &0x40, $0x0004200f");	latch cntr 1 of timer     */
	/* asm("	MOVB $0x00042007,%r8");	/* read LEAST sig byte (LSB) */
	/* asm("	MOVB $0x00042007,%r0");	/* read MOST sig byte (MSB)  */	
	/* asm("	LLSW3 &8,%r0,%r0"); */		
	/* asm("	ORW2 %r0,%r8"); */	/* combine LSB and MSB	     */

	/* Put counter in count down mode */
	outb(pitctl_port, PIT_COUNTDOWN);
	/* output a count of -1 to counter 0 */
	outb(pitctr0_port, 0xff);
	outb(pitctr0_port, 0xff);
	microdata=MICROCOUNT;
	tenmicrosec();
	/* Read the value left in the counter */
	byte = inb(pitctr0_port);	/* least siginifcant */
	leftover = inb(pitctr0_port);	/* most significant */
	leftover = (leftover<<8) + byte ;
	/* Formula for delaycount is :
	 *  (loopcount * timer clock speed)/ (counter ticks * 1000)
	 *  Note also that 1000 is for figuring out milliseconds
	 */
	microdata = (MICROCOUNT * CLKNUM) / ((0xffff-leftover)*(100000/HZ));
	if (!microdata)
		microdata++;

	kpfptr->kp_time = (hr_lbolt * 1000) + (1000 - counter); 

	/* XXX - how is counter getting initialized to some value ???
	   - need to look into future versions . The above code for
	   386 specific determines the number of clock ticks in 10 usec.
	   the 3B2 assembly code looks like doing nothing.  
	 */
	
	/* these 3 lines were added to compensate for a hardware problem
	** with time, i.e, if hr_lbolt was not reset before an interrupt
	*/
	/* 3b2 specific */
	/* if ( kpfptr->kp_time < KPtime )
		kpfptr->kp_time = kpfptr->kp_time + 1000;
	*/

	KPtime = kpfptr->kp_time;
	if (++numrccount >= NUMRC) {
		numrccount -= NUMRC;
		putphase = (putphase+1) % NUMPHASE;		
		if (putphase == takephase) { /* buffers are full */
			cmn_err(CE_CONT, "OUT OF TRACE BUFFERS\n");
			/* don't write any more records, otherwise infinite loop */
			outbuf = 1;
			kpftraceflg = 0;
			wakeprocs((caddr_t) &kpft[takephase*NUMRC], PRMPT);
			return;
		}
		wakeprocs((caddr_t)&kpft[takephase*NUMRC], PRMPT);
	}
	/* get out of critical section */
	/* restore psw before exiting the critical section */
	/* asm(" MOVW  KPF_opsw,%psw");   */
	intr_restore();         /* restore interrupt state */
}
#endif	/* KPERF */
