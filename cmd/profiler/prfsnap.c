/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)profil-3b15:prfsnap.c	1.4.2.1"
/*
 *	prfsnap - dump profile data to a log file
 */
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

# define PRF_ON   1
# define PRF_VAL  2

int	maxprf;
int	*buf;
int	prfmax;

main(argc, argv)
	int	argc;
	char	**argv;
{
	register  int  prf, log;
	int	tvec;
	int	*getspace();
	buf = getspace();

	if(argc != 2)
		error("usage: prfsnap  logfile");
	if((prf = open("/dev/prf", O_RDONLY)) < 0)
		error("cannot open /dev/prf");
	if((log = open(argv[1], O_WRONLY)) < 0)
		if((log = creat(argv[1],
					(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))) < 0)
			error("cannot creat log file");

	lseek(log, 0, 2);
	if(ioctl(prf, 3, PRF_ON))
		error("cannot activate profiling");
	prfmax = ioctl(prf, 2, 0);
	time(&tvec);
	read(prf, buf, (prfmax * 2 + 1) * sizeof (int));
	if(lseek(log, 0, 1) == 0) {
		write(log, &prfmax, sizeof prfmax);
		write(log, buf, prfmax * sizeof (int));
	}
	write(log, &tvec, sizeof tvec);
	write(log, &buf[prfmax], (prfmax + 1) * sizeof (int));
}

error(s)
	char	*s;
{
	write(2, "prfsnap: ", 9);
	write(2, s, strlen(s));
	write(2, "\n", 1);
	exit(1);
}

 int *
getspace()
	{
	struct stat ubuf,syb;
	int *space;
	int f;
	char *flnm = "/tmp/prf.adrfl";

	stat ("/stand/unix",&syb);
	if ((stat(flnm,&ubuf) == -1) ||
		 (ubuf.st_mtime <= syb.st_ctime)) {
		perror("Bad address file, execute  prfld");
		exit(2);
	}
	if((f = open(flnm, O_RDONLY)) == -1) {
		perror("Cannot open /tmp/prf.adrfl");
		exit(errno);
	}
	if (read (f,(char *) &maxprf, sizeof maxprf) != sizeof maxprf) {
		perror("Cannot read /tmp/prf.adrfl");
		exit(errno);
	}
	close(f);
	if ((space = (int *)malloc((maxprf * 2 + 1) * sizeof (int))) == NULL) {
		perror("Cannot malloc space for symbol table");
		exit(2);
	}
	return(space);
	}
