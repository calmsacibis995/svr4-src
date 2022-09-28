/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)unlink:unlink.c	1.5"
main(argc, argv) char *argv[]; {
	if(argc!=2) {
		write(2, "Usage: /usr/sbin/unlink name\n", 29);
		exit(1);
	}
	unlink(argv[1]);
	exit(0);
}
