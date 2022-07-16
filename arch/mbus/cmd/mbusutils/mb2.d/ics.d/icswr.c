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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/icswr.c	1.3"

static char icswr_copyright[] = "Copyright 1988 Intel Corp. 462804";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mps.h>
#include <sys/ics.h>
#include <ctype.h>

char buf[512];

main(argc,argv)
	int argc;
	char *argv[];
{
	int r;
	int value;
	int count;
	int fd;
	int slot;
	int reg;
	
	if(argc != 5) {
		printf("Usage:%s <slotid> <register> <count> <value>\n",
				argv[0]);
		exit(-1);
	}
	
	slot = atoi(argv[1]);
	reg  = atoi(argv[2]);
	count  = atoi(argv[3]);
	value  = atoi(argv[4]);
	
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


	for (r = reg; r < (reg+count); r++);
		buf[r] = value;
	
	ics_write(fd,slot,reg, &buf[r], count);
}
