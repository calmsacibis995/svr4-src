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

#ident	"@(#)mbus:uts/i386/master.d/atcs/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/atcs.h"
#include "sys/atcsmp.h"

/* maximum number of lines that can be driven by ATCS */
/*  (The coding restriction is 256 lines) */
#define MAXATCSLINES (ATCSMINORMSK+1)	/* 127 is ATCS console, if any */
int atcsMaxLines = MAXATCSLINES;

/* maximum number of characters to read for at a time */
/*  (If less than ATCSRSmallChars then only unsolicited messages asked for) */
#define MAXICHARS 18
int atcsMaxIChars = MAXICHARS;

/* maximum number of characters to write at a time */
/*  (If less than ATCSSIMaxChars then only unsolicited messages sent) */
#define MAXOCHARS 128
int atcsMaxOChars = MAXOCHARS;

/* one structure for each atcs line being controlled */
struct atcs_lines atcs_line[MAXATCSLINES];

/* table that maps minor number into CCI line number */
/*   entry of -1 says no mapping.  Table value used to index atcs_line */
int atcsMin2CCILine[MAXATCSLINES];

/* table that maps the Bxxxx codes in termio.h to the controller codes */
int atcsbaud[] = {
	ATCSBHANG,  ATCSB50,   ATCSB75,    ATCSB110,
	ATCSB134_5, ATCSB150,  ATCSB200,   ATCSB300,
	ATCSB600,   ATCSB1200, ATCSB1800,  ATCSB2400,
	ATCSB4800,  ATCSB9600, ATCSB19200, ATCSB38400
};

/*
 *  An entry for each ATCS server: slot, beginning minor number, ending
 *  minor number, beginning port number, ending port number.  There are
 *  assumptions in the rc scripts that there are 6 minors for i354 and
 *  then 6 per ATCS board.  If you change this table, you may have to
 *  change the rc scripts too.
*/
struct atcs_info atcs_info[ICS_MAX_SLOT] = {
	{ 0,		 6,	11,	0 },	/* i279 in slot 0 */
	{ 1,		12,	17,	0 },	/* i279 in slot 1 */
	{ 6,		42,	47,	0 },	/* i410 */
	{ ICS_MAX_SLOT,  0,	 0,	0 }	/* end of struct */
};
