/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/ics/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/ics.h"

/*
 * I/O addresses for ICS access for 386/1xx compatible boards 
 */
unsigned long ics_hi_addr= 0x34;
unsigned long ics_low_addr= 0x30;
unsigned long ics_data_addr= 0x3c;

/* slot map of boards in the system */
#define ICS_MAX_SLOT 21
ics_slot_t ics_slotmap[ICS_MAX_SLOT];


/* list of supported cpu boards */

char *ics_cpu_cfglist[] = {
	"386/100", 
	"386/116", 
	"386/120", 
	"386/133", 
	"486/125DU", 
	"486/125", 
	"486/133SE", 
	 0
};

#define MAX_CPU 4
long ics_max_numcpu = MAX_CPU;

/* CPUs are counted from lowest slot(slot #0).
 * only x86/1xx boards are counted as CPUs 
 * Each value is the device number computed as makedev(major,minor)
 */
struct ics_bdev ics_bdev[MAX_CPU] = {
/*	ROOT   SWAP	PIPE     DUMP       CPU	*/
	 0,1,	0,2,	 0,1,	  0,6,       /* 0 */
	 0,17,	0,18,	 0,17,	  0,22,      /* 1 */
	 0,33,	0,34,	 0,33,	  0,38,      /* 2 */
	 0,49,	0,50,	 0,49,	  0,54,      /* 3 */
};
