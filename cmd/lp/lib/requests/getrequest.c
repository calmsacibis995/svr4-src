/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/requests/getrequest.c	1.14.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "requests.h"

extern struct {
	char			*v;
	short			len;
}			reqheadings[];

/**
 ** getrequest() - EXTRACT REQUEST STRUCTURE FROM DISK FILE
 **/

REQUEST *
#if	defined(__STDC__)
getrequest (
	char *			file
)
#else
getrequest (file)
	char			*file;
#endif
{
	static REQUEST		reqbuf;

	char			buf[BUFSIZ],
				*path,
				*p;

	FILE			*fp;

	int			fld;


	/*
	 * Full pathname? If so the file must lie in LP's
	 * regular temporary directory.
	 */
	if (*file == '/') {
		if (!STRNEQU(file, Lp_Tmp, strlen(Lp_Tmp))) {
			errno = EINVAL;
			return (0);
		}
		path = Strdup(file);

	/*
	 * A relative pathname (such as system/name)?
	 * If so we'll locate it under LP's regular temporary
	 * directory.
	 */
	} else if (strchr(file, '/')) {
		if (!(path = makepath(Lp_Tmp, file, (char *)0)))
			return (0);

	/*
	 * It must be a simple name. Locate this under the
	 * special temporary directory that is linked to the
	 * regular place for the local system.
	 */
	} else if (!(path = makepath(Lp_Temp, file, (char *)0)))
		return (0);
    

	if (!(fp = open_lpfile(path, "r", 0))) {
		Free (path);
		return (0);
	}
	Free (path);

	reqbuf.copies		= 1;
	reqbuf.destination	= 0;
	reqbuf.file_list	= 0;
	reqbuf.form		= 0;
	reqbuf.actions		= 0;
	reqbuf.alert		= 0;
	reqbuf.options		= 0;
	reqbuf.priority		= -1;
	reqbuf.pages		= 0;
	reqbuf.charset		= 0;
	reqbuf.modes		= 0;
	reqbuf.title		= 0;
	reqbuf.input_type	= 0;
	reqbuf.user		= 0;
	reqbuf.outcome		= 0;

	while (fgets(buf, BUFSIZ, fp)) {

		buf[strlen(buf) - 1] = 0;

		for (fld = 0; fld < RQ_MAX; fld++)
			if (
				reqheadings[fld].v
			     && reqheadings[fld].len
			     && STRNEQU(
					buf,
					reqheadings[fld].v,
					reqheadings[fld].len
				)
			) {
				p = buf + reqheadings[fld].len;
				break;
			}

		/*
		 * To allow future extensions to not impact applications
		 * using old versions of this routine, ignore strange
		 * fields.
		 */
		if (fld >= RQ_MAX)
			continue;

		switch (fld) {

		case RQ_COPIES:
			reqbuf.copies = atoi(p);
			break;

		case RQ_DEST:
			reqbuf.destination = Strdup(p);
			break;

		case RQ_FILE:
			appendlist (&reqbuf.file_list, p);
			break;

		case RQ_FORM:
			if (!STREQU(p, NAME_ANY))
				reqbuf.form = Strdup(p);
			break;

		case RQ_HANDL:
			if (STREQU(p, NAME_RESUME))
				reqbuf.actions |= ACT_RESUME;
			else if (STREQU(p, NAME_HOLD))
				reqbuf.actions |= ACT_HOLD;
			else if (STREQU(p, NAME_IMMEDIATE))
				reqbuf.actions |= ACT_IMMEDIATE;
			break;

		case RQ_NOTIFY:
			if (STREQU(p, "M"))
				reqbuf.actions |= ACT_MAIL;
			else if (STREQU(p, "W"))
				reqbuf.actions |= ACT_WRITE;
			else if (STREQU(p, "N"))
				reqbuf.actions |= ACT_NOTIFY;
			else
				reqbuf.alert = Strdup(p);
			break;

		case RQ_OPTS:
			reqbuf.options = Strdup(p);
			break;

		case RQ_PRIOR:
			reqbuf.priority = atoi(p);
			break;

		case RQ_PAGES:
			reqbuf.pages = Strdup(p);
			break;

		case RQ_CHARS:
			if (!STREQU(p, NAME_ANY))
				reqbuf.charset = Strdup(p);
			break;

		case RQ_TITLE:
			reqbuf.title = Strdup(p);
			break;

		case RQ_MODES:
			reqbuf.modes = Strdup(p);
			break;

		case RQ_TYPE:
			reqbuf.input_type = Strdup(p);
			break;

		case RQ_USER:
			reqbuf.user = Strdup(p);
			break;

		case RQ_RAW:
			reqbuf.actions |= ACT_RAW;
			break;

		case RQ_FAST:
			reqbuf.actions |= ACT_FAST;
			break;

		case RQ_STAT:
			reqbuf.outcome = (ushort)strtol(p, (char **)0, 16);
			break;

		}

	}
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	/*
	 * Now go through the structure and see if we have
	 * anything strange.
	 */
	if (
		reqbuf.copies <= 0
	     || !reqbuf.file_list || !*(reqbuf.file_list)
	     || reqbuf.priority < -1 || 39 < reqbuf.priority
	     || STREQU(reqbuf.input_type, NAME_ANY)
	     || STREQU(reqbuf.input_type, NAME_TERMINFO)
	) {
		freerequest (&reqbuf);
		errno = EBADF;
		return (0);
	}

	/*
	 * Guarantee some return values won't be null or empty.
	 */
	if (!reqbuf.destination || !*reqbuf.destination) {
		if (reqbuf.destination)
			Free (reqbuf.destination);
		reqbuf.destination = Strdup(NAME_ANY);
	}
	if (!reqbuf.input_type || !*reqbuf.input_type) {
		if (reqbuf.input_type)
			Free (reqbuf.input_type);
		reqbuf.input_type = Strdup(NAME_SIMPLE);
	}

	return (&reqbuf);
}
