/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/mergelist.c	1.5.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** mergelist() - ADD CONTENT OF ONE LIST TO ANOTHER
 **/

int
#if	defined(__STDC__)
mergelist (
	char ***		dstlist,
	char **			srclist
)
#else
mergelist (dstlist, srclist)
	register char		***dstlist,
				**srclist;
#endif
{
	if (!srclist || !*srclist)
		return (0);

	while (*srclist)
		if (addlist(dstlist, *srclist++) == -1)
			return (-1);
	return (0);
}
