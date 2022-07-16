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

#ident	"@(#)mbus:cmd/ots/otsstats.c	1.3"

static char otsstats_copyright[] = "Copyright 1988 Intel Corp. 463556";

/*
** ABSTRACT:	Displays the SV-ots Driver statistics
**
** MODIFICATIONS:
*/

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stropts.h>
#include <sys/otsuser.h>

extern int errno;
extern char *sys_errlist[];

struct strioctl ioc;
ulong ots_stat[OTS_SCNT];

main(argc, argv)
int	argc;
char	*argv[];
{
	int	fd;
	int	err;

	if ((fd = open("/dev/ots-00", 0)) < 0)
	{
		printf("%s: open failed: %s\n", argv[0], sys_errlist[errno]);
		exit(1);
	}
	ioc.ic_cmd = 1;
	ioc.ic_timout = 0;
	ioc.ic_len = sizeof(ots_stat);
	ioc.ic_dp = (char *)ots_stat;
	if (ioctl(fd, I_STR, &ioc) < 0)
	{
		printf("%s: ioctl failed: %s\n", argv[0], sys_errlist[errno]);
		exit(1);
	}
	close(fd);
	printf("SV-ots driver statistics:\n");
	printf("\tCount of STREAMS buffer allocation failures:\t%d\n",
 		ots_stat[ST_ALFA]);
	printf("\tCount of received messages:\t\t\t%d\n",
 		ots_stat[ST_RMSG]);
	printf("\tCount of received expedited messages:\t\t%d\n",
 		ots_stat[ST_REXP]);
	printf("\tCount of received datagrams:\t\t\t%d\n",
 		ots_stat[ST_RUNI]);
	printf("\tCount of sent messages:\t\t\t\t%d\n",
 		ots_stat[ST_SMSG]);
	printf("\tCount of sent expedited messages:\t\t%d\n",
 		ots_stat[ST_SEXP]);
	printf("\tCount of sent datagrams:\t\t\t%d\n",
 		ots_stat[ST_SUNI]);
	printf("\tCount of data bytes passed to users:\t\t%d\n",
 		ots_stat[ST_BRCV]);
	printf("\tCount of datagram bytes passed to users:\t%d\n",
 		ots_stat[ST_URCV]);
	printf("\tCount of expedited data bytes passed to users:\t%d\n",
 		ots_stat[ST_ERCV]);
	printf("\tCount of data bytes sent for users:\t\t%d\n",
 		ots_stat[ST_BSNT]);
	printf("\tCount of datagram bytes sent for users:\t\t%d\n",
 		ots_stat[ST_USNT]);
	printf("\tCount of expedited data bytes sent for users:\t%d\n",
 		ots_stat[ST_ESNT]);
	printf("\tCount of sent packets (streams message blocks):\t%d\n",
 		ots_stat[ST_SPCK]);
	printf("\tCount of sent expedited packets:\t\t%d\n",
 		ots_stat[ST_EPCK]);
	printf("\tCount of sent datagram packets:\t\t\t%d\n",
 		ots_stat[ST_UPCK]);
	printf("\tNumber of currently open endpoints:\t\t%d\n",
 		ots_stat[ST_CURO] - 1);
	printf("\tTotal number of opens done:\t\t\t%d\n",
 		ots_stat[ST_TOTO]);
	printf("\tNumber of currently connected endpoints:\t%d\n",
 		ots_stat[ST_CURC]);
	printf("\tTotal number of connections made:\t\t%d\n",
 		ots_stat[ST_TOTC]);
	printf("\tConnect failure count: no Transport resources:\t%d\n",
 		ots_stat[ST_BADCON1]);
	printf("\tConnect failure count: no driver resources:\t%d\n",
 		ots_stat[ST_BADCON2]);
	printf("\tConnect failure count: no data ports:\t\t%d\n",
 		ots_stat[ST_BADCON3]);
	printf("\tCount of retries on transport sends:\t\t%d\n",
 		ots_stat[ST_TKIRETRY]);
 	printf("\tCount of open channel, xmt and rcv failures:\t%d\n",
		ots_stat[ST_TKIFAILS]);
	printf("\tFailures to allocate TKI message blocks:\t%d\n",
		ots_stat[ST_TKIMBLK]);
	printf("\tFailures to allocate TKI data buf descriptors:\t%d\n",
		ots_stat[ST_TKIDBLK]);
	printf("\tFailures to allocate TKI transaction ids:\t%d\n",
		ots_stat[ST_TKITID]);
}
