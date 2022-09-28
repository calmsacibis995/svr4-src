/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/lpsched/rexec.c	1.8.3.1"

#include "limits.h"
#if	defined(__STDC__)
#include "stdarg.h"
#else
#include "varargs.h"
#endif

#include "lpsched.h"


MESG *			Net_md;

static void
#if	defined(__STDC__)
rex_send_job (
	SSTATUS *		pss,
	int			job_type,
	char *			job_file,
	char *			msgbuf
)
#else
rex_send_job (pss, job_type, job_file, msgbuf)
	SSTATUS *		pss;
	int			job_type;
	char *			job_file;
	char *			msgbuf;
#endif
{
	ENTRY ("rex_send_job")

	schedlog (
		"rex_send_job: sending S_SEND_JOB to %s\n",
		pss->system->name
	);
	mputm (pss->exec->md,
		S_SEND_JOB,
		pss->system->name,
		job_type,
		job_file,
		msize(msgbuf),
		msgbuf
	);
	pss->exec->flags |= EXF_WAITJOB;
	return;
}

/**
 ** rexec() - FORWARD EXEC REQUEST TO REMOTE MACHINE
 **/

/*VARARGS2*/
int
#if	defined(__STDC__)
rexec (
	SSTATUS *		pss,
	int			type,
	...
)
#else
rexec (pss, type, va_alist)
	SSTATUS			*pss;
	int			type;
	va_dcl
#endif
{
	ENTRY ("rexec")

	va_list			args;

	PSTATUS *		pps;

	RSTATUS *		prs;

	EXEC *			ep;

	char			msgbuf[MSGMAX];

	char *			full_user;
	char *			req_file;


	if (!pss) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Extract useful values, sanity check request.
	 */
#if	defined(__STDC__)
	va_start (args, type);
#else
	va_start (args);
#endif
	switch (type) {

	case REX_INTERF:
		pps = va_arg(args, PSTATUS *);
		prs = va_arg(args, RSTATUS *);
		if (!(pps->status & PS_REMOTE)) {
			errno = EINVAL;
			return (-1);
		}
		break;

	case REX_CANCEL:
	case REX_NOTIFY:
	case REX_STATUS:
		prs = va_arg(args, RSTATUS *);
		break;

	default:
		errno = EINVAL;
		return (-1);

	}
	va_end (args);

	schedlog (
		"rexec, type %d: trying to send request %s to %s%s%s\n",
		type,
		prs->secure->req_id,
		pss->system->name,
		prs->status & RSS_GETSTATUS? " status protocol is" : "",
		prs->status & RSS_GETSTATUS?
		   (pss->system->protocol == S5_PROTO ? "S5" : "BSD") : ""
	);

	/*
	 * If the connection is ``busy'', we can't do anything yet.
	 */
	if (pss->exec->flags & (EXF_WAITJOB | EXF_WAITCHILD)) {
		schedlog ("rexec, type %d: connection is busy\n", type);
		return (0);	/* only a tiny lie */
	}

	/*
	 * If we don't yet have a connection to the network server
	 * child for this system, get one. The exec structure is
	 * marked to avoid us asking more than once.
	 *
	 *	md		EXF_WAITCHILD	action
	 *	--------	-------------	--------------------
	 *	NOT SET		NOT SET		send S_NEW_CHILD
	 *	NOT SET		SET		awaiting R_NEW_CHILD
	 *	SET		NOT SET		send S_SEND_JOB
	 *	SET		SET		uh oh!
	 */
	ep = pss->exec;
	if (!ep->md && !(ep->flags & EXF_WAITCHILD)) {
		schedlog (
			"Sending S_NEW_CHILD to lpNet, for system %s\n",
			pss->system->name
		);
		mputm (Net_md, S_NEW_CHILD, pss->system->name);
		ep->flags |= EXF_WAITCHILD;
		ep->type = (short)type; /* so as to pick up where we left off */
		return (0);	/* a minor lie indeed */
	}

	/*
	 * We already have a connection, and we know it is not busy,
	 * so we may proceed.
	 */

	/*
	 * Set flags that keep the scheduler informed of progress.
	 * Some of these flags may be little white lies, but they will
	 * keep the scheduler operating correctly.
	 */

	switch (type) {

	case REX_INTERF:
		prs->request->outcome |= RS_SENDING;
		break;

	case REX_CANCEL:
	case REX_NOTIFY:
		prs->request->outcome |= RS_SENDING;
		prs->status &= ~RSS_SENDREMOTE;
		break;

	case REX_STATUS:
		prs->request->outcome |= RS_SENDING;
		prs->status |= RSS_RECVSTATUS;
		break;

	}

	/*
	 * Attach the request that's going out (for whatever
	 * reason--printing, cancelling, etc.) to the system
	 * so we can easily match a returning R_SEND_JOB message
	 * with the request. Also, mark this request as no longer
	 * needing to be sent.
	 */
	pss->exec->ex.request = prs;
	prs->status &= ~RSS_SENDREMOTE;

	/*
	 * Do S_SEND_JOB.
	 */
	switch (type) {

	case REX_INTERF:
		putmessage (msgbuf, S_PRINT_REQUEST, prs->req_file);
		putjobfiles (prs);
		rex_send_job (pss, 1, prs->req_file, msgbuf);
		break;

	case REX_CANCEL:
		if (strchr(prs->secure->user, BANG_C))
			full_user = Strdup(prs->secure->user);
		else
			full_user = makestr(
				Local_System,
				BANG_S,
				prs->secure->user,
				(char *)0
			);
		putmessage (
			msgbuf,
			S_CANCEL,
			prs->printer->remote_name,
			full_user,
			prs->secure->req_id
		);
		rex_send_job (pss, 0, "", msgbuf);
		Free (full_user);
		break;

	case REX_NOTIFY:
		req_file = makereqerr(prs);
		putmessage (
			msgbuf,
			S_JOB_COMPLETED,
			prs->request->outcome,
			prs->secure->req_id,
			req_file
		);
		rex_send_job (pss, 0, req_file, msgbuf);
		break;

	case REX_STATUS:
		switch (pss->system->protocol) {

		case S5_PROTO:
			putmessage (msgbuf, S_INQUIRE_REQUEST_RANK, 0, "", "", "", "", "");
			rex_send_job (pss, 0, "", msgbuf);
			break;

		case BSD_PROTO:
			for (pps = walk_ptable(1); pps; pps = walk_ptable(0))
			{
			    if (pps->system != pss)

				continue;
		    
			    (void) putmessage(msgbuf, S_GET_STATUS,
					      pps->remote_name,
					      pps->alert->msgfile);
			    rex_send_job(pss, 0, "", msgbuf);
			}
			break;
		}
		break;
	}

	return (0);
}

/**
 ** resend_remote() - RESET SYSTEM AND REQUEST, FOR ANOTHER SEND TO REMOTE
 **/

void
#if	defined(__STDC__)
resend_remote (
	SSTATUS *		pss,
	int			when
)
#else
resend_remote (pss, when)
	SSTATUS *		pss;
	int			when;
#endif
{
	ENTRY ("resend_remote")

	RSTATUS *		prs = pss->exec->ex.request;


	pss->exec->flags &= ~(EXF_WAITJOB|EXF_WAITCHILD);
	if (prs) {
		prs->request->outcome &= ~RS_GONEREMOTE;
		prs->status |= RSS_SENDREMOTE;
	}

	switch (when) {

	case -1:
		break;

	case 0:
		schedule (EV_SYSTEM, pss);
		break;

	default:
		schedule (EV_LATER, when, EV_SYSTEM, pss);
		break;

	}
	return;
}
