/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)yes:yes.c	1.1.1.1"

/*
 *	@(#) yes.c 1.1.1.1 90/11/08 yes:yes.c
 */
/***	yes --
 *
 *	MODIFICATION HISTORY
 *	M000	27 May 83	andyp	3.0 upgrade
 *	- No changes.
 *	M001	16 Dec 87	davidby 
 *	- Complete rewrite.  Optimized for Merged Product port.
 */
#include <stdio.h>

void
main(argc, argv)
int argc;
char *argv[];
{
	register char *str;

	str = (argc > 1) ? argv[1]: "y";
	while (puts(str) != EOF)
		;
	exit(-1);
}
