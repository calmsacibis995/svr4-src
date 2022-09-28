/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ttymon:tmhandler.c	1.14.8.1"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <termio.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stropts.h>
#include "ttymon.h"
#include "tmstruct.h"
#include "tmextern.h"
#include "sac.h"

extern	int	Retry;
static	struct	pmtab	*find_pid();
static	void	kill_children();

static 	struct	pmtab	*find_fd();
extern  void	sigalarm();
extern	void	tmchild();

/*
 *	fork_tmchild	- fork child on the device
 */
static	void
fork_tmchild(pmptr)
struct	pmtab	*pmptr;
{
	pid_t	pid;
	sigset_t	cset;
	sigset_t	tset;

#ifdef	DEBUG
	debug("in fork_tmchild");
#endif
	pmptr->p_inservice = FALSE;

	/* protect following region from SIGCLD */
	(void)sigprocmask(SIG_SETMASK, NULL, &cset);
	tset = cset;
	(void)sigaddset(&tset, SIGCLD);
	(void)sigprocmask(SIG_SETMASK, &tset, NULL);
	if( (pid=fork()) == 0 ) {
	 	/* The CHILD */
		tmchild(pmptr); 
		/* tmchild should never return */
		(void)sprintf(Scratch,"tmchild for <%s> returns unexpected",
			pmptr->p_device);
		logexit(1, Scratch);
	}
	else if (pid < 0) {
		(void) sprintf(Scratch, "fork failed, errno = %d",errno);
		log(Scratch);
		pmptr->p_status = VALID;
		pmptr->p_pid = 0;
		Retry = TRUE;
	}
	else {
		/*
		 * The PARENT - store pid of child and close the device
		 */
		pmptr->p_pid = pid;
	}
	if (pmptr->p_fd > 0) {
		(void)close(pmptr->p_fd); 
		pmptr->p_fd = 0; 
	}
	(void)sigprocmask(SIG_SETMASK, &cset, NULL);
}

/*
 * got_carrier - carrier is detected on the stream
 *	       - depends on the flags, different action is taken
 *	       - R_FLAG - wait for data
 *	       - C_FLAG - if port is not disabled, fork tmchild
 *	       - A_FLAG - wait for data 
 *	       - otherwise - write out prompt, then wait for data
 */
void
got_carrier(pmptr)
struct	pmtab	*pmptr;
{
	flush_input(pmptr->p_fd);
	if (pmptr->p_ttyflags & R_FLAG) {
		return;
	}
	else if ((pmptr->p_ttyflags & C_FLAG) &&
		(State != PM_DISABLED) &&
		(!(pmptr->p_flags & X_FLAG))) {
		fork_tmchild(pmptr);
	}
	else if (pmptr->p_ttyflags & A_FLAG) {
		return;
	}
	else if (pmptr->p_timeout) {
		fork_tmchild(pmptr);
	}
	else {
		write_prompt(pmptr->p_fd,pmptr,TRUE,TRUE);
	}
}

/*
 * got_data - data is detected on the stream, fork tmchild
 */
static void
got_data(pmptr)
struct	pmtab	*pmptr;
{
	struct	sigaction sigact;

	if (tm_checklock(pmptr->p_fd) != 0) {
		pmptr->p_status = LOCKED;
		(void)close(pmptr->p_fd);
		pmptr->p_fd = 0;
		Nlocked++;
		if (Nlocked == 1) {
			sigact.sa_flags = SA_RESETHAND | SA_NODEFER;
			sigact.sa_handler = sigalarm;
			(void)sigemptyset(&sigact.sa_mask);
			(void)sigaction(SIGALRM, &sigact, NULL);
			(void)alarm(ALARMTIME);
		}
	}
	else 
		fork_tmchild(pmptr);
}
/*
 * got_hup - stream hangup is detected, close the device
 */
static void
got_hup(pmptr)
struct	pmtab	*pmptr;
{
#ifdef	DEBUG
	debug("in got hup");
#endif
	(void)close(pmptr->p_fd);
	pmptr->p_fd = 0;
	pmptr->p_inservice = 0;
	Retry = TRUE;
}


/*
 *	do_poll	- poll device
 *		- if POLLHUP received, close the device
 *		- if POLLIN received, fork tmchild.
 */
void
do_poll(fdp,nfds)
struct 	pollfd *fdp; 
int 	nfds;
{
	int	i,n;
	struct	pmtab	*pmptr;

	n = poll(fdp, (unsigned long)nfds, -1);	/* blocked poll */
#ifdef	DEBUG
	debug("poll return");
#endif
	if (n < 0) {
		if (errno == EINTR)	/* interrupt by signal */
			return;
		(void)sprintf(Scratch, "do_poll: poll failed, errno = %d",
				errno);
		logexit(1,Scratch);
	}
	for (i = 0; (i < nfds)&&(n); i++,fdp++) {
		if (fdp->revents != 0) {
			n--;
			if ((pmptr = find_fd(fdp->fd)) == NULL) {
				(void)sprintf(Scratch,
				"do_poll: cannot find fd %d in pmtab",fdp->fd);
				log(Scratch);
				continue;
			}
			else if (fdp->revents & POLLHUP) {
				got_hup(pmptr);
			}
			else if (fdp->revents & POLLIN) {
#ifdef	DEBUG
				debug("got POLLIN");
#endif
				got_data(pmptr);
			}
		}
	}
}

/*
 *	sigchild	- handler for SIGCLD
 *			- find the pid of dead child
 *			- clean utmp if U_FLAG is set
 */
void
/*ARGSUSED*/
sigchild(n)
int	n;	/* this is declared to make cc happy, but it is not used */
{
	struct	pmtab	*pmptr;
	struct	sigaction	sigact;
	int 	status;
	pid_t 	pid;

#ifdef	DEBUG
	debug("in sigchild");
#endif
	while (1) {

		/* find the process that died */
		pid = waitpid(-1, &status, WNOHANG);	
		if (pid <= 0)
			return;

		if ((pmptr = find_pid(pid)) == NULL) {
#ifdef	DEBUG
			(void)sprintf(Scratch,
				"cannot find dead child (%ld) in pmtab", pid);
			log(Scratch);
#endif
			/*
			 * This may happen if the entry is deleted from pmtab
			 * before the service exits.
			 * We try to cleanup utmp entry
			 */
			cleanut(pid,status);
			continue;
		}

		if (pmptr->p_flags & U_FLAG)
			cleanut(pid,status);

		pmptr->p_status = VALID;
		pmptr->p_fd = 0;
		pmptr->p_pid = 0;
		pmptr->p_inservice = 0;
		Retry = TRUE;
	}
}

/*
 *	sigterm	- handler for SIGTERM
 */
void
sigterm()
{
	logexit(1,"caught SIGTERM");

}

/*
 *	state_change	- this is called when ttymon changes
 *			  its internal state between enabled and disabled
 */
void
state_change()
{
	struct pmtab *pmptr;

#ifdef	DEBUG
	debug("in state_change");
#endif

	/* 
	 * closing PCpipe will cause attached non-service children 
	 * to get SIGPOLL and exit
	 */
	(void)close(PCpipe[0]);
	(void)close(PCpipe[1]);

	/* reopen PCpipe */
	setup_PCpipe();

	/*
	 * also close all open ports so ttymon can start over
	 * with new internal state
	 */
	for (pmptr = PMtab; pmptr; pmptr = pmptr->p_next) {
		if ((pmptr->p_fd > 0) && (pmptr->p_pid == 0)) {
			(void)close(pmptr->p_fd);
			pmptr->p_fd = 0;
		}
	}
	Retry = TRUE;

}

/*
 *	re_read	- reread pmtab
 *		- kill tmchild if entry changed
 */
void
re_read()
{
	extern	struct	pollfd	*Pollp;
	sigset_t	cset;
	sigset_t	tset;

	(void)sigprocmask(SIG_SETMASK, NULL, &cset);
	tset = cset;
	(void)sigaddset(&tset, SIGCLD);
	(void)sigprocmask(SIG_SETMASK, &tset, NULL);
	if (Nlocked > 0) {
		alarm(0);
		Nlocked = 0;
	}
	read_pmtab();
	kill_children();
	(void)sigprocmask(SIG_SETMASK, &cset, NULL);
	purge();

	if (Nentries > Npollfd) {
#ifdef	DEBUG
		debug("Nentries > Npollfd, reallocating pollfds");
#endif
		/* need to malloc more pollfd structure */
		free((char *)Pollp);
		Npollfd = Nentries + 10;
		if (Npollfd > Maxfds)
			Npollfd = Maxfds;
		if ((Pollp = (struct pollfd *)
			malloc((unsigned)(Npollfd * sizeof(struct pollfd))))
			== (struct pollfd *)NULL) 
			logexit(1, "malloc for Pollp failed");
	}
	Retry = TRUE;
}

/*
 *	find_pid(pid)	- find the corresponding pmtab entry for the pid
 */
static	struct pmtab *
find_pid(pid)
pid_t	pid;
{
	struct pmtab *pmptr;

	for (pmptr = PMtab; pmptr; pmptr = pmptr->p_next) {
		if (pmptr->p_pid == pid) {
			return(pmptr);
		}
	}
	return((struct pmtab *)NULL);
}

/*
 *	find_fd(fd)	- find the corresponding pmtab entry for the fd
 */
static struct pmtab *
find_fd(fd)
int	fd;
{
	struct pmtab *pmptr;

	for (pmptr = PMtab; pmptr; pmptr = pmptr->p_next) {
		if (pmptr->p_fd == fd) {
			return(pmptr);
		}
	}
	return((struct pmtab *)NULL);
}

/*
 *	kill_children()	- if the pmtab entry has been changed,
 *			  kill tmchild if it is not in service.
 *			- close the device if there is no tmchild
 */
static	void
kill_children()
{
	struct pmtab *pmptr;
	for (pmptr = PMtab; pmptr; pmptr = pmptr->p_next) {
		if (pmptr->p_status == VALID)
			continue;
		if ((pmptr->p_fd > 0) && (pmptr->p_pid == 0)) {
			(void)close(pmptr->p_fd);
			pmptr->p_fd = 0;
		}
		else if ((pmptr->p_fd == 0) && (pmptr->p_pid > 0)
			&& (pmptr->p_inservice == FALSE)) {
			(void)kill(pmptr->p_pid, SIGTERM);
		}
	}
}

static	void
mark_service(pid)
pid_t	pid;
{
	struct	pmtab	*pmptr;
#ifdef	DEBUG
	debug("in mark_service");
#endif
	if ((pmptr = find_pid(pid)) == NULL) {
		(void)sprintf(Scratch,
			"mark_service: cannot find child (%ld) in pmtab",pid);
		log(Scratch);
		return;
	}
	pmptr->p_inservice = TRUE;
	return;
}

/*
 * read_pid(fd)	- read pid info from PCpipe
 */
static	void
read_pid(fd)
int	fd;
{
	int	ret;
	pid_t	pid;

	for (;;) {
		if ((ret = read(fd,&pid,sizeof(pid))) < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN) 
				return;
			(void)sprintf(Scratch,"read PCpipe failed, errno = %d",
					errno);
			logexit(1, Scratch);
		}
		if (ret == 0)
			return;
		if (ret != sizeof(pid)) {
			(void)sprintf(Scratch,
			"read return size incorrect, ret = %d", ret);
			logexit(1, Scratch);
		}
		mark_service(pid);
	}
}

/*
 * sipoll_catch()	- signal handle of SIGPOLL for ttymon
 *			- it will check both PCpipe and pmpipe
 */
void
sigpoll_catch()
{
	int	ret;
	struct	pollfd	pfd[2];

#ifdef	DEBUG
	debug("in sigpoll_catch");
#endif

	pfd[0].fd = PCpipe[0];
	pfd[1].fd = Pfd;
	pfd[0].events = POLLIN;
	pfd[1].events = POLLIN;
	if ((ret = poll(pfd, 2, 0)) < 0) {
		(void)sprintf(Scratch,"sigpoll_catch: poll failed, errno = %d",
				errno);
		logexit(1, Scratch);
	}
	if (ret > 0) {
		if (pfd[0].revents & POLLIN) 
			read_pid(pfd[0].fd);
		if (pfd[1].revents & POLLIN)
			sacpoll();
	}
}

/*ARGSUSED*/
void
sigalarm(signo)
int	signo;
{
	struct pmtab *pmptr;
	struct sigaction sigact;
	int	fd;
	extern	int	check_session();

#ifdef	DEBUG
	(void)sprintf(Scratch,
	"in sigalarm, Nlocked = %d", Nlocked);
	debug(Scratch);
#endif
	for (pmptr = PMtab; pmptr; pmptr = pmptr->p_next) {
		if ((pmptr->p_status == LOCKED) && (pmptr->p_fd == 0)) {
			if ((fd=open(pmptr->p_device,O_RDWR|O_NONBLOCK)) == -1){
				(void)sprintf(Scratch, 
					"open (%s) failed, errno = %d",
					pmptr->p_device, errno);
				log(Scratch);
				pmptr->p_status = VALID;
				Nlocked--;
				Retry = TRUE;
			}
			else {
				if (tm_checklock(fd) == 0) {
					Nlocked--;
					pmptr->p_fd = fd;
					Retry = TRUE;
				}
				else
					(void)close(fd);
			}
		}
		else if ((pmptr->p_status == SESSION) && (pmptr->p_fd == 0)) {
			if ((fd=open(pmptr->p_device,O_RDWR|O_NONBLOCK)) == -1){
				(void)sprintf(Scratch, 
					"open (%s) failed, errno = %d",
					pmptr->p_device, errno);
				log(Scratch);
				pmptr->p_status = VALID;
				Nlocked--;
				Retry = TRUE;
			}
			else { 
				if (check_session(fd) == 0) {
					Nlocked--;
					pmptr->p_fd = fd;
					Retry = TRUE;
				}
				else
					(void)close(fd);
			}
		}
	}
	if (Nlocked > 0) {
		sigact.sa_flags = SA_RESETHAND | SA_NODEFER;
		sigact.sa_handler = sigalarm;
		(void)sigemptyset(&sigact.sa_mask);
		(void)sigaction(SIGALRM, &sigact, NULL);
		(void)alarm(ALARMTIME);
	}
	else {
		sigact.sa_flags = 0;
		sigact.sa_handler = SIG_IGN;
		(void)sigemptyset(&sigact.sa_mask);
		(void)sigaction(SIGALRM, &sigact, NULL);
	}
}
