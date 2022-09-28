/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)disksetup:partsize.c	1.3.1.1"

/* disksize has the function of returning the size of the ACTIVE UNIX system */
/* partition. The size is returned in megabytes. If an error occurs a non-   */
/* zero return code is returned. If no active UNIX partition is found a size */
/* 0 is outputted. If an active UNIX system partition is found, the number   */
/* of sectors in it are multiplied by the number of bytes/sector. THe total  */
/* bytes are divided 2^20 (1048576) and rounded up. The resulting value is   */
/* printed.								     */

#include <sys/types.h>
#include <stdio.h>
#include <sys/vtoc.h>
#include <sys/stat.h>
#include <sys/fdisk.h>
#include <sys/fcntl.h>

#define ONEMB	1048576L

void
main(argc,argv)
int argc;
char *argv[];
{
	int     diskfd;         	/* file descriptor for raw wini disk */
	struct  disk_parms      dp;     /* Disk parms returned from driver */
	char    *devname;		/* pointer to device name */
	daddr_t	unix_size;		/* # sectors in UNIX System partition */
	struct absio	absio;		/* buf used for RDABS & WRABS ioctls */
	struct mboot	mboot;		/* masterboot block data struct */
	struct ipart	*fdp, *unix_part; /* partition table data struct */
	char	errstring[12] = "Partsize: ";
	int	i;
	struct stat statbuf;

	if (argc == 1) {
		fprintf(stderr,"Usage: partsize raw-device\n");
		exit(1);
	}
	devname = argv[1];
	if (stat(devname, &statbuf)) {
		fprintf(stderr, "Partsize stat of %s failed\n", devname);
		perror(errstring);
		exit(1);
	}
	if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
		fprintf(stderr, "Partsize: device %s is not character special\n",devname);
		fprintf(stderr, "Usage: partsize raw-device\n");
		exit(1);
	}
	if ((diskfd=open(devname,O_RDONLY)) == -1) {
		fprintf(stderr,"Partsize unable to open %s\n",devname);
		perror(errstring);
		exit(2);
	}
	if (ioctl(diskfd,V_GETPARMS,&dp) == -1) {
		fprintf(stderr,"Partsize V_GETPARMS failed on %s\n",devname);
		perror(errstring);
		exit(3);
	}

	absio.abs_sec = 0;
	absio.abs_buf = (char *)&mboot;
	if (ioctl(diskfd, V_RDABS, &absio) < 0) {
		fprintf(stderr,"Partsize unable to read partition table from %s\n",devname);
		perror(errstring);
		exit(4);
	}
	close(diskfd);

	/* find an active UNIX System partition */
	unix_part = NULL;
	fdp = (struct ipart *)mboot.parts;
	for (i = FD_NUMPART; i-- > 0; ++fdp) {
		if ((fdp->systid == UNIXOS) && (fdp->bootid == ACTIVE))
				unix_part = fdp;
	}
	if (unix_part == NULL) {
		printf("0\n");
		exit(0);
	}
	unix_size = (unix_part->numsect * (long)dp.dp_secsiz + ONEMB/2) / ONEMB;
	printf("%ld \n",unix_size);
	exit(0);
}
