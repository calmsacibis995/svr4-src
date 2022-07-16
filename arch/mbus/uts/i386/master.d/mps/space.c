/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/mps/space.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/cred.h"
#include "sys/ddi.h"

/* outgoing transactions */
#define MAX_TRAN 400
tinfo_t mps_tinfo[MAX_TRAN];
unsigned char mps_t_ids[MAX_TRAN];
int mps_max_tran = MAX_TRAN;

/* open ports(channels) */
#define MAX_PORT 64
int mps_max_port = MAX_PORT;
port_t mps_port_defs[MAX_PORT];
unsigned short mps_port_ids[MAX_PORT];

/* priority queues of messages to be delivered */
#define MAX_PRIO 8
msgque_t mps_prioq[MAX_PRIO];
int mps_ckprio = -1;

/*
 *	V.4 can run out of memory for short periods of time when it
 *	gets busy.  The mps_msg_lowat variable should be set to the
 *	number of msgbufs likely to be needed by interrupt threads
 *	during these brief periods.
*/
int mps_msg_lowat = 50;
