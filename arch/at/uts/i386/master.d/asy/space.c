#ident	"@(#)space.c	1.2	92/09/25	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/master.d/asy/space.c	1.3.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/termio.h"
#include "sys/asy.h"
#include "sys/sysinfo.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/debug.h"
#include "sys/eucioctl.h"
#include "sys/ddi.h"
#include "config.h"
#ifdef VPIX
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/v86.h"
#endif

time_t  asylast_time[ASY_UNITS];     /* output char watchdog timeout */
struct strtty asy_tty[ASY_UNITS];

int	asyrflag[ASY_UNITS] = 
	{
		0 
#ifdef ASY_1
		 ,
		0
#endif  /* ASY_1 */
#ifdef ASY_2
		 ,
		0
#endif  /* ASY_2 */
#ifdef ASY_3
		 ,
		0
#endif  /* ASY_3 */
#ifdef ASY_4
		 ,
		0
#endif  /* ASY_4 */
#ifdef ASY_5
		 ,
		0
#endif  /* ASY_5 */
	};
int	asyiflag[ASY_UNITS] = 
	{
		0 
#ifdef ASY_1
		 ,
		0
#endif  /* ASY_1 */
#ifdef ASY_2
		 ,
		0
#endif  /* ASY_2 */
#ifdef ASY_3
		 ,
		0
#endif  /* ASY_3 */
#ifdef ASY_4
		 ,
		0
#endif  /* ASY_4 */
#ifdef ASY_5
		 ,
		0
#endif  /* ASY_5 */
	};
int	asyoflag[ASY_UNITS] = 
	{
		0 
#ifdef ASY_1
		 ,
		0
#endif  /* ASY_1 */
#ifdef ASY_2
		 ,
		0
#endif  /* ASY_2 */
#ifdef ASY_3
		 ,
		0
#endif  /* ASY_3 */
#ifdef ASY_4
		 ,
		0
#endif  /* ASY_4 */
#ifdef ASY_5
		 ,
		0
#endif  /* ASY_5 */
	};
int	space[ASY_UNITS] = {
		0
#ifdef ASY_1
		 , 
		0
#endif  /* ASY_1 */
#ifdef ASY_2
		 ,
		0
#endif  /* ASY_2 */
#ifdef ASY_3
		 ,
		0
#endif  /* ASY_3 */
#ifdef ASY_4
		 ,
		0
#endif  /* ASY_4 */
#ifdef ASY_5
		 ,
		0
#endif  /* ASY_5 */
	};  /* count for global bp */

#ifdef VPIX
v86_t *asystash[ASY_UNITS];
int asyintmask[ASY_UNITS];
int	asy_v86_prp_pd[2*ASY_UNITS];
struct termss asyss[ASY_UNITS];
int asy_closing[ASY_UNITS] =
	{
		0
#ifdef ASY_1
		,
		0
#endif /* ASY_1 */
#ifdef ASY_2
		 ,
		0
#endif  /* ASY_2 */
#ifdef ASY_3
		 ,
		0
#endif  /* ASY_3 */
#ifdef ASY_4
		 ,
		0
#endif  /* ASY_4 */
#ifdef ASY_5
		 ,
		0
#endif  /* ASY_5 */
	};
char asy_opened[ASY_UNITS];
#endif /* VPIX */

struct asy asytab[ASY_UNITS] =
	{
	0,		/* flags */
	ASY_0_SIOA+0,	/* transmit/recv register port address */
	ASY_0_SIOA+1,	/* interrupt control reg. port address */
	ASY_0_SIOA+2,	/* interrupt status reg. port address */
	ASY_0_SIOA+3,	/* line control register port address */
	ASY_0_SIOA+4,	/* modem control reg. port address */
	ASY_0_SIOA+5,	/* line status register port address */
	ASY_0_SIOA+6,	/* modem status reg. port address */
	ASY_0_VECT,	/* interrupt vector (machine dependent) */
	0,		/* global bp structure */
	0		/* minor device number of port */
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#ifdef ASY_1
		,
			/* next structure */
	0,
	ASY_1_SIOA+0,
	ASY_1_SIOA+1,
	ASY_1_SIOA+2,
	ASY_1_SIOA+3,
	ASY_1_SIOA+4,
	ASY_1_SIOA+5,
	ASY_1_SIOA+6,
	ASY_1_VECT,
	0,		/* global bp structure */
	1
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#endif  /* ASY_1 */

#ifdef ASY_2
		,
			/* next structure */
	0,
	ASY_2_SIOA+0,
	ASY_2_SIOA+1,
	ASY_2_SIOA+2,
	ASY_2_SIOA+3,
	ASY_2_SIOA+4,
	ASY_2_SIOA+5,
	ASY_2_SIOA+6,
	ASY_2_VECT,
	0,		/* global bp structure */
	1
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#endif  /* ASY_2 */

#ifdef ASY_3
		,
			/* next structure */
	0,
	ASY_3_SIOA+0,
	ASY_3_SIOA+1,
	ASY_3_SIOA+2,
	ASY_3_SIOA+3,
	ASY_3_SIOA+4,
	ASY_3_SIOA+5,
	ASY_3_SIOA+6,
	ASY_3_VECT,
	0,		/* global bp structure */
	1
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#endif  /* ASY_3 */

#ifdef ASY_4
		,
			/* next structure */
	0,
	ASY_4_SIOA+0,
	ASY_4_SIOA+1,
	ASY_4_SIOA+2,
	ASY_4_SIOA+3,
	ASY_4_SIOA+4,
	ASY_4_SIOA+5,
	ASY_4_SIOA+6,
	ASY_4_VECT,
	0,		/* global bp structure */
	1
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#endif  /* ASY_4 */

#ifdef ASY_5
		,
			/* next structure */
	0,
	ASY_5_SIOA+0,
	ASY_5_SIOA+1,
	ASY_5_SIOA+2,
	ASY_5_SIOA+3,
	ASY_5_SIOA+4,
	ASY_5_SIOA+5,
	ASY_5_SIOA+6,
	ASY_5_VECT,
	0,		/* global bp structure */
	1
#ifdef MERGE386
                ,
        0,
        0
#endif /* MERGE386 */

#endif  /* ASY_5 */
	};

int	num_asy = ASY_UNITS;
unsigned	p_asy0 = ASY_0_SIOA;

#ifdef ASY_1
unsigned        p_asy1 = ASY_1_SIOA;
extern int      asyinitialized;

/*
 * put a character out the second serial port.
 * Do not use interrupts.  If char is LF, put out LF, CR.
 */
int
asyputchar2(c)
unsigned char   c;
{
        if (! asyinitialized)
                asyinit();
        if (inb(p_asy1+ISR) & 0x38)
                return;
        while((inb(p_asy1+LSR) & XHRE) == 0) /* wait for xmit to finish */
        {
                if ((inb(p_asy1+MSR) & DCD) == 0)
                        return;
                tenmicrosec();
        }
        outb(p_asy1+DAT, c); /* put the character out */
        if (c == '\n')
                asyputchar2(0x0d);
}

/*
 * get a character from the second serial port.
 *
 * If no character is available, return -1.
 * Run in polled mode, no interrupts.
 */

int
asygetchar2()
{
        if ((inb(p_asy1+ISR) & 0x38) || (inb(p_asy1+LSR) & RCA) == 0) {
                tenmicrosec();
                return -1;
        }
        return inb(p_asy1+DAT);
}
#else
int
asyputchar2()
{
}
int
asygetchar2()
{
}
#endif /* P_ASY1 */
