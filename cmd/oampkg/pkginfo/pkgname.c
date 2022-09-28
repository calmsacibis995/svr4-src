/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginfo/pkgname.c	1.3.4.1"

extern int	pkgnmchk();

main(argc, argv)
int argc;
char *argv[];
{
	while(--argc > 0) {
		if(pkgnmchk(argv[argc], (char *)0, 1))
			exit(1);
	}
	exit(0);
}
