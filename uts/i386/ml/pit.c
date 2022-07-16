/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-ml:pit.c	1.3.2.1"

/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

#include "sys/types.h"
#include "sys/dl.h"
#include "sys/param.h"
#include "sys/pit.h"
#include "sys/inline.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"

/* for hrtimer in clkstart() */
extern uint	timer_resolution;
extern dl_t	tick;
extern uint	ticks_til_clock;
extern uint	unix_tick;

/* #define	TIME_CHECK			/* Debugging */
/* #define TIME_CHECKD		/* detailed Debugging */

#ifdef AT386
extern int eisa_bus;
extern int sanity_clk;
#endif

int pitctl_port  = PITCTL_PORT;		/* For 386/20 Board */
int pitctr0_port = PITCTR0_PORT;	/* For 386/20 Board */
int pitctr1_port = PITCTR1_PORT;	/* For 386/20 Board */
int pitctr2_port = PITCTR2_PORT;	/* For 386/20 Board */
/* We want PIT 0 in square wave mode */

int pit0_mode = PIT_C0|PIT_SQUAREMODE|PIT_READMODE ;
#ifdef AT386
int sanity_ctl	 = SANITY_CTL;		/* EISA sanity ctl word */
int sanity_ctr0	 = SANITY_CTR0;		/* EISA sanity timer */
int sanity_port	 = SANITY_CHECK;	/* EISA sanity enable/disable port */
int sanity_mode = PIT_C0|PIT_ENDSIGMODE|PIT_READMODE ;
int sanity_enable = ENABLE_SANITY;
int sanity_reset = RESET_SANITY;
#endif

unsigned int delaycount;		/* loop count in trying to delay for
					 * 1 millisecond
					 */
unsigned long microdata=50;		/* loop count for 10 microsecond wait.
					   MUST be initialized for those who
					   insist on calling "tenmicrosec"
					   it before the clock has been
					   initialized.
					 */
unsigned int clknumb = CLKNUM;		/* interrupt interval for timer 0 */
#ifdef AT386
unsigned int sanitynum = SANITY_NUM;	/* interrupt interval for sanitytimer*/
#endif
clkstart()
{
	unsigned int	flags;
	unsigned char	byte;

	findspeed();
	microfind();
	intr_disable();         /* disable interrupts */
	/* Since we use only timer 0, we program that.
	 * 8254 Manual specifically says you do not need to program
	 * timers you do not use
	 */
	outb(pitctl_port, pit0_mode);
	byte = clknumb;
	outb(pitctr0_port, byte);
	byte = clknumb>>8;
	outb(pitctr0_port, byte); 

	/* initialize hrtimer variables */

	ticks_til_clock = timer_resolution / HZ;
	unix_tick = ticks_til_clock;
 
 	/*
  	 * The variable "tick" is used in hrt_alarm for
  	 * HRT_RALARM case.
         */
	tick.dl_lop = SCALE / timer_resolution;

	intr_restore();         /* restore interrupt state */
}

sanity_init()
{
#ifdef AT386
 	unsigned char	byte;

        /* Now set sanity timer if EISA machine and tunable on */
	if (eisa_bus && sanity_clk) {
		intr_disable();		/* disable interrupts */
		outb(sanity_ctl, sanity_mode);
		byte = sanitynum;
		outb(sanity_ctr0, byte);
		byte = sanitynum>>8;
		outb(sanity_ctr0, byte);
		outb(sanity_port, sanity_reset);
		outb(sanity_port, sanity_enable);
		intr_restore();		/* restore interrupt state */
	}
#endif
}

#define COUNT	0x2000

findspeed()
{
	unsigned int flags;
	unsigned char byte;
	unsigned int leftover;
	int i;
	int j;
#ifdef TIME_CHECK
	register int	tval;
#endif	/* TIME_CHECK */

	intr_disable();                 /* disable interrupts */
	/* Put counter in count down mode */
#define PIT_COUNTDOWN PIT_READMODE|PIT_NDIVMODE
#ifdef TIME_CHECKD
	for(tval=0x400;tval<=0x8000;tval=tval*2) {
		outb(pitctl_port, PIT_COUNTDOWN);
		outb(pitctr0_port, 0xff);outb(pitctr0_port, 0xff);
		delaycount=tval; spinwait(1);
		byte = inb(pitctr0_port);leftover = inb(pitctr0_port);
		leftover = (leftover<<8) + byte ;
		/* delaycount = (tval * CLKNUM) / ((0xffff-leftover)*(1000/HZ)); */
		/* delaycount = (tval * CLKNUM * HZ) / ((0xffff-leftover)*1000); */
		delaycount = (((tval * CLKNUM)/1000) * HZ) / (0xffff-leftover);
		printf("findspeed:tval=%x, dcnt=%d, left=%x, top=%x, btm=%x\n",
			tval, delaycount, leftover, 
			(((tval * CLKNUM)/1000) * HZ), (0xffff-leftover) );
		/*	tval*CLKNUM*HZ, ((0xffff-leftover)*1000) ); */
		/*	tval*CLKNUM, ((0xffff-leftover)*(1000/HZ)) ); */
	}
#endif	/* TIME_CHECKD */
	outb(pitctl_port, PIT_COUNTDOWN);
	/* output a count of -1 to counter 0 */
	outb(pitctr0_port, 0xff);
	outb(pitctr0_port, 0xff);
	delaycount = COUNT;
	spinwait(1);
	/* Read the value left in the counter */
	byte = inb(pitctr0_port);	/* least siginifcant */
	leftover = inb(pitctr0_port);	/* most significant */
	leftover = (leftover<<8) + byte ;
	/* Formula for delaycount is :
	 *  (loopcount * timer clock speed)/ (counter ticks * 1000)
	 * 1000 is for figuring out milliseconds 
	 */
	delaycount = (((COUNT * CLKNUM)/1000) * HZ) / (0xffff-leftover);
#ifdef TIME_CHECK
	printf("delaycount for one millisecond delay is %d\n", delaycount);
#endif	/* TIME_CHECK */
	intr_restore();         /* restore interrupt state */
}

spinwait(millis)
	int millis;		/* number of milliseconds to delay */
{
	int i, j;

	for (i=0;i<millis;i++)
		for (j=0;j<delaycount;j++)
			;
}

#define MICROCOUNT	0x2000

microfind()
{
	unsigned int flags;
	unsigned char byte;
	unsigned short leftover;
#ifdef TIME_CHECK
	register int	tval;
#endif	/* TIME_CHECK */


	intr_disable();                 /* disable interrupts */
#ifdef TIME_CHECKD
	for(tval=0x80;tval<=0x80000;tval=tval*2) {
		outb(pitctl_port, PIT_COUNTDOWN);
		outb(pitctr0_port, 0xff);outb(pitctr0_port, 0xff);
		microdata=tval;	tenmicrosec();
		byte = inb(pitctr0_port);leftover = inb(pitctr0_port);
		leftover = (leftover<<8) + byte ;
		microdata = (unsigned)(tval * CLKNUM * HZ) /
				((unsigned)(0xffff-leftover)*100000);
		microdata = (unsigned)(tval * CLKNUM) /
				((unsigned)(0xffff-leftover)*(100000/HZ));
		printf("tval=%x, mdat=%x, left=%x\n",tval,microdata,leftover);
	}
#endif	/* TIME_CHECKD */

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
	microdata = (unsigned)(MICROCOUNT * CLKNUM) /
			((unsigned)(0xffff-leftover)*(100000/HZ));
	if (!microdata)
		microdata++;
#ifdef TIME_CHECK
	printf("delaycount for ten microsecond delay is %d\n", microdata);
#endif	/* TIME_CHECK */
	intr_restore();         /* restore interrupt state */
}
