/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcsvc:rusers.c	1.4.1.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static	char sccsid[] = "@(#)rusers.c 1.12 89/06/21 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include <netconfig.h>
#include <netdir.h>
#include <rpc/rpc.h>
#include <rpcsvc/rusers.h>
#include <string.h>

struct ru_utmp dummy;
#define NMAX sizeof(dummy.ut_name)
#define LMAX sizeof(dummy.ut_line)
#define	HMAX sizeof(dummy.ut_host)

#define MACHINELEN 16		/* length of machine name printed out */
#define NUMENTRIES 200
#define MAXINT 0x7fffffff
#define min(a,b) ((a) < (b) ? (a) : (b))

struct entry {
	int cnt;
	int idle;		/* set to MAXINT if not present */
	char *machine;
	struct utmpidle *users;
} entry[NUMENTRIES];

int curentry;
int hflag;			/* host: sort by machine name */
int iflag;			/* idle: sort by idle time */
int uflag;			/* users: sort by number of users */
int lflag;			/* print out long form */
int aflag;			/* all: list all machines */
int dflag;			/* debug: list only first n machines */
int debug;
int vers;
char *nettype;

int hcompare(), icompare(), ucompare();
int collectnames();


main(argc, argv)
	char **argv;
{
	int single;
	struct utmpidlearr utmpidlearr;

	single = 0;
	while (argc > 1) {
		if (argv[1][0] != '-') {
			single++;
			singlehost(argv[1]);
		}
		else {
			switch(argv[1][1]) {
	
			case 'n' :
				nettype = argv[2];
				argc--;
				argv++;
				break;
			case 'h':
				hflag++;
				break;
			case 'a':
				aflag++;
				break;
			case 'i':
				iflag++;
				break;
			case 'l':
				lflag++;
				break;
			case 'u':
				uflag++;
				break;
			case 'd':
				dflag++;
				if (argc < 3)
					usage();
				debug = atoi(argv[2]);
				argc--;
				argv++;
				break;
			default:
				usage();
			}
		}
		argv++;
		argc--;
	}
	if (iflag + hflag + uflag > 1)
		usage();
	if (single > 0) {
		if (iflag || hflag || uflag)
			printnames();
		exit(0);
	}

	if (iflag || hflag || uflag) {
		printf("collecting responses... ");
		fflush(stdout);
	}
	vers = RUSERSVERS_IDLE;
	utmpidlearr.uia_arr = NULL;
	(void) rpc_broadcast(RUSERSPROG, RUSERSVERS_IDLE,
		RUSERSPROC_NAMES, xdr_void, NULL,
		xdr_utmpidlearr, &utmpidlearr, collectnames, nettype);
#ifdef DEBUG
	fprintf(stderr, "starting second round of broadcasting\n");
#endif
	vers = RUSERSVERS_ORIG;
	utmpidlearr.uia_arr = NULL;
	(void) rpc_broadcast(RUSERSPROG, RUSERSVERS_ORIG,
		RUSERSPROC_NAMES, xdr_void, NULL,
		xdr_utmparr, &utmpidlearr, collectnames, nettype);
	if (iflag || hflag || uflag)
		printnames();
	exit(0);
	/* NOTREACHED */
}

singlehost(name)
	char *name;
{
	enum clnt_stat err;
	struct utmpidlearr utmpidlearr;

	vers = RUSERSVERS_ORIG;
	utmpidlearr.uia_arr = NULL;

	err = rpc_call(name, RUSERSPROG, RUSERSVERS_IDLE,
			RUSERSPROC_NAMES, xdr_void, 0,
			xdr_utmpidlearr, &utmpidlearr, nettype);
	if (err == RPC_PROGVERSMISMATCH) {
		if (err = rpc_call(name, RUSERSPROG, RUSERSVERS_ORIG,
				RUSERSPROC_NAMES, xdr_void, 0,
				xdr_utmparr, &utmpidlearr, nettype)) {
			fprintf(stderr, "%*s: ", name);
			clnt_perrno(err);
			return;
		}
	} else if (err == RPC_SUCCESS) {
		vers = RUSERSVERS_IDLE;
	} else {
		fprintf(stderr, "%s: ", name);
		clnt_perrno(err);
		return;
	}

	print_info((char *)&utmpidlearr, name);
	return;
}

collectnames(resultsp, raddrp, nconf)
	char *resultsp;
	struct netbuf *raddrp;
	struct netconfig *nconf;
{
	struct utmpidlearr utmpidlearr;
	static int debugcnt;
	register struct entry *entryp, *lim;
	struct nd_hostservlist *hs;
	char host[MACHINELEN + 1];

	utmpidlearr = *(struct utmpidlearr *)resultsp;
	if (utmpidlearr.uia_cnt < 1 && !aflag)
		return(0);
	if (netdir_getbyaddr(nconf, &hs, raddrp)) {
		sprintf(host, "%s", taddr2uaddr(nconf, raddrp));
	} else {
		sprintf(host, "%s", MACHINELEN, hs->h_hostservs ->h_host);
		netdir_free((char *)hs, ND_HOSTSERVLIST);
	}
	/*
	 * weed out duplicates
	 */
	lim = entry + curentry;
	for (entryp = entry; entryp < lim; entryp++)
		if (!strcmp(entryp->machine, host))
			return (0);
	debugcnt++;
	return (print_info(resultsp, host));
}

print_info(resultsp, name)
	char *resultsp;
	char *name;
{
	struct utmpidlearr utmpidlearr;
	int i, cnt, minidle;
	char host[MACHINELEN + 1];
	static int debugcnt;
	struct utmpidle *p, *q;

	utmpidlearr = *(struct utmpidlearr *)resultsp;
	if ((cnt = utmpidlearr.uia_cnt) < 1 && !aflag) 
		return(0);
	(void) sprintf(host, "%.*s", MACHINELEN, name);
	/*
	 * if raw, print this entry out immediately
	 * otherwise store for later sorting
	 */
	if (!iflag && !hflag && !uflag) {
		if (lflag &&  (cnt > 0) )  
			for (i = 0; i < cnt; i++)
				putline(host, utmpidlearr.uia_arr[i], vers);
		else {
			printf("%-*.*s", MACHINELEN, MACHINELEN, host);
			for (i = 0; i < cnt; i++)
				printf(" %.*s", NMAX, 
					utmpidlearr.uia_arr[i]->ui_utmp.ut_name);
			printf("\n");
		}
	} else {
		entry[curentry].machine = (char *)malloc(MACHINELEN+1);
		if (entry[curentry].machine == (char *) NULL) {
			fprintf(stderr,"Ran out of memory - exiting\n");
			exit(1);
		}
		(void) strcpy(entry[curentry].machine, name);
		entry[curentry].cnt = cnt;
		q = (struct utmpidle *)
				malloc(cnt * sizeof(struct utmpidle));
		p = q;
		minidle = MAXINT;
		for (i = 0; i < cnt; i++) {
			memcpy((char *)q, (char *)utmpidlearr.uia_arr[i],
					sizeof(struct utmpidle));
			if (vers == RUSERSVERS_IDLE)
				minidle = min(minidle, q->ui_idle);
			q++;
		}
		entry[curentry].users = p;
		entry[curentry].idle = minidle;
	}
	if (curentry >= NUMENTRIES) {
		fprintf(stderr, "too many hosts on network\n");
		exit(1);
	}
	curentry++;
	if (dflag && debugcnt >= debug)
		return (1);
	return (0);
}


printnames()
{
	int i, j, v;
	int (*compare)();

	/* the name of the machine should already be in the structure */
	if (iflag)
		compare = icompare;
	else if (hflag)
		compare = hcompare;
	else
		compare = ucompare;
	qsort(entry, curentry, sizeof(struct entry), compare);
	printf("\n");
	for (i = 0; i < curentry; i++) {
		if (!lflag || (entry[i].cnt < 1) ) {
			printf("%-*.*s", MACHINELEN,
					MACHINELEN, entry[i].machine);
			for (j = 0; j < entry[i].cnt; j++)
				printf(" %.*s", NMAX,
					entry[i].users[j].ui_utmp.ut_name);
			printf("\n");
		}
		else {
			if (entry[i].idle == MAXINT)
				v = RUSERSVERS_ORIG;
			else
				v = RUSERSVERS_IDLE;
			for (j = 0; j < entry[i].cnt; j++)
				putline(entry[i].machine,
						&entry[i].users[j], v);
		}
	}
}

hcompare(a,b)
	struct entry *a, *b;
{
	return (strcmp(a->machine, b->machine));
}

ucompare(a,b)
	struct entry *a, *b;
{
	return (b->cnt - a->cnt);
}

icompare(a,b)
	struct entry *a, *b;
{
	return (a->idle - b->idle);
}

putline(host, uip, vers)
	char *host;
	struct utmpidle *uip;
	int vers;
{
	register char *cbuf;
	struct ru_utmp *up;
	char buf[100];

	up = &uip->ui_utmp;
	printf("%-*.*s ", NMAX, NMAX, up->ut_name);

	(void) strcpy(buf, host);
	(void) strcat(buf, ":");
	(void) strncat(buf, up->ut_line, LMAX);
	(void) printf("%-*.*s", MACHINELEN+LMAX, MACHINELEN+LMAX, buf);

	cbuf = (char *)ctime(&up->ut_time);
	(void) printf("  %.12s  ", cbuf+4);
	if (vers == RUSERSVERS_IDLE) {
		prttime(uip->ui_idle, "");
	}
	else
		printf("    ??");
	if (up->ut_host[0])
		printf(" (%.*s)", HMAX, up->ut_host);
	putchar('\n');
}

/*
 * prttime prints a time in hours and minutes.
 * The character string tail is printed at the end, obvious
 * strings to pass are "", " ", or "am".
 */
prttime(tim, tail)
	time_t tim;
	char *tail;
{
	register int didhrs = 0;

	if (tim >= 60) {
		printf("%3d:", tim/60);
		didhrs++;
	} else {
		printf("    ");
	}
	tim %= 60;
	if (tim > 0 || didhrs) {
		printf(didhrs&&tim<10 ? "%02d" : "%2d", tim);
	} else {
		printf("  ");
	}
	printf("%s", tail);
}

#ifdef DEBUG
/* 
 * for debugging
 */
printit(i)
{
	int j, v;
	
	printf("%12.12s: ", entry[i].machine);
	if (entry[i].cnt) {
		if (entry[i].idle == MAXINT)
			v = RUSERSVERS_ORIG;
		else
			v = RUSERSVERS_IDLE;
		putline(entry[i].machine, &entry[i].users[0], v);
		for (j = 1; j < entry[i].cnt; j++) {
			printf("\t");
			putline(entry[i].machine, &entry[i].users[j], vers);
		}
	} else
		printf("\n");
}
#endif

usage()
{
	fprintf(stderr, "Usage: rusers [-a] [-h] [-i] [-l] [-u] [host ...]\n");
	exit(1);
}
