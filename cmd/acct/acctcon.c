/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:acctcon.c	1.6.5.1"
/*
 *	acctcon [-l file] [-o file] <wtmp-file 
 *	-l file	causes output of line usage summary
 *	-o file	causes first/last/reboots report to be written to file
 *	reads input (normally /var/wtmp), produces
 *	list of sessions, sorted by ending time in tacct.h format
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/param.h>
#include "acctdef.h"
#include <ctype.h>
#include <time.h>
#include <utmp.h>
#include <locale.h>


int tcmp(), ccmp();
int	tsize	= -1;	/* used slots in tbuf table */
static	csize;
struct  utmp	wb;	/* record structure read into */
struct	ctmp	cb;	/* record structure written out of */
struct	tacct	tb;	

struct tbuf {
	char	tline[LSZ];	/* /dev/* */
	char	tname[NSZ];	/* user name */
	time_t	ttime;		/* start time */
	dev_t	tdev;		/* device */
	int	tlsess;		/* # complete sessions */
	int	tlon;		/* # times on (ut_type of 7) */
	int	tloff;		/* # times off (ut_type != 7) */
	long	ttotal;		/* total time used on this line */
} tbuf[A_TSIZE];

struct ctab {
	uid_t	ct_uid;
	char	ct_name[NSZ];
	long 	ct_con[2];
	ushort	ct_sess;
} ctab[A_TSIZE];

int	nsys;
struct sys {
	char	sname[LSZ];	/* reasons for ACCOUNTING records */
	char	snum;		/* number of times encountered */
} sy[NSYS];

static char time_buf[50];
time_t	datetime;	/* old time if date changed, otherwise 0 */
time_t	firstime;
time_t	lastime;
int	ndates;		/* number of times date changed */
int	exitcode;
char	*report	= NULL;
char	*replin = NULL;

char	*strncpy(), *strcpy();
long	ftell();
uid_t	namtouid();
dev_t	lintodev();

extern char 	*optarg;
extern int	optind;			

main(argc, argv) 
char **argv;
{
	int c;

	(void)setlocale(LC_ALL, "");
	while ((c = getopt(argc, argv, "l:o:")) != EOF)
		switch(c) {
		case 'l':
			replin = optarg;
			break;
		case 'o':
			report = optarg;
			break;
		case '?':
			fprintf(stderr, "usage: %s [-l lineuse] [-o reboot]\n", argv[0]);
			exit(1);
		}

	init();

	while ( fread(&wb, sizeof(wb), 1, stdin) == 1 ) {
		if (firstime == 0)
			firstime = wb.ut_time;
		if (valid())
			loop();
		else
			fixup(stderr);
	}
	wb.ut_name[0] = '\0';
	strcpy(wb.ut_line, "acctcon");
	wb.ut_type = ACCOUNTING;
	wb.ut_time = lastime;
	loop();

	squeeze();
	qsort((char *) ctab, csize, sizeof(ctab[0]), ccmp);
	output();

	if (report != NULL)
		printrep();
	if (replin != NULL)
		printlin();

	exit(exitcode);
}

init()
{
	register i;

	for (i=0; i < A_TSIZE; i++)
		ctab[i].ct_sess = 0;
}


/*
 * valid: check input wtmp record, return 1 if looks OK
 */
valid()
{
	register i, c;

	for (i = 0; i < NSZ; i++) {
		c = wb.ut_name[i];
		if (isalnum(c) || c == '$' || c == ' ' || c == '.')
			continue;
		else if (c == '\0')
			break;
		else
			return(0);
	}

	if((wb.ut_type >= EMPTY) && (wb.ut_type <= UTMAXTYPE))
		return(1);

	return(0);
}

fixup(stream)
register FILE *stream;
{
	fprintf(stream, "bad wtmp: offset %lu.\n", ftell(stdin)-sizeof(wb));
	fprintf(stream, "bad record is:  %.12s\t%.8s\t%lu",
		wb.ut_line,
		wb.ut_name,
		wb.ut_time);
	cftime(time_buf, DATE_FMT, &wb.ut_time);
	fprintf( stream, "\t%s", time_buf);
	exitcode = 1;
}

loop()
{
	register timediff;
	register struct tbuf *tp;

	if( wb.ut_line[0] == '\0' )	/* It's an init admin process */
		return;			/* no connect accounting data here */
	switch(wb.ut_type) {
	case OLD_TIME:
		datetime = wb.ut_time;
		return;
	case NEW_TIME:
		if(datetime == 0)
			return;
		timediff = wb.ut_time - datetime;
		for (tp = tbuf; tp <= &tbuf[tsize]; tp++)
			tp->ttime += timediff;
		datetime = 0;
		ndates++;
		return;
	case BOOT_TIME:
		upall();
	case ACCOUNTING:
	case RUN_LVL:
		lastime = wb.ut_time;
		bootshut();
		return;
	case USER_PROCESS:
	case LOGIN_PROCESS:
	case INIT_PROCESS:
	case DEAD_PROCESS:	/* WHCC mod 3/86  */
		update(&tbuf[iline()]);
		return;
	case EMPTY:
		return;
	default:
		cftime(time_buf, DATE_FMT, &wb.ut_time);
		fprintf(stderr, "acctcon: invalid type %d for %s %s %s",
			wb.ut_type,
			wb.ut_name,
			wb.ut_line,
			time_buf);
	}
}

/*
 * bootshut: record reboot (or shutdown)
 * bump count, looking up wb.ut_line in sy table
 */
bootshut()
{
	register i;

	for (i = 0; i < nsys && !EQN(wb.ut_line, sy[i].sname); i++)
		;
	if (i >= nsys) {
		if (++nsys > NSYS) {
			fprintf(stderr,
				"acctcon: recompile with larger NSYS\n");
			nsys = NSYS;
			return;
		}
		CPYN(sy[i].sname, wb.ut_line);
	}
	sy[i].snum++;
}

/*
 * iline: look up/enter current line name in tbuf, return index
 * (used to avoid system dependencies on naming)
 */
iline()
{
	register i;

	for (i = 0; i <= tsize; i++)
		if (EQN(wb.ut_line, tbuf[i].tline))
			return(i);
	if (++tsize >= A_TSIZE) {
		fprintf(stderr, "acctcon: RECOMPILE WITH LARGER A_TSIZE\n");
		exit(2);
	}

	CPYN(tbuf[tsize].tline, wb.ut_line);
	tbuf[tsize].tdev = lintodev(wb.ut_line);
	return(tsize);
}

upall()
{
	register struct tbuf *tp;

	wb.ut_type = DEAD_PROCESS;	/* fudge a logoff for reboot record. Also, WHCC mod 3/86 */
	for (tp = tbuf; tp <= &tbuf[tsize]; tp++)
		update(tp);
}

/*
 * update tbuf with new time, write ctmp record for end of session
 */
update(tp)
struct tbuf *tp;
{
	long	told,	/* last time for tbuf record */
		tnew;	/* time of this record */
			/* Difference is connect time */

	told = tp->ttime;
	tnew = wb.ut_time;
	if (told > tnew) {
		cftime(time_buf, DATE_FMT, &told);
		fprintf(stderr, "acctcon: bad times: old: %s", time_buf);
		cftime(time_buf, DATE_FMT, &tnew);
		fprintf(stderr, "new: %s", time_buf);
		exitcode = 1;
		tp->ttime = tnew;
		return;
	}
	tp->ttime = tnew;
	switch(wb.ut_type) {
	case USER_PROCESS:
		tp->tlsess++;
		if(tp->tname[0] != '\0') { /* Someone logged in without */
					   /* logging off. Put out record. */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			enter(&cb);
			tp->ttotal += tnew-told;
		}
		else	/* Someone just logged in */
			tp->tlon++;
		CPYN(tp->tname, wb.ut_name);
		break;
/*	case INIT_PROCESS:  WHCC mod */
/*	case LOGIN_PROCESS: WHCC mod */
	case DEAD_PROCESS:	/*  WHCC mod 3/86  */
		tp->tloff++;
		if(tp->tname[0] != '\0') { /* Someone logged off */
			/* Set up and print ctmp record */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			enter(&cb);
			tp->ttotal += tnew-told;
			tp->tname[0] = '\0';
		}
	}
}

printrep()
{
	register i;

	freopen(report, "w", stdout);
	cftime(time_buf, DATE_FMT, &firstime);
	printf("from %s", time_buf);
	cftime(time_buf, DATE_FMT, &lastime);
	printf("to   %s", time_buf);
	if (ndates)
		printf("%d\tdate change%c\n",ndates,(ndates>1 ? 's' : '\0'));
	for (i = 0; i < nsys; i++)
		printf("%d\t%.12s\n", sy[i].snum, sy[i].sname);
}

/*
 *	print summary of line usage
 *	accuracy only guaranteed for wtmp file started fresh
 */
printlin()
{
	register struct tbuf *tp;
	double timet, timei;
	double ttime;
	int tsess, ton, toff;

	freopen(replin, "w", stdout);
	ttime = 0.0;
	tsess = ton = toff = 0;
	timet = MINS(lastime-firstime);
	printf("TOTAL DURATION IS %.0f MINUTES\n", timet);
	printf("LINE         MINUTES  PERCENT  # SESS  # ON  # OFF\n");
	qsort((char *) tbuf, tsize, sizeof tbuf[0], tcmp);
	for (tp = tbuf; tp <= &tbuf[tsize]; tp++) {
		timei = MINS(tp->ttotal);
		ttime += timei;
		tsess += tp->tlsess;
		ton += tp->tlon;
		toff += tp->tloff;
		printf("%-12.12s %-7.0f  %-7.0f  %-6d  %-4d  %-5d\n",
			tp->tline,
			timei,
			(timet > 0.)? 100*timei/timet : 0.,
			tp->tlsess,
			tp->tlon,
			tp->tloff);
	}
	printf("TOTALS       %-7.0f  --       %-6d  %-4d  %-5d\n", ttime, tsess, ton, toff);
}

tcmp(t1, t2)
struct tbuf *t1, *t2;
{
	return( strncmp(t1->tline, t2->tline, LSZ) );
}

enter(c)
register struct ctmp *c;
{
        register unsigned i;
        int j;

        i=(unsigned)c->ct_uid;
        j=0;
        for (i %= A_TSIZE; !CBEMPTY && j++ < A_TSIZE; i = (i+1) % A_TSIZE)
                if (c->ct_uid == ctab[i].ct_uid) 
                        break;
        if (j >= A_TSIZE)
                fprintf(stderr, "acctcon: INCREASE A_TSIZE\n"), exit(1);
	if (CBEMPTY) {
        	ctab[i].ct_uid = c->ct_uid;
        	CPYN(ctab[i].ct_name, c->ct_name);
	}
        ctab[i].ct_con[0] += c->ct_con[0];
        ctab[i].ct_con[1] += c->ct_con[1];
        ctab[i].ct_sess++;
}

squeeze()               /*eliminate holes in hash table*/
{
        register i, k;

        for (i = k = 0; i < A_TSIZE; i++)
                if (!CBEMPTY) {
                        ctab[k].ct_uid = ctab[i].ct_uid;
                        CPYN(ctab[k].ct_name, ctab[i].ct_name);
                        ctab[k].ct_con[0] = ctab[i].ct_con[0];
                        ctab[k].ct_con[1] = ctab[i].ct_con[1];
                        ctab[k].ct_sess = ctab[i].ct_sess;
                        k++;
                }
        csize = k;
}

ccmp(c1, c2)
register struct ctab *c1, *c2;
{
	if (c1->ct_uid > c2->ct_uid)
                return 1;
        else if (c1->ct_uid < c2->ct_uid)
                return (-1);
        else 
                return (0);
}

output()
{
        register i; 

	for (i = 0; i < csize; i++) {
                tb.ta_uid = ctab[i].ct_uid;
                CPYN(tb.ta_name, ctab[i].ct_name);
                tb.ta_con[0] = ctab[i].ct_con[0] / 60.0;
                tb.ta_con[1] = ctab[i].ct_con[1] / 60.0;
                tb.ta_sc = ctab[i].ct_sess;
                fwrite(&tb, sizeof(tb), 1, stdout);
        }
}
