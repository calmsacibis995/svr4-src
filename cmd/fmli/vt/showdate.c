/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:vt/showdate.c	1.6"

#include	<time.h>
#include	<curses.h>
#include	"wish.h"
#include	"vtdefs.h"
/* #include	"status.h"  empty include file abs 9/14/88 */
#include	"vt.h"
#include	"ctl.h"

void
showdate()
{
	register struct tm	*t;
	char	*ctime();
	static int	oldday;
	static char	*day[] = {
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
	};
	static char	*month[] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
	};
	extern time_t	Cur_time;	/* EFT abs k16 */

	Cur_time = time( (time_t)0L );

	t = localtime(&Cur_time);
	if (oldday != t->tm_mday) {
		char	datebuf[DATE_LEN];
		register int	n, s;
		register vt_id	oldvid;
		int	r, c;
		int	datecol;

		vt_ctl(STATUS_WIN, CTGETSIZ, &r, &c);
		datecol = (c - DATE_LEN) / 2;
		oldday = t->tm_mday;
		oldvid = vt_current(STATUS_WIN);
		wgo(0, datecol);
		sprintf(datebuf, "AT&T FACE - %s %s %d, %4d", day[t->tm_wday], month[t->tm_mon], t->tm_mday, t->tm_year + 1900);
		s = strlen(datebuf);
		n = (DATE_LEN - s) / 2;
		wprintf("%*s%s%*s", n, "", datebuf, DATE_LEN - n - s, "");
		vt_current(oldvid);
	}
}
