/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CLOCKCAL_H
#define _SYS_CLOCKCAL_H

/*	Copyright (c) 1986  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/clockcal.h	1.3"

/*
 *  The structure for the 546 clock
 */

struct	clkcal	{
	unsigned char	ck_milsec;	/* 10 thous. of a second, .x0 */
	unsigned char	ck_decsec;	/* 100th and 10th of a second, 0-99 */
	unsigned char	ck_sec;		/* second of minute, 0-59 */
	unsigned char	ck_min;		/* minute of hour, 0-59 */
	unsigned char	ck_hour;	/* hour of the day, 1-23 */
	unsigned char	ck_wday;	/* day of the week, 1-7 */
	unsigned char	ck_mday;	/* day of the month, 1-31 */
	unsigned char	ck_month;	/* month of the year, 1-12 */
	unsigned char	ck_reserv1;	/* reserved field */
	unsigned char	ck_year;	/* year of the centur, 0-99 */
	unsigned char	ck_reserv2;	/* reserved field */
	};
#define CLKSIZE	11			/* the size of the above structure.
					   since the compiler pads it, it
					   doesn't know it is only 11  */

#define	TIME_SET	(('c'<<8)|0)	/* set time on 546 */
#define	TIME_GET	(('c'<<8)|1)	/* get the time from the 546 */


/* the following is for the csm clock  in MBII */

/* command register flag bits */

#define CSM_TD_BUSY   0x80
#define CSM_TD_RDY    0x40
#define CSM_TD_NOP    0x00
#define CSM_TD_RDALL  0x01
#define CSM_TD_WRALL  0x04

/* constants and other definitions */

#define CSM_SLOT  	0
#define CSM_TD_RETRY  	4
#define ICS_HEADER 	6
#define CSM_TD_RECTYPE  0x09

/* csm Time date record cmd registers */

#define CSM_TD_COMREG_OFF 2
#define CSM_TD_SECREG_OFF 3

/* the following is the structure for the csm clock */

struct	csmclk	{
	unsigned char 	csm_msec;
	unsigned char	csm_sec;		/* second of minute, 0-59 */
	unsigned char	csm_min;		/* minute of hour, 0-59 */
	unsigned char	csm_hour;	/* hour of the day, 1-23 */
	unsigned char	csm_mday;	/* day of the month, 1-31 */
	unsigned char	csm_month;	/* month of the year, 1-12 */
	unsigned char	csm_year1;	/* low order year */
	unsigned char	csm_year2;	/* high order year */
};

#define IN_KERNEL	0x1
#define NOT_IN_KERNEL	0x2

#define CLK_I546	0x1
#define CLK_ICSM	0x2

/*
 * the clock structure switch for multiplexing between the 546 clock and
 * the csmclock.
 */

struct clkrtn {
	int	(*c_open)();
	int	(*c_close)();
	int	(*c_read)();
	int	(*c_write)();
};


#endif	/* _SYS_CLOCKCAL_H */
