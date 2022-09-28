/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)whodo:whodo.c	1.30.1.2"

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

#include   <stdio.h>
#include   <stdlib.h>
#include   <fcntl.h>
#include   <time.h>
#include   <sys/errno.h>
#include   <sys/types.h>

#ifdef i386
#include   <sys/user.h>
#endif

#include   <sys/param.h>
#include   <utmp.h>
#include   <sys/utsname.h>
#include   <sys/stat.h>
#include   <dirent.h>
#include   <sys/procfs.h>	/* /proc header file */
#include   <sys/proc.h>		/* needed for process states */

#define NMAX sizeof(ut->ut_name)
#define LMAX sizeof(ut->ut_line)
#define DIV60(t)	((t+30)/60)    /* x/60 rounded */

#define ENTRY   sizeof(struct psdata)
#define ERR     (-1)
#define error(str)      fprintf(stderr, "%s: %s\n", arg0, str)

#define DEVNAMELEN  	14
#define HSIZE		256		/* size of process hash table 	*/
#define PROCDIR		"/proc"
#define PS_DATA		"/etc/ps_data"	/*  ps_data file built by ps command 	*/
#define INITPROCESS	(pid_t)1	/* init process pid */
#define NONE		'n'		/* no state */
#define VISITED		'v'		/* marked node as visited */
					   

/**
 * File /etc/ps_data is built by ps command; we only use the
 * only use 1st part (device info)
 **/
struct psdata {
        char    device[DEVNAMELEN];	/* device name 		 */
        dev_t   dev;    		/* major/minor of device */
} ;
struct psdata   *psptr;

int 	ndevs;				/* number of configured devices */

struct uproc {
	pid_t	p_upid;			/* user process id */
	char	p_state;		/* numeric value of process state */
        dev_t   p_ttyd;			/* controlling tty of process */
        time_t  p_time;			/* ticks of user & system time */
	time_t	p_ctime;		/* ticks of child user & system time */
	int	p_igintr;		/* 1=ignores SIGQUIT and SIGINT*/
        char    p_comm[PRARGSZ+1];	/* command */
        char    p_args[PRARGSZ+1];	/* command line arguments */
	struct uproc	*p_child,	/* first child pointer */
			*p_sibling,	/* sibling pointer */
			*p_pgrplink,	/* pgrp link */
			*p_link;	/* hash table chain pointer */
};

/**
 *	define hash table for struct uproc 
 *	Hash function uses process id
 * 	and the size of the hash table(HSIZE)
 *	to determine process index into the table. 
 **/
struct uproc	pr_htbl[HSIZE];

struct 	uproc	*findhash();
time_t  	findidle();
int		clnarglist();

unsigned        size;
int             fd;
char            *arg0;
extern	time_t	time();
extern char	*strrchr();
extern int      errno;
extern char     *sys_errlist[];
extern char     *optarg;
int		header = 1;	/* true if -h flag: don't print heading */
int		lflag = 0;	/* true if -l flag: w command format */
char *  	sel_user;	/* login of particular user selected */
time_t		now;		/* current time of day */
time_t  	uptime;		/* time of last reboot & elapsed time since */
int     	nusers;		/* number of users logged in now */
time_t  	idle; 		/* number of minutes user is idle */
time_t  jobtime;                /* total cpu time visible */
char    doing[520];             /* process attached to terminal
*/
time_t  proctime;               /* cpu time of process in doing
*/
int	curpid, empty;

main(argc, argv)
int argc;
char *argv[];
{
        register struct utmp    *ut;
	struct utmp		*utmpbegin;
        register struct tm      *tm;
        struct uproc    *up, *parent, *pgrp;
	struct prpsinfo		info;
	struct sigaction	actinfo[MAXSIG];
	struct prstatus		statinfo;
        unsigned        utmpend;
        struct tm       *localtime();
        struct stat     sbuf;
        struct utsname  uts;
	DIR		*dirp;
	struct	dirent  *dp;
        char            *getty();
	char 		pname[MAXNAMELEN];
	int		procfd;
        register 	int i;
        int 		days, hrs, mins;
	long		nsec;

        arg0 = argv[0];
	
        while (argc > 1) {
                if (argv[1][0] == '-') {
                        for (i=1; argv[1][i]; i++) {
                               switch(argv[1][i]) {

                                case 'h':
                                        header = 0;
                                        break;

                                case 'l':
                                        lflag++;
                                        break;

                                default:
                                	printf("usage: %s [ -hl ] [ user ]\n", arg0);
                                        exit(1);
                                }
                        }
                } else {
                        if (!isalnum(argv[1][0]) || argc > 2) {
                                printf("usage: %s [ -hl ] [ user ]\n", arg0);
                                exit(1);
                        } else
                                sel_user = argv[1];
                }
                argc--; argv++;
        }      

	/**
	 *  read  UTMP_FILE which contains the information about
	 *  each login users 
	 *  
	 **/
        if(stat(UTMP_FILE, &sbuf) == ERR) {
		fprintf(stderr,"%s: stat error of %s: %s\n",
			arg0, UTMP_FILE, sys_errlist[errno]);
                exit(1);
        }
        size = (unsigned)sbuf.st_size;
        if((ut = (struct utmp *)malloc(size)) == NULL) {
		fprintf(stderr,"%s: malloc error of %s: %s\n",
			arg0, UTMP_FILE, sys_errlist[errno]);
                exit(1);
        }
        if((fd = open(UTMP_FILE, O_RDONLY)) == ERR) {
		fprintf(stderr, "%s: open error of %s: %s\n",
			arg0, UTMP_FILE, sys_errlist[errno]);
                exit(1);
        }
        if(read(fd, (char *)ut, size) == ERR) {
		fprintf(stderr, "%s: read error of %s: %s\n",
			arg0, UTMP_FILE, sys_errlist[errno]);
                exit(1);
        }
	utmpbegin = ut;			/* ptr to start of utmp data*/
        utmpend = (unsigned)ut + size;  /* ptr to end of utmp data */
        close(fd);

        time(&now);	/* get current time */

	if (header) {	/* print a header */
		if (lflag) {	/* w command format header */
			prtat(&now);
			for (ut = utmpbegin; ut < (struct utmp *)utmpend; ut++){

				if(ut->ut_type == USER_PROCESS) {
					nusers++;
				} else if(ut->ut_type == BOOT_TIME) {
					uptime = now - ut->ut_time;
					uptime += 30;
					days = uptime / (60*60*24);
					uptime %= (60*60*24);
					hrs = uptime / (60*60);
					uptime %= (60*60);
					mins = uptime / 60;

					printf("  up");
					if (days > 0)
						printf(" %d day%s,", days, days>1?"s":"");
					if (hrs > 0 && mins > 0) {
						printf(" %2d:%02d,", hrs, mins);
		
					} else {
						if (hrs > 0)
							printf(" %d hr%s,", hrs, hrs>1?"s":"");
						if (mins > 0)

							printf(" %d min%s,", mins, mins>1?"s":"");

					}
				}
			}
			
			ut = utmpbegin;	/* rewind utmp data */
                	printf("  %d user%s\n", nusers, nusers>1?"s":"");
			printf("User     tty           login@  idle   JCPU   PCPU  what\n");
		} else {	/* standard whodo header */

			/**
			* print current time and date
			**/
        		printf("%s", ctime(&now));

        		/**
         		 * print system name
         		 **/
        		uname(&uts);
        		printf("%s\n", uts.sysname);

		}
	}


        /**
         * read in device info from PS_DATA file
         **/
        if((fd = open(PS_DATA, O_RDONLY)) == ERR) {
		fprintf(stderr, "%s: open error of %s: %s\n",
			arg0, PS_DATA, sys_errlist[errno]);
                exit(1);
        }
        /* first int tells how many entries follow */
        if(read(fd, (char *)&ndevs, sizeof(ndevs)) == ERR) {
		fprintf(stderr, "%s: read error of size of device table info: %s\n",
			arg0, sys_errlist[errno]);
                exit(1);
        }
	/**
	 * allocate memory and read in device table from PS_DATA file
	 **/
        if((psptr = (struct psdata *)malloc(ndevs*ENTRY)) == NULL) {
		fprintf(stderr, "%s: malloc error of %s device table: %s\n",
			arg0, PS_DATA, sys_errlist[errno]);
                exit(1);
        }
        if(read(fd, (char *)psptr, ndevs*ENTRY) == ERR) {
		fprintf(stderr,"%s: read error of %s device info: %s\n",
			arg0, PS_DATA, sys_errlist[errno]);
                exit(1);
        }
        close(fd);


	/**
	 * loop through /proc, reading info about each process
	 * and build the parent/child tree
	 **/
	if(!(dirp = opendir(PROCDIR))) {
		fprintf(stderr, "%s: could not open %s: %s\n",
			arg0, PROCDIR, sys_errlist[errno]);
		exit(1);
	}

	while(dp = readdir(dirp)) {
retry:
		if(dp->d_name[0] == '.')
			continue;
		sprintf(pname,"%s/%s", PROCDIR, dp->d_name);
		if((procfd = open(pname, O_RDONLY)) == -1) {
			continue;
		}
		if(ioctl(procfd, PIOCPSINFO, (char *)&info) == -1) {
                        if (errno == EAGAIN) {
                                /*
                                 * Target process probably exec'ed a
                                 * setuid program; attempt to re-open
                                 * and try again.
                                 */
                                close(procfd);
                                goto retry;
                        }
			fprintf(stderr, "whodo: ioctl (PIOCPSINFO) failed on %s: %s \n"
				, pname, sys_errlist[errno]);
			close(procfd);
			continue;
		}

		up = findhash(info.pr_pid);
		up->p_ttyd = info.pr_ttydev;
		up->p_state = info.pr_state;
		strncpy(up->p_comm, info.pr_fname, sizeof(info.pr_fname));

		if(up->p_state != NONE && up->p_state != SZOMB && up->p_state != SIDL) { 
			if(ioctl(procfd, PIOCSTATUS, (char *)&statinfo) == -1) {
				fprintf(stderr, "whodo: ioctl (PIOCSTATUS) failed on %s: %s \n"
					, pname, sys_errlist[errno]);
				close(procfd);
				continue;
			}
		/*
		 * compute times, rounding nanoseconds to seconds while avoiding
		 *	overflow
		 */
			up->p_time = statinfo.pr_utime.tv_sec +
				statinfo.pr_stime.tv_sec;
			nsec = statinfo.pr_utime.tv_nsec +
				statinfo.pr_stime.tv_nsec;
			if (nsec >= 1500000000)
				up->p_time += 2;
			else if (nsec >= 500000000)
				up->p_time++;

			up->p_ctime = statinfo.pr_cutime.tv_sec +
				statinfo.pr_cstime.tv_sec;
			nsec = statinfo.pr_cutime.tv_nsec +
				statinfo.pr_cstime.tv_nsec;
			if (nsec >= 1500000000)
				up->p_ctime += 2;
			else if (nsec >= 500000000)
				up->p_ctime++;

			if(ioctl(procfd, PIOCACTION, actinfo) == -1) {
				fprintf(stderr, "whodo: ioctl (PIOCACTION) failed on %s: %s \n"
					, pname, sys_errlist[errno]);
				close(procfd);
				continue;
			}
			up->p_igintr = 
		    		actinfo[SIGINT-1].sa_handler == SIG_IGN &&
		    		actinfo[SIGQUIT-1].sa_handler == SIG_IGN;

			up->p_args[0] = 0;

			/*
			 * Process args if there's a chance we'll print it.
			 */
			if (lflag) { /* w command needs args */
		    		clnarglist(info.pr_psargs);
		    		strcpy(up->p_args, info.pr_psargs);
		    		if (up->p_args[0] == 0 ||
				up->p_args[0] == '-' && up->p_args[1] <= ' ' ||
				up->p_args[0] == '?') {
					strcat(up->p_args, " (");
					strcat(up->p_args, up->p_comm);
					strcat(up->p_args, ")");
		    		}
			}

		}

		/* link pgrp together in case parents go away 
		 * Pgrp chain is a single linked list originating
		 * from the pgrp leader to its group member. 
 		 */
		if(info.pr_pgrp != info.pr_pid) {	/* not pgrp leader */
			pgrp = findhash(info.pr_pgrp);
			up->p_pgrplink = pgrp->p_pgrplink;
			pgrp->p_pgrplink = up;
		}
		parent = findhash(info.pr_ppid);

		/* if this is the new member, link it in */
		if (parent->p_upid != INITPROCESS) {
			if (parent->p_child) {
				up->p_sibling = parent->p_child;
				up->p_child = 0;
			}
			parent->p_child = up;
		}
		
		close(procfd); 
        }  	/* end while(dp=readdir(dirp)) */

	closedir(dirp);

        /**
         * loop through utmp file, printing process info
         * about each logged in user
	 **/
        for (; ut < (struct utmp *)utmpend; ut++) {
                if (ut->ut_type != USER_PROCESS)
                        continue;
		if (sel_user && strncmp(ut->ut_name, sel_user, NMAX) !=0)
			continue;	/* we're looking for somebody else */
                tm = localtime(&ut->ut_time);
		if (lflag) {	/* -l flag format (w command) */
			/* print login name of the user */
			printf("%-*.*s ", NMAX, NMAX, ut->ut_name);

			/* print tty user is on */
			printf("%-*.*s", LMAX, LMAX, ut->ut_line);
			
			/* print when the user logged in */
			prtat(&ut->ut_time);

        		/* print idle time */
			idle = findidle(ut->ut_line);
			if (idle >= 36 * 60)
				printf("%2ddays ", (idle + 12 * 60) / (24 * 60));
			else
				prttime(idle," ");	
			showtotals(findhash((pid_t)ut->ut_pid));

		} else {	/* standard whodo format */
                	printf("\n%-12.12s %-8.8s %2.1d:%2.2d\n",
			    ut->ut_line, ut->ut_name, tm->tm_hour, tm->tm_min);
			showproc(findhash((pid_t)ut->ut_pid));
		}
        }
		
	
}

/**************************************************
 *	showproc(up)
 *
 *  Used for standard whodo format. 
 *  This is the recursive routine descending the process
 *  tree starting from the given process pointer(up).
 *  It used depth-first search strategy and also marked
 *  each node as printed as it traversed down the tree.
 *  
 ***************************************************/
showproc(up)
register struct uproc	*up;
{
	struct	uproc	*zp;
        char            *getty();

	if(up->p_state == VISITED) /* we already been here */
		return;
	/* print the data for this process */
	if(up->p_state == SZOMB || up->p_state == SIDL)
		printf("    %-12.12s %5ld %4.1ld:%2.2ld %s\n",
			"  ?", up->p_upid, 0L, 0L, "<defunct>");
	else if(up->p_state != NONE) {
		printf("    %-12.12s %5ld %4.1ld:%2.2ld %s\n",
			getty(up->p_ttyd), up->p_upid,
			(up->p_time)/60L, (up->p_time)%60L,
			up->p_comm);
	}
	up->p_state = VISITED;

	/* descend for its children */
	if(up->p_child) {
		showproc(up->p_child);
		for(zp = up->p_child->p_sibling; zp; zp = zp->p_sibling) {
			showproc(zp);
		}  /* end for */
	}

	/* print the pgrp relation */
	if(up->p_pgrplink)
		showproc(up->p_pgrplink);
}


/**************************************************
 * 	showtotals(up)
 *
 *  Used for -l flag (w command) format.
 *  Prints the CPU time for all processes & children,
 *  and the cpu time for interesting process,
 *  and what the user is doing.
 *
 **************************************************/
showtotals(up)
register struct uproc	*up;
{
	jobtime = 0;
        proctime = 0;
	empty = 1;
	curpid = -1;
	strcpy(doing, "-");     /* default act: normally never prints */
	calctotals(up);

	/* print CPU time for all processes & children */
	/* and need to convert clock ticks to seconds first */
	prttime((time_t)(jobtime)," ");	

	/* print cpu time for interesting process */
	/* and need to convert clock ticks to seconds first */
	prttime((time_t)(proctime)," ");

	/* what user is doing, current process */
	printf(" %-.32s\n",doing);
}

/**************************************************
 *	calctotals(up)
 *
 *  Used for -l flag (w command) format.
 *  This recursive routine descends the process
 *  tree starting from the given process pointer(up).
 *  It used depth-first search strategy and also marked
 *  each node as visited as it traversed down the tree.
 *  It calulates the process time for all processes &
 *  children.  It also finds the "interesting" process
 *  and determines its cpu time and command.
 *  
 ***************************************************/
calctotals(up)
register struct uproc	*up;
{
	register struct uproc   *zp;

	if(up->p_state == VISITED)
		return;
	up->p_state = VISITED;
	if (up->p_state == NONE || up->p_state == SZOMB || up->p_state == SIDL)
		return;
	jobtime += up->p_time + up->p_ctime;
	proctime += up->p_time;

	if (empty && !up->p_igintr) {
		empty = 0;
		curpid = -1;
	}

	if (up->p_upid > curpid && (!up->p_igintr || empty)) {
		curpid = up->p_upid;
		strcpy(doing, up->p_args);
	}

	/* descend for its children */
	if(up->p_child) {
		calctotals(up->p_child);
		for(zp = up->p_child->p_sibling; zp; zp = zp->p_sibling) {
			calctotals(zp);
		}  /* end for */
	}
}

/************************************************************
 *	getty(dev)
 *
 *   This routine gives back a corresponding device name
 *   from the device number given. 
 ************************************************************/
char *
getty(dev)
register dev_t dev;
{
        register struct psdata *ps, *ps_end;
	
	ps_end = &psptr[ndevs];
        for(ps = psptr; ps < ps_end; ps++) {
                if(ps->dev == dev)
                        return(ps->device);
        }
        return("  ?  ");
}

/************************************************************
 *	findhash(pid)
 *
 *   Findhash  finds the appropriate entry in the process
 *   hash table (pr_htbl) for the given pid in case that
 *   pid exists on the hash chain. It returns back a pointer
 *   to that uproc structure. If this is a new pid, it allocates
 *   a new node, initializes it, links it into the chain (after
 *   head) and returns a structure pointer.
 *
 ************************************************************/
struct uproc *
findhash(pid)
pid_t pid;
{
	register struct uproc *up, *tp;

	tp = up = &pr_htbl[(int)pid % HSIZE];
	if(up->p_upid == 0) {			/* empty slot */
		up->p_upid = pid;
		up->p_state = NONE;
		up->p_child = up->p_sibling = up->p_pgrplink = up->p_link = 0;
		return(up);
	}
	if(up->p_upid == pid) {			/* found in hash table */
		return(up);
	}
	for( tp = up->p_link; tp; tp = tp->p_link ) {	/* follow chain */
		if(tp->p_upid == pid) {
			return(tp);
		}
	}
	tp = (struct uproc *)malloc(sizeof(*tp));	/* add new node */
	if(!tp) {
		fprintf(stderr, "%s: out of memory!: %s\n",    
			arg0, sys_errlist[errno]);
		exit(1);
	}
	tp->p_upid = pid;
	tp->p_state = NONE;
	tp->p_child = tp->p_sibling = tp->p_pgrplink = (pid_t)0;
	tp->p_link = up->p_link;		/* insert after head */
	up->p_link = tp;
	return(tp);
}

#define	HR	(60 * 60)
#define	DAY	(24 * HR)
#define	MON	(30 * DAY)

/*
 * prttime(tim,tail)
 * prints a time in hours and minutes or minutes and seconds.
 * The character string tail is printed at the end, obvious
 * strings to pass are "", " ", or "am".
 */
prttime(tim, tail)
	time_t tim;
	char *tail;
{

	if (tim >= 60) {
		printf("%3d:", tim/60);
		tim %= 60;
		printf("%02d", tim);
	} else if (tim > 0)
		printf("    %2d", tim);
	else
		printf("      ");
	printf("%s", tail);
}

char *weekday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

 
/* 
 * prtat(time)
 * prints a 12 hour time given a pointer to a time of day 
 */
prtat(time)
	time_t *time;
{
	struct tm *p;
	register int hr, pm;

	p = localtime(time);
	hr = p->tm_hour;
	pm = (hr > 11);
	if (hr > 11)
		hr -= 12;
	if (hr == 0)
		hr = 12;
	if (now - *time <= 18 * HR)
		prttime(((time_t)(hr * 60 + p->tm_min)), pm ? "pm" : "am");
	else if (now - *time <= 7 * DAY)
		printf(" %s%2d%s", weekday[p->tm_wday], hr, pm ? "pm" : "am");
	else
		printf(" %2d%s%2d", p->tm_mday, month[p->tm_mon], p->tm_year);
}
/*
 * findidle(devname)
 * 	find & return number of minutes current tty has been idle 
 */
time_t
findidle(devname)
char	*devname;
{
	struct stat stbuf;
	time_t lastaction, diff;
	char ttyname[20];

	strcpy(ttyname, "/dev/");
	strcat(ttyname, devname);
	stat(ttyname, &stbuf);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return(diff);
}


/*
 * clnarglist: given pointer to the argument string clean out
 * "unsavory" characters.
 */
clnarglist(arglist)
char	*arglist;
{
	register char	*c;
	register int 	err = 0;

	/* get rid of unsavory characters */
	for (c = arglist;*c == NULL; c++) {
		if ((*c < ' ') || (*c > 0176)) {
			if (err++ > 5) {
				*arglist = NULL;
				break;
			}
			*c = '?';
		}
	}
}
