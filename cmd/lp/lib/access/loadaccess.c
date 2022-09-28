/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/access/loadaccess.c	1.9.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"
#include "access.h"

#if	defined(__STDC__)
static char		**_loadaccess ( char * );
#else
static char		**_loadaccess();
#endif

/**
 ** load_userform_access() - LOAD ALLOW/DENY LISTS FOR USER+FORM
 **/

int
#if	defined(__STDC__)
load_userform_access (
	char *			form,
	char ***		pallow,
	char ***		pdeny
)
#else
load_userform_access (form, pallow, pdeny)
	char			*form,
				***pallow,
				***pdeny;
#endif
{
	return (loadaccess(Lp_A_Forms, form, "", pallow, pdeny));
}

/**
 ** load_userprinter_access() - LOAD ALLOW/DENY LISTS FOR USER+PRINTER
 **/

int
#if	defined(__STDC__)
load_userprinter_access (
	char *			printer,
	char ***		pallow,
	char ***		pdeny
)
#else
load_userprinter_access (printer, pallow, pdeny)
	char			*printer,
				***pallow,
				***pdeny;
#endif
{
	return (loadaccess(Lp_A_Printers, printer, UACCESSPREFIX, pallow, pdeny));
}

/**
 ** load_formprinter_access() - LOAD ALLOW/DENY LISTS FOR FORM+PRINTER
 **/

int
#if	defined(__STDC__)
load_formprinter_access (
	char *			printer,
	char ***		pallow,
	char ***		pdeny
)
#else
load_formprinter_access (printer, pallow, pdeny)
	char			*printer,
				***pallow,
				***pdeny;
#endif
{
	return (loadaccess(Lp_A_Printers, printer, FACCESSPREFIX, pallow, pdeny));
}

/**
 ** loadaccess() - LOAD ALLOW OR DENY LISTS
 **/

int
#if	defined(__STDC__)
loadaccess (
	char *			dir,
	char *			name,
	char *			prefix,
	char ***		pallow,
	char ***		pdeny
)
#else
loadaccess (dir, name, prefix, pallow, pdeny)
	char			*dir,
				*name,
				*prefix,
				***pallow,
				***pdeny;
#endif
{
	register char		*allow_file	= 0,
				*deny_file	= 0;

	int			ret;

	if (
		!(allow_file = getaccessfile(dir, name, prefix, ALLOWFILE))
	     || !(*pallow = _loadaccess(allow_file)) && errno != ENOENT
	     || !(deny_file = getaccessfile(dir, name, prefix, DENYFILE))
	     || !(*pdeny = _loadaccess(deny_file)) && errno != ENOENT
	)
		ret = -1;
	else
		ret = 0;

	if (allow_file)
		Free (allow_file);
	if (deny_file)
		Free (deny_file);

	return (ret);
}

/**
 ** _loadaccess() - LOAD ALLOW OR DENY FILE
 **/

static char **
#if	defined(__STDC__)
_loadaccess (
	char *			file
)
#else
_loadaccess (file)
	char			*file;
#endif
{
	register size_t		nalloc,
				nlist;

	register char		**list;

	FILE			*fp;

	char			buf[BUFSIZ];


	if (!(fp = open_lpfile(file, "r", 0)))
		return (0);

	/*
	 * Preallocate space for the initial list. We'll always
	 * allocate one more than the list size, for the terminating null.
	 */
	nalloc = ACC_MAX_GUESS;
	list = (char **)Malloc((nalloc + 1) * sizeof(char *));
	if (!list) {
		close_lpfile (fp);
		errno = ENOMEM;
		return (0);
	}

	for (nlist = 0; fgets(buf, BUFSIZ, fp); ) {

		buf[strlen(buf) - 1] = 0;

		/*
		 * Allocate more space if needed.
		 */
		if (nlist >= nalloc) {
			nalloc += ACC_MAX_GUESS;
			list = (char **)Realloc(
				(char *)list,
				(nalloc + 1) * sizeof(char *)
			);
			if (!list) {
				close_lpfile (fp);
				return (0);
			}
		}

		list[nlist] = Strdup(buf);   /* if fail, minor problem */
		list[++nlist] = 0;

	}
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		freelist (list);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	/*
	 * If we have more space allocated than we need,
	 * return the extra.
	 */
	if (nlist != nalloc) {
		list = (char **)Realloc(
			(char *)list,
			(nlist + 1) * sizeof(char *)
		);
		if (!list) {
			errno = ENOMEM;
			return (0);
		}
	}
	list[nlist] = 0;

	return (list);
}
