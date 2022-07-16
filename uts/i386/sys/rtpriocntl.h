/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RTPRIOCNTL_H
#define _SYS_RTPRIOCNTL_H

#ident	"@(#)head.sys:sys/rtpriocntl.h	1.4.7.1"
/*
 * Real-time class specific structures for the priocntl system call.
 */

typedef struct rtparms {
	short	rt_pri;		/* real-time priority */
	ulong	rt_tqsecs;	/* seconds in time quantum */
	long	rt_tqnsecs;	/* additional nanosecs in time quantum */
} rtparms_t;


typedef struct rtinfo {
	short	rt_maxpri;	/* maximum configured real-time priority */
} rtinfo_t;


#define	RT_NOCHANGE	-1
#define RT_TQINF	-2
#define RT_TQDEF	-3


/*
 * The following is used by the dispadmin(1M) command for
 * scheduler administration and is not for general use.
 */

typedef struct rtadmin {
	struct rtdpent	*rt_dpents;
	short		rt_ndpents;
	short		rt_cmd;
} rtadmin_t;

#define	RT_GETDPSIZE	1
#define	RT_GETDPTBL	2
#define	RT_SETDPTBL	3


#endif	/* _SYS_RTPRIOCNTL_H */
