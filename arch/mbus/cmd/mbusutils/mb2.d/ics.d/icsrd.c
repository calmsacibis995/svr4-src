/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/icsrd.c	1.3"

static char icsrd_copyright[] = "Copyright 1988 Intel Corp. 462677";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ics.h>
#include <ctype.h>

char buf[512];

main(argc,argv)
	int argc;
	char *argv[];
{
	int c;
	int errflg = 0;
	int sflag = 0;
	int dflag = 0;
	int hflag = 0;
	extern char *optarg;
	extern int optind;
	int r;
	int count;
	int fd;
	int slot;
	int reg;
	
	while ((c = getopt(argc, argv, "sdh")) != -1)
		switch (c) {
		case 's':
			sflag++;
			break;
		case 'd':
			dflag++; 
			break;
		case 'h':
			hflag++;
			break;
		case '?':
			errflg++;
			break;
		}

	if((errflg) | ((argc - optind) != 3)) {
		printf("Usage:%s <-s> <-d> <-h> <slotid> <register> <count>\n",argv[0]);
		exit(-1);
	}
	
	slot  = atoi(argv[optind++]);
	reg   = atoi(argv[optind++]);
	count = atoi(argv[optind]);

	if(slot<0 || slot>ICS_MAX_SLOT) {
		printf("%s:Slot number %d out of range\n",argv[0]);
		exit(-1);
	}
	
	if(reg<0 || reg>511) {
		printf("%s:Register number %d out of range\n",argv[0]);
		exit(-1);
	}
	
	if(count<0 || (reg + count) > 512) {
		printf("%s:Count %d out of range\n",argv[0]);
		exit(-1);
	}
	
	fd=open("/dev/ics",O_RDWR);
	
	if (fd == -1) {
		perror("/dev/ics");
		exit(1);
	}

	ics_read(fd,slot,reg, &buf[reg], count);

	if (sflag)
	{
		printf("%s", &buf[reg]);
	}
	else
	{
		for (r=reg;r<(reg+count);r++) {
			if (hflag) 
				printf("%02xH ", buf[r] & 0xff);
			else if (dflag)
				printf("%3d ", buf[r] & 0xff);
			else
			{
				printf("%02d:%03d (%03xH) - %3d (%02xH)", slot,
					r, r, buf[r] & 0xff, buf[r] & 0xff);
				if (isprint(buf[r] & 0xff))
					printf(" [%c]", buf[r] & 0xff);
				printf("\n");
			}
		}
	}
	printf("\n");
}
