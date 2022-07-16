/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*      Portions Copyright (c) 1988, Sun Microsystems, Inc.     */ 
/*      All Rights Reserved.                                    */ 
 
#ident	"@(#)ucbprt:lib/date_ab.c	1.1.1.1"

# include	"../hdr/defines.h"
# include	<time.h>

/*
	Function to convert date in the form "yymmddhhmmss" to
	standard UNIX time (seconds since Jan. 1, 1970 GMT).
	Units left off of the right are replaced by their
	maximum possible values.

	The function corrects properly for leap year,
	daylight savings time, offset from Greenwich time, etc.

	Function returns -1 if bad time is given (i.e., "730229").
*/
#define	dysize(A) (((A)%4)? 365: 366)

char *Datep;
static int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int	mosize(), g2();

int
date_ab(adt,bdt)
char *adt;
long *bdt;
{
	int y, t, d, h, m, s, i;
	long tim;
	struct tm *localtime();
	extern long timezone;

	tzset();
	Datep = adt;

	if((y=g2()) == -2) y = 99;
	if(y<70 || y>99) return(-1);

	if((t=g2()) == -2) t = 12;
	if(t<1 || t>12) return(-1);

	if((d=g2()) == -2) d = mosize(y,t);
	if(d<1 || d>mosize(y,t)) return(-1);

	if((h=g2()) == -2) h = 23;
	if(h<0 || h>23) return(-1);

	if((m=g2()) == -2) m = 59;
	if(m<0 || m>59) return(-1);

	if((s=g2()) == -2) s = 59;
	if(s<0 || s>59) return(-1);

	tim = 0L;
	y += 1900;
	for(i=1970; i<y; i++)
		tim += dysize(i);
	while(--t)
		tim += mosize(y,t);
	tim += d - 1;
	tim *= 24;
	tim += h;
	tim *= 60;
	tim += m;
	tim *= 60;
	tim += s;

	tim += timezone;			/* GMT correction */
	if((localtime(&tim))->tm_isdst)
		tim += -1*60*60;		/* daylight savings */
	*bdt = tim;
	return(0);
}

static int
mosize(y,t)
int y, t;
{

	if(t==2 && dysize(y)==366) return(29);
	return(dmsize[t-1]);
}

static int
g2()
{
	register int c;
	register char *p;

	for (p = Datep; *p; p++)
		if (numeric(*p))
			break;
	if (*p) {
		c = (*p++ - '0') * 10;
		if (numeric(*p))
			c += (*p++ - '0');
		else
			c = c/10;
	}
	else
		c = -2;
	Datep = p;
	return(c);
}
