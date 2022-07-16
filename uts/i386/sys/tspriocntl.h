/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TSPRIOCNTL_H
#define _SYS_TSPRIOCNTL_H

#ident	"@(#)head.sys:sys/tspriocntl.h	1.5.6.1"
/*
 * Time-sharing class specific structures for the priocntl system call.
 */

typedef struct tsparms {
	short	ts_uprilim;	/* user priority limit */
	short	ts_upri;	/* user priority */
} tsparms_t;


typedef struct tsinfo {
	short	ts_maxupri;	/* configured limits of user priority range */
} tsinfo_t;

#define	TS_NOCHANGE	-32768

/*
 * The following is used by the dispadmin(1M) command for
 * scheduler administration and is not for general use.
 */

typedef struct tsadmin {
	struct tsdpent	*ts_dpents;
	short		ts_ndpents;
	short		ts_cmd;
} tsadmin_t;

#define	TS_GETDPSIZE	1
#define	TS_GETDPTBL	2
#define	TS_SETDPTBL	3


#endif	/* _SYS_TSPRIOCNTL_H */
