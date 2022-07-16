/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/time_data.c	1.3"
#include	"synonyms.h"
#include 	<time.h>
#include 	<tzfile.h>

/* This file contains constant data used by the functions in time_comm.c.
 * The data is in a separate file to get around a limitation in the current
 * compiler; when a file is compiled with -KPIC, it doesn't have enough
 * information to know that it can put const arrays in rodata.  The amount
 * of data has an impact on dynamic shared library performance
 */

const short	__month_size[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


const int	__mon_lengths[2][MONS_PER_YEAR] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const int	__year_lengths[2] = {
	DAYS_PER_NYEAR, DAYS_PER_LYEAR
};

const int __yday_to_month[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
const int __lyday_to_month[12] = {0,31,60,91,121,152,182,213,244,274,305,335};

const struct {
	int	yrbgn;
	int	daylb;
	int	dayle;
} __daytab[] = {
	87,	96,	303,	/* new legislation - 1st Sun in April	*/
	76,	119,	303,
	75,	58,	303,	/* 1975: Last Sun in Feb - last Sun in Oct */
	74,	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	69,	119,	303,	/* start GMT	*/
	0,	119,	303,	
};
