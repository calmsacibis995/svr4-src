/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/log.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)log.c	3.13	LCC);	/* Modified: 18:08:53 8/29/89 */

/****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

#include	<pci_types.h>
#include	<stdio.h>
#include	<varargs.h>
#include	<fcntl.h>
#include	<log.h>
#include	<errno.h>

#ifdef	VERBOSE_LOG
#define	dbgInit		~(DBG_LOG | DBG_RLOG)
#else
#define	dbgInit		0
#endif	/* VERBOSE_LOG */

unsigned long
	dbgEnable = dbgInit;		/* Debug channel enables */

extern char
	*myname;			/* Name of program */

extern FILE *popen();

FILE
	*logFile = NULL;                       /* Log file stream */

char logName[64];			/* Full log file name */
static char savlogName[50];

/*
   In order to create a clean and controlled interface to the logging
   calls, it is necessary to have access to the internal printf interface
   which accepts a pointer to an argument vector, instead of an argument
   list itself.  This is C-library dependant.
*/
#undef	prfKnown

#ifdef	SYS5
#define	prfKnown			/* Sys5 has vfprintf in library */
#ifdef	IX370
#define	vfprintf(file,fmt,aList)        _doprnt(fmt,aList,file)
#endif	/* IX370 */
#endif	/* SYS5 */

#ifdef	DOPRNT41
#define	vfprintf(file, fmt, aList)	_doprnt(fmt, aList, file)
#define	prfKnown
#endif	/* DOPRNT41 */

#ifndef	prfKnown
#include	"log: varargs printf interface unknown"
#endif	/* !prfKnown */


#ifndef NOLOG
/*
   logOpen: Open a log file
*/

void
logOpen(logBase, logPid)
char
	*logBase;			/* Base name of log file to create */
int
	logPid;				/* Pid for name extension */
{
		
	/* If logBase is not a full path, it is prefixed with LOGDIR. */
	/* If logPid is non-zero, it is appended to logName. */
	if (*logBase == '/') {
		if (logPid)
			sprintf(logName, "%s.%d", logBase, logPid);
		else
			strncpy(logName, logBase, 64);
	} else {
		if (logPid)
			sprintf(logName, "%s/%s.%d", LOGDIR, logBase, logPid);
		else
			sprintf(logName, "%s/%s", LOGDIR, logBase);
	}

	strcpy(savlogName, logName);

	/* Close an already existing log file */
	if (logFile != NULL)
		fclose(logFile);

	logFile = fopen(logName, "a");
	if (logFile == NULL) {
		serious("logOpen:error opening %s\n",logName);
	}
	
	/* KLO0168  - begin */
	if (chmod(logName,0600) < 0) {
		serious("logOpen:chmod error on %s, errno %d\n",
			logName,errno);
	}
	/* KLO0168  - end */

	log("logOpen: savlogName = %s\n", savlogName);
}

/* KLO0168 - begin */
logChown(uid,gid)
int uid;
int gid;
{
	if (logFile) {
		if (chown(logName,uid,gid) < 0) {
			serious("logChown:chown error %s %d %d,errno %d\n",
				logName,uid,gid,errno);
		}
	}
}
/* KLO0168 - end */

/*
   logDOpen: Establish a unix file descriptor as the log stream
*/

void
logDOpen(logDesc)
int
	logDesc;
{
	if (logFile != NULL)
		fclose(logFile);

	logFile = fdopen(logDesc, "a");
	if (logFile == NULL) {
		serious("logDOpen:can't fdopen %d\n",logDesc);
	}
}


void
logClose()
{
	if (logFile == NULL)
		return;

	fclose(logFile);
	logFile = (FILE *)NULL;
}


/*
   log: Log message to standard log file
*/

int
log(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */
	register int	prfRet;		/* Return value from printf */
	static int efbig_err = FALSE;
	int tmp;

	/* Only print logs if enabled and logFile is open */
	if (!logsOn())
		return 0;

/*
 *	If the log file is too big but we dont know the log name then
 *	return. This is for the dosout process which doesn't know the 
 *	log name whereas dossvr does, so we'll wait for dossvr to log
 *	and it can truncate the log file.
 *	We do this here because if we continue to write to the file
 *	when it is already to big software signals will get sent to us
 *	and they will interupt the pipe reads done by dosout and everybody
 *	will become unnecessarily confused.
 */
	if (efbig_err == TRUE && savlogName[0] == '\0')
		return(TRUE);

	va_start(args);
	fmt = va_arg(args, char *);
	prfRet = vfprintf(logFile, fmt, args);
	va_end(args);

	/* Flush output immediately */
	if ((fflush(logFile)) == EOF) { /* file to big, truncate it */
		if (errno == EFBIG) {
			efbig_err = TRUE;
			if (savlogName[0] != '\0') { /* make sure we know who */
				logClose();
				do
					tmp = open(savlogName, O_TRUNC);
				while (tmp == -1 && errno == EINTR);
				close(tmp);
				logFile = fopen(savlogName, "a");
				if (logFile == NULL) {
					serious("error re-opening %s errno = %d\n",
							savlogName, errno);
				}
				efbig_err = FALSE;
			}
		}
	}
	return prfRet;
}


/*
   ulog: Log message unconditionally - mostly for debug() macro
*/

int
ulog(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */
	register int	prfRet;		/* Printf return value */

	if (logFile == NULL)
		return(0);		/* need a file to write to */
	va_start(args);
	fmt = va_arg(args, char *);
	prfRet = vfprintf(logFile, fmt, args);
	va_end(args);

	/* Force output to log file */
	fflush(logFile);
	return prfRet;
}

/*
   tlog: Log message - used by Vlog() macro
	use first format when DBG_VLOG is on (verbose logs),
	else if DBG_LOG is on (normal logs) use second format.
*/

int
tlog(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt1;			/* printf verbose format string */
	char	*fmt2;			/* printf terse format string */
	register int	prfRet;		/* Printf return value */

	/* Only print when normal or verbose logs are enabled */
	/* and logFile is open */
	if ( !dbgCheck(DBG_LOG | DBG_VLOG) || logFile == NULL)
		return 0;

	va_start(args);
	fmt1 = va_arg(args, char *);
	fmt2 = va_arg(args, char *);
	if (dbgCheck(DBG_VLOG))
		prfRet = vfprintf(logFile, fmt1, args);
	else
		prfRet = vfprintf(logFile, fmt2, args);
	va_end(args);

	/* Force output to log file */
	fflush(logFile);
	return prfRet;
}


/*
   logv: Log call with argument vector instead of argument list
*/

void
logv(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */

	/* If remote logs disabled, or no log file open, return immediately */
	if (!(dbgEnable & DBG_RLOG) || logFile == NULL)
		return;

	va_start(args);
	fmt = va_arg(args, char *);
	/* Format and print message */
	vfprintf(logFile, fmt, args);
	va_end(args);

	/* Flush output immediately */
	fflush(logFile);
}
#endif /* ~NOLOG */


/*
   serious: Log serious error message and exit
*/

serious(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */
	FILE	*seriousFile;		/* Stream to error logger */

	popenSafe();

	if ((seriousFile = popen(ERR_LOGGER, "w")) == NULL) {
		log("serious: Cannot create logger `%s' (%d)\n",
			ERR_LOGGER, errno);
		seriousFile = fopen("/dev/console","w");
		fprintf(seriousFile, "Serious error in %s %d:\n",
			myname, getpid());
		va_start(args);
		fmt = va_arg(args, char *);
		vfprintf(seriousFile, fmt, args);
		va_end(args);
		fclose(seriousFile);
	} else {
		fprintf(seriousFile, "Serious error in %s %d:\n",
			myname, getpid());
		va_start(args);
		fmt = va_arg(args, char *);
		vfprintf(seriousFile, fmt, args);
		va_end(args);
		pclose(seriousFile);
	}
}


/*
   fatal: Log fatal error message and exit
*/

void
fatal(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */
	FILE	*fatalFile;		/* Stream to error logger */

	popenSafe();

	if ((fatalFile = popen(ERR_LOGGER, "w")) == NULL) {
		log("fatal: Cannot create logger `%s' (%d)\n",
			ERR_LOGGER, errno);
		fatalFile = fopen("/dev/console","w");
		fprintf(fatalFile, "Fatal error in %s %d:\n",
			myname, getpid());
		va_start(args);
		fmt = va_arg(args, char *);
		vfprintf(fatalFile, fmt, args);
		va_end(args);
		fclose(fatalFile);
	} else {
		fprintf(fatalFile, "Fatal error in %s %d:\n",
			myname, getpid());
		va_start(args);
		fmt = va_arg(args, char *);
		vfprintf(fatalFile, fmt, args);
		va_end(args);
		pclose(fatalFile);
	}

	exit(3);
}


/*
   popenSafe: Make things safe for a popen
*/

popenSafe()
{
int
	dummyDesc;			/* Dummy variable for descriptors */

	/*
	   "Making things safe for popen" means making sure that file
	   descriptors 0, 1 and 2 are open before popen is called.
	   If any of them are not, they are opened to /dev/null before
	   popen is called.  The problem is that popen does some file
	   descriptor manipulations that assume descriptors 0 and 1
	   are open before they begin.  If this assumption is not met,
	   popen clobbers it's own pipe file descriptors.
	*/
	do
		dummyDesc = open("/dev/null", O_RDONLY);
	while (dummyDesc == -1 && errno == EINTR);

	if (dummyDesc > 2)
		close(dummyDesc);
	else
		while (dummyDesc < 3)
			do
				dummyDesc = fcntl(dummyDesc, F_DUPFD, 0);
			while (dummyDesc == -1 && errno == EINTR);
}


#ifndef NOLOG
/*
   newLogs: Set new log bits from channel file in /tmp - called on receipt
	of logging control signal
*/

long
newLogs(logName, namePid, childEnable, dbg)
char
	*logName;				/* Name of log file */
int
	namePid;				/* Process id for unique name */
long
	*childEnable;				/* Use these channel bits */
struct
	dbg_struct *dbg ;			/* Used for control from pc */
{
char
	chanName[32];				/* Name of debug channel file */
int
	chanDesc;				/* Descriptor of channel file */
long
	logChanges,				/* Which changes to make */
	setChans,				/* New absolute channel set */
	onChans,				/* Turn these channels on */
	offChans,				/* Turn these off */
	flipChans;				/* Invert these */

	/* Get name of channel file */
	sprintf(chanName, chanPat, getpid());

	if (dbg != NULL) {
		logChanges = dbg->change;
		setChans = dbg->set;
		onChans = dbg->on;
		offChans = dbg->off;
		flipChans = dbg->flip;
	} else {
		do
			chanDesc = open(chanName, O_RDONLY);
		while (chanDesc == -1 && errno == EINTR);
		/* If there's no channel file, use default changes */
		if (chanDesc < 0) {
			/* Default changes is to toggle standard logs */
			logChanges = CHG_INV;
			flipChans = DBG_LOG;
			setChans = onChans = offChans = 0L;
		} else {
		    while (read(chanDesc, &logChanges, sizeof logChanges) == -1
							&& errno == EINTR)
			;
		    while (read(chanDesc, &setChans, sizeof setChans) == -1
							&& errno == EINTR)
			;
		    while (read(chanDesc, &onChans, sizeof onChans) == -1
							&& errno == EINTR)
			;
		    while (read(chanDesc, &offChans, sizeof offChans) == -1
							&& errno == EINTR)
			;
		    while (read(chanDesc, &flipChans, sizeof flipChans) == -1
							&& errno == EINTR)
			;
		    close(chanDesc);
		    unlink(chanName);
		}
	}

	
	/* Are changes for child? */
	if ((logChanges & CHG_CHILD) && childEnable != NULL) {
		/* Record child changes, if childEnable is non-NULL */
		if (childEnable != NULL) {
			if (logChanges & CHG_SET)
				*childEnable = setChans;
			if (logChanges & CHG_ON)
				*childEnable |= onChans;
			if (logChanges & CHG_OFF)
				*childEnable &= ~offChans;
			if (logChanges & CHG_INV)
				*childEnable ^= flipChans;
		}

		/* No need for both terse and verbose logs - terse overrides */
		if ((*childEnable & DBG_LOG) && (*childEnable & DBG_VLOG))
			*childEnable &= ~DBG_VLOG;

		return logChanges;
	}

	/* Change debug bits as requested by logChanges */
	if (logChanges & CHG_SET)
		dbgSet(setChans);
	if (logChanges & CHG_ON)
		dbgOn(onChans);
	if (logChanges & CHG_OFF)
		dbgOff(offChans);
	if (logChanges & CHG_INV)
		dbgToggle(flipChans);

	/* No need for both terse and verbose logs */
	if (dbgCheck(DBG_LOG) && dbgCheck(DBG_VLOG))
#ifdef	DEBUG
		dbgOff(DBG_LOG);	/* Verbose override */
#else
		dbgOff(DBG_VLOG);	/* Terse override */
#endif	/* DEBUG */

	/*
	   Close the log file, if requested -
	   otherwise if not already open, open new log file.
	*/
	if (logChanges & CHG_CLOSE)
		logClose();
	else
		if (logFile == NULL && dbgEnable != 0)
			logOpen(logName, namePid);

	return logChanges;
}
#endif /* ~NOLOG */
