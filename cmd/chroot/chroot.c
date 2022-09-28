/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)chroot:chroot.c	1.9"
# include <stdio.h>
# include <errno.h>
# include <unistd.h>

#define	ROOT	0
main(argc, argv)
char **argv;
{
	extern char *sys_errlist[];
	extern int sys_nerr;

	if(argc < 3) {
		printf("usage: chroot rootdir command arg ...\n");
		exit(1);
	}

	if ( geteuid() != ROOT ) {
		printf("chroot: not super-user\n");
		exit(1);
	}

	argv[argc] = 0;
	if(argv[argc-1] == (char *) -1) /* catches potential problems in
					 old 16 bit implimentations */
		argv[argc-1] = (char *) -2;
	if (chroot(argv[1]) < 0) {
		perror(argv[1]);
		exit(1);
	}
	if (chdir("/") < 0) {
		printf("Can't chdir to new root\n");
		exit(1);
	}
	execv(argv[2], &argv[2]);
	if((errno > 0) && (errno <= sys_nerr)) 
		printf("chroot: %s\n",sys_errlist[errno]);
	else printf("chroot: exec failed, errno = %d\n",errno);
	exit(1);
}
