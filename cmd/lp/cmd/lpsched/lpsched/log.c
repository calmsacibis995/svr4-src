/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/log.c	1.1.4.1"

#if	defined(__STDC__)
#include "stdarg.h"
#else
#include "varargs.h"
#endif

#include "lpsched.h"

#if	defined(__STDC__)
static void		log ( char * , va_list );
#else
static void		log();
#endif

/**
 ** open_logfile() - OPEN FILE FOR LOGGING MESSAGE
 ** close_logfile() - CLOSE SAME
 **/

FILE *
#if	defined(__STDC__)
open_logfile (
	char *			name
)
#else
open_logfile (name)
	char			*name;
#endif
{
	ENTRY ("open_logfile")

	register char		*path;

	register FILE		*fp;


#if	defined(MALLOC_3X)
	/*
	 * Don't rely on previously allocated pathnames.
	 */
#endif
	path = makepath(Lp_Logs, name, (char *)0);
	fp = fopen(path, "a");
	Free (path);
	return (fp);
}

void
#if	defined(__STDC__)
close_logfile (
	FILE *			fp
)
#else
close_logfile (fp)
	FILE			*fp;
#endif
{
	ENTRY ("close_logfile")

	fclose (fp);
	return;
}

/**
 ** fail() - LOG MESSAGE AND EXIT (ABORT IF DEBUGGING)
 **/

/*VARARGS1*/
void
#if	defined(__STDC__)
fail (
	char *			format,
	...
)
#else
fail (format, va_alist)
	char			*format;
	va_dcl
#endif
{
	ENTRY ("fail")

	va_list			ap;
    
#if	defined(__STDC__)
	va_start (ap, format);
#else
	va_start (ap);
#endif
	log (format, ap);
	va_end (ap);

#if	defined(DEBUG)
	if (debug & DB_ABORT)
		abort ();
	else
#endif
		exit (1);
	/*NOTREACHED*/
}

/**
 ** note() - LOG MESSAGE
 **/

/*VARARGS1*/
void
#if	defined(__STDC__)
note (
	char *			format,
	...
)
#else
note (format, va_alist)
	char			*format;
	va_dcl
#endif
{
	ENTRY ("note")

	va_list			ap;

#if	defined(__STDC__)
	va_start (ap, format);
#else
	va_start (ap);
#endif
	log (format, ap);
	va_end (ap);
	return;
}

/**
 ** schedlog() - LOG MESSAGE IF IN DEBUG MODE
 **/

/*VARARGS1*/
void
#if	defined(__STDC__)
schedlog (
	char *			format,
	...
)
#else
schedlog (format, va_alist)
	char			*format;
	va_dcl
#endif
{
	ENTRY ("schedlog")

	va_list			ap;

#if	defined(DEBUG)
	if (debug & DB_SCHEDLOG) {

#if	defined(__STDC__)
		va_start (ap, format);
#else
		va_start (ap);
#endif
		log (format, ap);
		va_end (ap);

	}
#endif
	return;
}

/**
 ** mallocfail() - COMPLAIN ABOUT MEMORY ALLOCATION FAILURE
 **/

void
#if	defined(__STDC__)
mallocfail (
	void
)
#else
mallocfail ()
#endif
{
	ENTRY ("mallocfail")

	fail ("Memory allocation failed!\n");
	/*NOTREACHED*/
}

/**
 ** log() - LOW LEVEL ROUTINE THAT LOGS MESSSAGES
 **/

static void
#if	defined(__STDC__)
log (
	char *			format,
	va_list			ap
)
#else
log (format, ap)
	char			*format;
	va_list			ap;
#endif
{
	ENTRY ("log")

	int			close_it;

	FILE			*fp;

	static int		nodate	= 0;


	if (!am_in_background) {
		fp = stdout;
		close_it = 0;
	} else {
		if (!(fp = open_logfile("lpsched")))
			return;
		close_it = 1;
	}

	if (am_in_background && !nodate) {
		long			now;

		time (&now);
		fprintf (fp, "%24.24s: ", ctime(&now));
	}
	nodate = 0;

	vfprintf (fp, format, ap);
	if (format[strlen(format) - 1] != '\n')
		nodate = 1;

	if (close_it)
		close_logfile (fp);
	else
		fflush (fp);

	return;
}

/**
 ** execlog()
 **/

/*VARARGS1*/
void
#if	defined(__STDC__)
execlog (
	char *			format,
	...
)
#else
execlog (format, va_alist)
	char			*format;
	va_dcl
#endif
{
	ENTRY ("execlog")

	va_list			ap;

#if	defined(DEBUG)
	FILE			*fp	= open_logfile("exec");

	time_t			now = time((time_t *)0);

	char			buffer[BUFSIZ];

	EXEC *			ep;

	static int		nodate	= 0;

#if	defined(__STDC__)
	va_start (ap, format);
#else
	va_start (ap);
#endif
	if (fp) {
		setbuf (fp, buffer);
		if (!nodate)
			fprintf (fp, "%24.24s: ", ctime(&now));
		nodate = 0;
		if (!STREQU(format, "%e")) {
			vfprintf (fp, format, ap);
			if (format[strlen(format) - 1] != '\n')
				nodate = 1;
		} else switch ((ep = va_arg(ap, EXEC *))->type) {
		case EX_INTERF:
			fprintf (
				fp,
				"      EX_INTERF %s %s\n",
				ep->ex.printer->printer->name,
				ep->ex.printer->request->secure->req_id
			);
			break;
		case EX_SLOWF:
			fprintf (
				fp,
				"      EX_SLOWF %s\n",
				ep->ex.request->secure->req_id
			);
			break;
		case EX_ALERT:
			fprintf (
				fp,
				"      EX_ALERT %s\n",
				ep->ex.printer->printer->name
			);
			break;
		case EX_FALERT:
			fprintf (
				fp,
				"      EX_FALERT %s\n",
				ep->ex.form->form->name
			);
			break;
		case EX_PALERT:
			fprintf (
				fp,
				"      EX_PALERT %s\n",
				ep->ex.pwheel->pwheel->name
			);
			break;
		case EX_NOTIFY:
			fprintf (
				fp,
				"      EX_NOTIFY %s\n",
				ep->ex.request->secure->req_id
			);
			break;
		default:
			fprintf (fp, "      EX_???\n");
			break;
		}
		close_logfile (fp);
	}
#endif
	return;
}
