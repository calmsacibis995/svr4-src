/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/getstatus.c	1.6.2.1"

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lp.h"
#include "msgs.h"
#include "lpd.h"

#if defined (__STDC__)
static	char	* r_get_status(int, char *);
static	int	  parseJob(char *, char **, int *, char **, char **);
#else
static	char	* r_get_status();
static	int	  parseJob();
#endif

/*
 * Gather job status from remote lpd
 * (S_GET_STATUS processing)
 */
char *
#if defined (__STDC__)
s_get_status(char *msg)
#else
s_get_status(msg)
char	*msg;
#endif
{
	FILE	*fp;
	char	*filename;
	char	*user, *host, *jobid;
	int	 status;
	int	 rank = 0;
	int	 n;
	int	 doing_queue = 0;
	int	 more = 0;
	mode_t	 omask;

	(void)getmessage(msg, S_GET_STATUS, &Printer, &filename);
	Printer = strdup(Printer);
	logit(LOG_DEBUG, "S_GET_STATUS(\"%s\", \"%s\")", Printer, filename);
	omask = umask(077);
	fp = fopen(filename, "w");
	(void)umask(omask);
	if (!fp) {
		logit(LOG_WARNING,
		      "s_get_status: can't open %s: %s", filename, PERROR); 
		status = MNOINFO;
		goto out;
	}
	if (!snd_lpd_msg(DISPLAYQL, Printer, "", "")) {
		status = MNOINFO;
		goto out;
	}
	while (getNets(Buf, BUFSIZ)) {
		n = strlen(Buf);
		if (strspn(Buf, " \t\n") == n)	/* get rid of blank lines */
			continue;
		if (parseJob(Buf, &user, &rank, &jobid, &host)) {
			doing_queue = 1;
			if (!STREQU(Lhost, host))	/* ignore non-locally */
				continue;		/* submitted jobs     */
			(void)fprintf(fp, 
				      "%s:%d:%s:%s\n", user, rank, jobid, host);
		} else
			if (doing_queue)   /* only pick up first status lines */
				continue;
			else {
				if (STREQU(NOENTRIES, Buf))
					continue;
				if (!more)
					fputs(PRINTER_STATUS_TAG, fp);
				(void)fprintf(fp, "%s", Buf);
				if (Buf[n-1] == '\n')
					more = 0;
				else
					more = 1;
			}
	}
	status = MOK;
out:
	if (fp)
		fclose(fp);			/* close status file */
	msg = r_get_status(status, Printer);
	free(Printer);
	return(msg);
}

/*
 * Parse job status returned from remote lpd
 */
static
#if defined (__STDC__)
parseJob(char *p, char **puser, int *prank, char **pjobid, char **phost)
#else
parseJob(p, puser, prank, pjobid, phost)
char	 *p;
char	**puser;
int	 *prank;
char	**pjobid;
char	**phost;
#endif
{
	register char	*t1, *t2;
	int		 rank;

	/*
	 * Attempt to parse lines of the form:
	 * user: rank                            [job jobidHostname]
	 * user: rank                            [job reqid Hostname]
	 */
	if (isspace(*p))
		return(0);
	if ((t1 = strchr(p, ':')) == NULL)
		return(0);
	*puser = p;
	for (p = t1+1; isspace(*p); p++)
		;
	if (STRNEQU(p, "active", STRSIZE("active")))
		rank = 0;
	else {
		if (!isdigit(*p))
			return(0);
		rank = atoi(p);
	}
	if ((p = strchr(p, '[')) == NULL || 
	   !STRNEQU(p, "[job", STRSIZE("[job")))
		return(0);
	for (p += STRSIZE("[job"); isspace(*p); p++)
		;
	if ((t2 = strchr(p, ']')) == NULL)
		return(0);

	/* Now OK to alter input string */ 

	*t1 = *t2 = NULL;
	if (t1 = strchr(p, ' ')) {	/* check for S5 format */
		*t1 = NULL;
		*pjobid = p;
		*phost = t1+1;
	} else {
		strncpy(p - SIZEOF_JOBID, p, SIZEOF_JOBID);	/* avoid overlap  */
		*p = NULL;
		*pjobid = p - SIZEOF_JOBID;
		*phost = p + SIZEOF_JOBID;
	}
	if (rank < *prank)	/* in case status returned from > 1 machine */
		(*prank)++;
	else
		*prank = rank;
	return(1);
}

static char *
#if defined (__STDC__)
r_get_status(int status, char * printer)
#else
r_get_status(status, printer)
int	status;
char	*printer;
#endif
{
	logit(LOG_DEBUG, "R_GET_STATUS(%d, \"%s\")", status, printer);
	if (putmessage(Msg, R_GET_STATUS, status, printer) < 0)
		return(NULL);
	else
		return(Msg);
}
