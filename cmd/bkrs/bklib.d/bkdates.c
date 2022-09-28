/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkdates.c	1.7.2.1"

/*
#include <sys/types.h>
*/
#include <time.h>
#include <stdio.h>
#include <backup.h>

#define SECS_PER_DAY (24*60*60)
#define ABS(x)	((x>0)?x:-x)
/* Convert a 2-digit year into a 4-digit one - this will work until the 
	year 2074 */
#define YCONV(y)	((y < 75)? (2000 + y): (1900 + y))
#define is_leap(y) ( !(y%4) && (y%100) || !(y%400) )

/* Return the day of the year: 1 - 365
	given month: 1 - 12; day: 1 - 31; year: 1987
*/
#define day_of_year(m,d,y) (ydaytab[ is_leap(y) ][m - 1] + d - 1)

static int ydaytab [2][12] = {
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};
static int mdaytab [2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
/* ROUTINES THAT DO THINGS WITH DATES */

/* Return the day of the week: 0 - Sunday, 1 - Monday, ...
	given month: 1 - 12; day: 1 - 31; year: 1987
*/
static
day_of_week( month, day, year )
int month, day, year;
{
	register tmp, doy = day_of_year( month, day, year );
	tmp = year - 1751 + (year - 1753)/4 - (year - 1701)/100 + (year - 1601)/400
		+ doy - 1;
	tmp = tmp%7;
	return( tmp );
}

/* Return the number of leap years between two years */
static
n_leaps( a, b )
int a, b;
{
	register tmp;
	if( b < a ) {
		tmp = a;
		a = b;
		b = tmp;
	}
	for( tmp = 0; a < b; a++ )
		if( is_leap(a) ) tmp++;
	return( tmp );
}

/* Convert date to previous Sunday */
static void
conv_to_sunday( month, day, year )
int *month, *day, *year;
{
	register wday = day_of_week( *month, *day, *year );
	if( !wday ) return;
	if( wday < *day ) {
		*day -= wday;
	} else {
		*day = *day + mdaytab[ is_leap( *year ) ][ (*month - 1)%12 ] - wday;
		if( *month > 1 ) (*month)--;
		else {
			*month = 12;
			(*year)--;
		}
	}
}

/*
	Return week/day in current cycle
*/
int
bkget_now( str, period, week, day )
unsigned char *str;
int period, *week, *day;
{
	int cday, cmonth, cyear;
	struct tm *date;
	time_t now = time( 0 );
	register c_yday, totaldays;
	date = localtime( &now );
	if( sscanf( (char *)str, "%2d%2d%2d", &cyear, &cmonth, &cday ) != 3 )
		return( FALSE );
	/* Convert years to 4-digit format */
	cyear = YCONV(cyear);
	date->tm_year += 1900;

	/* XXX - for now: if cmonth, cday, cyear is not a SUNDAY, use the
		previous Sunday */
	conv_to_sunday( &cmonth, &cday, &cyear );

	c_yday = day_of_year( cmonth, cday, cyear );

	if( date->tm_year == cyear ) {
		totaldays = ABS( date->tm_yday - c_yday );
	} else if( date->tm_year > cyear ) {
		totaldays = 365 + is_leap( cyear ) - c_yday + date->tm_yday
			+ n_leaps( cyear, date->tm_year ) + date->tm_year - cyear - 1;
	} else totaldays = 365 + is_leap( date->tm_year ) 
		- date->tm_yday + c_yday + n_leaps( date->tm_year, cyear )
		+ cyear - date->tm_year - 1;

	*week = ((totaldays/7) % period) + 1;
	/* This works only if the day of rotate start is 0 (Sunday) */
	*day = (totaldays % 7);
	return( TRUE );
}

/*
	Return YYMMDD of 'cweek' Sundays previous to today.
*/
unsigned char *
bkget_rotatestart( cweek )
int cweek;
{
	static unsigned char buffer[ 10 ];
	struct tm *date;
	time_t now = time( 0 );
	date = localtime( &now );
	now -= date->tm_wday * SECS_PER_DAY + cweek * 7 * SECS_PER_DAY;
	date = localtime( &now );
	(void) sprintf( (char *)buffer, "%2.2d%2.2d%2.2d", date->tm_year, date->tm_mon + 1,
		date->tm_mday );
	return( buffer );
}
