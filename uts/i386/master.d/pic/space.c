/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:pic/space.c	1.3"

#include "sys/ipl.h"
#include "sys/pic.h"


/* Initialized data for Programmable Interupt Controllers */

unsigned short cmdport[NPIC]    /* command port addrs for pics */
	= { MCMD_PORT, SCMD_PORT };

unsigned short imrport[NPIC]    /* intr mask port addrs for pics */
	= { MIMR_PORT, SIMR_PORT };

unsigned char masterpic[NPIC]   /* index of this pic's master (for 82380) */
	= { 0, 0 };

unsigned char masterline[NPIC]  /* line on master this slave connected to */
	= { 0, MASTERLINE };

unsigned char curmask[NPIC];    /* current pic masks */

unsigned char iplmask[IPLHI*NPIC];    /* pic masks for intr priority levels */

unsigned char picbuffered = PICBUFFERED;        /* PICs in buffered mode */

unsigned char i82380 = I82380;  /* i82380 chip used */

int npic = NPIC;                /* number of pics configured */
