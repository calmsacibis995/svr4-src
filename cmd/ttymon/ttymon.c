/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:ttymon.c	1.20.5.1"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stropts.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>

# include "sac.h"
# include "ttymon.h"
# include "tmstruct.h"
# include "tmextern.h"

static	int	Initialized;

extern	int	Retry;
extern	struct	pollfd	*Pollp;
static	void	initialize();
static	void	open_all();
static	int	set_poll();
static	int	check_spawnlimit();
static	int	mod_ttydefs();

void	open_device();

#ifdef MERGE386
long	sc_flag;
#endif

extern	int	check_session();
extern	void	sigalarm();

/*
 * 	ttymon	- a port monitor under SAC
 *		- monitor ports, set terminal modes, baud rate
 *		  and line discipline for the port
 *		- invoke service on port if connection request received
 *		- Usage: ttymon
 *			 ttymon -g [options]
 *			 Valid options are
 *			 -h
 *			 -d device
 *			 -l ttylabel
 *			 -t timeout
 *			 -m modules
 *			 -p prompt
 *
 *		- ttymon without args is invoked by SAC
 *		- ttymon -g is invoked by process that needs to
 *		  have login service on the fly
 */

main(argc, argv)
int	argc;
char	*argv[];
{
	int	nfds;
	extern	char	*lastname();

	if ((argc > 1) || (strcmp(lastname(argv[0]), "getty") == 0)) {
		ttymon_express(argc,argv);
		exit(1);	/*NOTREACHED*/
	}
	/* remember original signal mask and dispositions */
	(void)sigprocmask(SIG_SETMASK, NULL, &Origmask);
	(void)sigaction(SIGINT, &Sigint, NULL);
	(void)sigaction(SIGALRM, &Sigalrm, NULL);
	(void)sigaction(SIGPOLL, &Sigpoll, NULL);
	(void)sigaction(SIGCLD, &Sigcld, NULL);
	(void)sigaction(SIGTERM, &Sigterm, NULL);
#ifdef	DEBUG
	(void)sigaction(SIGUSR1, &Sigusr1, NULL);
	(void)sigaction(SIGUSR2, &Sigusr2, NULL);
#endif
	initialize();

	for (;;) {
		nfds = set_poll(Pollp);
		if (!Reread_flag) {
			if (nfds > 0)
				do_poll(Pollp,nfds);
			else
				(void)pause();
		}
		/*
		 * READDB messages may arrive during poll or pause.
		 * So the flag needs to be checked again.
		 */
		if (Reread_flag) {
			Reread_flag = FALSE;
			re_read();
		}
		while (Retry) {
			Retry = FALSE;
			open_all();
		}
	}
}

static	void
initialize()
{
	struct	pmtab	*tp;
	register struct passwd *pwdp;
	register struct	group	*gp;
	struct	rlimit rlimit;
	extern	struct	rlimit	Rlimit;
	extern	 uid_t	Uucp_uid;
	extern	 gid_t	Tty_gid;
	extern 	 int	setrlimit(), getrlimit();

#ifdef 	DEBUG
	extern	opendebug();
#endif
	Initialized = FALSE;
	/*
	 * get_environ() must be called first, 
	 * otherwise we don't know where the log file is
	 */
	get_environ();
	openlog();
	openpid();
	openpipes();
	setup_PCpipe();

	(void)sprintf(Scratch, "PMTAG:\t\t %s",Tag);
	log(Scratch);
	(void)sprintf(Scratch, "Starting state: %s", 
		(State == PM_ENABLED) ? "enabled" : "disabled");
	log(Scratch);

#ifdef 	DEBUG
	opendebug(FALSE);
	debug("***** ttymon in initialize *****");
	log("debug mode is \t on");
#endif

	catch_signals();

	/* register to receive SIGPOLL when data comes to pmpipe */
	if (ioctl(Pfd, I_SETSIG, S_INPUT) < 0) {
		(void)sprintf(Scratch,"I_SETSIG on pmpipe failed, errno = %d",
					errno);
		logexit(1,Scratch);
	}
	sacpoll(); /* this is needed because there may be data already */
	
	Maxfiles = (int)ulimit(4, 0L);	/* get max number of open files */
	if (Maxfiles < 0) {
		(void)sprintf(Scratch, "ulimit(4,0L) failed, errno = %d",errno);
		logexit(1,Scratch);
	}
	if (getrlimit(RLIMIT_NOFILE, &Rlimit) == -1) {
		(void)sprintf(Scratch, "getrlimit failed, errno = %d",errno);
		logexit(1,Scratch);
	}
	rlimit.rlim_cur = rlimit.rlim_max = Rlimit.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rlimit) == -1) {
		(void)sprintf(Scratch, "setrlimit failed, errno = %d",errno);
		logexit(1,Scratch);
	}
	Maxfiles = rlimit.rlim_cur;
	Maxfds = Maxfiles - FILE_RESERVED;
	(void)sprintf(Scratch, "max open files\t = %d", Maxfiles);
	log(Scratch);
	(void)sprintf(Scratch, "max ports ttymon can monitor = %d", Maxfds);
	log(Scratch);

	read_pmtab();

	/*
	 * setup poll array 
	 * 	- we allocate 10 extra pollfd so that 
	 *   	  we do not have to re-malloc when there is
	 *   	  minor fluctuation in Nentries
	 */
	Npollfd = Nentries + 10;
	if (Npollfd > Maxfds)
		Npollfd = Maxfds;
	if ((Pollp = (struct pollfd *)
		malloc((unsigned)(Npollfd * sizeof(struct pollfd))))
		== (struct pollfd *)NULL) 
		logexit(1, "malloc for Pollp failed");

	(void) mod_ttydefs();	/* just to initialize Mtime */
	if (check_version(TTYDEFS_VERS, TTYDEFS) != 0) {
		logexit(1, "check /etc/ttydefs version failed");
	}
	read_ttydefs(NULL,FALSE);

	/* initialize global variables, Uucp_uid & Tty_gid */
	if ((pwdp = getpwnam(UUCP)) != NULL) 
		Uucp_uid = pwdp->pw_uid;
	if ((gp = getgrnam(TTY)) == NULL) 
		log("no group entry for <tty>, default is used");
	else
		Tty_gid = gp->gr_gid;
	endgrent();
	endpwent();
#ifdef	DEBUG
	(void)sprintf(Scratch,"Uucp_uid = %ld, Tty_gid = %ld",Uucp_uid,Tty_gid);
	debug(Scratch);
#endif

	log("Initialization Completed");

	/* open the devices ttymon monitors */
	Retry = TRUE;
	while (Retry) {
		Retry = FALSE;
		for (tp = PMtab; tp; tp = tp->p_next) {
			if ((tp->p_status > 0) && (tp->p_fd == 0) &&
			  (tp->p_pid == 0) &&
			  (!((State == PM_DISABLED) && 
			  ((tp->p_dmsg == NULL)||(*(tp->p_dmsg) == '\0')) ))) {
				open_device(tp);
				if (tp->p_fd > 0) 
					got_carrier(tp);
			}
		}
	}
	Initialized = TRUE;
}

/*
 *	open_all - open devices in pmtab if the entry is
 *	         - valid, fd = 0, and pid = 0
 */
static void
open_all()
{
	struct	pmtab	*tp;
	int	check_modtime;
	static	void	free_defs();
	sigset_t cset;
	sigset_t tset;

#ifdef	DEBUG
	debug("in open_all");
#endif
	check_modtime = TRUE;

	for (tp = PMtab; tp; tp = tp->p_next) {
		if ((tp->p_status > 0) && (tp->p_fd == 0) && (tp->p_pid == 0)
			&& (!((State == PM_DISABLED) && 
			  ((tp->p_dmsg == NULL)||(*(tp->p_dmsg) == '\0')) ))) {
			/* 
			 * if we have not check modification time and
			 * /etc/ttydefs was modified, need to re-read it
			 */
			if (check_modtime && mod_ttydefs()) {
				check_modtime = FALSE;
				(void)sigprocmask(SIG_SETMASK, NULL, &cset);
				tset = cset;
				(void)sigaddset(&tset, SIGCLD);
				(void)sigprocmask(SIG_SETMASK, &tset, NULL);
				free_defs();
#ifdef	DEBUG
				debug("/etc/ttydefs is modified, re-read it");
#endif
				read_ttydefs(NULL,FALSE);
				(void)sigprocmask(SIG_SETMASK, &cset, NULL);
			}
			open_device(tp);
			if (tp->p_fd > 0) 
				got_carrier(tp);
		}
		else if (((tp->p_status == LOCKED)||(tp->p_status == SESSION))&&
			 (tp->p_fd > 0) && 
			 (!((State == PM_DISABLED) && 
			   ((tp->p_dmsg == NULL)||(*(tp->p_dmsg) == '\0')) ))) {
			if (check_modtime && mod_ttydefs()) {
				check_modtime = FALSE;
				(void)sigprocmask(SIG_SETMASK, NULL, &cset);
				tset = cset;
				(void)sigaddset(&tset, SIGCLD);
				(void)sigprocmask(SIG_SETMASK, &tset, NULL);
				free_defs();
#ifdef	DEBUG
				debug("/etc/ttydefs is modified, re-read it");
#endif
				read_ttydefs(NULL,FALSE);
				(void)sigprocmask(SIG_SETMASK, &cset, NULL);
			}
			tp->p_status = VALID;
			open_device(tp);
			if (tp->p_fd > 0) 
				got_carrier(tp);
		}
	}
}

/*
 *	open_device(pmptr)	- open the device
 *				- check device lock
 *				- change owner of device
 *				- push line disciplines
 *				- set termio
 */

void
open_device(pmptr)
struct	pmtab	*pmptr;
{
	int	fd, tmpfd;
	struct	sigaction	sigact;

#ifdef	DEBUG
	debug("in open_device");
#endif

	if (pmptr->p_status == GETTY) {
		if ((fd = open(pmptr->p_device, O_RDWR)) == -1) {
			(void)sprintf(Scratch,"open (%s) failed, errno = %d",
				pmptr->p_device, errno);
			log(Scratch);
			exit(1);
		}
	}
	else  {
		if (check_spawnlimit(pmptr) == -1) {
			pmptr->p_status = NOTVALID;
			(void)sprintf(Scratch, 
			"service <%s> is respawning too rapidly",pmptr->p_tag);
			log(Scratch);
			return;
		}
		if (pmptr->p_fd > 0) { /* file already open */
			fd = pmptr->p_fd;
			pmptr->p_fd = 0;
		}
		else if ((fd=open(pmptr->p_device,O_RDWR|O_NONBLOCK)) == -1) {
			(void)sprintf(Scratch, "open (%s) failed, errno = %d",
				pmptr->p_device, errno);
			log(Scratch);
			Retry = TRUE;
			return;
		}
		/* set close-on-exec flag */
		if (fcntl(fd, F_SETFD, 1) == -1) {
			(void)sprintf(Scratch, 
				"F_SETFD fcntl failed, errno = %d", errno);
			logexit(1,Scratch);
		}
		if (tm_checklock(fd) != 0) {
			pmptr->p_status = LOCKED;
			(void)close(fd);
			Nlocked++;
			if (Nlocked == 1) {
				sigact.sa_flags = SA_RESETHAND | SA_NODEFER;
				sigact.sa_handler = sigalarm;
				(void)sigemptyset(&sigact.sa_mask);
				(void)sigaction(SIGALRM, &sigact, NULL);
				(void)alarm(ALARMTIME);
			}
			return;
		}
		if (check_session(fd) != 0) {
			if ((Initialized) && (pmptr->p_inservice != SESSION)){
				(void)sprintf(Scratch,
				"Warning -- active session exists on <%s>",
				pmptr->p_device);
				log(Scratch);
			}
			else {  
				/* 
				 * this may happen if a service is running
				 * and ttymon dies and is restarted,
				 * or another process is running on the
				 * port.
				 */
				pmptr->p_status = SESSION;
				pmptr->p_inservice = 0;
				(void)close(fd);
				Nlocked++;
				if (Nlocked == 1) {
					sigact.sa_flags = SA_RESETHAND | SA_NODEFER;
					sigact.sa_handler = sigalarm;
					(void)sigemptyset(&sigact.sa_mask);
					(void)sigaction(SIGALRM, &sigact, NULL);
					(void)alarm(ALARMTIME);
				}
				return;
			}
		}
		pmptr->p_inservice = 0;
	}

	if (pmptr->p_ttyflags & H_FLAG) {
		/* drop DTR */
		(void)hang_up_line(fd);
		/*
		 * After hang_up_line, the stream is in STRHUP state.
		 * We need to do another open to reinitialize streams
		 * then we can close one fd
		 */
		if ((tmpfd=open(pmptr->p_device, O_RDWR|O_NONBLOCK)) == -1 ) {
			(void)sprintf(Scratch,"open (%s) failed, errno = %d",
				pmptr->p_device, errno);
			log(Scratch);
			Retry = TRUE;
			(void)close(fd);
			return;
		}
		(void)close(tmpfd);
	}

#ifdef DEBUG
	(void)sprintf(Scratch,"open_device (%s), fd = %d",pmptr->p_device,fd);
	debug(Scratch);
#endif

	/* Change ownership of the tty line to root/uucp and */
	/* set protections to only allow root/uucp to read the line. */
	if (pmptr->p_ttyflags & B_FLAG) {
		(void)fchown(fd,Uucp_uid,Tty_gid); 
	}
	else
		(void)fchown(fd,ROOTUID,Tty_gid);
	(void)fchmod(fd,0620);

	if ((pmptr->p_modules != NULL)&&(*(pmptr->p_modules) != '\0')) {
		if (push_linedisc(fd,pmptr->p_modules,pmptr->p_device) == -1) {
			Retry = TRUE;
			(void)close(fd);
			return; 
		}
	}

	if (initial_termio(fd, pmptr) == -1)  {
		Retry = TRUE;
		(void)close(fd);
		return;
	}

	pmptr->p_fd = fd;
}

/*
 *	set_poll(fdp)	- put all fd's in a pollfd array
 *			- set poll event to POLLIN and POLLMSG
 *			- return number of fd to be polled
 */

static	int
set_poll(fdp)
struct pollfd *fdp;
{
	struct	pmtab	*tp;
	int 	nfd = 0;

	for (tp = PMtab; tp; tp = tp->p_next) {
		if (tp->p_fd > 0)  {
			fdp->fd = tp->p_fd;
			fdp->events = POLLIN;
			fdp++;
			nfd++;
		}
	}
	return(nfd);
}

/*
 *	check_spawnlimit	- return 0 if spawnlimit is not reached
 *				- otherwise return -1
 */
static	int
check_spawnlimit(pmptr)
struct	pmtab	*pmptr;
{
	long	now;
	extern	time_t	time();

	(void)time(&now);
	if (pmptr->p_time == 0L)
		pmptr->p_time = now;
	if (pmptr->p_respawn >= SPAWN_LIMIT) {
		if ((now - pmptr->p_time) < SPAWN_INTERVAL) {
			pmptr->p_time = now;
			pmptr->p_respawn = 0;
			return(-1);
		}
		pmptr->p_time = now;
		pmptr->p_respawn = 0;
	}
	pmptr->p_respawn++;
	return(0);
}

/*
 * mod_ttydefs	- to check if /etc/ttydefs has been modified
 *		- return TRUE if file modified
 *		- otherwise, return FALSE
 */
static	int
mod_ttydefs()
{
	struct	stat	statbuf;
	extern	long	Mtime;
	if (stat(TTYDEFS, &statbuf) == -1) {
		/* if stat failed, don't bother reread ttydefs */
		return(FALSE);
	}
	if ((long)statbuf.st_mtime != Mtime) {
		Mtime = (long)statbuf.st_mtime;
		return(TRUE);
	}
	return(FALSE);
}

/*
 *	free_defs - free the Gdef table
 */
static	void
free_defs()
{
	int	i;
	struct	Gdef	*tp;
	tp = &Gdef[0];
	for (i=0; i<Ndefs; i++,tp++) {
		free(tp->g_id);
		free(tp->g_iflags);
		free(tp->g_fflags);
		free(tp->g_nextid);
		tp->g_id = NULL;
		tp->g_iflags = NULL;
		tp->g_fflags = NULL;
		tp->g_nextid = NULL;
	}
	Ndefs = 0;
	return;
}

/*
 * struct Gdef *get_speed(ttylabel) 
 *	- search "/etc/ttydefs" for speed and term. specification 
 *	  using "ttylabel". If "ttylabel" is NULL, default
 *	  to DEFAULT
 * arg:	  ttylabel - label/id of speed settings.
 */

struct Gdef *
get_speed(ttylabel)
char	*ttylabel;
{
	register struct Gdef *sp;
	extern   struct Gdef DEFAULT;

	if ((ttylabel != NULL) && (*ttylabel != '\0')) {
		if((sp = find_def(ttylabel)) == NULL) {
			(void)sprintf(Scratch,"unable to find <%s> in \"%s\"",
			    ttylabel,TTYDEFS);
			log(Scratch);
			sp = &DEFAULT; /* use default */
		}
	} else sp = &DEFAULT; /* use default */
	return(sp);
}

/*
 * setup_PCpipe()	- setup the pipe between Parent and Children
 *			- the pipe is used for a tmchild to send its
 *			  pid to inform ttymon that it is about to
 *			  invoke service
 *			- the pipe also serves as a mean for tmchild
 *			  to detect failure of ttymon
 */
void
setup_PCpipe()
{
	int	flag = 0;

	if (pipe(PCpipe) == -1 ) {
		(void)sprintf(Scratch,"pipe() failed, errno = %d", errno);
		logexit(1,Scratch);
	}
	
	/* set close-on-exec flag */
	if (fcntl(PCpipe[0], F_SETFD, 1) == -1) {
		(void)sprintf(Scratch,"F_SETFD fcntl failed, errno = %d",errno);
		logexit(1,Scratch);
	}
	if (fcntl(PCpipe[1], F_SETFD, 1) == -1) {
		(void)sprintf(Scratch,"F_SETFD fcntl failed, errno = %d",errno);
		logexit(1,Scratch);
	}

	/* set O_NONBLOCK flag */
	if (fcntl(PCpipe[0], F_GETFL, flag) == -1) {
		(void)sprintf(Scratch,"F_GETFL failed, errno = %d", errno);
		logexit(1,Scratch);
	}
	flag |= O_NONBLOCK;
	if (fcntl(PCpipe[0], F_SETFL, flag) == -1) {
		(void)sprintf(Scratch,"F_SETFL failed, errno = %d", errno);
		logexit(1,Scratch);
	}

	/* set message discard mode */
	if (ioctl(PCpipe[0], I_SRDOPT, RMSGD) == -1) {
		(void)sprintf(Scratch,"I_SRDOPT RMSGD failed, errno = %d",
				errno);
		logexit(1,Scratch);
	}

	/* register to receive SIGPOLL when data come */
	if (ioctl(PCpipe[0], I_SETSIG, S_INPUT) == -1) {
		(void)sprintf(Scratch,"I_SETSIG S_INPUT failed, errno = %d",
				errno);
		logexit(1,Scratch);
	}

#ifdef 	DEBUG
	(void)sprintf(Scratch,"PCpipe[0]\t = %d", PCpipe[0]);
	log(Scratch);
	(void)sprintf(Scratch,"PCpipe[1]\t = %d", PCpipe[1]);
	log(Scratch);
#endif
}
