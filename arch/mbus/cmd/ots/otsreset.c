/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/ots/otsreset.c	1.3"

static char otsreset_copyright[] = "Copyright 1988 Intel Corp. 463555";

/*
** ABSTRACT:	Resets the SV-ots Driver
**
** MODIFICATIONS:
*/

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stropts.h>

struct strioctl ioc;

extern int errno;
main(argc, argv)
int	argc;
char	*argv[];
{
	int	fd;
	int	err;
	int	level;

	if ( (fd = open("/dev/ots-00", 0)) == -1 )
	{
		printf("%s: open failed: error = %d\n", argv[0], err = errno);
		exit(err);
	}

	ioc.ic_cmd = 6;
	ioc.ic_timout = 0;
	if( ioctl(fd, I_STR, &ioc) < 0 )
	{
		printf("%s: ioctl failed: error = %d\n", argv[0], err = errno);
		exit(err);
	}

	close(fd);
}
