/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:syssetup/datechk.c	1.1"
#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
	int val;
	char *month;
	int date, year;

	month=argv[1];
	date=atoi(argv[2]);
	year=atoi(argv[3]);

	if (strcmp (month, "Feb")==0 || strcmp (month, "February")==0) {
		val=leap (date, year);
		exit (val);
	}
	else
		if (strcmp (month, "Apr")==0 || strcmp (month, "Jun")==0 ||
		   strcmp (month, "April")==0 || strcmp (month, "June")==0 ||
		   strcmp (month, "September")==0 || strcmp (month, "November")==0 ||
		    strcmp (month, "Sep")==0 || strcmp (month, "Nov")==0)
			if (date==31)
				exit(1);
			else
				exit(0);
		else
			exit(0);
}

leap(d, y)
int  d, y;
{
	if (y % 4==0 && y % 100 != 0 || y % 400==0)  
		if (d >= 30)
			return (1);
		else	return (0);
	else	/* not leap */
		if (d >= 29)
			return (1);
		else
			return (0);
}
