/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)getclk:getclk.c	1.1"

/*
 * Getclk -- get a string suitable for 'date' from the AT/386 real-time
 * clock and write it to stdout.  Exit code 1 if it doesn't work, 0 if
 * ok...
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/rtc.h>

int rtcfile;                            /* rtc file descriptor */
unsigned char rtcbuf[RTC_NREG];         /* for bytes back from rtc */
char outstr[11];                        /* for output string */
char *strptr = outstr;                  /* pointer into outstr */
int regorder[] = {8, 7, 4, 2, 9, -1};	/* the order we look in rtcbuf */
int *rptr = regorder;			/* -> next index to look at */

main()
{

if ((rtcfile=open("/dev/rtc",O_RDONLY)) < 0)
	{
	perror("getclk: opening /dev/rtc");
	exit(1);
	}

if(ioctl(rtcfile,RTCRTIME,rtcbuf) < 0)
	{
	perror("getclk: RTCRTIME ioctl");
	exit(1);
	}

while (*rptr > 0)
	{
	*strptr++ = (rtcbuf[*rptr] >> 4) + '0';
	*strptr++ = (rtcbuf[*rptr++] & 0xf) + '0';
	}
*strptr = 0;
printf("%s\n",outstr);
exit(0);
}
