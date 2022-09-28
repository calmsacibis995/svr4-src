/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmsac.c	1.10.3.1"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "ttymon.h"	
#include "sac.h" 

extern	char	Scratch[];
extern	void	log();
extern	void	logexit();

/*
 *	openpid	- open the pid and put ttymon's pid in it
 *		- put an advisory lock on the file 
 *		- to prevent another instance of ttymon in same directory
 *		- SAC also makes use of the lock
 *		- fd 0 is reserved for pid file
 */
void
openpid()
{
	extern	int Lckfd;

	(void)close(0);
	/* open for read first, otherwise, may delete the pid already there*/
	if ((Lckfd = open(PIDFILE, O_RDONLY)) != -1) {
	     	if (lockf(Lckfd, F_TEST, 0L) == -1) {
			logexit(1,"pid file is locked. ttymon may already be running!");
		}
		(void)close(Lckfd);
	}
	if ((Lckfd = open(PIDFILE, O_WRONLY|O_CREAT|O_TRUNC, 0644 )) != 0) {
		(void)sprintf(Scratch, "open pid file failed, errno = %d", errno);
		logexit(1,Scratch);
	}
	if (lockf(Lckfd, F_LOCK, 0L) == -1)  {
		(void)sprintf(Scratch, "lock pid file failed, errno = %d", errno);
		logexit(1,Scratch);
	}

	(void)sprintf(Scratch, "%ld", getpid());
	(void)write(Lckfd, Scratch, (unsigned)(strlen(Scratch)+1));
#ifdef	DEBUG
	(void)sprintf(Scratch,"fd(pid)\t = %d",Lckfd);
	log(Scratch);
#endif
}

/*
 * openpipes() -- open pmpipe and sacpipe to communicate with SAC
 *	       -- Pfd, Sfd are global file descriptors for pmpipe, sacpipe
 */

void
openpipes()
{
	extern	int Pfd, Sfd;

	Sfd = open(SACPIPE, O_WRONLY);
	if (Sfd < 0) {
		(void)sprintf(Scratch,"open sacpipe failed, errno = %d", errno);
		logexit(1,Scratch);
	}

	Pfd = open(PMPIPE, O_RDWR|O_NONBLOCK); 
	if (Pfd < 0) {
		(void)sprintf(Scratch, "open pmpipe failed, errno = %d", errno);
		logexit(1,Scratch);
	}
#ifdef	DEBUG
	(void)sprintf(Scratch,"fd(sacpipe)\t = %d",Sfd);
	log(Scratch);
	(void)sprintf(Scratch,"fd(pmpipe)\t = %d",Pfd);
	log(Scratch);
#endif
}

/*
 * remove_env(env) - remove an environment variable from the environment
 */
static	void
remove_env(env)
char	*env;
{
	extern	char	**environ;
	char	**p;
	char	**rp = NULL;

	p = environ;
	if (p == NULL)
		return;
	while (*p) {
		if (strncmp(*p, env,strlen(env)) == 0)
			rp = p;
		p++;
	}
	if (rp) {
		*rp = *--p;
		*p = NULL;
	}
}

/*
 * get_environ() -- get env variables PMTAG, ISTATE
 *		 -- set global variables Tag, State
 */

void
get_environ()
{
	extern 	char State, *Istate, *Tag;

	if ((Tag = getenv("PMTAG")) == NULL)
		logexit(1, "PMTAG is missing"); 

	if ((Istate = getenv("ISTATE")) == NULL) 
		logexit(1, "ISTATE is missing");

	State = (!strcmp(Istate, "enabled")) ? PM_ENABLED : PM_DISABLED;

	/*
	 * remove the environment variables so they will not
	 * be passed to the children
	 */
	remove_env("ISTATE");
	remove_env("PMTAG");
}

/*
 * sacpoll	- the event handler when sac event is posted
 */
void
sacpoll()
{
	int 	ret;
	char	oldState;
	struct 	sacmsg sacmsg;
	struct 	pmmsg pmmsg;
	sigset_t	cset;
	sigset_t	tset;
	extern	char	State, *Tag;
	extern	int Pfd, Sfd;
	extern 	state_change();
	extern 	int Reread_flag;
	extern	void	sigchild();

#ifdef	DEBUG
	debug("in sacpoll");
#endif

	/* we don't want to be interrupted by sigchild now */
	(void)sigprocmask(SIG_SETMASK, NULL, &cset);
	tset = cset;
	(void)sigaddset(&tset, SIGCLD);
	(void)sigprocmask(SIG_SETMASK, &tset, NULL);

	/*
	 *	read sac messages, one at a time until no message
	 *	is left on the pipe.
	 *	the pipe is open with O_NONBLOCK, read will return -1
	 *	and errno = EAGAIN if nothing is on the pipe
	 */
	for (;;) {

		ret = read(Pfd, &sacmsg, sizeof(sacmsg));
		if (ret < 0) {
			switch(errno) {
			case EAGAIN:
				/* no more data on the pipe */
				(void)sigprocmask(SIG_SETMASK, &cset, NULL);
				return;
			case EINTR:
				break;
			default: 
				(void)sprintf(Scratch,
				   "pmpipe read failed, errno = %d", errno);
				logexit(1, Scratch);
				break;  /*NOTREACHED*/
			}
		}
		else if (ret == 0) {
			/* no more data on the pipe */
			(void)sigprocmask(SIG_SETMASK, &cset, NULL);
			return;
		}
		else {
			pmmsg.pm_size = 0;
			(void) strcpy(pmmsg.pm_tag, Tag);
			pmmsg.pm_maxclass = TM_MAXCLASS;
			pmmsg.pm_type = PM_STATUS;
			switch(sacmsg.sc_type) {
			case SC_STATUS:
				break;
			case SC_ENABLE:
				log("Got SC_ENABLE message");
				oldState = State;
				State = PM_ENABLED;
				if (State != oldState) { 
#ifdef	DEBUG
					debug("state changed to ENABLED");
#endif
					state_change();
				}
				break;
			case SC_DISABLE:
				log("Got SC_DISABLE message");
				oldState = State;
				State = PM_DISABLED;
				if (State != oldState) { 
#ifdef	DEBUG
					debug("state changed to DISABLED");
#endif
					state_change();
				}
				break;
			case SC_READDB:
				log("Got SC_READDB message");
				Reread_flag = 1;
				break;
			default:
				log("Got unknown message");
				pmmsg.pm_type = PM_UNKNOWN;
				break;
			} /* end switch */
			pmmsg.pm_state = State;

			while (write(Sfd, &pmmsg, sizeof(pmmsg)) != sizeof(pmmsg)) {
				if (errno == EINTR)
					continue;
				(void)sprintf(Scratch,
				"sanity response to SAC failed, errno = %d",
				errno);
				log(Scratch);
				break;
			}
		}
	} /* end for loop */
}
