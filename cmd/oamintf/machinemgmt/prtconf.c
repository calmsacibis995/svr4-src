/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:machinemgmt/prtconf.c	1.1"

#include <sys/errno.h>
#include <sys/cram.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/vtoc.h>
#include <sys/hd.h>

int errno;

main()
{
        unsigned char x;
        struct disk_parms dp;
        int fd, drivea, driveb, hd, memsize;
        long size;
        char buf[2];
	if ((fd = open("/dev/cram",O_RDONLY)) == -1 ) {
                printf("can't open cram, errno %d\n",errno);
                exit(-1);
        }
        printf ("\n    AT&T 386 SYSTEM CONFIGURATION:\n\n");

        buf[0] = EMHIGH;
        crio(fd,buf);
        memsize = buf[1] / 4 + 1;
        printf("    Memory Size: %d Megabytes\n", memsize);
        printf ("    System Peripherals:\n\n");
        printf ("        Device Name\n\n");

        /* check for floppy drive */
        buf[0] = DDTB;
        crio(fd,buf);
        drivea = ((buf[1] >> 4) & 0x0f);
        (void) printflp(0, drivea);
        driveb = (buf[1] & 0x0f);
        (void) printflp(1, driveb);

/* check for hard disk drives */
        if((hd = open("/dev/rdsk/0s0", 2)) != -1)
                if(ioctl(hd, V_GETPARMS, &dp) != -1) {
                        size = ((long)(dp.dp_cyls*dp.dp_sectors*512*dp.dp_heads))/(long)(1024*1024);
                        printf("        %ld Megabyte Disk0\n", size);
                }

        if((hd = open("/dev/rdsk/1s0", 2)) != -1)
                if(ioctl(hd, V_GETPARMS, &dp) != -1) {
                        size = ((long)(dp.dp_cyls*dp.dp_sectors*512*dp.dp_heads))/(long)(1024*1024);
                        printf("        %ld Megabyte Disk1\n", size);
                }
/* check 80387 chip */
        buf[0] = EB;
        crio(fd,buf);
        if (buf[1] & 0x02)
                printf("        80387 Math Processor\n");

/* last but not least add a blank line for asthetic appeal */


	printf("\n\n");
	exit(0);
}
printflp(drivenum, drivetype)
int drivenum;
int drivetype;
{
        switch (drivetype) {
                case 1:
                        printf("        Floppy Disk%d - 360KB 5.25\n", drivenum);
                        break;
                case 2:
                        printf("        Floppy Disk%d - 1.2MB 5.25\n", drivenum);
                        break;
                case 3:
                        printf("        Floppy Disk%d - 720KB 5.25\n", drivenum);
                        break;
                case 4:
                        printf("        Floppy Disk%d - 1.44MB 3.5\n", drivenum);
                        break;
                default:
                        break;
        }
        return;
}

crio(fd,buf)
int fd;
char *buf;
{
        if(ioctl(fd,CMOSREAD, buf) < 0)
        {
                printf("can't open iocntl, cmd=%x, errno %d\n",buf[0], errno);
                exit(-1);
        }
}

