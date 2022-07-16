/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

static char icsgetrec_copyright[] = "Copyright 1989 Intel Corp. 465605-010";

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/icsgetrec.c	1.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ics.h>
#include <ctype.h>

#define MAX_REC	256		/* maximuum record type in interconnect space */

char buf[512];

main(argc,argv)
	int argc;
	char *argv[];
{
	int c;
	int errflg = 0;
	extern char *optarg;
	extern int optind;
	int rec_type;
	int reg;
	int fd;
	int slot;
	int hflag = 0;

	while ((c = getopt(argc, argv, "h")) != -1)
		switch(c) {
			case 'h':
				hflag++;
				break;
			default:
				errflg++;
				break;
		}
	
	if((errflg) | ((argc - optind) != 2)) {
		printf("Usage:%s [-h] <slotid> <record_type>\n",argv[0]);
		exit(-1);
	}
	
	slot     = atoi(argv[optind++]);
	rec_type = atoi(argv[optind++]);

	if ((slot < 0) || (slot > ICS_MAX_SLOT)) {
		printf("%s:Slot number %d out of range\n", argv[0], slot);
		exit(-1);
	}
	
	if ((rec_type < 0) || (rec_type > MAX_REC)) {
		printf("%s:ICS Record type %d out of range\n", argv[0], rec_type);
		exit(-1);
	}
	
	if ((fd = open("/dev/ics",O_RDWR)) == -1) {
		perror("/dev/ics");
		exit(-1);
	}

	reg = ics_find_rec(fd, slot, rec_type);

	if (hflag) 
		printf("0x%xH\n", reg);
	else
		printf("%d\n", reg);
	close(fd);
}
