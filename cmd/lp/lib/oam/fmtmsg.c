/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/oam/fmtmsg.c	1.8.3.1"
/* LINTLIBRARY */

#include "stdio.h"
#include "string.h"

#include "oam.h"

/*
 * Define the following if you want the ID tags to follow the message.
 * We don't currently because (1) they're obnoxious, and (2) are really
 * meaningless as they stand (in SVR3.2). Also, they change as we add
 * messages (this is a fault of the implementation), and they aren't
 * likely to track into SVR4.0.
 */
/* #define OBNOXIOUS	/* */

static char		*severity_names[MAX_SEVERITY-MIN_SEVERITY+1] = {
	"HALT",
	"ERROR",
	"WARNING",
	"INFO"
};

static char		*TOFIX	= "TO FIX";

#if	defined(__STDC__)
static int		wrap ( char * , char * , int , char * , int , int );
#else
static int		wrap();
#endif

/**
 ** fmtmsg()
 **/

int
#if	defined(__STDC__)
fmtmsg (
	long			class,
	char *			label,
	int			severity,
	char *			text,
	char *			action,
	char *			tag
)
#else
fmtmsg ( class, label, severity, text, action, tag )
	long			class;
	char			*label;
	int			severity;
	char			*text;
	char			*action;
	char			*tag;
#endif
{
	int			label_len	= strlen(label),
				tofix_len	= strlen(TOFIX),
				severity_len,
				max_len,
				n,
				nchars,
				have_act;

	char			*severity_name,
				*source;

/*
 * DON'T MESS WITH THE CONTENT OF STRINGS!
 */
	/*
	 * The requirements don't say to return -1 if the
	 * severity isn't recognized, but we do.
	 */
	if (severity < MIN_SEVERITY || MAX_SEVERITY < severity)
		return (-1);
	else {
		severity_name = severity_names[severity];
		severity_len = strlen(severity_name);
	}

	/*
	 * The requirements don't say to return -1 if the
	 * label isn't structured correctly, but we do.
	 */
	if (!(source = strchr(label, ':')))
		return (-1);
	source++;

	max_len = label_len + severity_len + 2;
	if (max_len < tofix_len)
		max_len = tofix_len;

	have_act = (action && *action);

	if ((n = wrap(label, severity_name, max_len, text, 50, !have_act)) <= 0)
		return (-1);
	else
		nchars = n;

	if (have_act) {
		if (fputc('\n', stderr) == EOF)
			return (-1);
		else
			nchars++;

		if ((n = wrap(TOFIX, "", max_len, action, 40, 1)) <= 0)
			return (-1);
		else
			nchars += n;
	}

#if	defined(OBNOXIOUS)
	n = fprintf(
		stderr,
		"%.*s%s%s\n",
		source - label - 1,
		label,
		source,
		tag
	);
#else
	n = fprintf(stderr, "\n");
#endif
	if (n <= 0)
		return (-1);
	else
		nchars += n;

	fflush (stderr);

	return (nchars);
}

/**
 ** wrap() - PUT OUT "STUFF: string", WRAPPING string AS REQUIRED
 **/

static int
#if	defined(__STDC__)
wrap (
	char *			prefix,
	char *			suffix,
	int			max_len,
	char *			str,
	int			str_brk,
	int			tag
)
#else
wrap (prefix, suffix, max_len, str, str_brk, tag)
	char			*prefix,
				*suffix;
	int			max_len;
	char			*str;
	int			str_brk,
				tag;
#endif
{
	int			len,
				n,
				nchars;

	char			*p;

#if	defined(SMART_WRAP)
	char			*pw;
#endif

	/*
	 * Display the initial stuff followed by a colon.
	 */
	if ((len = strlen(suffix)))
		n = fprintf(
			stderr,
			"%*s: %s: ",
			max_len - len - 2,
			prefix,
			suffix
		);
	else
		n = fprintf(stderr, "%*s: ", max_len, prefix);
	if (n <= 0)
		return (-1);
	nchars = n;

	/*
	 * Loop once for each line of the string to display.
	 */
	for (p = str; *p; ) {

		/*
		 * Display the next "len" bytes of the string, where
		 * "len" is the smallest of:
		 *
		 *	- "str_brk"
		 *	- # bytes before control character
		 *	- # bytes left in string
		 *
#if	defined(SMART_WRAP)
		 * Modify "len" to avoid splitting ``small'' words.
#endif
		 */

		len = strcspn(p, "\r\n");
		if (len > str_brk) {

			len = str_brk;
#if	defined(SMART_WRAP)
			/*
			 * Don't split ``small'' words or before the
			 * last ``few'' characters of any word.
			 */
# define SMART_SMALL	4
# define SMART_FEW	0
			for (
				pw = p + len - 1;
				pw >= p && *pw != ' ';
				pw--
			)
				;
			if (
				strcspn(++pw, " ") <= (size_t) SMART_SMALL
			     || strcspn(p + len, " ") <= (size_t) SMART_FEW
			)
				len = pw - p;
#endif

		}

		n = fprintf(stderr, "%.*s", len, p);
		if (n <= 0)
			return (-1);
		nchars += n;

		/*
		 * If we displayed up to a control character,
		 * put out the control character now; otherwise,
		 * put out a newline unless we've put out all
		 * the text.
		 */
		p += len;
		if (*p == '\r' || *p == '\n') {

			if (fputc(*p, stderr) == EOF)
				return (-1);
			else
				nchars++;
			p++;

		} else if (*p) {

			if (fputc('\n', stderr) == EOF)
				return (-1);
			else
				nchars++;

		} else if (tag) {

#if	defined(OBNOXIOUS)
#define LEAVE_ROOM 3
			while (len++ < str_brk - LEAVE_ROOM)
				if (fputc(' ', stderr) == EOF)
					return (-1);
				else
					nchars++;
#endif

		}

		/*
		 * If the loop won't end this time (because we
		 * have more stuff to display) put out leading
		 * blanks to align the next line with the previous
		 * lines.
		 */
		if (*p) {
			for (n = -2; n < max_len; n++)
				if (fputc(' ', stderr) == EOF)
					return (-1);
			nchars += max_len + 2;
		}

	}

	return (nchars);
}
