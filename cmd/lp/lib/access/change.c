/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/access/change.c	1.8.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"
#include "access.h"

#if	defined(__STDC__)

static int		chgaccess ( int , char ** , char * , char * , char * );
static char **		empty_list ( void );

#else

static int		chgaccess();
static char		**empty_list();

#endif

/**
 ** deny_user_form() - DENY USER ACCESS TO FORM
 **/

int
#if	defined(__STDC__)
deny_user_form (
	char **			user_list,
	char *			form
)
#else
deny_user_form (user_list, form)
	char			**user_list,
				*form;
#endif
{
	return (chgaccess(0, user_list, form, Lp_A_Forms, ""));
}

/**
 ** allow_user_form() - ALLOW USER ACCESS TO FORM
 **/

int
#if	defined(__STDC__)
allow_user_form (
	char **			user_list,
	char *			form
)
#else
allow_user_form (user_list, form)
	char			**user_list,
				*form;
#endif
{
	return (chgaccess(1, user_list, form, Lp_A_Forms, ""));
}

/**
 ** deny_user_printer() - DENY USER ACCESS TO PRINTER
 **/

int
#if	defined(__STDC__)
deny_user_printer (
	char **			user_list,
	char * 			printer
)
#else
deny_user_printer (user_list, printer)
	char			**user_list,
				*printer;
#endif
{
	return (chgaccess(0, user_list, printer, Lp_A_Printers, UACCESSPREFIX));
}

/**
 ** allow_user_printer() - ALLOW USER ACCESS TO PRINTER
 **/

int
#if	defined(__STDC__)
allow_user_printer (
	char **			user_list,
	char *			printer
)
#else
allow_user_printer (user_list, printer)
	char			**user_list,
				*printer;
#endif
{
	return (chgaccess(1, user_list, printer, Lp_A_Printers, UACCESSPREFIX));
}

/**
 ** deny_form_printer() - DENY FORM USE ON PRINTER
 **/

int
#if	defined(__STDC__)
deny_form_printer (
	char **			form_list,
	char *			printer
)
#else
deny_form_printer (form_list, printer)
	char			**form_list,
				*printer;
#endif
{
	return (chgaccess(0, form_list, printer, Lp_A_Printers, FACCESSPREFIX));
}

/**
 ** allow_form_printer() - ALLOW FORM USE ON PRINTER
 **/

int
#if	defined(__STDC__)
allow_form_printer (
	char **			form_list,
	char *			printer
)
#else
allow_form_printer (form_list, printer)
	char			**form_list,
				*printer;
#endif
{
	return (chgaccess(1, form_list, printer, Lp_A_Printers, FACCESSPREFIX));
}

/**
 ** chgaccess() - UPDATE ALLOW/DENY ACCESS OF ITEM TO RESOURCE
 **/

static int
#if	defined(__STDC__)
chgaccess (
	int			isallow,
	char **			list,
	char *			name,
	char *			dir,
	char *			prefix
)
#else
chgaccess (isallow, list, name, dir, prefix)
	int			isallow;
	char			**list,
				*name,
				*dir,
				*prefix;
#endif
{
	register char		***padd_list,
				***prem_list,
				**pl;

	char			**allow_list,
				**deny_list;

	if (loadaccess(dir, name, prefix, &allow_list, &deny_list) == -1)
		return (-1);

	if (isallow) {
		padd_list = &allow_list;
		prem_list = &deny_list;
	} else {
		padd_list = &deny_list;
		prem_list = &allow_list;
	}

	for (pl = list; *pl; pl++) {

		/*
		 * Do the ``all'' and ``none'' cases explicitly,
		 * so that we can clean up the lists nicely.
		 */
		if (STREQU(*pl, NAME_NONE)) {
			isallow = !isallow;
			goto AllCase;
		}
		if (
			STREQU(*pl, NAME_ALL)
		     || STREQU(*pl, NAME_ANY)
		     || STREQU(*pl, ALL_BANG_ALL)
		) {
AllCase:
			freelist (allow_list);
			freelist (deny_list);
			if (isallow) {
				allow_list = 0;
				deny_list = empty_list();
			} else {
				allow_list = 0;
				deny_list = 0;
			}
			break;

		} else {

			/*
			 * For each regular item in the list,
			 * we add it to the ``add list'' and remove it
			 * from the ``remove list''. This is not
			 * efficient, especially if there are a lot of
			 * items in the caller's list; doing it the
			 * way we do, however, has the side effect
			 * of skipping duplicate names in the caller's
			 * list.
			 *
			 * Do a regular "addlist()"--the resulting
			 * list may have redundancies, but it will
			 * still be correct.
			 */
			if (addlist(padd_list, *pl) == -1)
				return (-1);
			if (bang_dellist(prem_list, *pl) == -1)
				return (-1);

		}

	}

	return (dumpaccess(dir, name, prefix, &allow_list, &deny_list));
}

/**
 ** empty_list() - CREATE AN EMPTY LIST
 **/

static char **
#if	defined(__STDC__)
empty_list (
	void
)
#else
empty_list ()
#endif
{
	register char		**empty;


	if (!(empty = (char **)Malloc(sizeof(char *)))) {
		errno = ENOMEM;
		return (0);
	}
	*empty = 0;
	return (empty);
}
