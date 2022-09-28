/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ckbupscd:ckbupscd.c	1.3.1.1"

/*
 * ckbupscd -	Check Backup Schedule and display list
 *		of filesystems needing to be backed up.
 *
 * 	Based on the program "agenda" by Alan Hastings
 *		Provided at the IH Computer Center as
 *		an "exptool" (experimental tool).
 *
 *	Usage:
 *		ckbupsch [-m]
 *		
 *			-m => Turn off header message on list(s)
 *
 * 	files:
 *		/etc/bupsched -	table of filesystems to be backed up
 */

#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#define	isspace(x)	(x==' ' || x=='\t')
#define	isletter(x)	(x>='a' && x<='z')
#define	isdigit(x)	(x>='0' && x<='9')
#define	isodigit(x)	(x>='0' && x<='7')
#define	iswild(x)	(x=='*')
#define	isfield(x)	(x && !isspace(x) && x!='\n')
#define	isupper(x)	(x>='A' && x<='Z')
#define	tolower(x)	(x+'a'-'A')
#define	getnum(n, p)	for (n=0; isdigit(*p); p++) n = n*10 + *p-'0'
#define	getonum(n, p)	{int _x; for (n=0,_x=0; isodigit(*p) && _x<3; p++,_x++) n = n*8 + *p-'0';}
#define	putch(x)	(void) putc(x, stdout)

#define	TRUE		(-1)
#define	FALSE		(0)

/*
 * tweekable parameters
 */
#define	LINESIZ		1024		/* max size of a list of filesystems to be backed up */
#define	DEFILE		"/etc/bupsched"	/* default ckbupscd filename */
#define	CESC		'!'		/* ESCape to shell Character */
#define	CCON		'&'		/* CONtinue Character */
#define	BQUAL		'('		/* Begin QUALifier for weekday */
#define	EQUAL		')'		/* End QUALifier */
#define	CCOM		'#'		/* COMment Character */
#define	RANGE		'-'		/* Specify RANGE of values */
#define	DELIM		','		/* Intra-field DELIMiter */

/*
 * internal form of /etc/bupsched line
 */
struct entry 
{	char	*b_time,
		*b_day,
		*b_month,
		*b_buplist;
};

/*
 * type of match
 */
enum  fieldtype 
{	WEEKDAY, MONTHDAY, TIME, MONTH	};

struct timeb 
{	time_t		time;
	unsigned short	millitm;
	short		timezone;
	short		dstflag;
};

char *day_name[] =
{	"sunday",
	"monday",
	"tuesday",
	"wednesday",
	"thursday",
	"friday",
	"saturday",
	(char *)0
};

char *month_name[] =
{	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december",
	(char *)0
};

int month_day[] =
{	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	};

char		*strcat(), *strcpy(), *getenv();
struct entry	*get_entry();
char		*gettim(), *getday(), *getmon();
int		getopt();
extern char	*optarg;

struct tm	*T;			/* current time */
short		Line = 1;		/* current line */
char		Stdoutbuf[BUFSIZ];	/* stdout is line buffered */
FILE		*File;			/* current input file */
int		Mflag = TRUE;		/* Turn on header message */

main(argc, argv)
	int argc;
	char **argv;
{
	time_t			now;
	register struct entry	*e;
	int			c;

	while( (c = getopt(argc, argv, "m") ) != EOF)
	{	switch(c)
		{	case 'm':
			{	Mflag = FALSE;
				break;
			}
			default:
			{	/* ignore garbage options */
				break;
			}
		}
	}

	setbuf(stdout, Stdoutbuf);
/*
 *	initialize current time
 */
	(void) time(&now);
	T = localtime(&now);

/*
 *	adjust february
 */
	if ((T->tm_year % 4) == 0)
		month_day[1]++;

	if ((File = fopen(DEFILE, "r"))==(FILE *)NULL) 
	{	fprintf(stderr, "ckbupscd: cannot open %s - aborting\n", DEFILE);
		return(1);
	}
/* 
 *	process entries in /etc/bupsched file one by one 
 */
	while (e = get_entry()) 
	{	if (match(e->b_month, getmon) && match(e->b_day, getday) && match(e->b_time, gettim))
		{	if (Mflag)
			{	printf("The following file systems are due for backup at this time:\n");
				Mflag=FALSE;
			}
			emit(e->b_buplist);
		}
	}
	(void) fclose(File);
	argv++;
	return(0);
}

/*
 * get_entry -	get next entry from ckbupscd file
 */
struct entry *
get_entry()
{
	register char		*p;
	static char		Ebuf[LINESIZ];
	static struct entry	E;

/*
 *	Syntax: time<space>day<space>month<space>filesystems
 *	get next line if this syntax is violated
 */
	while (getstr(Ebuf, File))
	{	for (p=Ebuf; isspace(*p); p++)	; /* skip whitespace */
		if (*Ebuf=='\0' || *Ebuf==CCOM)
			continue;		/* blank or comment */
		for (E.b_time=p; isfield(*p); p++)
			if (isupper(*p))
				*p = tolower(*p);
		if (isspace(*p))
			*p++ = '\0';
		else 
		{	warn("missing day field");
			continue;
		}
		while (isspace(*p))
			p++;
		for (E.b_day=p; isfield(*p); p++)
			if (isupper(*p))
				*p = tolower(*p);
		if (isspace(*p))
			*p++ = '\0';
		else 
		{	warn("missing month field");
			continue;
		}
		while (isspace(*p))
			p++;
		for (E.b_month=p; isfield(*p); p++)
			if (isupper(*p))
				*p = tolower(*p);
		if (isspace(*p))
			*p++ = '\0';
		else 
		{	warn("missing message field");
			continue;
		}
		while (isspace(*p))
			p++;
		E.b_buplist = p;
		return(&E);
	}
	return((struct entry *)0);
}

/*
 * getstr -	return string, treating escaped newlines specially
 */
getstr(s, f)
	register char *s;
	register FILE *f;
{
	register int c;
	register char *ss = s;
	static int lineinc = 0;

	Line += lineinc;
	lineinc = 0;
	while ((c = fgetc(f))!=EOF) 
	{	if (s >= &ss[LINESIZ-2]) 
		{	fprintf(stderr, "ckbupscd: list of filesystems longer than %d characters\n", LINESIZ);
/*
 *			reset to beginning of buffer, continue processing 
 */
			s = ss;
		}
		if (c=='\\') 
		{	if ((c = fgetc(f))==EOF)
				break;
			/* put the \ in too (emit() will take care of it) */
			*s++ = '\\';
			*s++ = c;
			/* lineinc keeps track of continued lines */
			if (c=='\n')
				lineinc++;
		} 
		else 
			if (c=='\n') 
			{	*s = '\0';
				lineinc++;
				return(TRUE);
			}
			else
				*s++ = c;
	}
	return(FALSE);
}

/*
 * emit -	Emit a list of file systems due for backup 
 *
 * 		If the line starts with the "Escape to Shell" 
 *		character, then treat it as a command and run 
 *		the command)
 */
emit(s)
	register char *s;
{
	register int n, newline = TRUE;

	if (*s==CCON) 
	{	register struct entry *e;

/*
 *		multiple date/time list: read lines until a real
 *		(non-continued) filesystem list is found
 */
		while (e = get_entry())
			if (*e->b_buplist!=CCON) 
			{	s = e->b_buplist;
				break;
			}
		if (e==(struct entry *)0)
			return;
/*
 *	got a filesystem list: fall through to 'emit' it 
 */
	}
	if (*s==CESC)
		(void) system(++s);
	else 
	{	while (*s) 
		{	
/*
 *			 expand \? and \???
 */
			if (*s=='\\') 
			{	switch (*++s) 
				{	case '\0':	/* just in case... */
					break;
					case 'n':	putch('\n'); s++; break;
					case 'r':	putch('\r'); s++; break;
					case 't':	putch('\t'); s++; break;
					case 'f':	putch('\f'); s++; break;
					case 'c':	newline = FALSE; s++; break;
					case 'b':	putch('\b'); s++; break;
					default:
						if (isodigit(*s)) 
						{	getonum(n, s);
							putch((char)n);
						} 
						else
							putch(*s++);
				}
			} 
			else
				putch(*s++);
		}
		if (newline)
			putch('\n');
	}
	(void) fflush(stdout);
}

/*
 * match -	see if current date/time matches the given string mstr
 */
match(mstr, fparse)
	register char *mstr;
	char *(*fparse)();
{
	int		m_begin, m_end = -1;
	enum fieldtype	btype, etype;
	register int	n;

/*
 *	parse DELIM-separated fields using passed function
 *
 *	m_begin/m_end result in lower and upper bounds for this filesystem list. 
 *	btype/etype specify what type of match we're doing.
 */
	while (*mstr) 
	{	if ((mstr = (*fparse)(mstr, &m_begin, &btype))==(char *)0)
			break;
		if (*mstr==RANGE) 
		{	if ((mstr = (*fparse)(++mstr, &m_end, &etype))==(char *)0)
				break;
		} 
		else
			etype = btype;
		if (*mstr && *mstr++!=DELIM) 
		{	warn("missing delimiter");
			break;
		}
		if (btype!=etype) 
		{	warn("incompatible range");
			break;
		}
		switch((int)btype) 
		{	case WEEKDAY:
				n = T->tm_wday;  break;
			case MONTHDAY:
				n = T->tm_mday; break;
			case TIME:
				n = T->tm_hour*60 + T->tm_min; break;
			case MONTH:
				n = T->tm_mon; break;
		}
		if (m_end>=0) 
		{	if (m_begin > m_end) 
			{	warn("reversed range");
				break;
			}
			if (n>=m_begin && n<=m_end)
				return(TRUE);
		} 
		else 
			if (n==m_begin)
				return(TRUE);
	}
	return(FALSE);
}

/*
 * getmon - parse month string
 */
char *
getmon(m, amonth, atype)
	register char *m;
	register int  *amonth;
	register enum fieldtype *atype;
{
	register char	*mp, *mn;
	register int	n;

	*atype = MONTH;
	if (iswild(*m)) 
	{	*amonth = T->tm_mon;
		return(++m);
	} 
	else 
	{	if (isdigit(*m)) 
		{	getnum(n, m);
			*amonth = n-1;
		} 
		else 
		{	if (isletter(*m)) 
			{	for (n=0; month_name[n]; n++) 
				{	for (mp=m, mn=month_name[n]; isletter(*mp) && *mn; mp++, mn++)
						if (*mp!=*mn)
							goto nextmonth;
					*amonth = n;
					m = mp;
					break;
nextmonth:				;	/* end of outer loop */
				}
				if (month_name[n]==(char *)0) 
				{	warn("unrecognized month name");
					return((char *)0);
				}
			}
			else
				return((char *)0);
		}
	}
	return(m);
}

/*
 * getday - parse day string
 */
char *
getday(d, aday, atype)
	register char *d;
	register int  *aday;
	register enum fieldtype *atype;
{
	register int	n;
	register char	*dp, *dn;

	if (iswild(*d)) 
	{	*atype = WEEKDAY;
		*aday = T->tm_wday;
		d++;
		goto qualifier;
	} 
	else 
	{	if (isdigit(*d)) 
		{	getnum(n, d);
			*aday = n;
			*atype = MONTHDAY;
			if (n > 31)
				warn("imaginary day");
		} 
		else 
		{	if (isletter(*d)) 
			{	if (*d=='l') 
				{	for (dn="last"; isletter(*d) && *dn; dn++, d++)
						if (*dn != *d)
							break;
					if (*d && *d != DELIM && *d != '-') 
					{	warn("bad usage of 'last'");
						return((char *)0);
					} 
					else 
					{	*aday = month_day[T->tm_mon];
						*atype = MONTHDAY;
						if (*d=='-') 
						{	d++;
							if (isdigit(*d))
								getnum(n, d);
							else 
							{	warn("missing number for 'last'");
								return((char *)0);
							}
							if ((*aday -= n) <= 0 || (*d && *d != DELIM)) 
							{	warn("bad qualifier for 'last'");
								return((char *)0);
							}
						}
						return(d);
					}
				}
				for (n=0; day_name[n]; n++) 
				{	for (dn=day_name[n], dp=d; isletter(*dp) && *dn; dn++, dp++)
						if (*dp!=*dn)
							goto nextday;
					*aday = n;
					*atype = WEEKDAY;
					d = dp;
					break;
nextday:				;
				}
				if (day_name[n]==(char *)0) 
				{	warn("unrecognized day name");
					return((char *)0);
				}
qualifier:
				if (*d==BQUAL) 
				{	d++;
					if (isdigit(*d)) 
					{	getnum(n, d);
						*aday = firstday(*aday) + 7*(n-1);
					} 
					else 
					{	if (*d=='l') 
						{	*aday = lastday(*aday);
							d++;
						} 
						else 
						{	warn("bad qualifier");
							return((char *)0);
						}
						if (*d=='-' || *d=='+') 
						{	int minus = (*d++=='-');
							if (isdigit(*d)) 
							{	getnum(n, d);
								*aday = (minus) ? *aday-n : *aday+n;
							} 
							else
								warn("bad qualifier adjustment");
						}
					}
					if (*d++!=EQUAL) 
					{	warn("missing qualifier delimiter");
						return((char *)0);
					}
					*atype = MONTHDAY;
					if (*aday < 1 || *aday > 31)
						warn("day outa range");
				}
			} 
			else 
			{	warn("bad day spec");
				return((char *)0);
			}
		}
	}
	return(d);
}

/*
 * gettim -	parse hour/minute string
 */
char *
gettim(t, amin, atype)
	register char *t;
	register int  *amin;
	register enum fieldtype *atype;
{
	register int	n;

	*atype = TIME;
	if (iswild(*t)) 
	{	*amin = T->tm_hour*60;
		t++;
	} 
	else 
	{	if (isdigit(*t)) 
		{	getnum(n, t);
			*amin = n*60;
			if (n>23)
				warn("hour outa range");
		} 
		else 
		{	warn("bad hour spec");
			return((char *)0);
		}
	}
	if (*t==':') 
	{	t++;
		if (iswild(*t)) 
		{	*amin += T->tm_min;
			t++;
		} 
		else 
		{	if (isdigit(*t)) 
			{	getnum(n, t);
				*amin += n;
				if (n>59)
					warn("minutes outa range");
			} 
			else 
			{	warn("bad minutes spec");
				return((char *)0);
			}
		}
	} 
	else
		*amin += T->tm_min;

	if (*t=='p' && *(t+1)=='m') 
	{	t += 2;
		if (*amin < 12*60 && *amin >= 60)
			*amin += 12 * 60;	/* valid from 1:00 to 12:59 */
		else 
		{	warn("pm not applicable");
			return((char *)0);
		}
	} 
	else 
	{	if (*t=='a' && *(t+1)=='m') 
		{	t += 2;
			if (*amin < 60 || *amin >= 13*60) 
			{	warn("am not applicable");
				return((char *)0);
			}
		/* no op otherwise */
		}
	}
	return(t);
}

/*
 * firstday -	return number (1-31) of the first weekday of type DAY (0-7)
 *		of the current month.
 */
firstday(day)
	int day;
{
	register int thatweek;

	thatweek = T->tm_mday + (day - T->tm_wday);
	return(thatweek - ((thatweek-1)/7)*7);
}

/*
 * lastday -	return number (21-31) of the last weekday of type DAY (0-7)
 *		of the current month.
 */
lastday(day)
	int day;
{
	register int	fday = firstday(day);
	register int	ndays = month_day[T->tm_mon];

	return(fday + ((ndays-fday)/7)*7);
}

/*
 * warn -	print warning message 
 */
warn(s)
	register char *s;
{
	fprintf(stderr, "%d: %s\n", Line, s);
}

/*
 *	ftime - functionality of BSD ftime(2)
 */
extern long timezone;
extern int  daylight;

ftime(tp)
	struct timeb *tp;
{
	time_t t;

	(void) time(&t);
	tp->time = t;
	tp->millitm = 0;
	tp->timezone = (short)timezone/60;
	tp->dstflag = (short)daylight;
}
