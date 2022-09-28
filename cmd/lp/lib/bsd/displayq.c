/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/bsd/displayq.c	1.4.3.1"

#include <string.h>
#include "lp.h"
#include "msgs.h"
#include "requests.h"
#include "printers.h"
#include "systems.h"
#include "lpd.h"
#include "oam_def.h"

static int	 Col;		/* column on screen */

#if defined (__STDC__)
static	char	* printerstatus(void);
static	int	  inlist(char *, char *, char *);
static	void	  show(REQUEST *, int);
static	void 	  blankfill(int);
static	void 	  header(void);
static	void 	  prank(int);
#else
static	char	* printerstatus();
static	int	  inlist();
static	void	  show();
static	void 	  blankfill();
static	void 	  header();
static	void 	  prank();
#endif

/*
 * Display lpq-like status to stdout
 */
int
#if defined (__STDC__)
displayq(int format)
#else
displayq(format)
int	format;
#endif
{
	short	 status;
	char	*host;
	char	*owner;
	char	*reqid;
	size_t	 size;
	time_t	 date;
	short	 outcome;
	char	*dest, *form, *pwheel;
	short	 rank;
	REQUEST	*rqp;
	char	*p;
	int      need_header = format == 0;
	char	*prst;
	char	 rqfn[10];

	prst = printerstatus();		/* have to get status ahead of time */
	snd_msg(S_INQUIRE_REQUEST_RANK, 1, "", Printer, "", "", "");
	do {
		rcv_msg(R_INQUIRE_REQUEST_RANK, &status, &reqid, &owner,
					        &size, &date, &outcome, 
					  	&dest, &form, &pwheel, &rank);
		switch(status) {
		case MOK:
		case MOKMORE:
			if (prst) {
				(void)printf("%s", prst);
				fflush(stdout);
				prst = NULL;
			}
			if (need_header) {	/* short format */
				header();
				need_header = 0;
			}
			break;

		case MNOINFO:
			(void)printf(NOENTRIES);
			fflush(stdout);
			return(0);

		default:
			return(0);
		}
		logit(LOG_DEBUG, 
		"R_INQUIRE_REQUEST_RANK(\"%s\", \"%s\", \"%s\", %d)", 
			  		reqid, owner, dest, rank);
		parseUser(owner, &host, &owner);
		if (!(inlist(host, owner, reqid)))
			continue;
		(void)sprintf(rqfn, "%s-0", strrchr(reqid, '-')+1), 
		p = makepath(host, rqfn, (char *)0);
		if (p) {
			rqp = getrequest(p);
			if (!rqp)
				logit(LOG_WARNING, "%s can't getrequest(\"%s\")", 
								Name, p);
			free(p);
		} else
			rqp = NULL;
		if (format == 0) {		/* short format */
			Col = 0;
			prank(rank);
			blankfill(OWNCOL);
			Col += printf("%s", owner);
			blankfill(REQCOL);
			Col += printf("%s", reqid);
			blankfill(FILCOL);
		} else {			/* long format */
			Col = printf("\n%s: ", owner) - 1;
			prank(rank);
			blankfill(JOBCOL);
			(void)printf("[job %s %s]\n", reqid, host); 
			Col = 0;
		}
		show(rqp, format);
		freerequest(rqp);
	} while (status == MOKMORE);
	fflush(stdout);
	return(1);
}

/*
 * Return pointer to printer status string
 */
static char *
#if defined (__STDC__)
printerstatus(void)
#else
printerstatus()
#endif
{
	short	status;
	char	*prname, *form, *pwheel, *dis_reason, *rej_reason;
	short	 prstatus;
	char	*reqid;
	time_t	*dis_date, *rej_date;
	int	 type;
	int	 n = 0;
	PRINTER	*pr;
	SYSTEM	*sys = NULL;
	static char	 buf[1024];
	char		 id[20];

	if (!(pr = getprinter(Printer))) {
		fatal("unknown printer");
		/*NOTREACHED*/
	}
	if (pr->remote)  {
		if (prname = strchr(pr->remote, '!'))
			*prname = NULL;
		if (sys = getsystem(pr->remote))
			type = S_INQUIRE_REMOTE_PRINTER;
		else {
			logit(LOG_WARNING, "No system entry for %s", pr->remote);
			type = S_INQUIRE_PRINTER_STATUS;
		}
	} else
		type = S_INQUIRE_PRINTER_STATUS;

	buf[0] = NULL;
	id[0] = NULL;
get_status:
	snd_msg(type, Printer);
	rcv_msg(R_INQUIRE_PRINTER_STATUS, &status, &prname, &form, &pwheel, 
					  &dis_reason, &rej_reason, &prstatus, 
					  &reqid, &dis_date, &rej_date);
	switch(status) {
	case MNODEST:
		fatal("unknown printer");
		/*NOTREACHED*/
	case MOK:
		/* 
		 * It is not possible to know if remote really responded
		 * to request for status.  Lpsched may return old status;
		 * however, if we can determine that status is indeed old,
		 * then we might consider printing:
		 *	if (Rhost)
		 *		printf("%s: ", Lhost);
		 *	printf("connection to %s is down\n", pr->remote);
		 */
		break;
	default:
		lp_fatal(E_LP_BADSTATUS, status);
		/*NOTREACHED*/
	}

	if (type == S_INQUIRE_REMOTE_PRINTER && sys->protocol == BSD_PROTO) {
		if (*dis_reason)
			n += sprintf(buf+n, "%s", dis_reason);
		if (*rej_reason)	
			n += sprintf(buf+n, "%s", rej_reason);
	} else {
		if (prstatus & PS_DISABLED)
			n += sprintf(buf+n, "%sWarning: %s is down: %s\n", 
							id, prname, dis_reason);
		if (prstatus & PS_FAULTED)
			n += sprintf(buf+n,
				"%swaiting for %s to become ready (offline?)\n",
								id, prname);
		if (prstatus & PS_REJECTED)
			n += sprintf(buf+n,
				"%sWarning: %s queue is turned off: %s\n",
						id, prname, rej_reason);
		if (!(prstatus & (PS_DISABLED|PS_FAULTED)))
			n += sprintf(buf+n, "%s%s is ready and printing\n", 
							id, prname);
	}
	if (type == S_INQUIRE_REMOTE_PRINTER) {
		type = S_INQUIRE_PRINTER_STATUS;
		sprintf(id, "%.*s: ", sizeof(id)-3, Lhost);
		goto get_status;
	}
	/*
	 * There are other types of status that lpd reports, e.g.:
	 *	sending to remote
	 *	waiting for remote to come up
	 *	waiting for queue to be enabled on remote
	 *	no space on remote; waiting for queue to drain
	 *	waiting for printer to become ready (offline?)
	 * but there is no way to determine this information from the
	 * lpsched printer status
	 */ 
	freesystem(sys);
	freeprinter(pr);
	return(buf);
}

/*
 * Return 1 if user or request-id has been selected, 0 otherwise
 */
static
#if defined (__STDC__)
inlist(char *host, char *name, char *reqid)
#else
inlist(host, name, reqid)
char	*host;
char	*name;
char	*reqid;
#endif
{
	register char 	**p;

	if (!Nusers && !Nrequests)
		return(1);
	/*
	 * Check to see if it's in the user list
	 */
	for (p = User; p < &User[Nusers]; p++)
		if (STREQU(*p, name))
			return(1);
	/*
	 * Check the request list
	 */
	if (Rhost ? !STREQU(host, Rhost) : !STREQU(host, Lhost))
		return(0);
	for (p = Request; p < &Request[Nrequests]; p++)
		if (STREQU(*p, reqid))
			return(1);
	return(0);
}

/*
 * Print lpq header
 */
static void
#if defined (__STDC__)
header(void)
#else
header()
#endif
{
	Col = printf(HEAD0);
	blankfill(SIZCOL);
	(void)printf(HEAD1);
	fflush(stdout);
	Col = 0;
}

static void
#if defined (__STDC__)
blankfill(register int n)
#else
blankfill(n)
register int	n;
#endif
{
	do				/* always output at least one blank */
		putchar(' ');
	while (++Col < n);
}

static void
#if defined (__STDC__)
prank(int n)
#else
prank(n)
int	n;
#endif
{
	static char 	*r[] = {
		"th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
	};
	char		 buf[20];

	if (n == 0) {
		Col += printf("active");
		return;
	}
	if ((n/10) == 1)
		(void)sprintf(buf, "%dth", n);
	else
		(void)sprintf(buf, "%d%s", n, r[n%10]);
	Col += printf("%s", buf);
}

static void
#if defined (__STDC__)
show(REQUEST *rqp, int f)
#else
show(rqp, f)
REQUEST	*rqp;
int	 f;
#endif
{
	char	*files[MAX_LPD_FILES];
	char	*sizes[MAX_LPD_FILES];
	char	*flist;
	char	*fill = "";
	size_t	 totsize, size;
	int	 n, N, c;

	if (!rqp || !(flist = find_strfld(FLIST, rqp->options))) {
		if (f == 0) {
			Col += printf("%s", NO_FILENAME);
			blankfill(SIZCOL);
			(void)printf("??? bytes\n");
		} else {
			Col += printf("        %s", NO_FILENAME);
			blankfill(JOBCOL);
			(void)printf("??? bytes\n", NO_FILENAME);
		}
		return;
	}
	totsize = 0;
	N = parseflist(flist+STRSIZE(FLIST), MAX_LPD_FILES, files, sizes);
	for (n = 0; n < N; n++) {
		if (STREQU(files[n], ""))
			files[n] = "standard input";
		size = atoi(sizes[n]);
		if (f == 0) {		/* short format */
	 		if (Col + (strlen(files[n]) + strlen(fill)) >=
					       (n == N-1 ? SIZCOL : SIZCOL-4)) {
				if (Col < SIZCOL)
					Col += printf(" ...");
			} else {
				Col += printf("%s%s", fill, files[n]);
				fill = ", ";
			}
			totsize += size;
		} else {		/* long format */
			if (rqp->copies > 1)
				Col += printf("        %-2d copies of %s", 
							rqp->copies, files[n]);
			else
				Col += printf("        %s", files[n]);
			blankfill(JOBCOL);
			(void)printf("%lu bytes\n", size);
			Col = 0;
		}
	}
	if (N && f == 0) {			/* short format */
		blankfill(SIZCOL);
		(void)printf("%lu bytes\n", totsize * rqp->copies);
	}
	free(flist);
}
