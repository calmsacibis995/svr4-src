/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/forms/wrform.c	1.8.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"
#include "sys/stat.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"
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

#if	defined(__STDC__)
static void	print_sdn (FILE *, char *, SCALED);
static void	print_str (FILE *, char *, char *);
#else
static void	print_sdn();
static void	print_str();
#endif

/**
 ** wrform()
 **/

int
#if	defined(__STDC__)
wrform (
	char *			name,
	FORM *			formp,
	FILE *			fp,
	int			(*error_handler)( int , int , int ),
	int *			which_set
)
#else
wrform (name, formp, fp, error_handler, which_set)
	char *			name;
	FORM *			formp;
	FILE *			fp;
	int			(*error_handler)();
	int *			which_set;
#endif
{
	int			fld;

	char *			cp;


	for (fld = 0; fld < FO_MAX; fld++)
	  if (
		(!which_set || which_set[fld])
	     && (formheadings[fld].infile || error_handler)
	  ) switch (fld) {

#define HEAD	formheadings[fld].v

		case FO_PLEN:
			print_sdn (fp, HEAD, formp->plen);
			break;

		case FO_PWID:
			print_sdn (fp, HEAD, formp->pwid);
			break;

		case FO_LPI:
			print_sdn (fp, HEAD, formp->lpi);
			break;

		case FO_CPI:
			if (formp->cpi.val == N_COMPRESSED)
				print_str (fp, HEAD, NAME_COMPRESSED);
			else
				print_sdn (fp, HEAD, formp->cpi);
			break;

		case FO_NP:
			fprintf (fp, "%s %d\n", HEAD, formp->np);
			break;

		case FO_CHSET:
			fprintf (fp, "%s %s", HEAD, formp->chset);
			if (formp->mandatory == 1)
				fprintf (fp, ",%s", MANSTR);
			fprintf (fp, "\n");
			break;

		case FO_RCOLOR:
			print_str (fp, HEAD, formp->rcolor);
			break;

		case FO_CMT:
			if ((cp = formp->comment) && *cp) {
				fprintf (fp, "%s\n", HEAD);
				do {
					char *	nl = strchr(cp, '\n');

					if (nl)
						*nl = 0;
					if (_search_fheading(cp) < FO_MAX)
						putc ('>', fp);
					fputs (cp, fp);
					putc ('\n', fp);
					if (nl)
						*nl = '\n';
					cp = nl;
				} while (cp++);	/* NOT *cp++ */
			}
			break;

		case FO_ALIGN:
			print_str (fp, HEAD, formp->conttype);
			/*
			 * Actual alignment pattern has to be written
			 * out by caller; we leave the file pointer ready.
			 */
			break;

		}

	if (ferror(fp))
		return (-1);

	/*
	 * Write out comment to a separate file (?)
	 */
	if (!error_handler) {

		char *			path;


		if (!(path = getformfile(name, COMMENT)))
			return (-1);

		if (formp->comment) {
			if (dumpstring(path, formp->comment) == -1) {
				Free (path);
				return (-1);
			}

		} else
			Unlink (path);

		Free (path);

	}

	return (0);
}

/**
 ** print_sdn() - PRINT SCALED DECIMAL NUMBER WITH HEADER
 ** print_str() - PRINT STRING WITH HEADER
 **/

static void
#if	defined(__STDC__)
print_sdn (
	FILE *			fp,
	char *			head,
	SCALED			sdn
)
#else
print_sdn (fp, head, sdn)
	FILE			*fp;
	char			*head;
	SCALED			sdn;
#endif
{
	if (sdn.val <= 0)
		return;

	(void)fprintf (fp, "%s ", head);
	printsdn (fp, sdn);

	return;
}

static void
#if	defined(__STDC__)
print_str (
	FILE *			fp,
	char *			head,
	char *			str
)
#else
print_str (fp, head, str)
	FILE			*fp;
	char			*head,
				*str;
#endif
{
	if (!str || !*str)
		return;

	(void)fprintf (fp, "%s %s\n", head, str);

	return;
}
