/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved. 					*/

#ident	"@(#)touch:touch.c	1.12"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

#define	dysize(y) \
	(((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0) ? 366 : 365)

struct	stat	stbuf;
int	status;
int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char	*cbp;
time_t	timbuf;

gtime()
{
	register int i, y, t;
	int d, h, m;
	long nt;

	tzset();

	t = gpair();
	if(t<1 || t>12)
		return(1);
	d = gpair();
	if(d<1 || d>31)
		return(1);
	h = gpair();
	if(h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if(m<0 || m>59)
		return(1);
	y = gpair();
	if (y<0) {
		(void) time(&nt);
		y = localtime(&nt)->tm_year;
	}
	if (*cbp == 'p')
		h += 12;
	if (h<0 || h>23)
		return(1);
	timbuf = 0;
	y += 1900;
	for(i=1970; i<y; i++)
		timbuf += dysize(i);
	/* Leap year */
	if (dysize(y)==366 && t >= 3)
		timbuf += 1;
	while(--t)
		timbuf += dmsize[t-1];
	timbuf += (d-1);
	timbuf *= 24;
	timbuf += h;
	timbuf *= 60;
	timbuf += m;
	timbuf *= 60;
	return(0);
}

gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

main(argc, argv)
char *argv[];
{
	register c;
	struct utbuf { time_t actime, modtime; } times;

	int mflg=1, aflg=1, cflg=0, fflg=0, nflg=0, errflg=0, optc, fd;
	int stflg=0;
	char *proto;
	struct  stat prstbuf; 
	char *usage = "usage: %s [%s] [mmddhhmm[yy]] file ...\n";
	char *tusage = "-amc";
	char *susage = "-f file";
	extern char *optarg, *basename();
	extern int optind;

	argv[0] = basename(argv[0]);
	if (!strcmp(argv[0], "settime")) {
		while ((optc = getopt(argc, argv, "f:")) != EOF)
			switch (optc) {
			case 'f':
				fflg++;
				proto = optarg;
				break;
			default:
				errflg++;
				break;
			};
		stflg = 1;
		++cflg;
	} 
	else
		while ((optc=getopt(argc, argv, "amcf")) != EOF)
			switch(optc) {
			case 'm':
				mflg++;
				aflg--;
				break;
			case 'a':
				aflg++;
				mflg--;
				break;
			case 'c':
				cflg++;
				break;
			case 'f':
				break;		/* silently ignore   */ 
			case '?':
				errflg++;
			}
	if(((argc-optind) < 1) || errflg) {
		(void) fprintf(stderr, usage, argv[0],
				stflg ? susage : tusage);
		exit(2);
	}
	status = 0;
	if (fflg) {
		if (stat(proto, &prstbuf) == -1) {
			(void) fprintf(stderr, "%s -- ", argv[0]);
			perror(proto);
			exit(2);
		}
	} 
	else if(!isnumber(argv[optind]))
		if((aflg <= 0) || (mflg <= 0))
                        prstbuf.st_atime = prstbuf.st_mtime = time((long *) 0);
		else
			nflg++;
	else {
		cbp = (char *)argv[optind++];
		if(gtime()) {
			(void) fprintf(stderr,"date: bad conversion\n");
			exit(2);
		}
		timbuf += timezone;
		if (localtime(&timbuf)->tm_isdst)
			timbuf += -1*60*60;
		prstbuf.st_mtime = prstbuf.st_atime = timbuf;
	}
	for(c=optind; c<argc; c++) {
		if(stat(argv[c], &stbuf)) {
			if(cflg) {
				status++;
				continue;
			}
			else if ((fd = creat (argv[c], (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))) < 0) {
				(void) fprintf(stderr,
					"%s: %s cannot create\n", 
					argv[0], argv[c]);
				status++;
				continue;
			}
			else {
				(void) close(fd);
				if(stat(argv[c], &stbuf)) {
					(void) fprintf(stderr,
						"%s: %s cannot stat\n",
						argv[0], argv[c]);
					status++;
					continue;
				}
			}
		}
		times.actime = prstbuf.st_atime;
		times.modtime = prstbuf.st_mtime;
		if (mflg <= 0)
			times.modtime = stbuf.st_mtime;
		if (aflg <= 0)
			times.actime = stbuf.st_atime;

		if(utime(argv[c], (struct utbuf *)(nflg? 0: &times))) {
			(void) fprintf(stderr,
				"%s: cannot change times on %s\n",
				argv[0], argv[c]);
			status++;
			continue;
		}
	}
	exit(status);	/*NOTREACHED*/
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}
