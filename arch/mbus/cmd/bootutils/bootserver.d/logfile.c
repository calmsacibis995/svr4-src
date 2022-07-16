/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/bootserver.d/logfile.c	1.3"

/************************************************
 *    BootServer logfile writing utilities.
 *
 * Included herein are:
 *		LogInit 	-- Initialize logfile writing
 *		DoLog 		-- Write timestamped entry in logfile.
 *
 ************************************************/

#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include "bootserver.h"

extern int write();
extern size_t strlen();
extern int LogLevel;
static int LogFile = 0;
static char *Months[] = { 
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};
long time();

/************************************************
 *  LogInit -- Initialize logfile writing.
 *
 *     If this routine is not called or if the passed log
 * filename is of zero length, things are set up so that
 * logging will goto standard out.
 *     Returns zero on success and -1 on failure.  The only
 * failure is inability of open the logfile for reading.
 *
 ************************************************/
int
LogInit(LogFilename)
	char *LogFilename;
{
	if ((unsigned int)(strlen(LogFilename)) != 0) {
		LogFile = open(LogFilename, O_WRONLY|O_APPEND|O_CREAT, 0744);
		if (LogFile == -1) {
			/* failure on the logfile front--info still in errno */
			LogFile = 0;
			return(-1);
		}
	} else {
		/* the logging is to happen to standard out */
		LogFile = 0;
	}
	return(0);
}

/************************************************
 *  DoLog -- write a line to the logfile.
 *
 *      Write a timestamped entry to the logfile.  ARGS are
 * optional arguments that are used by a 'sprintf' to format
 * the passed Text.  The timestamp is then added and a terminating
 * newline is added and the log line is written to the logfile.
 *
 ************************************************/
/* VARARGS2 */
void
DoLog(Level, Text, ARGS)
	int Level;
	char *Text;
	int ARGS;
{
	char mybuf[MAXLOGLINE];
	char mybuft[MAXLOGLINE];
	long thetime;
	struct tm *ct;

	if (Level && ((Level & LogLevel) == 0))
		return;

	thetime = time((long *)NULL);
	ct = localtime(&thetime);
	(void)sprintf(mybuft, Text, ARGS);
	(void)sprintf(mybuf, "%s %2.2d %2.2d:%02.2d:%02.2d: %s\n",
		Months[ct->tm_mon], ct->tm_mday, ct->tm_hour, ct->tm_min,
		ct->tm_sec, mybuft);
	if (LogFile == 0) {
		/* no logfile specified -- write to stdout */
		(void)fputs(mybuf, stdout);
	} else {
		(void)write(LogFile, mybuf, (unsigned int)(strlen(mybuf)));
	}
}
