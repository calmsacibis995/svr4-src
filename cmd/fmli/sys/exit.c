/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/exit.c	1.4"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"
#include	"retcodes.h"	/* abs */

char	*_tmp_ptr;
char	*Home;
char	*Filecabinet;
char	*Wastebasket;
char	*Filesys;
char	*Oasys;
char	*Progname;
char	**Remove;
int	(**Onexit)();
char	nil[] = "";
int	_Debug;

void
exit(n)
int	n;
{
	register int	i;
	int	lcv;

	if (n == R_BAD_CHILD)	/* abs */
	    _exit(n);
	else
	{
	    lcv = array_len(Onexit);
	    for (i = 0; i < lcv; i++)
		(*Onexit[i])(n);
	    lcv = array_len(Remove);
	    for (i = 0; i < lcv; i++)
		unlink(Remove[i]);
	    _cleanup();
	    _exit(n);
	}
}
