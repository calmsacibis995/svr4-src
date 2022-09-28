/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pkging:flop_num.c	1.3"

/*
** exits the number of floppy drives present -  0, 1, or 2.
*/

#ifdef MBUS
main ()
{
	exit(0); /* multibus only has one floppy */
}
#else

#include <fcntl.h>
#include <sys/cram.h>

main()
{
	int	fd;
	unsigned char buf[2];


	buf[0] = DDTB;
	if ((fd = open("/dev/cram", O_RDONLY)) < 0)
		exit (1);
	ioctl(fd, CMOSREAD, buf);
	close(fd);
	fd = 0;

	if ((buf[1] >> 4) & 0x0F)  { /* Floppy Drive 0	*/
	/* 				0 for nothing	*/
	/* 				2 for 5 1/4"	*/
	/* 				4 for 3 1/2"	*/
#ifdef DEBUG
		printf ("Floppy drive 0 = %d\n", ((buf[1] >> 4) & 0x0F));
#endif

		fd+=1;
	}
	if (buf[1] & 0x0F) {         /* Floppy Drive 1	*/
#ifdef DEBUG
		printf ("Floppy drive 1 = %d\n", (buf[1] & 0x0F));
#endif
		fd+=1;
	}
	exit (fd);
}
#endif /* MBUS */
