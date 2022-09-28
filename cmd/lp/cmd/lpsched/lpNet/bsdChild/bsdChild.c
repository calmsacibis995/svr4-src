/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/bsdChild.c	1.7.3.1"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include "lp.h"
#include "lpNet.h"
#include "lpd.h"
#include "oam_def.h"
#include "errorMgmt.h"
#include "debug.h"

char	 	 Buf[BUFSIZ];	/* general use buffer */
char		*Netbuf;	/* buffer for reading network messages */

extern MESG	*lp_Md;

#if defined (__STDC__)
static	int	getUsrReqs(char *);
static	void	Execlisten(void);
static	void	Netlisten(void);
static	void	lpdNetExit(void);
static	void	lpdNetInit(void);
static	void	s_send_job(char *);
#else
static	int	getUsrReqs();
static	void	Execlisten();
static	void	Netlisten();
static	void	lpdNetExit();
static	void	lpdNetInit();
static	void	s_send_job();
#endif

/*
 * Main entry point
 */
void
#if defined (__STDC__)
bsdChild(void)
#else
bsdChild()
#endif
{
		char	msgbuf [MSGMAX];
		short	status;
	static	char	FnName [] = "bsdChild";

	ENTRYP
	lpdNetInit();

	if (ProcessInfo.processRank == MasterChild)
	{
		if (mread (ProcessInfo.lpExecMsg_p,
		    msgbuf, sizeof (msgbuf)) == -1)
			TrapError (Fatal, Unix, FnName, "mread");

		if (mtype (msgbuf) != S_CHILD_SYNC)
			TrapError (Fatal, Internal, FnName, 
			"Bad message from lpExec.  Expected S_CHILD_SYNC.");

		if (getmessage (msgbuf, S_CHILD_SYNC, &status) == -1)
			TrapError (Fatal, Unix, FnName, "getmessage");

		if (status != MOK)
		{
			fatal ("Child services aborted.");
		}
	}

	if (CONNECTED_TO_REMOTE)
		Netlisten();
	else
		Execlisten();

	lpdNetExit();
	/*NOTREACHED*/
}

static void
#if defined (__STDC__)
lpdNetInit(void)
#else
lpdNetInit()
#endif
{
	struct utsname	utsname;

	DEFINE_FNNAME ("lpdNetInit")
	ENTRYP
	Name = LPDNET;
	logit(LOG_INFO, "%s starting (%s)", Name, CONNECTED_TO_REMOTE ? 
								"passive" : 
								"active"  );
	(void)umask(0);
	Lhost = ProcessInfo.systemName_p;
	Rhost = SIP->systemName_p;
	if (CONNECTED_TO_REMOTE) {
		/*
		 * The following code assumes that neither stdout nor stderr
		 * are in use (stderr may be required by lp_fatal()).
		 */

		 /*** stdout should really be line-buffered;		***/
		 /*** however, EMD seems unable to cope with this.	***/
		 /*** Using IOFBF (along with a sprinkling of fflushes)	***/
		 /*** should help to avoid the problem for now.		***/

		setvbuf(stdout, NULL, _IOFBF, 0);	/* network is stdout */
		(void)dup2(CIP->fd, fileno(stdout));
		setvbuf(stderr, NULL, _IOLBF, 0);	/* network is stderr */
		(void)dup2(CIP->fd, fileno(stderr));
		if (CIP->fd != fileno(stdout) && CIP->fd != fileno(stderr)) {
			t_close(CIP->fd);
			CIP->fd = fileno(stdout);
		}
		if (ioctl(CIP->fd, I_PUSH, "tirdwr") < 0) {
			/* could use t_snd() to notify remote of trouble */
			logit(LOG_ERR, "%s can't PUSH tirdwr: %s", Name, PERROR);
			DisconnectSystem(CIP);
			done(2);
			/*NOTREACHED*/
		} 
		/* 
		 * I guess it's OK to send messages back to lpd now even
		 * though lpd may be trying to send a job our way causing
		 * the messages to fall on deaf ears. 
		 * Any non-NULL byte recieved by lpd at this point (while
		 * it is trying to send a job) will cause it to think that
		 * the queue is disabled.  It (lpd) will then close the
		 * connection and try back in ~5 minutes.
		 */
		if ((Netbuf = (char *)malloc(BUFSIZ)) == NULL) {
			logit(LOG_ERR, "%s can't malloc(BUFSIZ): %s", 
					Name, PERROR);
			done(2);
			/*NOTREACHED*/
		}
		if (mopen() == -1) {		/* sets lp_Md */
			logit(LOG_ERR, "%s can't open pipe to lpsched: %s", 
					Name, PERROR);
			lp_fatal(E_LP_MOPEN);
			/*NOTREACHED*/
		}
	} else
		lp_Md = ProcessInfo.lpExecMsg_p;
}

static void
#if defined (__STDC__)
lpdNetExit(void)
#else
lpdNetExit()
#endif
{
	DEFINE_FNNAME ("lpdNetExit")
	ENTRYP
	done(0);
	/*NOTREACHED*/
}

char		*Person;		/* name of person doing lprm */
static char 	*cmdnames[] = {
	"",
	"printjob",
	"recvjob",
	"displayq short",
	"displayq long",
	"rmjob"
};

/* 
 * Process network requests.
 * All service routines will either return or exit via done().
 */
static void
#if defined (__STDC__)
Netlisten(void)
#else
Netlisten()
#endif
{
	register char	*cp;
	register int	 n;

	DEFINE_FNNAME ("Netlisten")
	ENTRYP
	if (contimeout())
	{
		TRACEP ("Timed-out.")
		done(1);
		/*NOTREACHED*/
	}
	cp = Netbuf;
	do {
		if (cp >= &Netbuf[BUFSIZ]) {
			fatal("Command line too long");
			/*NOTREACHED*/
		}
		if ((n = read(CIP->fd, cp, 1)) != 1) {
			if (n < 0)
			{
				TRACEP ("Lost connection.")
				fatal("Lost connection");
				/*NOTREACHED*/
			}
			EXITP
			return;
		}
	} while (*cp++ != '\n');
	*--cp = '\0';
	cp = Netbuf;
	if (LPD_PROTO_MSG(*cp))
		logit(LOG_INFO, "%s requests %s %s", Rhost, cmdnames[*cp], cp+1);
	else
		logit(LOG_INFO, "bad request (%d) from %s", *cp, Rhost);


	switch (*cp++) {

	case PRINTJOB:	/* check the queue and print any jobs there */
		TRACEP ("case PRINTJOB")
		Printer = cp;
		printjob();
		break;

	case RECVJOB:	/* receive files to be queued */
		TRACEP ("case RECVJOB")
		Printer = cp;
		recvjob();
		break;

	case DISPLAYQS:	/* display the queue (short form) */
	case DISPLAYQL:	/* display the queue (long form) */
		TRACEP ("case DISPLAYQ[SL]")
		Printer = cp;
		getUsrReqs(cp);		/* also terminates Printer */
		displayq(Netbuf[0] - '\3');
		break;

	case RMJOB:	/* remove a job from the queue */
		TRACEP ("case RMJOB")
		Printer = cp;
		if (cp = strchr(cp, ' '))
			*cp = NULL;
		else
			break;
		Person = ++cp;
		getUsrReqs(cp);		/* also terminates Person */
		rmjob();
		break;

	default:
		fatal("Illegal service request");
		/*NOTREACHED*/
	}
	alarm(0);
}

char	*User[MAXUSERS];	/* users to process */
int	 Nusers;		/* # of users in user array */
char	*Request[MAXREQUESTS];	/* job number of spool entries */
int	 Nrequests;		/* # of spool requests */

/*
 * Parse out user names and jobids
 */
static int
#if defined (__STDC__)
getUsrReqs(register char *cp)
#else
getUsrReqs(cp)
register char	*cp;
#endif
{
	register int	n;

	while (*cp) {
		if (*cp != ' ') {
			cp++;
			continue;
		}
		*cp++ = '\0';
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		if (isdigit(*cp)) {
			if (Nrequests >= MAXREQUESTS) {
				fatal("Too many requests");
				/*NOTREACHED*/
			}
			Request[Nrequests++] = cp;
		} else {
			if (Nusers >= MAXUSERS) {
				fatal("Too many users");
				/*NOTREACHED*/
			}
			User[Nusers++] = cp;
		}
	}
	for (n = Nrequests; n--; )	/* convert jobids to request-ids */
		Request[n] = mkreqid(Printer, Request[n]);
	return(1);
}

/*
 * Process lpExec requests.
 */
static void
#if defined (__STDC__)
Execlisten(void)
#else
Execlisten()
#endif
{
	struct pollfd 	 pollfd;
	int		 timeout;
	int		 n;

	DEFINE_FNNAME ("Netlisten")
	ENTRYP
	TRACEd (ProcessInfo.lpExec)
	pollfd.fd = ProcessInfo.lpExec;
	pollfd.events = POLLIN;
	if ((timeout = SIP->timeout) > 0)
		timeout *= 60000;		/* sec to msec */
	for(;;) {
		if ((n = poll(&pollfd, 1, timeout)) == -1)
			continue;
		if (!n) {
			logit(LOG_INFO, "%s timing-out", Name);
			EXITP
			return;			/* timed-out */
		}

		switch (pollfd.revents) {
		case 0:		/* shouldn't happen */
			break;

		case POLLIN:
			if ((n = mread(lp_Md, Msg, MSGMAX)) <= 0) {
				logit(LOG_WARNING, "mread() returned %d", n);
				break;
			}
			switch (n = getmessage(Msg, I_GET_TYPE)) {
			case S_SHUTDOWN:
				logit(LOG_INFO, "%s shutting down", Name);
				(void)mdisconnect(lp_Md);
				lp_Md = NULL;
				EXITP
				return;

			case S_SEND_JOB:
				s_send_job(Msg);
				break;

			default:	/* should we just return? */
				logit(LOG_ERR, "unexpected message: %d", n);
				break;
			}
			break;

		default:		/* something amiss */
			logit(LOG_INFO, "%s received%s%s on lpexec pipe", Name,
				pollfd.revents & POLLHUP ? " POLLHUP" : "",
				pollfd.revents & POLLERR ? " POLLERR" : "");
			EXITP
			return;
		}
	}
}

/*
 * Process S_SEND_JOB request.
 * All service routines will return NULL in the event of an error;
 * otherwise, they return a response message.
 */
static void
#if defined (__STDC__)
s_send_job(char *msg)
#else
s_send_job(msg)
char	*msg;
#endif
{
	char		*sysname, *rqfname;
	short		 ftype, msgsize;
	short		 status;
	int		 type;
 	int		 i;

	if ((type = getmessage(msg, S_SEND_JOB, &sysname,
				 		&ftype,
						&rqfname,
						&msgsize,
						&msg)) >= 0 && openRemote()) {
		logit(LOG_DEBUG, "S_SEND_JOB(ftype=%d, request=\"%s\")", 
								ftype, rqfname);
		if (!STREQU(Rhost, sysname)) {
			logit(LOG_ERR, "s_send_job: wrong system: %s", sysname);
			done(2);
			/*NOTREACHED*/
		}
		if (contimeout()) {
			msg = NULL;
			goto out;
		}
		switch (type = getmessage(msg, I_GET_TYPE)) {

		case S_PRINT_REQUEST:
 			i = 0;
 			alarm(0);  /* turn off timer for print requests? */
 			while (!(msg = s_print_request(msg))) {
 				if (SIP->retry < 0)
 					break;
 				closeRemote();
 				if (++i == 2)
 					logit(LOG_INFO,
				"waiting for remote queue to be enabled");
 				if (SIP->retry > 0)
 					(void)sleep(SIP->retry*60);
 				if (!openRemote())
 					break;
 			}
			break;

		case S_CANCEL:
			msg = s_cancel(msg);
			break;

		case S_GET_STATUS:
			msg = s_get_status(msg);
			break;

		default:
			logit(LOG_ERR, "s_send_job: bad type = %d", type);
			done(2);
			/*NOTREACHED*/
		}
		alarm(0);
	} else {
		if (type < 0)
			logit(LOG_ERR, "badly formed S_SEND_JOB message");
		msg = NULL;
	}

out:
	r_send_job(msg ? MOK : MTRANSMITERR, msg);
	closeRemote();
}

/*
 * Exit routine
 */
void
#if defined (__STDC__)
done(int rc)
#else
done(rc)
int	rc;
#endif
{
	if (lp_Md) {	
		if (ProcessInfo.processRank == MasterChild)
			(void)mputm(lp_Md, S_SHUTDOWN, MOK);
		(void)mdisconnect(lp_Md);
	}
	DisconnectSystem(CIP);
	FreeConnectionInfo(&CIP);
	(void)fclose(stderr);		/* may not actually be open */
	logit(LOG_INFO, "%s exiting, status=%d", Name, rc);
	exit(rc);
	/*NOTREACHED*/
}
