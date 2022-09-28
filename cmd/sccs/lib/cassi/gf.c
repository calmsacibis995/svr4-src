/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/cassi/gf.c	6.3"

/* EMACS_MODES: c !fill tabstop=4 */

/*
 *	gf -- Get a .FRED file name for a particular application and subsystem.
 *
 *	The resulting pathname is placed in a static area that is overwritten
 *	by each call to gf ().
 *
 */

#include <stdio.h>
#include "../../hdr/filehand.h"

/* Debugging options */

#ifdef TRACE
#define TR(W,X,Y,Z) fprintf (stdout, W, X, Y, Z)
#else
#define TR(W,X,Y,Z) /* W X Y Z */
#endif

#define SIZE 132

char *gf (appl)
char	*appl;
{
	static char	filename[SIZE];
	char		inline[SIZE], *ptrs[3], *fmat[2], *tmp;
	extern char	*strrchr ();
	extern int	sweep ();
	char		*cat();

	TR("Gf: entry appl=(%s)\n", appl, EMPTY, EMPTY);
	(void) cat (filename, "/usr/lib/M2/", appl, EMPTY);
	fmat[0] = "DBBD";
	fmat[1] = EMPTY;
	TR("Gf: fmat[0]=(%s) fmat[1]=(%s)\n", fmat[0], fmat[1], EMPTY);
	if (sweep (VERIFY, filename, EMPTY, '\n', ':', SIZE, fmat, inline, ptrs,
	  (int (*)()) NULL, (int (*)()) NULL) != FOUND) {
		TR("Gf: not found\n", EMPTY, EMPTY, EMPTY);
		return (EMPTY);
		}
	tmp = strrchr (ptrs[1], (char) 01);
	*tmp = NULL;						/* Find and clobber control A. */
	(void) cat (filename, ptrs[1], "/.fred/.FRED", EMPTY);
	TR("Gf: returns (%s)\n", filename, EMPTY, EMPTY);
	return (filename);
}

