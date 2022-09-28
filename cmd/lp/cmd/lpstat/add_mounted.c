/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/add_mounted.c	1.8.3.1"

#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "printers.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


#define nuther_mount()	(MOUNTED *)Malloc(sizeof(MOUNTED))

static MOUNTED		mounted_forms_head	= { 0, 0, 0 },
			mounted_pwheels_head	= { 0, 0, 0 };

MOUNTED			*mounted_forms		= &mounted_forms_head,
			*mounted_pwheels	= &mounted_pwheels_head;

#if	defined(__STDC__)
static void	insert_mounted ( MOUNTED * , char * , char * , char * );
static int	cs_addlist ( char *** , char * );
#else
static void	insert_mounted();
static int	cs_addlist();
#endif

/**
 ** add_mounted() - ADD A FORM, PRINTWHEEL TO LIST
 **/

void
#if	defined(__STDC__)
add_mounted (
	char *			printer,
	char *			form,
	char *			pwheel
)
#else
add_mounted (printer, form, pwheel)
	char			*printer,
				*form,
				*pwheel;
#endif
{
	register PRINTER	*pp;

	register char		**pcs;


	/*
	 * Add this form to the list of mounted forms.
	 */
	if (form && *form)
		insert_mounted (
			mounted_forms,
			form,
			printer,
			(char *)0
		);

	/*
	 * Add this print wheel to the list of mounted print
	 * wheels.
	 */
	if (pwheel && *pwheel)
		insert_mounted (
			mounted_pwheels,
			pwheel,
			printer,
			"!"
		);

	if (!(pp = getprinter(printer))) {
		LP_ERRMSG2 (ERROR, E_LP_GETPRINTER, printer, PERROR);
		done(1);
	}

	/*
	 * Add the list of administrator supplied character-set
	 * or print wheel aliases to the list.
	 */
	if (pp->char_sets)
		for (pcs = pp->char_sets; *pcs; pcs++)
			if (pp->daisy)
				insert_mounted (
					mounted_pwheels,
					*pcs,
					printer,
					(char *)0
				);

			else {
				register char		*terminfo_name,
							*alias;

				if (
					(terminfo_name = strtok(*pcs, "="))
				     && (alias = strtok((char *)0, "="))
				)
					insert_mounted (
						mounted_pwheels,
						alias,
						printer,
						terminfo_name
					);
			}

	/*
	 * Do the aliases built into the Terminfo database for
	 * this printer's type. This applies only to printers
	 * that have defined character sets.
	 */
	if ((pcs = get_charsets(pp, 1)))
		for ( ; *pcs; pcs++) {
			register char		*terminfo_name,
						*alias;

			if (
				(terminfo_name = strtok(*pcs, "="))
			     && (alias = strtok((char *)0, "="))
			)
				insert_mounted (
					mounted_pwheels,
					alias,
					printer,
					terminfo_name
				);
		}


	return;
}

/**
 ** insert_mounted()
 **/

static void
#if	defined(__STDC__)
insert_mounted (
	MOUNTED *		ml,
	char *			name,
	char *			printer,
	char *			tag
)
#else
insert_mounted (ml, name, printer, tag)
	MOUNTED			*ml;
	char			*name,
				*printer,
				*tag;
#endif
{
	register MOUNTED	*pm;

	register char		*nm,
				*pnm;


	/*
	 * For forms: Each printer that has the form mounted is
	 * marked with a leading '!'.
	 * For print wheels and character sets: Each character
	 * set name is marked with a leading '!'; each printer
	 * that has a print wheel mounted is marked with a leading '!'.
	 */

	if (tag && tag[0] != '!') {
		nm = Malloc(1 + strlen(name) + 1);
		nm[0] = '!';
		(void)strcpy (nm + 1, name);
	} else
		nm = name;

	for (
		pm = ml;
		pm->name && !STREQU(pm->name, nm);
		pm = pm->forward
	)
		;

	if (tag && tag[0] == '!') {
		pnm = Malloc(1 + strlen(printer) + 1);
		pnm[0] = '!';
		(void)strcpy (pnm + 1, printer);

	} else if (tag) {
		pnm = Malloc(strlen(printer) + 1 + strlen(tag) + 1);
		(void)sprintf (pnm, "%s=%s", printer, tag);

	} else
		pnm = printer;


	if (cs_addlist(&(pm->printers), pnm) == -1) {
		LP_ERRMSG (ERROR, E_LP_MALLOC);
		done (1);
	}

	if (!pm->name) {
		pm->name = strdup(nm);
		pm = (pm->forward = nuther_mount());
		pm->name = 0;
		pm->printers = 0;
		pm->forward = 0;
	}

	return;
}

/**
 ** cs_addlist()
 **/

static int
#if	defined(__STDC__)
cs_addlist (
	char ***		plist,
	char *			item
)
#else
cs_addlist (plist, item)
	register char		***plist,
				*item;
#endif
{
	register char		**pl;

	register int		n;

	if (*plist) {

		n = lenlist(*plist);

		for (pl = *plist; *pl; pl++)
			if (
				(*pl)[0] == '!' && STREQU((*pl)+1, item)
			     || STREQU(*pl, item)
			)
				break;

		if (!*pl) {

			n++;
			*plist = (char **)Realloc(
				(char *)*plist,
				(n + 1) * sizeof(char *)
			);
			(*plist)[n - 1] = strdup(item);
			(*plist)[n] = 0;

		}

	} else {

		*plist = (char **)Malloc(2 * sizeof(char *));
		(*plist)[0] = strdup(item);
		(*plist)[1] = 0;

	}

	return (0);
}
