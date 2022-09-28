/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/access/bang.c	1.2.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/utsname.h"

#include "lp.h"

/*
 * The rules:
 *
 *	Key:	A - some system
 *		X - some user
 *
 *	X	all users named X from any system
 *	!X	the user named X from the local system
 *	A!X	the user named X from the system A
 *	all!X	all users named X from any system
 *	all	all users from any system
 *	!all	all users from the local system
 *	A!all	all users from the system A
 *	all!all	all users from any system
 *	!	all users from the local system
 *	A!	all users from the system A
 *	all!	all users from any system
 */

/*
 * See if two names match. We use a short-hand for the string
 * ``all''--a null pointer. So if either name is null, or if
 * they compare literally, they match.
 */
#define MATCH(A,B)	(!(A) || !(B) || STREQU((A), (B)))

/**
 ** bang_parse() - PARSE system!name INTO ITS CONSTITUENT PARTS
 ** bang_unparse() - UNDO WHAT bang_parse() DOES TO THE SOURCE STRING
 **/

#define bang_unparse(B)		if (B) (B)[0] = BANG_C; else

static void
#if	defined(__STDC__)
bang_parse (
	char *			src,
	char **			p_system,
	char **			p_bang,
	char **			p_name
)
#else
bang_parse (src, p_system, p_bang, p_name)
	char *			src;
	char **			p_system;
	char **			p_bang;
	char **			p_name;
#endif
{
	static char *		local	= 0;


	if (!local) {
		struct utsname		utsbuf;

		uname (&utsbuf);
		local = Strdup(utsbuf.nodename);
	}

	/*
	 * No allocation, no copying; just set pointers.
	 */
	*p_bang = strchr(src, BANG_C);
	if (*p_bang) {

		/*
		 * Got either "system!name" or "!name".
		 */
		if (*p_bang != src) {

			/*
			 * Got "system!name".
			 */
			*p_system = src;
			(*p_bang)[0] = 0;

		} else
			/*
			 * Got "!name".
			 */
			*p_system = local;

		*p_name = *p_bang + 1;

	} else {

		/*
		 * Got just "name", so "system" is ``all''.
		 */
		*p_system = 0;
		*p_name = src;
	}

	/*
	 * If either "system" or "name" are ``all'' or empty,
	 * set them to 0 as a short-hand for ``all'.
	 */

#define check_all_case(P) \
	if (*(P) && (!(*(P))[0] || STREQU(*(P), NAME_ALL))) *(P) = 0; else

	check_all_case (p_system);
	check_all_case (p_name);

	return;
}

/**
 ** bangequ() - LIKE STREQU, BUT HANDLES system!name CASES
 **/

int
#if	defined(__STDC__)
bangequ (
	char *			a,
	char *			b
)
#else
bangequ (a, b)
	char *			a;
	char *			b;
#endif
{
	int			ret;

	char *			a_system;
	char *			a_bang;
	char *			a_name;
	char *			b_system;
	char *			b_bang;
	char *			b_name;


	bang_parse (a, &a_system, &a_bang, &a_name);
	bang_parse (b, &b_system, &b_bang, &b_name);

	ret = (MATCH(a_system, b_system) && MATCH(a_name, b_name));

	bang_unparse (a_bang);
	bang_unparse (b_bang);

	return (ret);
}

/**
 ** bang_searchlist() - SEARCH (char **) LIST FOR "system!user" ITEM
 **/

int
#if	defined(__STDC__)
bang_searchlist (
	char *			item,
	char **			list
)
#else
bang_searchlist (item, list)
	register char		*item;
	register char		**list;
#endif
{
	if (!list || !*list)
		return (0);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (*list) {
		if (bangequ(item, *list))
			return (1);
		list++;
	}
	return (0);
}

/**
 ** bang_dellist() - REMOVE "system!name" ITEM FROM (char **) LIST
 **/

int
#if	defined(__STDC__)
bang_dellist (
	char ***		plist,
	char *			item
)
#else
bang_dellist (plist, item)
	register char		***plist,
				*item;
#endif
{
	register char **	pl;
	register char **	ql;

	register int		n;

				/*
				 * "hole" is a pointer guaranteed not
				 * to point to anyplace malloc'd.
				 */
	char *			hole	= "";


	/*
	 * There are two ways this routine is different from the
	 * regular "dellist()" routine: First, the items are of the form
	 * ``system!name'', which means there is a two part matching
	 * for ``all'' cases (all systems and/or all names). Second,
	 * ALL matching items in the list are deleted.
	 *
	 * Now suppose the list contains just the word ``all'', and
	 * the item to be deleted is the name ``fred''. What will
	 * happen? The word ``all'' will be deleted, leaving the list
	 * empty (null)! This may sound odd at first, but keep in mind
	 * that this routine is paired with the regular "addlist()"
	 * routine; the item (``fred'') is ADDED to an opposite list
	 * (we are either deleting from a deny list and adding to an allow
	 * list or vice versa). So, to continue the example, if previously
	 * ``all'' were allowed, removing ``fred'' from the allow list
	 * does indeed empty that list, but then putting him in the deny
	 * list means only ``fred'' is denied, which is the effect we
	 * want.
	 */

	if (*plist) {

		for (pl = *plist; *pl; pl++)
			if (bangequ(item, *pl)) {
				Free (*pl);
				*pl = hole;
			}

		for (n = 0, ql = pl = *plist; *pl; pl++)
			if (*pl != hole) {
				*ql++ = *pl;
				n++;
			}

		if (n == 0) {
			Free ((char *)*plist);
			*plist = 0;
		} else {
			*plist = (char **)Realloc(
				(char *)*plist,
				(n + 1) * sizeof(char *)
			);
			if (!*plist)
				return (-1);
			(*plist)[n] = 0;
		}
	}

	return (0);
}
