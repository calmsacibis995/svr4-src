/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rconsole:conflgs.c	1.1.2.1"

/* conflgs: does nothing on the generic product.
 */

#include <stdio.h>

main(argc, argv)
int	argc;
char	**argv;
{
	fprintf(stderr, "%s will not work on this core platform\n", argv[0]);
	exit(0);
}
