/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:devices/erase/spclsize.c	1.1"

#include	<fcntl.h>
#include	<stdio.h>
#include	<errno.h>
#include	<sys/vtoc.h>

main( argc, argv )
int	argc;
char	*argv[];
{
	int	fd;
	extern errno;
	struct  disk_parms dp;


	if((fd = open( argv[1], O_RDONLY | O_NDELAY )) < 0)
	{
		fprintf(stderr,"Can't open %s, errno %d\n",argv[1], errno);
		exit(-1);
	}

	if( ioctl(fd, V_GETPARMS, &dp) < 0){
		fprintf(stderr,"ioctl failed for device %s, errno %d\n",errno);
		exit(-1);
	}

	printf("%ld\n",(long)(dp.dp_heads*dp.dp_sectors*dp.dp_secsiz*dp.dp_cyls)/512);
	close(fd);
	exit(0);
}
