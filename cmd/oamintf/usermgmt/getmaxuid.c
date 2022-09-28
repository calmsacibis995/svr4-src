/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:usermgmt/getmaxuid.c	1.1.1.2"

#include	<sys/param.h>
#ifndef	MAXUID
#include	<limits.h>
#ifdef UID_MAX
#define	MAXUID	UID_MAX
#else 
#define	MAXUID	60000
#endif
#endif

main()
{
	printf("%d", MAXUID);
}
