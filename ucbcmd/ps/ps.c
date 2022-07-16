/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbps:ps.c	1.4.3.1"

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
 * ps -- print things about processes.
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <mnttab.h>
#include <dirent.h>
#include <sys/signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include <sys/procfs.h>
#include <sys/param.h>
#include <curses.h> 

#define NTTYS	2	/* max ttys that can be specified with the -t option  */
			/* only one tty can be specified with SunOS ps */
#define SIZ     30      /* max processes that can be specified with -p and -g */
#define ARGSIZ	30	/* size of buffer holding args for -t, -p, -u options */

#ifndef MAXLOGIN
#define MAXLOGIN 8	/* max number of chars in login that will be printed */
#endif

/* Structure for storing user info */
struct udata {
	uid_t	uid;	/* numeric user id */
	char	name[MAXLOGIN];	/* login name, may not be null terminated */
};

int	tplen, maxlen, twidth;
char	hdr[81];
struct  winsize win;

/* udata and devl granularity for structure allocation */
#define UDQ	50

/* Pointer to user data */
struct udata *ud;
int	nud = 0;	/* number of valid ud structures */
int	maxud = 0;	/* number of ud's allocated */

struct udata uid_tbl[SIZ];      /* table to store selected uid's */
int	nut = 0;		/* counter for uid_tbl */

struct prpsinfo info;	/* process information structure from /proc */
struct prstatus status;	/* process status structure from /proc */

int	retcode = 1;
int	lflg;	/* long format */
int	uflg;	/* user-oriented output */
int	aflg;	/* Display all processes */
int	gflg;	/* Display process group leaders */
int	tflg;   /* Processes running on specific terminals */
int	rflg;	/* Running processes only flag */
int	Sflg;	/* Accumulated time plus all reaped children */
int	xflg;	/* Include processes with no controlling tty */
int	cflg;	/* Display command name */
int	vflg;	/* Virtual memory-oriented output */
int	nflg;	/* Numerical output */
int	pflg;	/* Specific process id passed as argument */
int	Uflg;	/* Update private database, ps_data */
int	errflg;
int	isatty();
char	*gettty();
char	*ttyname();
char	argbuf[ARGSIZ];
char	*parg;
char	*p1;			/* points to successive option arguments */
static char	stdbuf[BUFSIZ];

int	ndev;			/* number of devices */
int	maxdev;			/* number of devl structures allocated */

#define DNSIZE	14
struct devl {			/* device list	 */
	char	dname[DNSIZE];	/* device name	 */
	dev_t	dev;		/* device number */
} *devl;

char	*tty[NTTYS];	/* for t option */
int	ntty = 0;
pid_t	pidsave;
int	npid = 0;
int	ngrpid = 0;

char	*rname  = NULL;		/* remote system name */
char	*rroot  = NULL;		/* remote system root path */
char	*procdir = "/proc";	/* standard /proc directory */
int	rd_only = 0;		/* flag for remote filesystem read-only */
char	*rpath();		/* determines root directory for remote */
void	usage();		/* print usage message and quit */

extern int	chown(), chroot(), chdir();
extern long	lseek();
extern int	errno;
extern char	*sys_errlist[];
extern int	stat(), fstat();

main(argc, argv)
	int	argc;
	char	**argv;
{
	register char	**ttyp = tty;
	char	*name;
	char	*p;
	int	c;
	uid_t   puid;           /* puid: process user id */
	pid_t   ppid;           /* ppid: parent process id */
	pid_t   ppgrp;          /* ppgrp: process group id */
	int	i, found;		
	extern char	*optarg;
	extern int	optind;
	void	getdev();

	unsigned	size;

	DIR *dirp;
	struct dirent *dentp;
	char	pname[100];
	int	pdlen;

	if (ioctl(1, TIOCGWINSZ, &win) == -1)
                twidth = 80;
	else
                twidth = (win.ws_col == 0 ? 80 : win.ws_col);


	setbuf(stdout, stdbuf);
	while ((c = getopt(argc, argv, "lcangrSt:xuvwUR:")) != EOF)
		switch (c) {
		case 'g':
			gflg++;	/* include process group leaders */
			break;
		case 'c':	/* display internal command name */
			cflg++;
			break;
		case 'r':	/* restrict output to running processes */
			rflg++;
			break;
		case 'S':	/* display time by process and all reaped children */
			Sflg++;
			break;
		case 'x':	/* process w/o controlling tty */
			xflg++;
			break;
		case 'l':	/* long listing */
			lflg++;
			uflg = vflg = 0;
			break;
		case 'u':	/* user-oriented output */
			uflg++;
			lflg = vflg = 0;
			break;
		case 'U': 	/* update private database ps_data */
			Uflg++;
			break;
		case 'w':	/* increase display width */
                        if (twidth < 132)
                                twidth = 132;
                        else	/* second w option */
                                twidth = BUFSIZ;
                        break;
		case 'v':	/* display virtual memory format */
			vflg++;
			lflg = uflg = 0;
			break;
		case 'a':
			/*
			 * display all processes except process group
			 * leaders and processes w/o controlling tty
			 */
			aflg++;
			gflg++;
			break;
		case 'n':	/* Display numerical output */
			nflg++;
			break;
		case 't':	/* restrict output to named terminal */
#define TSZ 30
			tflg++;
			gflg++; 
			xflg=0;

			p1 = optarg;
			do {	/* only loop through once (NTTYS = 2)*/
				parg = argbuf;
				if (ntty >= NTTYS-1)
					break;
				getarg();
				if ((p = malloc(TSZ)) == NULL) {
					fprintf(stderr, "ps: no memory\n");
					exit(1);
				}
				size = TSZ;
				if (isdigit(*parg)) {
					(void) strcpy(p, "tty");
					size -= 3;
				}

				if (parg && *parg == '?') 
					xflg++;

				(void) strncat(p, parg, (int)size);
				*ttyp++ = p;
				ntty++;
			} while (*p1);
			break;
		case 'R':	/* remote system name or root path */
			rname = optarg;
			break;
		default:			/* error on ? */
			errflg++;
			break;
		}

	if (errflg) 
		usage();

	if ( optind + 1 < argc ) { /* more than one additional argument */
		fprintf(stderr,"ps: too many arguments\n");
		usage();
	}

	if ( optind < argc ) { /* user specified a specific proc id */
		pflg++;
		p1 = argv[optind];
		parg = argbuf;
		getarg();
		if (!num(parg)) {
			fprintf(stderr,
			  "ps: %s is an invalid non-numeric argument for a process id\n", parg);
			usage();	
		}
		pidsave = (pid_t)atol(parg); 
		aflg = rflg = xflg = 0;
		gflg++;
	}

	if (tflg)
		*ttyp = 0;
	/*
	 * If an appropriate option has not been specified, use the
	 * current terminal as the default.
	 */
	if (!(aflg || tflg )) { /* add specific process */
		if (rname != NULL) {
			fprintf(stderr,
			  "ps: one of -edatpug must be used with -r sysname\n");
			usage();
		}
		name = NULL;
		for (i = 2; i >= 0; i--)
			if (isatty(i)) {
				name = ttyname(i);
				break;
			}
		if (name == NULL) {
			fprintf(stderr,
			  "ps: can't find controlling terminal\n");
			exit(1);
		}
		*ttyp++ = name + 5;
		*ttyp = 0;
		ntty++;
		tflg++;
	}

	/*
	 * Determine root path for remote machine.
	 */
	if (rname != NULL) {
		if ((rroot = rpath(rname)) == NULL)
			exit(1);
		/*
		 * Move over to remote machine.
		 */
		if (chroot(rroot) != 0 || chdir("/") != 0) {
			fprintf(stderr, "ps: cannot change root to %s\n", rroot);
			exit(1);
		}
	}

	if (Uflg) {	/* update psfile */

		/* allow update only if permissions for real uid allow it */
		setuid(getuid());    

		getdev();
		getpasswd();
		wrdata();
		exit(0);
	}

	if (!readata()) {	/* get data from psfile */
		getdev();
		getpasswd();
		wrdata();
	}
	uconv();

	if (lflg ) {
		sprintf(hdr," F   UID   PID  PPID CP PRI NI   SZ  RSS    WCHAN S TT        TIME COMMAND");
	} else if (uflg) {
		if (nflg) 
			sprintf(hdr,"   UID   PID CP   SZ  RSS TT       S    START  TIME COMMAND");
		else
			sprintf(hdr,"USER       PID CP   SZ  RSS TT       S    START  TIME COMMAND");
	} else if (vflg) {
		sprintf(hdr,"   PID TT       S  TIME SIZE  RSS COMMAND"); 
	} else
		sprintf(hdr,"   PID TT       S  TIME COMMAND"); 
	twidth = twidth - strlen(hdr) + 6;
	printf("%s\n",hdr);

	/*
	 * Determine which processes to print info about by searching
	 * the /proc directory and looking at each process.
	 */
	if ((dirp = opendir(procdir)) == NULL) {
		(void) fprintf(stderr, "ps: cannot open PROC directory %s%s\n",
		  rroot ? rroot : "", procdir);
		exit(1);
	}

	(void) strcpy(pname, procdir);
	pdlen = strlen(pname);
	pname[pdlen++] = '/';

	/* for each active process --- */
	while (dentp = readdir(dirp)) {
		int	procfd;		/* filedescriptor for /proc/nnnnn */

		if (dentp->d_name[0] == '.')		/* skip . and .. */
			continue;
		(void) strcpy(pname + pdlen, dentp->d_name);
retry:
		if ((procfd = open(pname, O_RDONLY)) == -1)
			continue;

		/*
		 * Get the info structure for the process 
		 */
		if (ioctl(procfd, PIOCPSINFO, (char *) &info) == -1) {
			int	saverr = errno;

			(void) close(procfd);
			if (saverr == EAGAIN)
				goto retry;
			if (saverr != ENOENT)
				(void) fprintf(stderr,
				  "ps: PIOCPSINFO on %s: %s\n",
				  pname, sys_errlist[saverr]);
			continue;
		}
                /*
                 * Get the status structure for the process if S option
                 */
		if (Sflg) {
                    if (ioctl(procfd, PIOCSTATUS, (char *) &status) == -1) {
                        int     saverr = errno;

                        (void) close(procfd);
                        if (saverr == EAGAIN)
                                goto retry;
                        if (saverr != ENOENT)
                                (void) fprintf(stderr,
                                  "ps: PIOCSTATUS on %s: %s\n",
                                  pname, sys_errlist[saverr]);
                        continue;
                    }
		}
		/*
		 * Close quickly
		 */
		(void) close(procfd);

		found = 0;
		if (info.pr_state == 0)		/* can't happen? */
			continue;
		puid = info.pr_uid;
		ppid = info.pr_pid;
		ppgrp = info.pr_pgrp;

		/* Display only process from command line */
		if (pflg) { 	/* pid in arg list */
			/*if (search(pidlist, npid, ppid))*/
			if (pidsave == ppid ) {
				found++; 
			}
			else
				continue;
		} 

		/*
		 * Omit process group leaders unless 'g' option.
		 */
		if ((ppid == ppgrp) && !(gflg))
			continue;

		/* 
		 * Omit non-running processes for 'r' option
		 */
		if (rflg && !(info.pr_sname =='O' || info.pr_sname =='R')) 
			continue;

		if (!found && !tflg && !aflg )
			continue;

		if (prcom(puid, found)) {
			printf("\n");
			retcode = 0;
		}
	}

	(void) closedir(dirp);
	exit(retcode);
	/* NOTREACHED */
}

static void	
usage()		/* print usage message and quit */
{
	static char usage1[] = "ps [ -acglnrSuvwx ] [ -t term ] [ num ]";

	fprintf(stderr, "usage: %s\n", usage1);
	exit(1);
}

/*
 * readata reads in the open devices (terminals) and stores 
 * info in the devl structure.
 */
static char	psfile[] = "/etc/ps_data";

int
readata()
{
	struct stat sbuf1, sbuf2;
	int fd;
	struct flock wflock;

	if ((fd = open(psfile, O_RDONLY)) == -1)
		return 0;

	wflock.l_type = F_RDLCK;
	wflock.l_whence = wflock.l_start = wflock.l_len = 0;
	if (fcntl(fd, F_SETLKW, &wflock) < 0 )  {
		(void) close(fd);
		return 0;
	}

	if (fstat(fd, &sbuf1) < 0
	  || sbuf1.st_size == 0
	  || stat("/dev", &sbuf2) == -1
	  || sbuf1.st_mtime <= sbuf2.st_mtime
	  || sbuf1.st_mtime <= sbuf2.st_ctime
	  || stat("/etc/passwd", &sbuf2) == -1
	  || sbuf1.st_mtime <= sbuf2.st_mtime
	  || sbuf1.st_mtime <= sbuf2.st_ctime) {
		if (!rd_only) {		/* if read-only, believe old data */
			(void) close(fd);
			return 0;
		}
	}

	/* Read /dev data from psfile. */
	if (psread(fd, (char *) &ndev, sizeof(ndev)) == 0)  {
		(void) close(fd);
		return 0;
	}

	if ((devl = (struct devl *)malloc(ndev * sizeof(*devl))) == NULL) {
		fprintf(stderr, "ps: malloc() for device table failed, %s\n",
		  sys_errlist[errno]);
		exit(1);
	}
	if (psread(fd, (char *)devl, ndev * sizeof(*devl)) == 0)  {
		(void) close(fd);
		return 0;
	}

	/* Read /etc/passwd data from psfile. */
	if (psread(fd, (char *) &nud, sizeof(nud)) == 0)  {
		(void) close(fd);
		return 0;
	}
	if ((ud = (struct udata *)malloc(nud * sizeof(*ud))) == NULL) {
		fprintf(stderr, "ps: not enough memory for udata table\n");
		exit(1);
	}
	if (psread(fd, (char *)ud, nud * sizeof(*ud)) == 0)  {
		(void) close(fd);
		return 0;
	}

	(void) close(fd);
	return 1;
}

/*
 * getdev() uses ftw() to pass pathnames under /dev to gdev()
 * along with a status buffer.
 */
void
getdev()
{
	int	gdev();
	int	rcode;

	ndev = 0;
	rcode = ftw("/dev", gdev, 17);

	switch (rcode) {
	case 0:
		return;		/* successful return, devl populated */
	case 1:
		fprintf(stderr, "ps: ftw() encountered problem\n");
		break;
	case -1:
		fprintf(stderr, "ps: ftw() failed, %s\n", sys_errlist[errno]);
		break;
	default:
		fprintf(stderr, "ps: ftw() unexpected return, rcode=%d\n",
		 rcode);
		break;
	}
	exit(1);
}

/*
 * gdev() puts device names and ID into the devl structure for character
 * special files in /dev.  The "/dev/" string is stripped from the name
 * and if the resulting pathname exceeds DNSIZE in length then the highest
 * level directory names are stripped until the pathname is DNSIZE or less.
 */
int
gdev(objptr, statp, numb)
	char	*objptr;
	struct stat *statp;
	int	numb;
{
	register int	i;
	int	leng, start;
	static struct devl ldevl[2];
	static int	lndev, consflg;

	switch (numb) {

	case FTW_F:	
		if ((statp->st_mode & S_IFMT) == S_IFCHR) {
			/* Get more and be ready for syscon & systty. */
			while (ndev + lndev >= maxdev) {
				maxdev += UDQ;
				devl = (struct devl *) ((devl == NULL) ? 
				  malloc(sizeof(struct devl ) * maxdev) : 
				  realloc(devl, sizeof(struct devl ) * maxdev));
				if (devl == NULL) {
					fprintf(stderr,
					    "ps: not enough memory for %d devices\n",
					    maxdev);
					exit(1);
				}
			}
			/*
			 * Save systty & syscon entries if the console
			 * entry hasn't been seen.
			 */
			if (!consflg
			  && (strcmp("/dev/systty", objptr) == 0
			    || strcmp("/dev/syscon", objptr) == 0)) {
				(void) strncpy(ldevl[lndev].dname,
				  &objptr[5], DNSIZE);
				ldevl[lndev].dev = statp->st_rdev;
				lndev++;
				return 0;
			}

			leng = strlen(objptr);
			/* Strip off /dev/ */
			if (leng < DNSIZE + 4)
				(void) strcpy(devl[ndev].dname, &objptr[5]);
			else {
				start = leng - DNSIZE - 1;

				for (i = start; i < leng && (objptr[i] != '/');
				  i++)
					;
				if (i == leng )
					(void) strncpy(devl[ndev].dname,
					  &objptr[start], DNSIZE);
				else
					(void) strncpy(devl[ndev].dname,
					  &objptr[i+1], DNSIZE);
			}
			devl[ndev].dev = statp->st_rdev;
			ndev++;
			/*
			 * Put systty & syscon entries in devl when console
			 * is found.
			 */
			if (strcmp("/dev/console", objptr) == 0) {
				consflg++;
				for (i = 0; i < lndev; i++) {
					(void) strncpy(devl[ndev].dname,
					  ldevl[i].dname, DNSIZE);
					devl[ndev].dev = ldevl[i].dev;
					ndev++;
				}
				lndev = 0;
			}
		}
		return 0;

	case FTW_D:
	case FTW_DNR:
	case FTW_NS:
		return 0;

	default:
		fprintf(stderr, "ps: gdev() error, %d, encountered\n", numb);
		return 1;
	}
}

/*
 * Get the passwd file data into the ud structure.
 */
getpasswd()
{
	struct passwd *pw, *getpwent();
	void	endpwent();

	ud = NULL;
	nud = 0;
	maxud = 0;

	while ((pw = getpwent()) != NULL) {
		while (nud >= maxud) {
			maxud += UDQ;
			ud = (struct udata *) ((ud == NULL) ? 
			  malloc(sizeof(struct udata ) * maxud) : 
			  realloc(ud, sizeof(struct udata ) * maxud));
			if (ud == NULL) {
				fprintf(stderr,
				  "ps: not enough memory for %d users\n", maxud);
				exit(1);
			}
		}
		/*
		 * Copy fields from pw file structure to udata.
		 */
		ud[nud].uid = pw->pw_uid;
		(void) strncpy(ud[nud].name, pw->pw_name, MAXLOGIN);
		nud++;
	}
	endpwent();
}

wrdata()
{
	int	fd;
	struct flock wflock;

	(void) umask(02);
	if (unlink(psfile) == -1 && errno != ENOENT)  {
		fprintf(stderr, "ps: unlink() failed\n");
		fprintf(stderr, "ps: /etc/ps_data, %s\n", sys_errlist[errno]);
		fprintf(stderr, "ps: Please notify your System Administrator\n");
		return;
	}
	if ((fd = open(psfile, O_WRONLY|O_CREAT, 0664)) == -1) {
		fprintf(stderr, "ps: open() for write failed\n");
		fprintf(stderr, "ps: /etc/ps_data, %s\n", sys_errlist[errno]);
		fprintf(stderr, "ps: Please notify your System Administrator\n");
		return;
	}

	wflock.l_type = F_WRLCK;
	wflock.l_whence = wflock.l_start = wflock.l_len = 0;
	if (fcntl(fd, F_SETLKW, &wflock) < 0)  {
		(void) close(fd);
		return;
	}

	/*
	 * Make owner root, group sys.
	 */
	(void) chown(psfile, (uid_t)0, (gid_t)3);

	/* write /dev data */
	pswrite(fd, (char *) &ndev, sizeof(ndev));
	pswrite(fd, (char *)devl, ndev * sizeof(*devl));

	/* write /etc/passwd data */
	pswrite(fd, (char *) &nud, sizeof(nud));
	pswrite(fd, (char *)ud, nud * sizeof(*ud));

	(void) close(fd);
}

/*
 * getarg() finds the next argument in list and copies arg into argbuf.
 * p1 first pts to arg passed back from getopt routine.  p1 is then
 * bumped to next character that is not a comma or blank -- p1 NULL
 * indicates end of list.
 */

getarg()
{
	char	*parga;

	parga = argbuf;
	while (*p1 && *p1 != ',' && *p1 != ' ')
		*parga++ = *p1++;
	*parga = '\0';

	while (*p1 && (*p1 == ',' || *p1 == ' '))
		p1++;
}

/*
 * gettty returns the user's tty number or ? if none.
 */
char *
gettty(ip)
	register int	*ip;	/* where the search left off last time */
{
	register int	i;

	if (info.pr_ttydev != PRNODEV && *ip >= 0) {
		for (i = *ip; i < ndev; i++) {
			if (devl[i].dev == info.pr_ttydev) {
				*ip = i + 1;
				return devl[i].dname;
			}
		}
	}
	*ip = -1;
	return "?";
}

/*
 * Print info about the process.
 */
prcom(puid, found)
	uid_t	puid; 
	int	found;
{
	register char	*cp;
	register char	*tp;
	char	*ctime();
	long	tm;
	int	i, wcnt, length;
	wchar_t	wchar;
	register char	**ttyp, *str;

	/*
	 * If process is zombie, call print routine and return.
	 */
	if (info.pr_zomb) {
		if (tflg && !found)
			return 0;
		else {
			przom(puid);
			return 1;
		}
	}

	/*
	 * Get current terminal.  If none ("?") and 'a' is set, don't print
	 * info.  If 't' is set, check if term is in list of desired terminals
	 * and print it if it is.
	 */
	i = 0;
	tp = gettty(&i);

	if (*tp == '?' && !found && !xflg ) 
		return 0;
	
	if ( !(*tp == '?' && xflg) && tflg && !found) {
		int match = 0;

		/*
		 * Look for same device under different names.
		 */
		while (i >= 0 && !match) {
			for (ttyp = tty; (str = *ttyp) != 0 && !match; ttyp++)
				if (strcmp(tp, str) == 0)
					match = 1;
			if (!match)
				tp = gettty(&i);
		}
		if (!match)
			return 0;
	} 

	if (lflg)
		printf("%2x", info.pr_flag & 0377);
	if (uflg) { 						
                if (!nflg) {
			if ((i = getunam(puid)) >= 0)
                        	printf("%-8.8s", ud[i].name);	/* USER */
                	else   
                        	printf("%8.8ld", puid);		/* UID */
		} else {
                        printf("%6ld", puid);			/* UID */
		}
	} else if (lflg)                
                printf("%6ld", puid);  				/* UID */
	
	printf("%6ld", info.pr_pid);				/* PID */
	if (lflg)
                printf("%6ld", info.pr_ppid);                    /* PPID */
	if (lflg || uflg)
                        printf("%3d", info.pr_cpu & 0377);      /* CPU */
	if (lflg) {
			printf("%4d", info.pr_pri);		/* PRI */
			printf("%3d", info.pr_nice);		/* NICE */
	}
	if (lflg || uflg) {
			printf("%5d", info.pr_size);   		/* SZ */
			printf("%5d", info.pr_rssize);		/* RSS */
	}
	if (lflg)
			printf("%9x", info.pr_wchan);           /* WCHAN */
	if (tplen = strlen(tp) > 9)
		maxlen = twidth - tplen + 9;
	else
		maxlen = twidth;

	if (!lflg)
		printf(" %-8.14s", tp);				/* TTY */
	printf(" %c", info.pr_sname);				/* STATE */
	if (lflg)
		printf(" %-8.14s", tp);                         /* TTY */
	if (uflg)
		prtime(info.pr_start);				/* START */

	/* time just for process */
	tm = info.pr_time.tv_sec;
	if (Sflg) {	/* calculate time for process and all reaped children*/	
		tm += (status.pr_cutime.tv_sec + status.pr_cstime.tv_sec);
	}

	printf(" %2ld:%.2ld", tm / 60, tm % 60);		/* TIME */

	if (vflg) {
		printf("%5d", info.pr_size);            	/* SZ */
		printf("%5d", info.pr_rssize);			/* RSS */
	}
	if (cflg) {                                            	/* CMD */
                wcnt = namencnt(info.pr_fname, 16, maxlen);
                printf(" %.*s", wcnt, info.pr_fname);
                return 1;
        }
	/*
	 * PRARGSZ == length of cmd arg string.
	 */
	for (cp = info.pr_psargs; cp < &info.pr_psargs[PRARGSZ]; ) {
		if (*cp == 0) 
			break;
		length = mbtowc(&wchar, cp, MB_LEN_MAX);
		if (length < 0 || !wisprint(wchar)) {
	        	printf(" [ %.8s ]", info.pr_fname);
			return 1;
		}
		cp += length;
	}
	wcnt = namencnt(info.pr_psargs, PRARGSZ, maxlen);
	printf(" %.*s", wcnt, info.pr_psargs);
	return 1;
}

/*
 * Returns 1 if arg is found in array arr, of length num; 0 otherwise.
 */
int
search(arr, number, arg)
	pid_t arr[];
	register int number;
	register pid_t arg;
{
	register int i;

	for (i = 0; i < number; i++)
		if (arg == arr[i]) {
			return 1;
		}
	return 0;
}

uconv()
{
	uid_t pwuid;
	int found, i, j;

	/*
	 * Search name array for oarg.
	 */
	for (i = 0; i < nut; i++) {
		found = -1;
		for (j = 0; j < nud; j++) {
			if (strncmp(uid_tbl[i].name, ud[j].name,
			  MAXLOGIN) == 0) {
				found = j;
				break;
			}
		}
		/*
		 * If not found and oarg is numeric, search number array.
		 */
		if (found < 0
		  && uid_tbl[i].name[0] >= '0'
		  && uid_tbl[i].name[0] <= '9') {
			pwuid = (uid_t)atol(uid_tbl[i].name);
			for (j = 0; j < nud; j++) {
				if (pwuid == ud[j].uid) {
					found = j;
					break;
				}
			}
		}

		/*
		 * If found, enter found index into tbl array.
		 */
		if (found != -1) {
			uid_tbl[i].uid = ud[found].uid;
			(void) strncpy(uid_tbl[i].name, ud[found].name,
			  MAXLOGIN);
		} else {
			fprintf(stderr,
			  "ps: unknown user %s\n", uid_tbl[i].name);
			for (j = i + 1; j < nut; j++) {
				(void) strncpy(uid_tbl[j-1].name,
				  uid_tbl[j].name, MAXLOGIN);
			}
			nut--;
			if (nut <= 0) 
				exit(1);
			i--;
		}
	}
}

/*
 * For printing user's name instead of number.
 * Search table of userid numbers and if puid is found, return the
 * the index for the corresponding name.  
 */
int
getunam(puid)
	register uid_t puid;
{
	register int i;

	for (i = 0; i < nud; i++)
		if (ud[i].uid == puid)
			return i;
	return -1;
}

/*
 * Return 1 if puid is in table, otherwise 0.
 */
int
ufind(puid)
	register uid_t puid;
{
	register int i;

	for (i = 0; i < nut; i++)
		if (uid_tbl[i].uid == puid)
			return 1;
	return 0;
}

/*
 * Special read; unlinks psfile on read error.
 */
int
psread(fd, bp, bs)
	int fd;
	char *bp;
	unsigned int bs;
{
	int rbs;

	if ((rbs = read(fd, bp, bs)) != bs) {
		fprintf(stderr, "ps: psread() error on read, rbs=%d, bs=%d\n",
		  rbs, bs);
		(void) unlink(psfile);
		return 0;
	}
	return 1;
}

/*
 * Special write; unlinks psfile on write error.
 */
pswrite(fd, bp, bs)
int	fd;
char	*bp;
unsigned	bs;
{
	int	wbs;

	if ((wbs = write(fd, bp, bs)) != bs) {
		fprintf(stderr, "ps: pswrite() error on write, wbs=%d, bs=%d\n",
		  wbs, bs);
		(void) unlink(psfile);
	}
}

/*
 * Print starting time of process unless process started more than 24 hours
 * ago, in which case the date is printed.
 */
prtime(st)
	timestruc_t st;
{
	char sttim[26];
	static time_t tim = 0L;
	time_t starttime;

	if (tim == 0L)
		tim = time((time_t *) 0);
	starttime = st.tv_sec;
	if (tim - starttime > 24*60*60) {
		cftime(sttim, "%b %d", &starttime);
		sttim[7] = '\0';
	} else {
		cftime(sttim, "%H:%M:%S", &starttime);
		sttim[8] = '\0';
	}
	printf("%9.9s", sttim);
}

przom(puid)
	uid_t	puid;
{
	int	i;
	long	tm;
        if (lflg)
                printf("%2x", info.pr_flag & 0377);
        if (uflg) {
                if ((i = getunam(puid)) >= 0)
                        printf("%-8.8s", ud[i].name);           /* USER */
                else
                        printf("%8.8ld", puid);                  /* UID */
        } else if (lflg)
                printf("%6ld", puid);                            /* UID */
         
        printf("%6ld", info.pr_pid);                             /* PID */
        if (lflg)
                printf("%6ld", info.pr_ppid);                    /* PPID */
        if (lflg || uflg)
                        printf("%3d", info.pr_cpu & 0377);      /* CPU */
        if (lflg) {
                        printf("%4d", info.pr_pri);             /* PRI */
                        printf("   ");				/* NICE */
        }
        if (lflg || uflg) {
                        printf("     ");			/* SZ */
                        printf("     ");			/* RSS */
        }
        if (lflg)
                        printf("         ");			/* WCHAN */
        printf("          ");					/* TTY */
        printf("%c", info.pr_sname);                            /* STATE */
        if (uflg)
                printf("         ");				/* START */

        /* time just for process */
        tm = info.pr_time.tv_sec;
        if (Sflg) {     /* calculate time for process and all reaped children*/
                tm = (status.pr_cutime.tv_sec + status.pr_cstime.tv_sec);
        }
        printf(" %2ld:%.2ld", tm / 60, tm % 60);                /* TIME */

        if (vflg) {
                printf("     ");				/* SZ */
                printf("     ");				/* RSS */
        }
	printf(" %.*s", maxlen, " <defunct>");
}

/*
 * Returns true iff string is all numeric.
 */
int
num(s)
	register char	*s;
{
	register int c;

	if (s == NULL)
		return 0;
	c = *s;
	do {
		if (!isdigit(c))
			return 0;
	} while (c = *++s);
	return 1;
}

/*
 * Function to compute the number of printable bytes in a multibyte
 * command string ("internationalization").
 */
int
namencnt(cmd, eucsize, scrsize)
	register char *cmd;
	int eucsize;
	int scrsize;
{
	register int eucwcnt = 0, scrwcnt = 0;
	register int neucsz, nscrsz;
	wchar_t  wchar;

	while (*cmd != '\0') {
		if ((neucsz = mbtowc(&wchar, cmd, MB_LEN_MAX)) < 0)
			return 8; /* default to use for illegal chars */
		if ((nscrsz = scrwidth(wchar)) == 0)
			return 8;
		if (eucwcnt + neucsz > eucsize || scrwcnt + nscrsz > scrsize)
			break;
		eucwcnt += neucsz;
		scrwcnt += nscrsz;
		cmd += neucsz;
	}
	return eucwcnt;
}

char *
rpath(name)	/* given remote system name, return its root directory path */
	register char	*name;
{
	struct mnttab mnttab;
	register FILE *fp;
	register char	*dirp;
	register char	*path = NULL;

	if (strchr(name, '/'))	/* if user specified a pathname, use it */
		return name;

	/*
	 * Open the mount table.
	 */
	if ((fp = fopen("/etc/mnttab", "r")) == NULL) {
		fprintf(stderr, "ps: cannot open /etc/mnttab\n");
		return NULL;
	}

	/*
	 * Search for a remote filesystem with last component matching name.
	 */
	while (fread((char *) & mnttab, (int)sizeof(mnttab), 1, fp) == 1) {
		if ((mnttab.mt_ro_flg & 02)	/* remote flag */
		  && (dirp = strrchr(mnttab.mt_filsys, '/'))
		  && strcmp(++dirp, name) == 0) {
			dirp = mnttab.mt_filsys;
			path = (char *)malloc((unsigned)(strlen(dirp) + strlen(procdir) + 2));
			if (path == NULL) {
				fprintf(stderr,
				  "ps: not enough memory for remote pathname\n");
				exit(1);
			}
			(void) strcpy(path, dirp);
			(void) strcat(path, procdir);
			if (isprocdir(path)) {	/* found a /proc directory */
				rd_only = mnttab.mt_ro_flg & 01; /* read-only */
				(void) strcpy(path, dirp);
				break;
			}
			free(path);
			path = NULL;
		}
	}
	(void) fclose(fp);
	if (path == NULL)
		fprintf(stderr, "ps: cannot find path to system %s\n", name);
	return path;
}

/*
 * Return true iff dir is a /proc directory.
 *
 * This works because of the fact that "/proc/0" and "/proc/00" are the
 * same file, namely process 0, and are not linked to each other.  Ugly.
 */
static int	
isprocdir(dir)		/* return TRUE iff dir is a PROC directory */
	char	*dir;
{
	extern int stat();
	struct stat stat1;	/* dir/0  */
	struct stat stat2;	/* dir/00 */
	char	path[200];
	register char	*p;

	/*
	 * Make a copy of the directory name without trailing '/'s
	 */
	if (dir == NULL)
		(void) strcpy(path, ".");
	else {
		(void) strncpy(path, dir, (int)sizeof(path) - 4);
		path[sizeof(path)-4] = '\0';
		p = path + strlen(path);
		while (p > path && *--p == '/')
			*p = '\0';
		if (*path == '\0')
			(void) strcpy(path, ".");
	}

	/*
	 * Append "/0" to the directory path and stat() the file.
	 */
	p = path + strlen(path);
	*p++ = '/';
	*p++ = '0';
	*p = '\0';
	if (stat(path, &stat1) != 0)
		return FALSE;

	/*
	 * Append "/00" to the directory path and stat() the file.
	 */
	*p++ = '0';
	*p = '\0';
	if (stat(path, &stat2) != 0)
		return FALSE;

	/*
	 * See if we ended up with the same file.
	 */
	if (stat1.st_dev != stat2.st_dev
	  || stat1.st_ino   != stat2.st_ino
	  || stat1.st_mode  != stat2.st_mode
	  || stat1.st_nlink != stat2.st_nlink
	  || stat1.st_uid   != stat2.st_uid
	  || stat1.st_gid   != stat2.st_gid
	  || stat1.st_size  != stat2.st_size)
		return FALSE;

	/*
	 * Return TRUE iff we have a regular file with a single link.
	 */
	return (stat1.st_mode & S_IFMT) == S_IFREG && stat1.st_nlink == 1;
}
