/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/mpc.h	1.3.1.1"

#ifndef _SYS_MPC_H
#define _SYS_MPC_H

#ifndef _SYS_MPS_H
#include "sys/mps.h"
#endif


/*
 * THIS FILE CONTAINS CODE WHICH IS DESIGNED TO BE
 * PORTABLE BETWEEN DIFFERENT MACHINE ARCHITECTURES
 * AND CONFIGURATIONS. IT SHOULD NOT REQUIRE ANY
 * MODIFICATIONS (except MPC_BASE) WHEN ADAPTING UNIX TO NEW HARDWARE.
 */

/*
 * This file contains the port offsets and masks used
 * to manipulate the MPC chip.
 */

/* MPC register address offsets */
#define MDATA	0x10
#define MCMD	0x1C
#define MSTAT	0x00
#define MRST	0x00
#define MCTL	0x0C
#define MERR	0x14
#define MSOCMP	0x20
#define MSICMP	0x24
#define MSOCAN	0x20
#define MSICAN	0x24
#define MCON	0x08
#define MID	0x04

/* register constant values */
#define SET_INTR	0x1A
#define MD_RESET	0x00
#define MD_CLEAR	0x00
#define MDISABLE	0x00
#define MD_EOI		0x00

/* status register masks */
#define XMTNF		0x01
#define RCVNE		0x02
#define XMTERR		0x04
#define SOCMP		0x08
#define SICMP		0x10
#define MD_INIT		0x80
#define MD_INTMASK	0x1F

#define MRID_SIZE	0x04

/* MPC request id mask - limits rid to 4 bits */
#define RID_MASK	0x0F

#define MD_DELAY 0x1000
#define MSG_CONF 0x8A 

/* Solicited completion error bits */
#define MD_SCANCL 0x10

/* logical mpc channels */
#define MPC_SICHAN	D258_CHAN2
#define MPC_SOCHAN	D258_CHAN3

#if defined __STDC__
extern int impcinit();
extern int impcintr(int);
extern int impc_start(int);
extern int impc_send(mps_msgbuf_t *);
extern int impc_sol_que(tinfo_t *,int);
extern int impc_sol_deque(tinfo_t *);
#else
extern int impcinit();
extern int impcintr();
extern int impc_start();
extern int impc_send();
extern int impc_sol_que();
extern int impc_sol_deque();
#endif

#endif	/* _SYS_MPC_H */
