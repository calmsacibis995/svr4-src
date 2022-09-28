/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)syslogd:syslogd.c	1.7.1.1"

/* from "@(#)syslogd.c 1.11 88/02/07 SMI"; from UCB 5.18 2/23/87 */

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * NLOGS   -- the maximum number of simultaneous log files.
 * DEFUPRI -- the default priority for user messages.
 * DEFSPRI -- the default priority for kernel messages.
 * NINLOGS -- the maximum number of inputs we can receive messages from.
 *
 */

#define	NLOGS		20		/* max number of log files */
#define NINLOGS		10		/* max number of inputs */
#define	MAXLINE		1024		/* maximum line length */
#define DEFUPRI		(LOG_USER|LOG_INFO)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define MARKCOUNT	/*10*/3		/* ratio of minor to major marks */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <netconfig.h>
#include <netdir.h>
#include <tiuser.h>
#include <utmp.h>

#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/stropts.h>
#include <sys/syslog.h>
#include <sys/strlog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/poll.h>
#include <sys/wait.h>

char	*LogName = "/dev/log";
char	*ConfFile = "/etc/syslog.conf";
char	*PidFile = "/etc/syslog.pid";
char	ctty[] = "/dev/syscon";

#define	dprintf		if (Debug) (void) printf

#define UNAMESZ		8	/* length of a login name */
#define UDEVSZ		12	/* length of a login device name */
#define MAXUNAMES	20	/* maximum number of user names */

#define NOPRI		0x10	/* the "no priority" priority */
#define	LOG_MARK	(LOG_NFACILITIES << 3)	/* mark "facility" */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define NOCOPY		0x004	/* don't suppress duplicate messages */
#define ADDDATE		0x008	/* add a date to the message */
#define MARK		0x010	/* this message is a mark */

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	time_t	f_time;			/* time this was last written */
	u_char	f_pmask[LOG_NFACILITIES+1];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][SYS_NMLN];
		struct {
			char	f_hname[SYS_NMLN];
			struct netbuf	f_addr;
		} f_forw;		/* forwarding address */
		char	f_fname[MAXPATHLEN];
	} f_un;
};

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */

char	*TypeNames[7] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL"
};

struct filed	Files[NLOGS];

/* simple hash table cache for host names */

struct hashent {
	struct hashent *next;
	unsigned char *addr;
	unsigned int len;
	char name[SYS_NMLN];
};

struct hashtab {
	struct hashent *list;
	unsigned char idx;
};

#define TABSZ	7

struct hashtab Table[TABSZ][256];	/* idx 0 unused */

int	Debug;			/* debug flag */
char	LocalHostName[SYS_NMLN];	/* our hostname */
char	*LocalDomain;		/* our local domain name */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	LogPort;		/* port number for INET connections */
char	PrevLine[MAXLINE + 1];	/* copy of last line to supress repeats */
char	PrevHost[SYS_NMLN];	/* previous host */
int	PrevFlags;
int	PrevPri;
int	PrevCount = 0;		/* number of times seen */
int	FlushTimer;		/* timer for flushing messages */
int	Initialized = 0;	/* set when we have initialized ourselves */
int	MarkInterval = 20;	/* interval between marks in minutes */
int	Marking = 0;		/* non-zero if marking some file */
int	MarkTimer;		/* timer for marks */
int	Ninputs = 0;		/* number of inputs */

struct pollfd Pfd[NINLOGS];
struct netbuf *Myaddrs[NINLOGS];
struct t_unitdata *Udp[NINLOGS];
struct t_uderr *Errp[NINLOGS];

void usage(), untty(), printsys(), printline(), getnets(), init();
void logmsg(), wallmsg(), reapchild(), doalarm(), flushmsg(), logerror();
void die(), cfline(), add();

extern	int errno, sys_nerr;
extern	char *sys_errlist[];
extern	int t_errno, t_nerr;
extern	char *t_errlist[];
extern	char *optarg;
extern	char *ctime();
extern	char *malloc();
extern	time_t time();
extern	struct netconfig *getnetconfig();

#define bzero(ADDR, SIZE)	memset((ADDR), 0, (SIZE))
#define bcopy(FROM, TO, SIZE)	memcpy((TO), (FROM), (SIZE))

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	int funix;
	FILE *fp;
	struct utsname *up;
	struct strioctl str;
	char line[MAXLINE + 1];

	while ((i = getopt(argc, argv, "f:dp:m:")) != -1) {
		switch (i) {
		case 'f':		/* configuration file */
			ConfFile = optarg;
			break;

		case 'd':		/* debug */
			Debug++;
			break;

		case 'p':		/* path */
			LogName = optarg;
			break;

		case 'm':		/* mark interval */
			MarkInterval = atoi(optarg);
			break;

		default:
			usage();
		}
	}

	if (!Debug) {
		if (fork())
			exit(0);
		for (i = 0; i < 10; i++)
			(void) close(i);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		untty();
	}
	up = (struct utsname *)malloc(sizeof(struct utsname));
	uname(up);
	strncpy(LocalHostName, up->nodename, SYS_NMLN);
	(void) free(up);
	LocalDomain = "";
	(void) signal(SIGTERM, die);
	(void) signal(SIGINT, Debug ? die : SIG_IGN);
	(void) signal(SIGQUIT, Debug ? die : SIG_IGN);
	(void) signal(SIGCHLD, reapchild);
	(void) signal(SIGALRM, doalarm);

	funix = open(LogName, O_RDONLY);
	if (funix < 0) {
		(void) sprintf(line, "cannot open %s", LogName);
		logerror(line);
		dprintf("cannot create %s (%d)\n", LogName, errno);
		die(0);
	}
	str.ic_cmd = I_CONSLOG;
	str.ic_timout = 0;
	str.ic_len = 0;
	str.ic_dp = NULL;
	if (ioctl(funix, I_STR, &str) < 0) {
		logerror("cannot register to log console messages");
		dprintf("cannot register to log console messages (%d)\n" , errno);
		die(0);
	}
	Pfd[Ninputs].fd = funix;
	Pfd[Ninputs].events = POLLIN;
	Ninputs++;
	getnets();

	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		(void) fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}

	dprintf("off & running....\n");

	init();
	(void) signal(SIGHUP, init);

	for (;;) {
		int nfds;
		struct strbuf ctl;
		struct strbuf dat;
		int flags = 0;
		struct log_ctl hdr;
		struct t_unitdata *udp;
		struct t_uderr *errp;
		char buf[MAXLINE+1];

		errno = 0;
		t_errno = 0;
		nfds = poll(Pfd, Ninputs, -1);
		dprintf("got a message (%d, %#x)\n", nfds, Ninputs);
		if (nfds == 0)
			continue;
		if (nfds < 0) {
			if (errno != EINTR)
				logerror("poll");
			continue;
		}
		if (Pfd[0].revents & POLLIN) {
			dat.maxlen = MAXLINE;
			dat.buf = buf;
			ctl.maxlen = sizeof(struct log_ctl);
			ctl.buf = (caddr_t)&hdr;
			i = getmsg(Pfd[0].fd, &ctl, &dat, &flags);
			if (i == 0 && dat.len > 0) {
				buf[dat.len] = '\0';
				printsys(&hdr, buf);
				nfds--;
			} else if (i < 0 && errno != EINTR) {
				logerror("klog");
				(void) close(Pfd[0].fd);
				Pfd[0].fd = -1;
				nfds--;
			}
		} else if (Pfd[0].revents & (POLLNVAL|POLLHUP|POLLERR)) {
				logerror("klog");
				(void) close(Pfd[0].fd);
				Pfd[0].fd = -1;
		}
		i = 1;
		while (nfds > 0 && i < NINLOGS) {
			if (Pfd[i].revents & POLLIN) {
				udp = Udp[i];
				udp->udata.buf = buf;
				udp->udata.maxlen = MAXLINE;
				udp->udata.len = 0;
				flags = 0;
				if (t_rcvudata(Pfd[i].fd, udp, flags) < 0) {
					errp = Errp[i];
					if (t_errno == TLOOK) {
						if (t_rcvuderr(Pfd[i].fd, errp) < 0) {
							logerror("t_rcvuderr");
							t_close(Pfd[i].fd);
							Pfd[i].fd = -1;
						}
					} else {
						logerror("t_rcvudata");
						t_close(Pfd[i].fd);
						Pfd[i].fd = -1;
					}
					nfds--;
					continue;
				}
				nfds--;
				if (udp->udata.len > 0) {
					extern char *cvthname();

					line[udp->udata.len] = '\0';
					printline(cvthname(&udp->addr), &udp->udata);
				}
			} else if (Pfd[i].revents & (POLLNVAL|POLLHUP|POLLERR)) {
				logerror("POLLNVAL|POLLHUP|POLLERR");
				(void) t_close(Pfd[i].fd);
				Pfd[i].fd = -1;
				nfds--;
			}
			i++;
		} 
	}
}

void
usage()
{
	(void) fprintf(stderr,
	    "usage: syslogd [-d] [-mmarkinterval] [-ppath] [-fconffile]\n");
	exit(1);
}

void
untty()
{
	if (!Debug)
		setsid();
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */
void
printline(hname, nbp)
	char *hname;
	struct netbuf *nbp;
{
	register char *p, *q;
	register int i;
	register int c;
	int pri;
	char line[MAXLINE + 1];

	/* test for special codes */
	pri = DEFUPRI;
	p = nbp->buf;
	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
			pri = 10 * pri + (*p - '0');
		if (*p == '>')
			++p;
		if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
			pri = DEFUPRI;
	}

	/* don't allow users to log kernel messages */
	if ((pri & LOG_PRIMASK) == LOG_KERN)
		pri |= LOG_USER;

	q = line;
	i = 0;
	while ((c = *p++ & 0177) != '\0' && c != '\n' && i < MAXLINE) {
		if (iscntrl(c)) {
			*q++ = '^';
			*q++ = c ^ 0100;
			i += 2;
		} else {
			*q++ = c;
			i++;
		}
	}
	*q = '\0';
	logmsg(hname, pri, line, 0);
}

void
printsys(lp, msg)
	struct log_ctl *lp;
	char *msg;
{
	register char *p, *q;
	register int c;
	register int i;
	int flags;
	time_t now;
	char line[MAXLINE + 1];

	(void) time(&now);
	flags = SYNC_FILE;	/* fsync file after write */
	for (p = msg; *p != '\0'; ) {
		(void) sprintf(line, "%.15s unix: ", ctime(&now) + 4);
		q = line + strlen(line);
		i = 0;
		while (*p != '\0' && (c = *p++) != '\n' && i < MAXLINE) {
			*q++ = c;
			i++;
		}
		*q = '\0';
		logmsg(LocalHostName, lp->pri, line, flags);
	}
}

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */
void
logmsg(from, pri, msg, flags)
	char *from;
	int pri;
	char *msg;
	int flags;
{
	register struct filed *f;
	register int l;
	register char *cp;
	int fac, prilev;
	time_t now;
	sigset_t osigs, sigs;
	char *text;
	struct t_unitdata ud;
	char line[MAXLINE*2];	/* watch for overflow */
	char line2[MAXLINE*2];

	dprintf("logmsg: pri %o, flags %x, from %s, msg %s\n", pri, flags,
	    from, msg);

	sigemptyset(&osigs);
	sigemptyset(&sigs);
	sigprocmask(SIG_BLOCK, NULL, &osigs);
	sigs = osigs;
	sigaddset(&sigs, SIGALRM);
	sigaddset(&sigs, SIGHUP);
	sigprocmask(SIG_SETMASK, &sigs, NULL);

	/*
	 * Check to see if msg looks non-standard.
	 */
	if (strlen(msg) < 16 || msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')
		flags |= ADDDATE;

	if (!(flags & NOCOPY)) {
		if (flags & (ADDDATE|MARK))
			flushmsg();
		else if (!strcmp(msg + 16, PrevLine + 16)) {
			/* we found a match, update the time */
			(void) strncpy(PrevLine, msg, 15);
			if (PrevCount == 0) {
				FlushTimer = MarkInterval * 60 / MARKCOUNT;
				setalarm(FlushTimer);
			}
			PrevCount++;
			sigprocmask(SIG_SETMASK, &osigs, NULL);
			return;
		} else {
			/* new line, save it */
			flushmsg();
			(void) strcpy(PrevLine, msg);
			(void) strcpy(PrevHost, from);
			PrevFlags = flags;
			PrevPri = pri;
		}
	}

	(void) time(&now);
	cp = line;
	if (flags & ADDDATE)
		strcpy(cp, ctime(&now) + 4);
	else
		strncpy(line, msg, 15);
	strcat(cp, " ");
	strcat(cp, from);
	strcat(cp, " ");
	text = cp + strlen(cp);
	if (flags & ADDDATE)
		strcat(cp, msg);
	else
		strcat(cp, msg+16);
	strcat(cp, "\r\n");

	/* extract facility and priority level */
	fac = (pri & LOG_FACMASK) >> 3;
	if (flags & MARK)
		fac = LOG_NFACILITIES;
	prilev = pri & LOG_PRIMASK;

	/* log the message to the particular outputs */
	if (!Initialized) {
		int cfd = open(ctty, O_WRONLY);

		if (cfd >= 0) {
			untty();
			strcat(cp, "\r\n");
			(void) write(cfd, cp, strlen(cp));
			(void) close(cfd);
		}
		sigprocmask(SIG_SETMASK, &osigs, NULL);
		return;
	}
	for (f = Files; f < &Files[NLOGS]; f++) {
		/* skip messages that are incorrect priority */
		if (f->f_pmask[fac] < (unsigned)prilev ||
		    f->f_pmask[fac] == NOPRI)
			continue;

		/* don't output marks to recently written files */
		if ((flags & MARK) && (now - f->f_time) < (MarkInterval * 60 / 2))
			continue;

		dprintf("Logging to %s", TypeNames[f->f_type]);
		f->f_time = now;
		errno = 0;
		t_errno = 0;
		switch (f->f_type) {
		case F_UNUSED:
			dprintf("\n");
			break;

		case F_FORW:
			dprintf(" %s\n", f->f_un.f_forw.f_hname);
			(void) sprintf(line2, "<%d>%.15s %s", pri, cp, text);
			l = strlen(line2);
			if (l > MAXLINE)
				l = MAXLINE;
			ud.opt.buf = NULL;
			ud.opt.len = 0;
			ud.udata.buf = line2;
			ud.udata.len = l;
			ud.addr.buf = f->f_un.f_forw.f_addr.buf;
			ud.addr.len = f->f_un.f_forw.f_addr.len;
			if (t_sndudata(f->f_file, &ud) < 0) {
				logerror("t_sndudata");
				(void) t_close(f->f_file);
				f->f_type = F_UNUSED;
			}
			break;

		case F_CONSOLE:
			if (flags & IGN_CONS) {
				dprintf(" (ignored)\n");
				break;
			}
			/* fall through */

		case F_TTY:
		case F_FILE:
			dprintf(" %s\n", f->f_un.f_fname);
			if (f->f_type != F_FILE) {
				strcat(cp, "\r\n");
			} else {
				strcat(cp, "\n");
			}
			if (write(f->f_file, cp, strlen(cp)) < 0) {
				int e = errno;
				(void) close(f->f_file);
				/*
				 * Check for EBADF on TTY's due to vhangup() XXX
				 */
				if (e == EBADF && f->f_type != F_FILE) {
					f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND);
					if (f->f_file < 0) {
						f->f_type = F_UNUSED;
						logerror(f->f_un.f_fname);
					}
					untty();
				} else {
					f->f_type = F_UNUSED;
					errno = e;
					logerror(f->f_un.f_fname);
				}
			} else if (flags & SYNC_FILE)
				(void) fsync(f->f_file);
			break;

		case F_USERS:
		case F_WALL:
			dprintf("\n");
			strcat(cp, "\r\n");
			wallmsg(f, from, cp);
			break;
		}
	}
	sigprocmask(SIG_SETMASK, &osigs, NULL);
}


/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */
void
wallmsg(f, from, msg)
	register struct filed *f;
	char *from;
	register char *msg;
{
	register int i;
	register char *cp;
	int ttyf, len;
	FILE *uf;
	static int reenter = 0;
	struct utmp ut;
	time_t now;
	char dev[100];
	char line[MAXLINE*2];

	if (reenter++)
		return;

	/* open the user login file */
	if ((uf = fopen("/etc/utmp", "r")) == NULL) {
		logerror("/etc/utmp");
		reenter = 0;
		return;
	}

	if (f->f_type == F_WALL) {
		(void) time(&now);
		(void) sprintf(line,
		    "\r\n\7Message from syslogd@%s at %.24s ...\r\n",
		    from, ctime(&now));
		(void) strcat(line, msg+16);
		cp = line;
		len = strlen(line);
	} else {
		cp = msg;
		len = strlen(msg);
	}

	/* scan the user login file */
	while (fread((char *) &ut, sizeof ut, 1, uf) == 1) {
		/* is this slot used? */
		if (ut.ut_name[0] == '\0')
			continue;

		/* should we send the message to this user? */
		if (f->f_type == F_USERS) {
			for (i = 0; i < MAXUNAMES; i++) {
				if (!f->f_un.f_uname[i][0]) {
					i = MAXUNAMES;
					break;
				}
				if (strncmp(f->f_un.f_uname[i], ut.ut_name,
				    UNAMESZ) == 0)
					break;
			}
			if (i >= MAXUNAMES)
				continue;
		}

		/* compute the device name */
		strcpy(dev, "/dev/");
		(void) strncat(dev, ut.ut_line, UDEVSZ);

		/*
		 * Might as well fork instead of using nonblocking I/O
		 * and doing notty().
		 */
		if (fork() == 0) {
			(void) signal(SIGALRM, SIG_DFL);
			(void) alarm(30);
			/* open the terminal */
			ttyf = open(dev, O_WRONLY);
			if (ttyf >= 0) {
				struct stat statb;

				if (fstat(ttyf, &statb) == 0 &&
				    (statb.st_mode & S_IWRITE))
					(void) write(ttyf, cp, len);
			}
			exit(0);
		}
	}
	/* close the user login file */
	(void) fclose(uf);
	reenter = 0;
}

void
reapchild()
{
	int status;

	while (waitpid((pid_t)(-1), &status, WNOHANG) > 0)
		;
}

/*
 * Return a printable representation of a host address.
 */
char *
cvthname(nbp)
	register struct netbuf *nbp;
{
	register char *p;
	register struct hashent *h;
	register int len;
	struct nd_hostservlist *hsp;
	struct netconfig *ncp;
	void *handle;

	/*
	 * First look in the hash table, and return if
	 * found.  Otherwise malloc a new entry, chain onto
	 * the end, and do a real host name lookup.  Returns
	 * a pointer to the string part of the table entry.
	 * We keep a hash list because the alternative borders
	 * on the brink of insanity.
	 */
	len = 1;
	p = nbp->buf;
	while (len < TABSZ && len <= nbp->len && Table[len][*p].idx != 0) {
		p++;
	}
	if ((h = Table[len][*p].list) != NULL) {
		for (; h; h = h->next) {
			if (h->len == nbp->len &&
			    same(nbp->buf, h->addr, nbp->len))
				return(h->name);
		}
	}
	h = (struct hashent *)malloc(sizeof(struct hashent));
	if (h == NULL)
		goto dunno;
	h->addr = (unsigned char *)malloc(nbp->len);
	if (h->addr == NULL) {
		free(h);
		goto dunno;
	}
	memcpy(h->addr, nbp->buf, nbp->len);
	h->len = nbp->len;
	*h->name = '\0';
	if ((handle = setnetconfig()) == NULL) {
		free(h->addr);
		free(h);
		goto dunno;
	}
	while ((ncp = getnetconfig(handle)) != NULL) {
		if (ncp->nc_semantics == NC_TPI_CLTS) {
			if (netdir_getbyaddr(ncp, &hsp, nbp) == 0) {
				if (!hsp)
					continue;
				if (hsp->h_cnt > 0) {
					/* ignore cnt > 1 */
					strcpy(h->name, hsp->h_hostservs->h_host);
					break;
				}
			}
		}
	}
	endnetconfig(handle);
	if (*h->name == '\0') {
		free(h->addr);
		free(h);
		goto dunno;
	}
	h->next = Table[len][*p].list;
	Table[len][*p].list = h;
	return (h->name);
dunno:
	return("???");
}

int	curalarm;		/* current alarm value */

/*
 * If the alarm is more than "secs" seconds in the future, set it to "secs"
 * seconds.  Adjust any timers by subtracting the time elapsed since the last
 * "alarm" call.
 */
setalarm(secs)
	int secs;
{
	register int alarmval;
	register int elapsed;

	alarmval = alarm((unsigned)0);
	elapsed = curalarm - alarmval;
	dprintf("setalarm: curalarm %d alarmval %d\n", curalarm, alarmval);
	if (PrevCount > 0)
		FlushTimer -= elapsed;
	if (Marking)
		MarkTimer -= elapsed;
	if (secs < alarmval || alarmval == 0)
		curalarm = secs;
	else
		curalarm = alarmval;
	(void) alarm((unsigned)curalarm);
	dprintf("Next alarm in %d seconds\n", curalarm);
}

/*
 * SIGALRM catcher: adjust the timers, call the appropriate timeout routines,
 * and set up the next alarm.
 */
void
doalarm()
{
	dprintf("doalarm: FlushTimer %d MarkTimer %d curalarm %d\n",
	    FlushTimer, MarkTimer, curalarm);
	if (PrevCount > 0) {
		FlushTimer -= curalarm;
		if (FlushTimer <= 0)
			flushmsg();
	}
	if (Marking) {
		MarkTimer -= curalarm;
		if (MarkTimer <= 0) {
			logmsg(LocalHostName, LOG_INFO, "-- MARK --",
			    ADDDATE|MARK);
			MarkTimer = MarkInterval * 60;
		}
	}
	curalarm = 0;
	if (FlushTimer > 0)
		curalarm = FlushTimer;
	if (Marking && MarkTimer > 0
	    && (MarkTimer < curalarm || curalarm == 0))
		curalarm = MarkTimer;
	(void) alarm((unsigned)curalarm);
	dprintf("Next alarm in %d seconds\n", curalarm);
}

void
flushmsg()
{
	FlushTimer = 0;
	if (PrevCount == 0)
		return;
	if (PrevCount > 1)
		(void) sprintf(PrevLine+16, "last message repeated %d times",
		    PrevCount);
	PrevCount = 0;
	logmsg(PrevHost, PrevPri, PrevLine, PrevFlags|NOCOPY);
	PrevLine[0] = '\0';
}

/*
 * Print syslogd errors some place.
 */
void
logerror(type)
	char *type;
{
	char buf[100];

	if (t_errno == 0 || t_errno == TSYSERR) {
		if (errno == 0)
			(void) sprintf(buf, "syslogd: %s", type);
		else if ((unsigned) errno > sys_nerr)
			(void) sprintf(buf, "syslogd: %s: error %d", type, errno);
		else
			(void) sprintf(buf, "syslogd: %s: %s", type, sys_errlist[errno]);
	} else {
		if ((unsigned)t_errno > t_nerr)
			(void) sprintf(buf, "syslogd: %s: t_error %d", type, t_errno);
		else
			(void) sprintf(buf, "syslogd: %s: %s", type, t_errlist[t_errno]);
	}
	errno = 0;
	t_errno = 0;
	dprintf("%s\n", buf);
	logmsg(LocalHostName, LOG_SYSLOG|LOG_ERR, buf, ADDDATE);
}

void
die(sig)
{
	char buf[100];

	if (sig) {
		dprintf("syslogd: going down on signal %d\n", sig);
		flushmsg();
		(void) sprintf(buf, "going down on signal %d", sig);
		errno = 0;
		logerror(buf);
	}
	exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */
void
init()
{
	register int i;
	register FILE *cf;
	register struct filed *f;
	register char *p;
	sigset_t osigs, sigs;
	char cline[BUFSIZ];

	dprintf("init\n");

	/* flush any pending output */
	flushmsg();

	/*
	 *  Close all open log files.
	 */
	Initialized = 0;
	for (f = Files; f < &Files[NLOGS]; f++) {
		switch (f->f_type) {
		case F_FILE:
		case F_TTY:
		case F_FORW:
		case F_CONSOLE:
			(void) close(f->f_file);
			f->f_type = F_UNUSED;
			break;
		}
	}

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
nofile:
		dprintf("cannot open %s\n", ConfFile);
		cfline("*.ERR\t/dev/syscon", 0, &Files[0]);
		cfline("*.PANIC\t*", 0, &Files[1]);
		return;
	}

	/*
	 * Run the configuration file through m4 to handle any ifdefs.
	 */
	(void) fclose(cf);
	(void) sprintf(cline, "echo '%s' | /usr/ccs/bin/m4 - %s",
	    amiloghost() ? "define(LOGHOST, 1)" : "", ConfFile);
	if ((cf = popen(cline, "r")) == NULL) {
		(void) sprintf(cline, "echo '%s' | /usr/bin/m4 - %s",
		    amiloghost() ? "define(LOGHOST, 1)" : "", ConfFile);
		if ((cf = popen(cline, "r")) == NULL) {
			goto nofile;
		}
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
	f = Files;
	i = 0;
	while (fgets(cline, sizeof cline, cf) != NULL && f < &Files[NLOGS]) {
		i++;
		/* check for end-of-section */
		if (cline[0] == '\n' || cline[0] == '#')
			continue;

		/* strip off newline character */
		p = strchr(cline, '\n');
		if (p)
			*p = '\0';
		cfline(cline, i, f++);
	}

	/* close the configuration file */
	(void) fclose(cf);

	Initialized = 1;

	if (Debug) {
		for (f = Files; f < &Files[NLOGS]; f++) {
			for (i = 0; i <= LOG_NFACILITIES; i++)
				if (f->f_pmask[i] == NOPRI)
					(void) printf("X ");
				else
					(void) printf("%d ", f->f_pmask[i]);
			(void) printf("%s: ", TypeNames[f->f_type]);
			switch (f->f_type) {
			case F_FILE:
			case F_TTY:
			case F_CONSOLE:
				(void) printf("%s", f->f_un.f_fname);
				break;

			case F_FORW:
				(void) printf("%s", f->f_un.f_forw.f_hname);
				break;

			case F_USERS:
				for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
					(void) printf("%s, ",
					    f->f_un.f_uname[i]);
				break;
			}
			(void) printf("\n");
		}
	}

	/*
	 * See if marks are to be written to any files.  If so, set up a
	 * timeout for marks.
	 */
	Marking = 0;
	for (f = Files; f < &Files[NLOGS]; f++) {
		if (f->f_type != F_UNUSED
		    && f->f_pmask[LOG_NFACILITIES] != NOPRI)
			Marking = 1;
	}
	if (Marking) {
		sigemptyset(&osigs);
		sigemptyset(&sigs);
		sigaddset(&sigs, SIGALRM);
		sigprocmask(SIG_SETMASK, &sigs, &osigs);
		setalarm(MarkInterval * 60);
		sigprocmask(SIG_SETMASK, &osigs, NULL);
	}
	logmsg(LocalHostName, LOG_SYSLOG|LOG_INFO, "syslogd: restart", ADDDATE);
	dprintf("syslogd: restarted\n");
}

/*
 * Crack a configuration file line
 */

struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	"none",		NOPRI,
	NULL,		-1
};

struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"mark",		LOG_MARK,
	"syslog",	LOG_SYSLOG,
	"lpr",		LOG_LPR,
	"news",		LOG_NEWS,
	"uucp",		LOG_UUCP,
	"cron",		LOG_CRON,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};

void
cfline(line, lineno, f)
	char *line;
	int lineno;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i;
	char *bp;
	int pri;
	struct netbuf *nbp;
	struct netconfig *ncp;
	char buf[MAXLINE];
	char xbuf[200];

	dprintf("cfline(%s)\n", line);

	errno = 0;	/* keep sys_errlist stuff out of logerror messages */

	/* clear out file entry */
	bzero((char *) f, sizeof *f);
	for (i = 0; i <= LOG_NFACILITIES; i++)
		f->f_pmask[i] = NOPRI;

	/* scan through the list of selectors */
	for (p = line; *p && *p != '\t';) {

		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !strchr("\t,;", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (strchr(", ;", *q))
			q++;

		/* decode priority name */
		pri = decode(buf, PriNames);
		if (pri < 0) {
			(void) sprintf(xbuf, "line %d: unknown priority name \"%s\"",
			    lineno,  buf);
			logerror(xbuf);
			return;
		}

		/* scan facilities */
		while (*p && !strchr("\t.;", *p)) {
			for (bp = buf; *p && !strchr("\t,;.", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*')
				for (i = 0; i < LOG_NFACILITIES; i++)
					f->f_pmask[i] = pri;
			else {
				i = decode(buf, FacNames);
				if (i < 0) {
					(void) sprintf(xbuf, "line %d: unknown facility name \"%s\"",
					    lineno, buf);
					logerror(xbuf);
					return;
				}
				f->f_pmask[i >> 3] = pri;
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (*p == '\t' || *p == ' ')
		p++;

	switch (*p)
	{
	case '\0':
		(void) sprintf(xbuf, "line %d: no action part", lineno);
		errno = 0;
		logerror(xbuf);
		break;

	case '@':
		(void) strcpy(f->f_un.f_forw.f_hname, ++p);
		if (gethost(p, &nbp, &ncp) < 0) {
			(void) sprintf(xbuf, "line %d: unknown host %s",
			    lineno, p);
			errno = 0;
			logerror(xbuf);
			break;
		}
		if (ismyaddr(nbp)) {
			(void) sprintf(xbuf,
			    "line %d: host %s is this host - logging loop",
			    lineno, p);
			errno = 0;
			logerror(xbuf);
			break;
		}
		f->f_un.f_forw.f_addr.buf = malloc(nbp->len);
		if (f->f_un.f_forw.f_addr.buf == NULL) {
			logerror("malloc");
			break;
		}
		bcopy(nbp->buf, f->f_un.f_forw.f_addr.buf, nbp->len);
		f->f_un.f_forw.f_addr.len = nbp->len;
		f->f_file = t_open(ncp->nc_device, O_RDWR, NULL);
		if (f->f_file < 0) {
			logerror("t_open");
			break;
		}
		if (t_bind(f->f_file, NULL, NULL) < 0) {
			logerror("t_bind");
			t_close(f->f_file);
			break;
		}
		f->f_type = F_FORW;
		break;

	case '/':
		(void) strcpy(f->f_un.f_fname, p);
		if ((f->f_file = open(p, O_WRONLY|O_APPEND)) < 0) {
			logerror(p);
			break;
		}
		if (isatty(f->f_file)) {
			f->f_type = F_TTY;
			untty();
		}
		else
			f->f_type = F_FILE;
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		f->f_type = F_WALL;
		break;

	default:
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			(void) strncpy(f->f_un.f_uname[i], p, UNAMESZ);
			if ((q - p) > UNAMESZ)
				f->f_un.f_uname[i][UNAMESZ] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
}


/*
 *  Decode a symbolic name to a numeric value
 */
decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);

	return (-1);
}

ismyaddr(nbp)
	register struct netbuf *nbp;
{
	register int i;

	if (nbp == NULL)
		return (0);
	for (i = 1; i < Ninputs; i++) {
		if (nbp->len == Myaddrs[i]->len &&
			same(nbp->buf, Myaddrs[i]->buf, nbp->len))
				return(1);
	}
	return (0);
}

void
getnets()
{
	struct nd_hostserv hs;
	struct netconfig *ncp;
	struct nd_addrlist *nap;
	struct netbuf *nbp;
	int i;
	void *handle;

	hs.h_host = LocalHostName;
	hs.h_serv = "syslog";

	if ((handle = setnetconfig()) == NULL)
		return;
	while ((ncp = getnetconfig(handle)) != NULL) {
		if (ncp->nc_semantics == NC_TPI_CLTS) {
			if (netdir_getbyname(ncp, &hs, &nap) == 0) {
				if (!nap)
					continue;
				nbp = nap->n_addrs;
				for (i = 0; i < nap->n_cnt; i++) {
					add(ncp, nbp);
					nbp++;
				}
			}
		}
	}
	endnetconfig(handle);
}

void
add(ncp, nbp)
	struct netconfig *ncp;
	struct netbuf *nbp;
{
	int fd;
	struct t_bind bind;
	struct t_bind *bound;

	if (Ninputs > NINLOGS)
		return;
	fd = t_open(ncp->nc_device, O_RDONLY, NULL);
	if (fd < 0)
		return;
	bound = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	bind.addr = *nbp;
	bind.qlen = 0;
	if (t_bind(fd, bind, bound) < 0) {
		t_close(fd);
		t_free(bound, T_BIND);
		return;
	}
	if ((bind.addr.len != bound->addr.len) ||
	    !same(bind.addr.buf, bound->addr.buf, bind.addr.len)) {
		t_close(fd);
		t_free(bound, T_BIND);
		return;
	}
	Udp[Ninputs] = (struct t_unitdata *)t_alloc(fd, T_UNITDATA, T_ADDR);
	if (Udp[Ninputs] == NULL) {
		t_close(fd);
		t_free(bound, T_BIND);
		return;
	}
	Errp[Ninputs] = (struct t_uderr *)t_alloc(fd, T_UDERR, T_ADDR);
	if (Errp[Ninputs] == NULL) {
		t_close(fd);
		t_free(Udp[Ninputs], T_UNITDATA);
		t_free(bound, T_BIND);
		return;
	}
	Pfd[Ninputs].fd = fd;
	Pfd[Ninputs].events = POLLIN;
	Myaddrs[Ninputs++] = &bound->addr;
	bound->addr.buf = NULL;
	t_free(bound, T_BIND);
}

int
gethost(p, nbpp, ncpp)
	char *p;
	struct netbuf **nbpp;
	struct netconfig **ncpp;
{
	struct nd_hostserv hs;
	struct netconfig *ncp;
	struct nd_addrlist *nap;
	void *handle;

	hs.h_host = p;
	hs.h_serv = "syslog";

	if ((handle = setnetconfig()) == NULL)
		return(-1);
	while ((ncp = getnetconfig(handle)) != NULL) {
		if (ncp->nc_semantics == NC_TPI_CLTS) {
			if (netdir_getbyname(ncp, &hs, &nap) == 0) {
				if (!nap)
					continue;
				*nbpp = nap->n_addrs;
				*ncpp = ncp;
				endnetconfig(handle);
				return(0);
			}
		}
	}
	endnetconfig(handle);
	return(-1);
}

int
amiloghost()
{
	struct nd_hostserv hs;
	struct netconfig *ncp;
	struct nd_addrlist *nap;
	struct netbuf *nbp;
	int i;
	void *handle;

	hs.h_host = LocalHostName;
	hs.h_serv = "loghost";

	if ((handle = setnetconfig())== NULL)
		return(0);
	while ((ncp = getnetconfig(handle)) != NULL) {
		if (ncp->nc_semantics == NC_TPI_CLTS) {
			if (netdir_getbyname(ncp, &hs, &nap) == 0) {
				if (!nap)
					continue;
				nbp = nap->n_addrs;
				for (i = 0; i < nap->n_cnt; i++) {
					if (ismyaddr(nbp)) {
						endnetconfig(handle);
						return(1);
					}
					nbp++;
				}
			}
		}
	}
	endnetconfig(handle);
	return(0);
}

int
same(a, b, n)
	register char *a;
	register char *b;
	register int n;
{
	if (n <= 0)
		return(0);
	while (n-- > 0)
		if (*a++ != *b++)
			return(0);
	return(1);
}
