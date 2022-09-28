/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:x286.c	1.1"

/*
** Stub for printing out what to run when XENIX isn't loaded.
*/

main(argc, argv)
int argc;
char *argv[];
{
	printf ("The \"%s\" executable is a XENIX 286 executable.\n", argv[0]);
	printf ("Please load the \"XENIX Compatibility Package\" to run this executable.\n");
	exit (1);
}
