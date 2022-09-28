/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:logent.c	2.7.3.1"

#include "uucp.h"

static FILE	*_Lf = NULL;
static int	_Cf = -1;
static int	_Sf = -1;
static int	CurRole = MASTER;	/* Uucico's current role. */

/*
 * Make log entry
 *	text	-> ptr to text string
 *	status	-> ptr to status string
 * Returns:
 *	none
 */
void
logent(text, status)
register char	*text, *status;
{
	static	char	logfile[MAXFULLNAME];

	if (*Rmtname == NULLCHAR) /* ignore logging if Rmtname is not yet set */
		return;
	if (Nstat.t_pid == 0)
		Nstat.t_pid = getpid();

	if (_Lf != NULL
	   && strncmp(Rmtname, BASENAME(logfile, '/'), MAXBASENAME) != 0) {
		fclose(_Lf);
		_Lf = NULL;
	}

	if (_Lf == NULL) {
		sprintf(logfile, "%s/%s", Logfile, Rmtname);
		_Lf = fopen(logfile, "a");
		(void) chmod(logfile, LOGFILEMODE);
		if (_Lf == NULL)
			return;
		setbuf(_Lf, CNULL);
	}
	(void) fseek(_Lf, 0L, 2);
	(void) fprintf(_Lf, "%s %s %s ", User, Rmtname, Jobid);
	(void) fprintf(_Lf, "(%s,%ld,%d) ", timeStamp(), (long) Nstat.t_pid, Seqn);
	(void) fprintf(_Lf, "%s (%s)\n", status, text);
	return;
}


/*
 * Make entry for a conversation (uucico only)
 *	text	-> pointer to message string
 * Returns:
 *	none
 */
void
syslog(text)
register char	*text;
{
	int	sbuflen;
	char	sysbuf[BUFSIZ];

	(void) sprintf(sysbuf, "%s!%s %s (%s) (%c,%ld,%d) [%s] %s\n",
		Rmtname, User, CurRole == SLAVE ? "S" : "M", timeStamp(),
		Pchar, (long) getpid(), Seqn, Dc, text);
	sbuflen = strlen(sysbuf);
	if (_Sf < 0) {
		errno = 0;
		_Sf = open(SYSLOG, 1);
		if (errno == ENOENT) {
			_Sf = creat(SYSLOG, LOGFILEMODE);
			(void) chmod(SYSLOG, LOGFILEMODE);
		}
		if (_Sf < 0)
			return;
	}
	(void) lseek(_Sf, 0L, 2);
	(void) write(_Sf, sysbuf, sbuflen);
	return;
}

/*
 * Make entry for a command 
 *	argc	-> number of command arguments
 *	argv	-> pointer array to command arguments
 * Returns:
 *	none
 */
void
commandlog(argc,argv)
int argc;
char **argv;
{
	char	sysbuf[BUFSIZ];
	int	lastbyte;

	if (_Cf < 0) {
		errno = 0;
		_Cf = open(CMDLOG, O_WRONLY | O_APPEND);
		if (errno == ENOENT) {
			_Cf = creat(CMDLOG, LOGFILEMODE);
			(void) chmod(CMDLOG, LOGFILEMODE);
		}
		if (_Cf < 0)
			return;
	}
	(void) sprintf(sysbuf, "%s (%s) ",User, timeStamp() );
	while (argc-- > 0) {
		lastbyte = strlen(sysbuf);
		(void)	sprintf(sysbuf+lastbyte,"%s%c",*argv++,(argc > 0)?' ':'\n');
	}
	(void) write(_Cf, sysbuf, strlen(sysbuf));
	return;
}

/*
 * Close log files before a fork
 */
void
closelog()
{
	if (_Sf >= 0) {
		(void) close(_Sf);
		_Sf = -1;
	}
	if (_Lf) {
		(void) fclose(_Lf);
		_Lf = NULL;
	}
	if (_Cf >= 0) {
		(void) close(_Cf);
		_Cf = -1;
	}
	return;
}

/*
 *	millitick()
 *
 *	return msec since last time called
 */
#ifdef ATTSV
time_t
millitick()
{
	struct tms	tbuf;
	time_t	now, rval;
	static time_t	past;	/* guaranteed 0 first time called */

	if (past == 0) {
		past = times(&tbuf);
		rval = 0;
	} else {
		rval = ((now = times(&tbuf)) - past) * 1000 / HZ;
		past = now;
	}
	return(rval);
}

#else /* !ATTSV */
time_t
millitick()
{
	struct timeb	tbuf;
	static struct timeb	tbuf1;
	static past;		/* guaranteed 0 first time called */
	time_t	rval;

	if (past == 0) {
		past++;
		rval = 0;
		ftime(&tbuf1);
	} else {
		ftime(&tbuf);
		rval = (tbuf.time - tbuf1.time) * 1000
			+ tbuf.millitm - tbuf1.millitm;
		tbuf1 = tbuf;
	}
	return(rval);
}
#endif /* ATTSV */
