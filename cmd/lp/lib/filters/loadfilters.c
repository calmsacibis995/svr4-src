/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/loadfilters.c	1.11.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "stdlib.h"

#include "lp.h"
#include "filters.h"

_FILTER			*filters;

size_t			nfilters;

#if	defined(__STDC__)
static int		getfields ( FILE * , char *[] , char * , int , int , char * );
static int		fs_cmp ( const void * , const void * );
#else
static int		getfields(),
			fs_cmp();
#endif

/**
 ** loadfilters() - READ FILTERS FROM FILTER TABLE INTO INTERNAL STRUCTURE
 **/

int
#if	defined(__STDC__)
loadfilters (
	char *			file
)
#else
loadfilters (file)
	char			*file;
#endif
{
	register _FILTER	*pf;

	FILE			*fp;

	char			*filt[FL_MAX],
				buf[BUFSIZ];

	size_t			nalloc;

	if (filters) {
		nalloc = nfilters;
		trash_filters ();
	} else
		nalloc = FL_MAX_GUESS;

	if (!(fp = open_filtertable(file, "r")))
		return (-1);

	/*
	 * Preallocate space for the internal filter table.
	 * Our guess is the number of filters previously read in,
	 * if any have been read in before (see above).
	 */
	filters = (_FILTER *)Malloc((nalloc + 1) * sizeof(_FILTER));
	if (!filters) {
		close_filtertable (fp);
		errno = ENOMEM;
		return (-1);
	}

	for (
		pf = filters, nfilters = 0;
		getfields(fp, filt, buf, BUFSIZ, FL_MAX, FL_SEP) != -1;
		pf++
	) {

		char			**list;

		/*
		 * Allocate more space if needed.
		 */
		if (++nfilters > nalloc) {
			nalloc = nfilters;
			filters = (_FILTER *)Realloc(
				filters,
				(nalloc + 1) * sizeof(_FILTER)
			);
			if (!filters) {
				close_filtertable (fp);
				errno = ENOMEM;
				return (-1);
			}
			pf = &filters[nfilters - 1];
		}

#define DFLT(X)	(filt[X] && *filt[X]? filt[X] : NAME_ANY)

		pf->name = Strdup(filt[FL_NAME]);
		pf->type = s_to_filtertype(filt[FL_TYPE]);
		pf->command = Strdup(filt[FL_CMD]);

		pf->printers = getlist(DFLT(FL_PRTRS), LP_WS, LP_SEP);

		list = getlist(DFLT(FL_PTYPS), LP_WS, LP_SEP);
		pf->printer_types = sl_to_typel(list);
		freelist (list);

		list = getlist(DFLT(FL_ITYPS), LP_WS, LP_SEP);
		pf->input_types = sl_to_typel(list);
		freelist (list);

		list = getlist(DFLT(FL_OTYPS), LP_WS, LP_SEP);
		pf->output_types = sl_to_typel(list);
		freelist (list);

		/*
		 * Note the use of "" instead of LP_WS. The
		 * "sl_to_templatel()" routine will take care
		 * of stripping leading blanks. Stripping trailing
		 * blanks would be nice but shouldn't matter.
		 */

/* quote reason #3 (in "getlist()") */
		list = getlist(filt[FL_TMPS], "", LP_SEP);

/* quote reason #4 (in "s_to_template()") */
		pf->templates = sl_to_templatel(list);
		freelist (list);

	}
	if (ferror(fp)) {
		int			save_errno = errno;

		free_filter (pf);
		close_filtertable (fp);
		errno = save_errno;
		return (-1);
	}
	close_filtertable (fp);

	/*
	 * If we have more space allocated than we need,
	 * return the extra.
	 */
	if (nfilters != nalloc) {
		filters = (_FILTER *)Realloc(
			filters,
			(nfilters + 1) * sizeof(_FILTER)
		);
		if (!filters) {
			errno = ENOMEM;
			return (-1);
		}
	}
	filters[nfilters].name = 0;

	/*
	 * Sort the filters, putting ``fast'' filters before
	 * ``slow'' filters. This preps the list for "insfilter()"
	 * so that it can easily pick fast filters over otherwise
	 * equivalent slow filters. This sorting is done every
	 * time we read in the table; one might think that if
	 * "putfilter()" would insert in the correct order then
	 * the table, when written out to disk, would be sorted
	 * already--removing the need to sort it here. We don't
	 * take that approach, because (1) sorting it isn't that
	 * expensive and (2) someone might tamper with the table
	 * file.
	 */
	qsort ((char *)filters, nfilters, sizeof(_FILTER), fs_cmp);

	return (0);
}

/**
 ** getfields() - PARSE NON-COMMENT LINE FROM FILE INTO FIELDS
 **/

static int
#if	defined(__STDC__)
getfields (
	FILE *			fp,
	char *			fields[],
	char *			buf,
	int			bufsiz,
	int			max,
	char *			seps
)
#else
getfields (fp, fields, buf, bufsiz, max, seps)
	FILE			*fp;
	char			*fields[];
	register char		*buf;
	int			bufsiz,
				max;
	register char		*seps;
#endif
{
	register char		*p,
				*q;

	register int		n	= 0;

	while (fgets(buf, bufsiz, fp) != NULL) {
		buf[strlen(buf) - 1] = 0;
		p = buf + strspn(buf, " \t");
		if (*p && *p != '#') {
			for (fields[n++] = q = p; *p; ) {
				if (*p == '\\') {
					if (
/* quote reason #1 */				p[1] == '\\'
/* quote reason #2 */			     || strchr(seps, p[1])
					)
						p++;
					*q++ = *p++;
				} else if (strchr(seps, *p)) {
					*q++ = 0;
					p++;
					if (n < max)
						fields[n++] = q;
				} else
					*q++ = *p++;
			}
			*q = 0;
			while (n < max)
				fields[n++] = "";
			return (n);
		}
	}
	return (-1);
}

/**
 ** fs_cmp() - COMPARE TWO FILTERS BY "FILTERTYPE"
 **/

static int
#if	defined(__STDC__)
fs_cmp (
	const void *		pfa,
	const void *		pfb
)
#else
fs_cmp (pfa, pfb)
	char			*pfa,
				*pfb;
#endif
{
	if (((_FILTER *)pfa)->type == ((_FILTER *)pfb)->type)
		return (0);
	else if (((_FILTER *)pfa)->type == fl_fast)
		return (-1);
	else
		return (1);
}
