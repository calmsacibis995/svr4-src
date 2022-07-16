/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/master.d/i8251/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/tty.h"
#include "sys/i8251.h"
#include "config.h"
#include "sys/cred.h"
#include "sys/ddi.h"

/*
 *  This is the configuration file for the 8251 device driver.  There is a
 *  single 8251 on the 386/[23]x board, which is the console.  This driver
 *  will also support an iSBX351 attached to the SBX connector.
 *
 *  NOTE: sdev file has 2 entries per 8251 so as to get input and output
 * 		  interrupt vectors defined for each 8251 using idconfig mechanism.
 */

#if defined(I8251_0) && defined(I8251_1) && defined(I8251_2) && defined(I8251_3)
#define	NUM8251	2
#elif defined(I8251_0) && defined(I8251_1)
#define	NUM8251	1
#endif

ushort i8251_cnt = NUM8251;	

/*
 * This table gives the board-base I/O address
 * for each possible 8251 controller.
 */

struct	i8251cfg i8251cfg[] = {
#if defined(I8251_0) && defined(I8251_1)
    {   
		/* Onboard 8251 parameters */
		I8251_0_SIOA+4,		/* USART Data Port			*/
		I8251_0_SIOA+6,		/* USART Control Port		*/
		I8251_0_SIOA,		/* Timer Data Port			*/
		I8251_0_SIOA+2,		/* Timer Control Port		*/
		I8251_0_VECT,		/* Input interrupt level	*/
		I8251_1_VECT,		/* Output interrupt level	*/
		2,					/* Timer number (on chip)	*/
		T_ONBOARD,      	/* Option specifications	*/
    },
#endif
#if defined(I8251_0) && defined(I8251_1) && defined(I8251_2) && defined(I8251_3)
    {   /* iSBC351 daughter board parameters */
		0x80,		/* USART Data Port		*/
		0x82,		/* USART Control Port		*/
		0x94,		/* Timer Data Port		*/
		0x96,		/* Timer Control Port		*/
		I8251_2_VECT,   /* Input interrupt level	*/
		I8251_3_VECT,   /* Output interrupt level	*/
		2,		/* Timer number (on chip)	*/
		T_iSBC351,      /* Option specifications	*/
    },
#endif
};

ushort  i8251alive[NUM8251];    /* does it live ??		*/
ushort  i8251speed[NUM8251];    /* current speed of tty		*/
ushort  i8251state[NUM8251];    /* current line characteristics	*/
ushort  i8251break[NUM8251];    /* break detected previously	*/
time_t  last_time[NUM8251];     /* output char watchdog timeout */
