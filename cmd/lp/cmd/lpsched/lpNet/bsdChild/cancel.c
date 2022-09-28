/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/cancel.c	1.2.2.1"

#include <stdio.h>
#include <string.h>
#include "msgs.h"
#include "lp.h"
#include "lpd.h"

#if defined (__STDC__)
static	char	* r_cancel(short, short, char *);
#else
static	char	* r_cancel();
#endif

static
struct status_map status_map[] = {
	"dequeued",		MOK,
	"Permission denied",	MNOPERM,
	"cannot dequeue",	M2LATE,
	NULL,			0
};

/*
 * Cancel job previously submitted to remote lpd
 * (S_CANCEL processing)
 */
char *
#if defined (__STDC__)
s_cancel(char *msg)
#else
s_cancel(msg)
char	*msg;
#endif
{
	short		 ostatus = -1;
	short		 status;
	char		 ojobid[MAX_REQID_SZ], *jobid;
	char		*dest;
	char		*host;
	char		*user;
	char		*reqid;
	struct status_map	*stmap;

	/* destination field should never be empty */
	(void)getmessage(msg, S_CANCEL, &dest, &user, &reqid);
	parseUser(user, &host, &user);		/* is this really required??? */
	logit(LOG_DEBUG, "S_CANCEL(\"%s\", \"%s\", \"%s\")", dest, user, reqid);
	if (STREQU(reqid, CURRENT_REQ)) {
		if (!snd_lpd_msg(RMJOB, dest, user, "", ""))
			return(NULL);
		reqid = NULL;
	} else if (!*reqid) {
		if (*user) {
			if (!snd_lpd_msg(RMJOB, dest, user, user, ""))
				return(NULL);
		} else
			if (!snd_lpd_msg(RMJOB, dest, ALL, "", ""))
				return(NULL);
		reqid = NULL;
	} else 
		/*
	 	 * Assumes S_CANCEL will not propogate this far if 
	 	 * submitting user does not have the proper credentials
	 	 */
		if (!snd_lpd_msg(RMJOB, dest, "root", "", rid2jid(reqid)))
			return(NULL);
	while (getNets(Buf, BUFSIZ)) {
		char		*t, c; 
		register char	*p;

		jobid = NULL;
		status = -1;
		logit(LOG_DEBUG, "s_cancel parsing: %s", Buf);
		for (p = Buf; p && *p && (!jobid || status<0); ) {
			p += strspn(p, " ");	/* strip initial blanks */
			if (*p == '\n') 
				break;
			if (!jobid) {
				if (!(t = strpbrk(p, " :\n")))
					break;
				c = *t;
				*t = NULL;
				if (STRNEQU(p, CFPREFIX, STRSIZE(CFPREFIX))) {
					/*
					 * Forget about status for jobs
					 * from other machines, since lpexec
					 * doesn't know about them and can't
					 * translate their jobids into
					 * request-ids.
					 */
					if (!STREQU(Lhost, LPD_HOSTNAME(p)))
						break;
					jobid = LPD_JOBID(p);
					*LPD_HOSTNAME(p) = NULL;
					p = t + 1;
					continue;
				} else
					/*
					 * Need to check to see if this field
					 * is a request-id, since the remote
					 * LPD machine may have remoted the
					 * printer to a machine running lpsched.
					 * It is probably not necessary to 
					 * filter-out status here.
					 */
					if (isrequest(p)) {
						jobid = p;
						p = t + 1;
						continue;
					} else
						*t = c;		/* back-out */
			}
			if (STRNEQU(p, DFPREFIX, STRSIZE(DFPREFIX)))
				break;		/* toss data file status */
			if (status < 0)
				for (stmap = status_map; stmap->msg; stmap++)
					if (STRNEQU(stmap->msg, p, 
							strlen(stmap->msg))) {
						status = stmap->status;
						p += strlen(stmap->msg);
						break;
					}
			p = strchr(p, ' ');
		}
		if (status < 0 || !jobid)	/* default: bad news */
			continue;
		if (ostatus >= 0) {
			if (!(msg = r_cancel(MOKMORE, ostatus, ojobid)))
				return(NULL);
			r_send_job(MOKMORE, msg);
		}
		ostatus = status;
		strcpy(ojobid, jobid);
	}
	if (ostatus < 0)		/* caller will send R_SEND_JOB */
		return(r_cancel(MNOINFO, MNOINFO, reqid));
	else
		return(r_cancel(MOK, ostatus, reqid ? reqid : ojobid));
}

static char *
#if defined (__STDC__)
r_cancel(short status1, short status2, char *reqid)
#else
r_cancel(status1, status2, reqid)
short	 status1;
short	 status2;
char	*reqid;
#endif
{
	logit(LOG_DEBUG, "R_CANCEL(%d, %d, \"%s\")", status1, status2, NB(reqid));
	if (putmessage(Msg, R_CANCEL, status1, status2, reqid) < 0)
		return(NULL);
	else
		return(Msg);
}
