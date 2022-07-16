/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1983, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/master.d/i546/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/termio.h"
#include "sys/tty.h"
#include "sys/i546.h"	/* 546/48 drive structures and constants */
#include "sys/cred.h"
#include "sys/ddi.h"
#include "config.h"


#define NUM546	I546_UNITS
int	i546_cnt = NUM546;

/*
 * The physical starting addresses of the i546 boards'
 * memory mapped I/O space.
 * Unfortunately, the base address does not fall within the limits specified
 * for SCMA field in sdevice file and hence idconfig mechanism cannot be
 * used.
 */
struct	i546cfg	i546cfg[] = {
#ifdef I546_0
	/* board # 0 */
		0xF90000,		/* base address */
		I546_0_SIOA,	/* wakeup port */
		I546_0_VECT,	/* interrupt level */
#endif
#ifdef I546_1
	/* board # 1 */
		0xFA0000,		/* base address */
		I546_1_SIOA,	/* wakeup port */
		I546_1_VECT,	/* interrupt level */
#endif
#ifdef I546_2
	/* board # 2 */
		0xFB0000,		/* base address */
		I546_2_SIOA,	/* wakeup port */
		I546_2_VECT,	/* interrupt level */
#endif
#ifdef I546_3
	/* board # 3 */
		0xF80000,		/* base address */
		I546_3_SIOA,	/* wakeup port */
		I546_3_VECT,	/* interrupt level  (slave 0 ) */
#endif
#ifdef I546_4
	/* board # 4 */
		0xFC0000,		/* base address */
		I546_4_SIOA,	/* wakeup port */
		I546_4_VECT,	/* interrupt level */
#endif
#ifdef I546_5
	/* board # 5 */
		0xFD0000,		/* base address */
		I546_5_SIOA,	/* wakeup port */
		I546_5_VECT,	/* interrupt level */
#endif
#ifdef I546_6
	/* board # 6 */
		0xFE0000,		/* base address */
		I546_6_SIOA,	/* wakeup port */
		I546_6_VECT,	/* interrupt level */
#endif
#ifdef I546_7
	/* board # 7 */
		0xFF0000,		/* base address */
		I546_7_SIOA,	/* wakeup port */
		I546_7_VECT,	/* interrupt level */
#endif
};

/*
 * baud rate translation table
 */
int	i546baud[CBAUD+1] = {
	    US_B0,    US_B50,    US_B75,   US_B110,
	  US_B134,   US_B150,   US_B200,   US_B300,
	  US_B600,  US_B1200,  US_B1800,  US_B2400,
	 US_B4800,  US_B9600, US_B19200, US_B38400
};

/*
 * Standard tty structure.
 * Device driver's private data. Space for i546LINES structures
 * is allocated for each board configured in the driver.
 */
/* streams doesn't care about this.
struct	tty	  i546_tty[NUM546*i546LINES];
*/

/*
 * Line and board state data for the iSBC 546/48.
 * Space allocated for each board configured in the driver.
 */
struct	i546board i546board[NUM546];
time_t	i546_time[NUM546];	
