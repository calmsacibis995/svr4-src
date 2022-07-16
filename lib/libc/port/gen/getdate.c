/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getdate.c	1.4"

#ifdef __STDC__
	#pragma weak getdate = _getdate
#endif
#include "synonyms.h"
#include "shlib.h"

#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <stdlib.h>
#include <nl_types.h>
#include <langinfo.h>
#include <errno.h>


extern char	*getenv();
extern int	read();
extern void	free();
extern int	close();
extern int	fstat();
extern int	stat();
extern int	tolower();


extern int __lyday_to_month[];
extern int __yday_to_month[];
extern int __mon_lengths[2][12];

extern int getdate_err;

/*
 * The following are the possible contents of the getdate_err
 * variable and the corresponding error conditions.
 ********************************************************************
 * 1	The DATEMASK environment variable is null or undefined.
 * 2	Error on open of the template file.
 * 3    Error on stat of the template file.
 * 4    The template file is not a regular file.
 * 5    Error on read of the template file.
 * 6	Malloc failed.
 * 7	There is no line in the template that matches the input.
 * 8	Invalid input specification.
 ********************************************************************
 */

static int settime();
static struct	tm  *calc_date();
static int lookup();
static int read_tmpl();
static int parse_fmt();
static void getnow();
static void strtolower();
static int verify();
static void Day();
static void DMY();
static int days();
static int jan1();
static void year();
static void MON();
static void Month();
static void DOW();
static void adddays();
static void DOY();

static char	*input;
static int	hour;
static int	wrong_input;
static int	meridian;
static int	linenum;
static struct   tm  *ct;

enum STR
{
	   aJan, aFeb, aMar, aApr, aMay, aJun, aJul, aAug, aSep, aOct, aNov, aDec,
	   Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec,
	   aSun, aMon, aTue, aWed, aThu, aFri, aSat,
	   Sun, Mon, Tue, Wed, Thu, Fri, Sat,
	   Local_time, Local_date, DFL_FMT,
	   AM, PM,
	   LAST
};
/*
 * Default values.
 */

#ifdef DSHLIB
static char  *saved_locale = 0;
static char **_time;
#else
static char  *saved_locale = "C";

static char *_time[(int)LAST + 1] = {
	"jan", "feb", "mar", "apr", "may", "jun", 
	"jul", "aug", "sep", "oct", "nov", "dec", 

	"january", "february", "march", "april", "may", "june", 
	"july", "august", "september", "october", "november", "december", 

	"sun", "mon", "tue", "wed", "thu", "fri", "sat", 

	"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", 
	"saturday", 

	"%H:%M:%S", "%m/%d/%y", "%a %b %d %H:%M:%S %Y",
	"am", "pm", NULL,
};
#endif
extern const nl_item _time_item[];


#define HUNKSZ	512		/* enough for "C" locale plus slop */

typedef struct Hunk Hunk;
struct	Hunk
{
	size_t	h_busy;
	size_t	h_bufsz;
	Hunk	*h_next;
	char	h_buf[1];
};
static Hunk	*hunk;
static void	inithunk();
static char	*savestr();


struct	tm *
getdate(expression)
const char	*expression;
{

	struct tm t;
	if (settime() == 0)
		return 0;
	wrong_input = 0;
	linenum = 1;
	if (read_tmpl((char *)expression, &t))
		return(calc_date(&t));
	else {
		if (wrong_input)
			getdate_err = 8;
		return(NULL);
	}
}

/*
 * Initialize pointers to month and weekday names and to meridian markers.
 */
extern void tzset();
extern char *tzname[2];
static char mytzname[2][4] = { "GMT", "   "};

static int
settime()
{
	register char *p;
	register int j, k;
	char *locale;

#ifdef DSHLIB
	if (_time == NULL &&
		((_time = malloc(sizeof(char *)*(LAST+1))) == NULL)) {
		errno = ENOMEM;
		goto error;
	}
#endif
	tzset();
	strncpy(&mytzname[0][0], tzname[0], 3);
	strncpy(&mytzname[1][0], tzname[1], 3);
	for(j=0; j<2; j++)
		for(k = 0; k<3; k++) {
			mytzname[j][k] = tolower(mytzname[j][k]);
			if(mytzname[j][k] == ' ') {
				mytzname[j][k] = '\0';
				break;
			}
		}
	locale = setlocale(LC_TIME, NULL);
	if (saved_locale != 0 && strcmp(locale, saved_locale) == 0)
		return 1;
	inithunk();
	if ((locale = savestr(locale)) == 0)
		goto error;
	for (j = 0; j < (int)LAST; ++j)
	{
		if ((p = savestr(nl_langinfo(_time_item[j]))) == 0)
			goto error;
		strtolower(p);
		_time[j] = p;
	}
	saved_locale = locale;
	return 1;
error:
	saved_locale = 0;
	return 0;
}


static void
inithunk()
{
	register Hunk	*hp;

	for (hp = hunk; hp != 0; hp = hp->h_next)
		hp->h_busy = 0;
	return;
}


static char *
savestr(str)
	const char	*str;
{
	register Hunk	*hp;
	size_t		len, avail, sz;
	char		*p;

	len = strlen(str) + 1;
	if ((sz = 2 * len) < HUNKSZ)
		sz = HUNKSZ;
	avail = 0;
	if ((hp = hunk) != 0)
		avail = hp->h_bufsz - hp->h_busy;
	if (hp == 0 || avail < len)
	{
		if ((hp = (Hunk *)malloc(sizeof(*hp) + sz)) == 0)
		{
			getdate_err = 6;
			return 0;
		}
		hp->h_bufsz = sz;
		hp->h_busy = 0;
		hp->h_next = hunk;
		hunk = hp;
	}
	p = &hp->h_buf[hp->h_busy];
	(void)strcpy(p, str);
	hp->h_busy += len;
	return p;
}


static void
strtolower(str)
	register char	*str;
{
	register unsigned char	c;

	while ((c = *str) != '\0')
	{
		if (c == '%' && *(str + 1) != '\0')
			++str;
		else
			*str = tolower(c);
		++str;
	}
}

/*
 * Parse the number given by the specification.
 * Allow at most length digits.
 */
static
number(length)
int	length;
{
	int	val;
	unsigned char c;

	val = 0;
	if(!isdigit((unsigned char)*input))
		return -1;
	while (length--) {
		if(!isdigit(c = *input))
			return val;
		val = 10*val + c - '0';
		input++;
	}
	return val;
}

/*
 * Search for format string in _time array
 */
static
search(start, end)
int	start;
int	end;
{
	int	i, length;
	for (i=start; i<=end; i++) {
		length = strlen(_time[i]);
		if(strncmp(_time[i], input, length) == 0)
		{
			input += strlen(_time[i]);
			return i;
		}
	}
	return(-1);
}

/*
 * Read the user specified template file by line
 * until a match occurs.
 * The DATEMSK environment variable points to the template file.
 */

static char *sinput; /* start of input buffer */

static int
read_tmpl(line, t)
char	*line;
struct tm *t;
{
	FILE  *fp;
	char	*file;
	char *bp, *start;
	struct stat sb;
	int	ret=0, c;


	if (((file = getenv("DATEMSK")) == 0) || file[0] == '\0')
	{
		getdate_err = 1;
		return(0);
	}
	if ((start = (char *)malloc(512)) == NULL)
	{
		getdate_err = 6;
		return(0);
	}
	if ((fp = fopen(file, "r")) == NULL)
	{
		getdate_err = 2;
		free(start);
		return(0);
	}
	if (stat(file, &sb) < 0)
	{
		getdate_err = 3;
		goto end;
	}
	if ((sb.st_mode & S_IFMT) != S_IFREG)
	{
		getdate_err = 4;
		goto end;
	}
	if((sinput = malloc(strlen(line)+1)) == (char *)0) {
		getdate_err = 6;
		goto end;
	}
	input = sinput;
	strcpy(sinput, line);
	while(c = (unsigned char)*input)
		*input++ = tolower(c);
	input = sinput;
	for(;;) {
	 	bp = start;
		if (!fgets(bp, 512, fp)) {
			if (!feof(fp))
			{
				getdate_err = 5;
				ret = 0;
				break;
			}
			getdate_err = 7;
			ret = 0;
			break;
		}
		if (*(bp+strlen(bp)-1) != '\n')  { /* terminating newline? */
			getdate_err = 5;
			ret = 0;
			break;
		}
		*(bp + strlen(bp) - 1) = '\0';
#ifdef DEBUG
printf("line number \"%2d\"---> %s\n", linenum, bp);
#endif
		if (strlen(bp))  /*  anything left?  */
			if (ret = parse_fmt(bp, 0, t))
				break;
		linenum++;
		input = sinput;
	}
end:
	(void) free(start);
	(void) fclose(fp);
	return ret;
}

/*
 * Match lines in the template with input specification.
 */
static int
parse_fmt(bp, flag, t)
	char	*bp;
	int	flag;
	struct  tm *t;
{
	char	*fmt;
	int	ret;
	unsigned char c, d;
	if(!flag)
		getnow(t);
	fmt = bp;
	while ((c = *fmt++) != '\0') {
		if ( c == '%' ) {
			c = *fmt++;
			if(c != 't' && c != 'c' && c != 'x' && c != 'X')
				while(isspace(d = *input))
					input++;
			switch (c) {
			case 'a':
				if((ret = search(aSun, aSat)) < 0)
					return(0);
				ret = ret - (int) aSun + 1;
				if (t->tm_wday && t->tm_wday != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_wday = ret;
				continue;
			case 'w':

				if((ret = number(1)) < 0 || ret > 6)
					return(0);
				if (t->tm_wday && t->tm_wday != ret + 1)
				{
					wrong_input++;
					return(0);
				}
				t->tm_wday = ret + 1;
				continue;

			case 'd':
			case 'e':
				if ((ret = number(2)) < 1 || ret > 31)
					return(0);
				if (t->tm_mday && t->tm_mday != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_mday = ret;
				continue;

			case 'A':
				if ((ret = search(Sun, Sat)) < 0)
					return(0);
				ret = ret - (int) Sun + 1;
				if (t->tm_wday && t->tm_wday != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_wday = ret;
				continue;

			case 'h':
			case 'b':
				if ((ret = search(aJan, aDec)) < 0)
					return(0);
				ret = ret - (int) aJan + 1;
				if (t->tm_mon && t->tm_mon != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_mon = ret;
				continue;
			case 'B':
				if ((ret = search(Jan, Dec)) < 0)
					return(0);
				ret = ret - (int) Jan + 1;
				if (t->tm_mon && t->tm_mon != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_mon = ret;
				continue;
			case 'Y':
	/* The last time UNIX can handle is 1/18/2038;
	   for simplicity stop at 2038 */
				if ((ret = number(4)) < 1970 || ret > 2037)
					return(0);
				else
					ret = ret - 1900;
				if (t->tm_year && t->tm_year != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_year = ret;
				continue;

			case 'y':
				ret = number(2);
				if ((ct->tm_year < 100 && ret >= 70 && ret < 100) || (ct->tm_year >=100 && ret < 38))
					ret = ct->tm_year>=100?100+ret:ret;
				else
					return(0);
				if (t->tm_year && t->tm_year != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_year = ret;
				continue;
			case 'm':
				if ((ret = number(2)) <= 0 || ret > 12)
					return(0);
				if (t->tm_mon && t->tm_mon != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_mon = ret;
				continue;
			case 'I':
				if ((ret = number(2)) < 1 || ret > 12)
					return(0);
				if (t->tm_hour && t->tm_hour != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_hour = ret;
				continue;
			case 'p':
				if ((ret  = search(AM, PM)) < 0)
					return(0);
				if (meridian && meridian != ret)
				{
					wrong_input++;
					return(0);
				}
				meridian = ret;
				continue;
			case 'H':
				if ((ret = number(2)) >= 0 && ret <= 23)
					ret = ret + 1;
				else
					return(0);
				if (hour && hour != ret)
				{
					wrong_input++;
					return(0);
				}
				hour = ret;
				continue;
			case 'M':
				if ((ret = number(2)) >= 0 && ret <= 59)
					ret = ret + 1;
				else
					return(0);
				if (t->tm_min && t->tm_min != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_min = ret;
				continue;
			case 'S':
				if ((ret  = number(2)) >= 0 && ret <= 59)
					ret = ret + 1;
				else
					return(0);
				if (t->tm_sec && t->tm_sec != ret)
				{
					wrong_input++;
					return(0);
				}
				t->tm_sec = ret;
				continue;

			case 'Z':
				if(!mytzname[0][0])
					continue;
				if(strncmp(&mytzname[0][0], input, strlen(&mytzname[0][0])) == 0) {
					input += strlen(&mytzname[0][0]);
					if(t->tm_isdst == 2) {
						wrong_input++;
						return 0;
					}
					t->tm_isdst = 1;
					continue;
				}
				if(strncmp(&mytzname[1][0], input, strlen(&mytzname[1][0])) == 0) {
					input += strlen(&mytzname[1][0]);
					if(t->tm_isdst == 1) {
						wrong_input++;
						return 0;
					}
					t->tm_isdst = 2;
					continue;
				}
				return 0;

			case 't':
				if(*input++ != '\t')
					return 0;
				continue;

			case 'n':
				if(*input++ != '\n')
					return 0;
				continue;

			/* composite formats */

			case 'c':
				if(parse_fmt(_time[DFL_FMT], 1))
					continue;
				return 0;

			case 'x':
				if(parse_fmt(_time[Local_date], 1))
					continue;
				return 0;

			case 'X':
				if(parse_fmt(_time[Local_time], 1))
					continue;
				return 0;

			case 'D':
				if(parse_fmt("%m/%d/%y", 1))
					continue;
				return 0;

			case 'r':
				if(parse_fmt("%I:%M:%S %p", 1))
					continue;
				return 0;

			case 'R':
				if(parse_fmt("%H:%M", 1))
					continue;
				return 0;

			case 'T':
				if(parse_fmt("%H:%M:%S", 1))
					continue;
				return 0;

			case '%':
				if(*input++ != '%')
					return 0;
				continue;
				
			default:
				wrong_input++;
				return(0);
			}
		}
		else {
			if(!isspace(c))
				while(isspace(d = *input))
					input++;
			d = *input++;
			if(d != c && !isspace(c))
				return 0;
			if(isspace(c) && d != c) {
				input--;
				continue;
			}
			if(!d) {
				input--;
				break;
			}
		}
	}
	if(flag)
		return 1;
	while(isspace(d = *input))
		input++;
	if (*input)
		return(0);
	if (verify(t))
		return(1);
	return(0);
}

static void
getnow(t)	/*  get current date */
struct tm *t;
{
	time_t now;

	now = time((time_t *)NULL);
	ct = localtime(&now);
	ct->tm_yday += 1;
	t->tm_year = t->tm_mon = t->tm_mday = t->tm_wday = t->tm_hour = t->tm_min = t->tm_sec = t->tm_isdst = hour = meridian = 0;
}

/*
 * Check validity of input
 */
static int
verify(t)
struct tm *t;
{
	int min = 0;
	int sec = 0;
	int hr = 0;
	int leap;

	leap = (days(ct->tm_year) == 366);
	if (t->tm_year)
		year(t->tm_year);
	if (t->tm_mon)
		MON(t->tm_mon - 1);
	if (t->tm_mday)
		Day(t->tm_mday);
	if (t->tm_wday)
		DOW(t->tm_wday - 1);
	if (  ((t->tm_mday)&&((t->tm_mday != ct->tm_mday)||(t->tm_mday > __mon_lengths[leap][ct->tm_mon])))
		||
	      ((t->tm_wday)&&((t->tm_wday-1)!=ct->tm_wday))
		||
	      ((t->tm_hour&&hour)||(t->tm_hour&&!meridian)||
	      (!t->tm_hour&&meridian)||(hour&&meridian))  )
	{
		wrong_input++;
		return(0);
	}
	if (t->tm_hour)
	{
		switch (meridian)
		{
			case PM:
				t->tm_hour %= 12;
				t->tm_hour += 12;
				break;
			case AM:
				t->tm_hour %= 12;
				if ( ! t->tm_hour )
					hr++;
				break;
			default:
				return(0);
		}
	}
	if (hour)
		t->tm_hour = hour - 1;
	if (t->tm_min)
	{
		min++;
		t->tm_min -= 1;
	}
	if (t->tm_sec)
	{
		sec++;
		t->tm_sec -= 1;
	}
	if ( (! t->tm_year && ! t->tm_mon && ! t->tm_mday && ! t->tm_wday) && ( (t->tm_hour < ct->tm_hour) || ((t->tm_hour == ct->tm_hour) && (t->tm_min < ct->tm_min)) || ((t->tm_hour == ct->tm_hour) && (t->tm_min == ct->tm_min) && (t->tm_sec < ct->tm_sec)) ) )
		t->tm_hour += 24;
	if (t->tm_hour || hour || hr || min || sec)
	{
		ct->tm_hour = t->tm_hour;
		ct->tm_min = t->tm_min;
		ct->tm_sec = t->tm_sec;
	}
	if(t->tm_isdst)
		ct->tm_isdst = t->tm_isdst - 1;
	else
		ct->tm_isdst = 0;
	return(1);
}


static void
Day(day)
int day;
{
	if (day < ct->tm_mday)
		if( ++ct->tm_mon == 12 )  ++ct->tm_year;
	ct->tm_mday = day;
	DMY();
}


static void
DMY()
{
	int i, doy;
	if ( days(ct->tm_year) == 366 )
		doy = __lyday_to_month[ct->tm_mon];
	else
		doy = __yday_to_month[ct->tm_mon];
	ct->tm_yday = doy + ct->tm_mday;
	ct->tm_wday = (jan1(ct->tm_year) + ct->tm_yday - 1) % 7;
}


static int
days(y)
int	y;
{
	y += 1900;
	return( y%4==0 && y%100!=0 || y%400==0 ? 366 : 365 );
}


/*
 *	return day of the week
 *	of jan 1 of given year
 */

static int
jan1(yr)
{
	register y, d;

/*
 *	normal gregorian calendar
 *	one extra day per four years
 */

	y = yr + 1900;
	d = 4+y+(y+3)/4;

/*
 *	julian calendar
 *	regular gregorian
 *	less three days per 400
 */

	if(y > 1800) {
		d -= (y-1701)/100;
		d += (y-1601)/400;
	}

/*
 *	great calendar changeover instant
 */

	if(y > 1752)
		d += 3;

	return(d%7);
}

static void
year(yr)
int	yr;
{
	ct->tm_mon = 0;
	ct->tm_mday = 1;
	ct->tm_year = yr;
	DMY();
}

static void
MON(month)
int month;
{
	ct->tm_mday = 1;
	Month(month);
}

static void
Month(month)
int month;
{
	if( month < ct->tm_mon )  ct->tm_year++;
	ct->tm_mon = month;
	DMY();
}

static void
DOW(dow)
int	dow;
{
	adddays((dow+7-ct->tm_wday)%7);
}

static void
adddays(n)
int	n;
{
	DOY(ct->tm_yday+n);
}

static void
DOY(doy)
int	doy;
{
	int i, leap;

	if ( doy>days(ct->tm_year) ) {
		doy -= days(ct->tm_year);
		ct->tm_year++;
	}
	ct->tm_yday = doy;

	leap = (days(ct->tm_year) == 366);
	for (i=0; doy>__mon_lengths[leap][i]; i++)
		doy -= __mon_lengths[leap][i];
	ct->tm_mday = doy;
	ct->tm_mon = i;
	ct->tm_wday = (jan1(ct->tm_year)+ct->tm_yday-1) % 7;
}

/*
 * return time from time structure
 */
static struct  tm
*calc_date(t)
struct tm *t;
{
	long	tv;
	struct  tm nct;

	nct = *ct;
	tv = mktime(ct);
	if(!t->tm_isdst && ct->tm_isdst != nct.tm_isdst) {
		nct.tm_isdst = ct->tm_isdst;
		tv = mktime(&nct);
	}
	ct = localtime(&tv);
	return ct;



}

