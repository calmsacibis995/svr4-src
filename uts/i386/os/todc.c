/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:todc.c	1.3"

#include "sys/types.h"
#include "sys/time.h"
#include "sys/rtc.h"
#include "sys/debug.h"

extern timestruc_t time;
time_t c_correct;

/*
 * This file contains interface routines to access the hardware
 * time of day clock:  rtodc to read it, wtodc to write it.
 * The actual routines for talking to the rtc chip are
 * in the rtc driver.  rtodc merely allows access to the driver
 * by code such as the sysi86 RTODC command.
 * wtodc takes the internal time (a form of GMT), converts it
 * to local time (see the uadmin A_CLOCK for how kernel gets
 * the needed correction) in the rtc's format and writes it.
 */

/* rtodc: return 0 on success, -1 on failure. */

rtodc(clkp)
	struct rtc_t	*clkp;
{
	int oldspl;
	int rval;
#ifdef AT386

	/* rtcget expects to be called at spl5 */
	oldspl = spl5();
	rval = rtcget(clkp, 0);
	splx (oldspl);

#elif MB1 || MB2

	rval = clkget(clkp, 0);
#else /* NOT AT386, MB1 or MB2 */
	rval = -1;
#endif /* AT386, MB1 and MB2 */
	return(rval);
}

/* partially copied from ctime.c libc code to be consistent */
#define dysize(A) (((A)%4)? 365: 366)

static int dmsize[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

wtodc()
{
	struct rtc_t	clkx;
	long hms, days;
	long tim;
	register int i;
	register int yr, ydc, mon;
	int oldspl;

#ifdef AT386

	/* rtcget/rtcput expect to be called at spl5 */
	oldspl = spl5();
	if (rtcget(&clkx, 0)) {
		splx(oldspl);
		return(-1);
	}
	splx (oldspl);

#elif MB1 || MB2

	if (clkget(&clkx, 0)) 
		return(-1);
#endif /* AT386, MB1 and MB2 */

	/* the format of a rtc byte:
	 * if the decimal value for the concept is TU,
	 * the hexadecimal value fro the rtc byte is 0xTU.
	 * Of course, I have no documentation on the conventions
	 * to use for dow (day of week) values.
	 * My machine has a value of 4 for Sunday???
	 */
#define hexizeit(i)	((((i)/10)<<4)|((i)%10))
	tim = hrestime.tv_sec;
	hms = days = tim - c_correct;
	hms = ((unsigned)hms) % 86400;
	days = ((unsigned)days) / 86400;
	i = hms % 60;
	clkx.rtc_sec = hexizeit(i);
	hms /= 60; /* now in minutes */
	i = hms % 60;
	clkx.rtc_min = hexizeit(i);
	hms /= 60; /* now in hours */
	clkx.rtc_hr = hexizeit(hms);
	/* 1/1/70 was a Thursday */
	i = (days+4) % 7;
	clkx.rtc_dow = i;
	for (yr = 70, ydc = dysize(yr); days >= ydc; yr++, ydc = dysize(yr))
		days -= ydc;
	/* assume that we wrap the rtc year back to zero at 2000 */
	if (yr >= 100) yr -= 100;
	clkx.rtc_yr = hexizeit(yr);
	if (ydc == 366)
		dmsize[1] = 29;
	for (mon = 0; days >= dmsize[mon]; mon++)
		days -= dmsize[mon];
	dmsize[1] = 28;
	mon++;
	clkx.rtc_mon = hexizeit(mon);
	days++;
	clkx.rtc_dom = hexizeit(days);
#ifdef AT386

	oldspl = spl5();
	rtcput(&clkx);
	splx(oldspl);

#elif MB1 || MB2

	if (clkput (&clkx))
		return (-1);
#endif /* AT386, MB1 and MB2 */

	return(0);
}
