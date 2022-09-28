/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/form.c	1.9.3.1"

#include "stdio.h"

#include "string.h"

#include "lp.h"
#include "form.h"
#include "access.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


#if	defined(__STDC__)
static void		putfline ( FORM * );
#else
static void		putfline();
#endif

/**
 ** do_form()
 **/

void
#if	defined(__STDC__)
do_form (
	char **			list
)
#else
do_form (list)
	char			**list;
#endif
{
	FORM			form;

	while (*list) {
		if (STREQU(NAME_ALL, *list))
			while (getform(NAME_ALL, &form, (FALERT *)0, (FILE **)0) != -1)
				putfline (&form);

		else if (getform(*list, &form, (FALERT *)0, (FILE **)0) != -1) {
			putfline (&form);

		} else {
			LP_ERRMSG1 (ERROR, E_LP_NOFORM, *list);
			exit_rc = 1;
		}

		list++;
	}
	printsdn_unsetup ();
	return;
}

/**
 ** putfline()
 **/

static void
#if	defined(__STDC__)
putfline (
	FORM *			pf
)
#else
putfline (pf)
	FORM			*pf;
#endif
{
	register MOUNTED	*pm;


	(void) printf("form %s", pf->name);

	(void) printf(
		" is %s to you",
is_user_allowed_form(getname(), pf->name) ? "available" : "not available"
	);

	for (pm = mounted_forms; pm->forward; pm = pm->forward)
		if (STREQU(pm->name, pf->name)) {
			if (pm->printers) {
				(void) printf(", mounted on ");
				printlist_setup (0, 0, ",", "");
				printlist (stdout, pm->printers);
				printlist_unsetup();
			}
			break;
		}

	(void) printf("\n");

	if (verbosity & V_LONG) {

		printsdn_setup ("\tPage length: ", 0, 0);
		printsdn (stdout, pf->plen);

		printsdn_setup ("\tPage width: ", 0, 0);
		printsdn (stdout, pf->pwid);

		(void) printf("\tNumber of pages: %d\n", pf->np);

		printsdn_setup ("\tLine pitch: ", 0, 0);
		printsdn (stdout, pf->lpi);

		(void) printf("\tCharacter pitch:");
		if (pf->cpi.val == N_COMPRESSED)
			(void) printf(" %s\n", NAME_COMPRESSED);
		else {
			printsdn_setup (" ", 0, 0);
			printsdn (stdout, pf->cpi);
		}

		(void) printf(
			"\tCharacter set choice: %s%s\n",
			(pf->chset? pf->chset : NAME_ANY),
			(pf->mandatory ? ",mandatory" : "")
		);

		(void) printf(
			"\tRibbon color: %s\n",
			(pf->rcolor? pf->rcolor : NAME_ANY)
		);

		if (pf->comment)
			(void) printf("\tComment:\n\t%s\n", pf->comment);

	}
	return;
}

