/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/Ckdev.c	1.1.2.1"
/*	Check if floppy drive 0 or 1 exists
 */

#include <sys/cram.h>
#include <fcntl.h>
#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
	int	drv, fd;
	unsigned char drvtyp, buf[2];

	if (strcmp(argv[1], "f0") == 0)
		drv = 0;
	else if (strcmp(argv[1], "f1") == 0)
		drv = 1;
	else if (strcmp(argv[1], "mt") == 0) {
		if (open("/dev/rmt", O_RDONLY) >= 0)
			printf("2\n");
		else
			printf("0\n");
		exit(0);
	} else if (strcmp(argv[1], "f1_or_mt") == 0) {
		if (open("/dev/rmt", O_RDONLY) >= 0) {
			printf("2\n");
			exit(0);
		}
		else
			drv = 1;
	} else {
		printf("0\n");
		exit(1);
	}

	buf[0] = DDTB;
	fd = open("/dev/cram", O_RDONLY);
	ioctl(fd, CMOSREAD, buf);
	if (drv == 1)
		drvtyp = (buf[1] & 0x0F);
	else
		drvtyp = ((buf[1] >> 4) & 0x0F);

	if (drvtyp == 0x00)
		printf("0\n");
	else
		printf("2\n");

	close(fd);
}

