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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/icsslot.c	1.3"

static char icsslot_copyright[] = "Copyright 1988 Intel Corp. 462805";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ics.h>
#include <ctype.h>

char buf;

main(argc,argv)
	int argc;
	char *argv[];
{
	int r;
	int count;
	int fd;
	int slot;
	int reg;
	
	if(argc != 1) {
		printf("Usage:%s\n",argv[0]);
		exit(-1);
	}
	
	slot = ICS_MY_SLOT_ID;
	
	fd=open("/dev/ics",O_RDWR);
	
	if (fd == -1) {
		perror("/dev/ics");
		exit(1);
	}

	ics_read(fd,slot,0x2D, &buf, 1);

	buf >>= 3;
	printf ("%d\n", buf);
	exit (buf);
}
