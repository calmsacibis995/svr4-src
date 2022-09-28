/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mkfifo:mkfifo.c	1.1.1.1"

#include <sys/types.h>
#include <sys/stat.h>

#include    <stdio.h>
#include    <errno.h>
#include    <string.h>

extern int mkfifo();
extern int errno;
extern char	*sys_errlist[];

main( argc, argv )
int   argc;
char       *argv[];
{
	char *path;
	int exitval = 0;
	int retval;
	int i;
	
    	if (argc < 2) {
		usage();
		exit(1);
    	}

    	for (i=1;i<argc;i++) {

		path = argv[i];
		retval =
			mkfifo (path,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		if (retval) {
			(void)fprintf(stderr, "mkfifo: %s\n",
				sys_errlist[errno]);
	    		exitval = 1;

		}

    	}

    	exit(exitval);

}


usage()
{
	(void)fprintf(stderr, "usage: mkfifo path ...\n");
}


