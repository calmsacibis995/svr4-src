/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/forms/rdform.c	1.17.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "form.h"

extern struct {
	char			*v;
	short			len;
	short			infile;
}			formheadings[];

#if	defined(__STDC__)
int		_search_fheading ( char * );
#else
int		_search_fheading();
#endif

int
#if	defined(__STDC__)
rdform (
	char *			name,
	FORM *			formp,
	FILE *			fp,
	int			(*error_handler)( int , int , int ),
	int *			which_set
)
#else
rdform (name, formp, fp, error_handler, which_set)
	char *			name;
	FORM *			formp;
	FILE *			fp;
	int			(*error_handler)();
	int *			which_set;
#endif
{
	char			buf[BUFSIZ];

	char *			rest;
	char *			mandp;
	char *			here;

	int			fld;
	int			have_line_already;
	int			found_alignment_pattern;
	int			size;
	int			add_size;
	int			linenum;
	int			i;

	SCALED			sdn;

	register char *		p;


	/*
	 * Initialize the entire structure, to ensure no random
	 * values get in it. However, make sure some values won't
	 * be null or empty. Do the latter here as opposed to
	 * after reading the file, because sometimes the file
	 * contains an empty header to FORCE a null/empty value.
	 */
	(void)memset ((char *)formp, 0, sizeof(*formp));
	formp->name = Strdup(name);
	formp->plen.val = DPLEN;
	formp->plen.sc = 0;
	formp->pwid.val = DPWIDTH;
	formp->pwid.sc = 0;
	formp->lpi.val = DLPITCH;
	formp->lpi.sc = 0;
	formp->cpi.val = DCPITCH;
	formp->cpi.sc = 0;
	formp->np = DNP;
	formp->chset = Strdup(DCHSET);
	formp->mandatory = 0;
	formp->rcolor = Strdup(DRCOLOR);
	formp->conttype = Strdup(DCONTYP);

	/*
	 * Read the file.
	 */

#define FGETS(B,S,F)	(linenum++, fgets(B,S,F))

	have_line_already = 0;
	found_alignment_pattern = 0;
	linenum = 0;
	while (
		!feof(fp)
	     && !found_alignment_pattern
	     && (have_line_already || FGETS(buf, BUFSIZ, fp))
	) {

		int			pos	= strlen(buf) - 1;


		have_line_already = 0;

		while (isspace(buf[pos]))
			buf[pos--] = 0;

		fld = _search_fheading(buf);
		if (fld >= FO_MAX) {
			lp_errno = LP_EBADHDR; 
BadFile:		errno = EBADF;
			if (error_handler) {
				if ((*error_handler)(errno, lp_errno, linenum) == -1)
					return (-1);
				continue;
			} else {
				/*
				 * To allow future extensions to not
				 * impact applications using old versions
				 * of this routine, ignore strange fields.
				 */
				continue;
			}
		}

		p = buf + formheadings[fld].len;
		while (isspace(*p))
			p++;

		if (which_set)
			which_set[fld] = 1;

		if (
			formheadings[fld].infile
		     || error_handler
		) switch (fld) {

		case FO_PLEN:
			sdn = getsdn(p);
			if (errno == EINVAL) {
				lp_errno = LP_EBADSDN;
				goto BadFile;
			}
			formp->plen = sdn;
			break;

		case FO_PWID:
			sdn = getsdn(p);
			if (errno == EINVAL) {
				lp_errno = LP_EBADSDN;
				goto BadFile;
			}
			formp->pwid = sdn;
			break;

		case FO_CPI:
			sdn = getcpi(p);
			if (errno == EINVAL) {
				lp_errno = LP_EBADSDN;
				goto BadFile;
			}
			formp->cpi = sdn;
			break;

		case FO_LPI:
			sdn = getsdn(p);
			if (errno == EINVAL) {
				lp_errno = LP_EBADSDN;
				goto BadFile;
			}
			formp->lpi = sdn;
			break;

		case FO_NP:
			if (
				(i = strtol(p, &rest, 10)) <= 0
			     || *rest
			) {
				lp_errno = LP_EBADINT;
				goto BadFile;
			}
			formp->np = i;
			break;

		case FO_CHSET:
			if (!(mandp = strchr(p, ',')))
				formp->mandatory = 0;
			else {
				do
					*mandp++ = 0;
				while (*mandp && isspace(*mandp));
				if (CS_STREQU(MANSTR, mandp))
					formp->mandatory = 1;
				else {
					lp_errno = LP_EBADARG;
					goto BadFile;
				}
			}
			if (!syn_name(p)) {
				lp_errno = LP_EBADNAME;
				goto BadFile;
			}
			if (formp->chset)
				Free (formp->chset);
			formp->chset = Strdup(p);
			break;

		case FO_RCOLOR:
			if (formp->rcolor)
				Free (formp->rcolor);
			formp->rcolor = Strdup(p);
			break;

		case FO_CMT:
			if (*p) {
				lp_errno = LP_ETRAILIN;
				goto BadFile;
			}
			if (formp->comment)
				Free (formp->comment);
			formp->comment = 0;
			size = 0;
			while (FGETS(buf, BUFSIZ, fp)) {
				p = buf;

				/*
				 * A recognized header ends the comment.
				 */
				if (_search_fheading(p) < FO_MAX) {
					have_line_already = 1;
					break;
				}

				/*
				 * On the other hand, a '>' may hide what
				 * would otherwise look like a header.
				 */
				if (
					p[0] == '>'
				     && _search_fheading(p+1) < FO_MAX
				)
					p++;

				/*
				 * (Re)allocate space to hold this
				 * (additional) line of the comment.
				 */
				add_size = strlen(p);
				if (formp->comment)
					formp->comment = Realloc(
						formp->comment,
						size + add_size + 1
					);
				else
					formp->comment = Malloc(
						size + add_size + 1
					);
				if (!formp->comment) {
					freeform (formp);
					close_lpfile (fp);
					errno = ENOMEM;
					return (-1);
				}

				/*
				 * Copy this (additional) line of the
				 * comment to the allocated space. "here"
				 * points to where to copy the line.
				 */
				strcpy (formp->comment + size, p);
				size += add_size;
			}
			if (ferror(fp))
				goto BadFile;

			/*
			 * The comment is held internally without a
			 * trailing newline.
			 */
			if (size && formp->comment[size - 1] == '\n')
				formp->comment[size - 1] = 0;

			break;

		case FO_ALIGN:
			if (*p) {
				if (!syn_type(p)) {
					lp_errno = LP_EBADCTYPE;
					goto BadFile;
				}
				if (formp->conttype)
					Free (formp->conttype);
				formp->conttype = Strdup(p);
			}

			/*
			 * Actual alignment pattern has to be read in
			 * by caller; we leave the file pointer ready.
			 */
			found_alignment_pattern = 1;
			break;

		}

	}
	if (ferror(fp)) {
		int			save_errno = errno;

		freeform (formp);
		errno = save_errno;
		return (-1);
	}

	/*
	 * Get the form description (if it exists) (?)
	 */
	if (!error_handler) {

		char *			path;


		if (!(path = getformfile(name, COMMENTFILE))) {
			freeform (formp);
			errno = ENOMEM;
			return (-1);
		}
		if (
			!(formp->comment = loadstring(path))
		     && errno != ENOENT
		) {
			Free (path);
			freeform (formp);
			return (-1);
		}
		Free (path);
	}

	return (0);
}
