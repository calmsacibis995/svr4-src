/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)saf:log.c	1.5.4.1"

# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include "extern.h"
# include "misc.h"
# include "msgs.h"
# include <sac.h>
# include "structs.h"


static	FILE	*Lfp;	/* log file */
# ifdef DEBUG
static	FILE	*Dfp;	/* debug file */
# endif


/*
 * openlog - open log file, sets global file pointer Lfp
 */


void
openlog()
{
	FILE *fp;		/* scratch file pointer for problems */

	Lfp = fopen(LOGFILE, "a+");
	if (Lfp == NULL) {
		fp = fopen("/dev/console", "w");
		if (fp) {
			(void) fprintf(fp, "SAC: could not open logfile\n");
		}
		exit(1);
	}

/*
 * lock logfile to indicate presence
 */

	if (lockf(fileno(Lfp), F_LOCK, 0) < 0) {
		fp = fopen("/dev/console", "w");
		if (fp) {
			(void) fprintf(fp, "SAC: could not lock logfile\n");
		}
		exit(1);
	}
}


/*
 * log - put a message into the log file
 *
 *	args:	msg - message to be logged
 */


void
log(msg)
char *msg;
{
	char *timestamp;	/* current time in readable form */
	long clock;		/* current time in seconds */
	char buf[SIZE];		/* scratch buffer */

	(void) time(&clock);
	timestamp = ctime(&clock);
	*(strchr(timestamp, '\n')) = '\0';
	(void) sprintf(buf, "%s; %ld; %s\n", timestamp, getpid(), msg);
	(void) fprintf(Lfp, buf);
	(void) fflush(Lfp);
}


/*
 * error - put an error message into the log file and exit if indicated
 *
 *	args:	msgid - id of message to be output
 *		action - action to be taken (EXIT or not)
 */


void
error(msgid, action)
int msgid;
int action;
{
	if (msgid < 0 || msgid > N_msgs)
		return;
	log(Msgs[msgid].e_str);
	if (action == EXIT) {
		log("*** SAC exiting ***");
		exit(Msgs[msgid].e_exitcode);
	}
}


# ifdef DEBUG

/*
 * opendebug - open debugging file, sets global file pointer Dfp
 */


void
opendebug()
{
	FILE *fp;	/* scratch file pointer for problems */

	Dfp = fopen(DBGFILE, "a+");
	if (Dfp == NULL) {
		fp = fopen("/dev/console", "w");
		if (fp) {
			(void) fprintf(fp, "SAC: could not open debugfile\n");
		}
		exit(1);
	}
}


/*
 * debug - put a message into debug file
 *
 *	args:	msg - message to be output
 */


void
debug(msg)
char *msg;
{
	char *timestamp;	/* current time in readable form */
	long clock;		/* current time in seconds */
	char buf[SIZE];		/* scratch buffer */

	(void) time(&clock);
	timestamp = ctime(&clock);
	*(strchr(timestamp, '\n')) = '\0';
	(void) sprintf(buf, "%s; %ld; %s\n", timestamp, getpid(), msg);
	(void) fprintf(Dfp, buf);
	(void) fflush(Dfp);
}

# endif
