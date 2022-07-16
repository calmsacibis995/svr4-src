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

#ident	"@(#)mbus:cmd/ots/otsdebug.c	1.3"

static char otsdebug_copyright[] = "Copyright 1988 Intel Corp. 463554";

/*
** ABSTRACT:	Set the SV-ots Driver debug level
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

	if (  (argc != 2)
	    ||((level = atoi(argv[1])) > 12)
	    ||(  (level > 4)
	       &&(level < 10)
	      )
	   )
	{
		usage(argv[0]);
		exit(1);
	}

	if ( (fd = open("/dev/ots-00", 0)) == -1 )
	{
		printf("%s: open failed: error = %d\n", argv[0], err = errno);
		exit(err);
	}

	ioc.ic_cmd = 2;
	ioc.ic_timout = 0;
	ioc.ic_len = sizeof(level);
	ioc.ic_dp = (char *)&level;
	if( ioctl(fd, I_STR, &ioc) < 0 )
	{
		printf("%s: ioctl failed: error = %d\n", argv[0], err = errno);
		exit(err);
	}

	close(fd);
}

ahtoi(ptr)
char	*ptr;
{
	unsigned	i = 0;

	while( *ptr != NULL )
	{
		if( isxdigit(*ptr) )
		{
			i *= 0x10;
			if( isupper(*ptr) )
				i += *ptr - 0x37;
			else if( islower(*ptr) )
				i += *ptr - 0x57;
			else
				i += *ptr - 0x30;
		}
		ptr++;
	}
	return(i);
}

usage(name)
char *name;
{
	printf("usage: %s level\n", name);
	printf("\n\twhere level is:\n\n");
	printf("\t0\tTurns off debug display\n");
	printf("\t1\tOnly error messages are generated\n");
	printf("\t2\tFunction entry/exit messages are generated\n");
	printf("\t3\tFull debug display is generated\n");
	printf("\t4\tDriver may break to monitor\n");
	printf("\t10\tDebug is output to console and kernel putbuf\n");
	printf("\t11\tDebug is output only to console\n");
	printf("\t12\tDebug is output only to kernel putbuf\n\n");
}
