/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/enet/enetreset.c	1.3.3.1"

static char *rcsid ="@(#)enetreset  $SV_enet SV-Eval01 - 06/25/90$";

static char enetreset_copyright[] = "Copyright 1987, 1988, 1989 Intel Corp. 464227";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>

#define	I552RESET	6;


extern	int		errno;
struct	strioctl	strioctl;
char			i552dev[] = "/dev/iso-tp4";

/*
 * enetreset: resets a specified 552 device.
 *
 *  parameter: device pathname, eg. /dev/iso-tp4	
 */
main()
{
	int	strfd, err;		
	int	num_boards, i;

	if((strfd = open(i552dev, O_WRONLY)) == -1) {
		perror("enetreset: Cannot open device");
		exit(errno);	
	}
	strioctl.ic_cmd = I552RESET;	
	strioctl.ic_timout = 10;
	strioctl.ic_len = sizeof(struct strioctl);
	strioctl.ic_dp = (char *)&strioctl;

	if ((ioctl(strfd, I_STR, &strioctl)) == -1)
	{
		perror("enetreset: ioctl failed");
		exit(errno);
	}
}
