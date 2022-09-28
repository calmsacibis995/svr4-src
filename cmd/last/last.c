/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)last:last.c	1.3.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * last
 */
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <utmpx.h>

/* NMAX, LMAX and HMAX are set to these values for now. They
 * should be much higher because of the max allowed limit in
 * utmpx.h
 */
#define NMAX	8
#define LMAX	12
#define HMAX	16
#define	SECDAY	(24*60*60)

#define	lineq(a,b)	(!strncmp(a,b,LMAX))
#define	nameq(a,b)	(!strncmp(a,b,NMAX))
#define	hosteq(a,b)	(!strncmp(a,b,HMAX))

#define MAXTTYS 256

char	**argv;
int	argc;
int	nameargs;

struct	utmpx buf[128];
char	ttnames[MAXTTYS][LMAX+1];
long	logouts[MAXTTYS];

char	*ctime(), *strspl();
void	onintr();

main(ac, av)
	char **av;
{
	register int i, k;
	int bl, wtmp;
	char *ct;
	register struct utmpx *bp;
	long otime;
	struct stat stb;
	int print;
	char * crmsg = (char *)0;
	long crtime;
	long outrec = 0;
	long maxrec = 0x7fffffffL;
	char *wtmpfile = "/var/adm/wtmpx";
 
	time(&buf[0].ut_xtime);
	ac--, av++;
	nameargs = argc = ac;
	argv = av;
	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			/* -[0-9]*   sets max # records to print */
		    	if (argv[i][1] >= '0' && argv[i][1] <= '9') {
				maxrec = atoi(argv[i]+1);
				nameargs--;
				continue;
			}
			/* -f name   sets filename of wtmp file */
			else if (argv[i][1] == 'f' && i < argc) {
				wtmpfile = argv[++i];
				nameargs -= 2;
				continue;
			}
			/* -n number sets max # records to print */
			else if (argv[i][1] == 'n' && i < argc) {
				if (*argv[++i] < '0' || *argv[i] > '9') {
					fprintf(stderr,"last: argument to -n is not a number\n");
					fprintf(stderr, "usage: last [-n number] [-f filename] [name | tty]\n");
					exit(1);
				}
				maxrec = atoi(argv[i]);
				nameargs-=2;
				continue;
			}
			else {
				fprintf(stderr, "usage: last [-n number] [-f filename] [name | tty]\n");
				exit(1);
			}
		}
		
		if (strlen(argv[i])>2)
			continue;
		if (!strcmp(argv[i], "~"))
			continue;
		if (!strcmp(argv[i], "ftp"))
			continue;
		if (!strcmp(argv[i], "uucp"))
			continue;
		if (getpwnam(argv[i]))
			continue;
		argv[i] = strspl("tty", argv[i]);
	}
	wtmp = open(wtmpfile, 0);
	if (wtmp < 0) {
		perror(wtmpfile);
		exit(1);
	}
	fstat(wtmp, &stb);
	bl = (stb.st_size + sizeof (buf)-1) / sizeof (buf);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, onintr);
		signal(SIGQUIT, onintr);
	}
	for (bl--; bl >= 0; bl--) {
		lseek(wtmp, bl * sizeof (buf), 0);
		bp = &buf[read(wtmp, buf, sizeof (buf)) / sizeof(buf[0]) - 1];
		for ( ; bp >= buf; bp--) {
			print = want(bp);
			if (print) {
				ct = ctime(&bp->ut_xtime);
				printf("%-*.*s  %-*.*s %-*.*s %10.10s %5.5s ",
				    NMAX, NMAX, bp->ut_name,
				    LMAX, LMAX, bp->ut_line,
				    HMAX, HMAX, bp->ut_host,
				    ct, 11+ct);
			}
			for (i = 0; i < MAXTTYS; i++) {
				if (ttnames[i][0] == 0) {
					strncpy(ttnames[i], bp->ut_line,
					    sizeof(bp->ut_line));
					otime = logouts[i];
					logouts[i] = bp->ut_xtime;
					break;
				}
				if (lineq(ttnames[i], bp->ut_line)) {
					otime = logouts[i];
					logouts[i] = bp->ut_xtime;
					break;
				}
			}
			if (print) {
				if (lineq(bp->ut_line, "~"))
					printf("\n");
				else if (otime == 0)
					printf("  still logged in\n");
				else {
					long delta;
					if (otime < 0) {
						otime = -otime;
						printf("- %s", crmsg);
					} else
						printf("- %5.5s",
						    ctime(&otime)+11);
					delta = otime - bp->ut_xtime;
					if (delta < SECDAY)
					    printf("  (%5.5s)\n",
						asctime(gmtime(&delta))+11);
					else
					    printf(" (%ld+%5.5s)\n",
						delta / SECDAY,
						asctime(gmtime(&delta))+11);
				}
				fflush(stdout);
				if (++outrec >= maxrec)
					exit(0);
			}
			if (lineq(bp->ut_line, "~")) {
				for (i = 0; i < MAXTTYS; i++)
					logouts[i] = -bp->ut_xtime;
				if (nameq(bp->ut_name, "shutdown"))
					crmsg = "down ";
				else
					crmsg = "crash";
			}
		}
	}
	ct = ctime(&buf[0].ut_xtime);
	printf("\nwtmp begins %10.10s %5.5s \n", ct, ct + 11);
	exit(0);
}

void onintr(signo)
	int signo;
{
	char *ct;

	if (signo == SIGQUIT)
		signal(SIGQUIT, (void(*)())onintr);
	ct = ctime(&buf[0].ut_xtime);
	printf("\ninterrupted %10.10s %5.5s \n", ct, ct + 11);
	fflush(stdout);
	if (signo == SIGINT)
		exit(1);
}

want(bp)
	struct utmpx *bp;
{
	register char **av;
	register int ac;

	if (bp->ut_line[0] == '~' && bp->ut_name[0] == '\0')
		strcpy(bp->ut_name, "reboot");		/* bandaid */
	if (strncmp(bp->ut_line, "ftp", 3) == 0)
		bp->ut_line[3] = '\0';
	if (strncmp(bp->ut_line, "uucp", 4) == 0)
		bp->ut_line[4] = '\0';
	if (bp->ut_name[0] == 0)
		return (0);
	if (nameargs == 0)
		return (1);
	av = argv;
	for (ac = 0; ac < argc; ac++, av++) {
		if (av[0][0] == '-')
			continue;
		if (nameq(*av, bp->ut_name) || lineq(*av, bp->ut_line))
			return (1);
	}
	return (0);
}

char *
strspl(left, right)
	char *left, *right;
{
	char *res = (char *)malloc(strlen(left)+strlen(right)+1);

	strcpy(res, left);
	strcat(res, right);
	return (res);
}
