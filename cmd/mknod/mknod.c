/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mknod:mknod.c	1.12"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) mknod.c 1.6 88/05/05 mknod:mknod.c
 */
/***	mknod - build special file
 *
 *	mknod  name [ b ] [ c ] major minor
 *	mknod  name m	( shared data )
 *	mknod  name p	( named pipe )
 *	mknod  name s	( semaphore )
 *
 *	MODIFICATION HISTORY
 *	M000	11 Apr 83	andyp	3.0 upgrade
 *	- (Mostly uncommented).  Picked up 3.0 source.
 *	- Added header.  Changed usage message.  Replaced hard-coded
 *	  makedev with one from <sys/types.h>. 
 *	- Added mechanism for creating name space files.
 *	- Added some error checks.
 *	- Semi-major reorganition.
 */

#include <stdio.h>
#ifdef RT
#include <rt/types.h>
#include <rt/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <sys/mkdev.h>
#define	ACC	0666

main(argc, argv)
int argc;
char **argv;
{
	register mode_t		mode;
	register dev_t		arg;
	register major_t	majno;
	register minor_t	minno;
	long			number();

	if(argc < 3 || argc > 5)
		usage();
	if(argv[2][1] != '\0')
		usage();
	if(argc == 3) {
		switch (argv[2][0]) {
		case 'p':
			mode = S_IFIFO;
			arg = 0;	/* (not used) */
			break;
		case 'm':
			mode = S_IFNAM;
			arg = S_INSHD;
			break;
		case 's':
			mode = S_IFNAM;
			arg = S_INSEM;
			break;
		default:
			usage();
			/* NO RETURN */
		}
	}
	else if(argc == 5) {
		switch(argv[2][0]) {
		case 'b':
			mode = S_IFBLK;		/* M000 was 060666 */
			break;
		case 'c':
			mode = S_IFCHR;		/* M000 was 020666 */
			break;
#ifdef RT
		case 'r':
			mode = S_IFREC;
			break;
#endif
		default:
			usage();
		}
		if(getuid()) {
			fprintf(stderr, "mknod: must be super-user\n");
			exit(2);
		}
		majno = (major_t)number(argv[3]);
		if (majno == (major_t)-1 || majno > MAXMAJ){
			fprintf(stderr,"mknod: invalid major number '%s' - valid range is 0-%d\n",
				argv[3], MAXMAJ);
			exit(2);
		}
		minno = (minor_t)number(argv[4]);
		if (minno == (minor_t)-1) {
			fprintf(stderr,"mknod: invalid minor number '%s' \n",
				argv[4]);
			exit(2);
		}
		arg = makedev(majno, minno);
	}
	else
		usage();

	exit(domk(argv[1], mode|ACC, arg) ? 2 : 0);
}

int
domk(path, mode, arg)
register char  *path;
mode_t	mode;
dev_t	arg;
{
	int ec;

	if ((ec = mknod(path, mode, arg)) == -1)
		perror("mknod");
	else			/* chown() return deliberately ignored */
		chown(path, getuid(), getgid());
	return(ec);
}

long
number(s)
register  char  *s;
{
	register long	n, c;

	n = 0;
	if(*s == '0') {
		while(c = *s++) {
			if(c < '0' || c > '7')
				return(-1);
			n = n * 8 + c - '0';
		}
	} else {
		while(c = *s++) {
			if(c < '0' || c > '9')
				return(-1);
			n = n * 10 + c - '0';
		}
	}
	return(n);
}

usage()
{
#ifdef RT
	fprintf(stderr,"usage: mknod name [ b/c/r major minor ] [ p ]\n");
#else
	fprintf(stderr,"usage: mknod name [ b/c major minor ] [ p ]\n");
#endif
	exit(2);
}

