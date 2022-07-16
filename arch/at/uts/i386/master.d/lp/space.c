/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/master.d/lp/space.c	1.3"

#include "config.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/lp.h"
#include "sys/dma.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/termio.h"
#include "sys/cmn_err.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/debug.h"
#include "sys/eucioctl.h"
#include "sys/ddi.h"

int NUMLP=LP_UNITS;

struct strtty lp_tty[LP_UNITS];   /* tty structs for each device */
time_t  last_time[LP_UNITS];     /* output char watchdog timeout */

/* intr_vec[] tells the driver what interrupt vector to expect from
   which printer. Acceptable values are 5 and 7. Two printers cannot
   share an interrupt vector
*/

 
struct lpcfg lpcfg[LP_UNITS] = {
	0,				/* state */
	LP_0_SIOA+0,			/* data register port address */
	LP_0_SIOA+1,			/* status register port address */
	LP_0_SIOA+2,			/* control register port address */
	LP_0_VECT	 		/* interrupt vector */
#ifdef LP_1
		,
					/* next structure */	
	0,
	LP_1_SIOA+0,
	LP_1_SIOA+1,
	LP_1_SIOA+2,
	LP_1_VECT
#endif	/* LP_1 */
#ifdef LP_2
		,
					/* next structure */	
	0,
	LP_2_SIOA+0,
	LP_2_SIOA+1,
	LP_2_SIOA+2,
	LP_2_VECT
#endif	/* LP_2 */
	};
