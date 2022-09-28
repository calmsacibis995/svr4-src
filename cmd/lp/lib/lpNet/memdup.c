/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/lpNet/memdup.c	1.3.2.1"

/*==================================================================*/
/*
*/
#include	<stdlib.h>
#include	<memory.h>
#include	<errno.h>
#include	"memdup.h"

#ifndef	NULL
#define	NULL	0
#endif

extern	int	errno;
/*==================================================================*/

/*==================================================================*/
/*
*/
void *
memdup (memoryp, length)

void	*memoryp;
int	length;
{
	/*----------------------------------------------------------*/
	/*
	*/
	void	*newp;

	/*----------------------------------------------------------*/
	/*
	*/
	if (length <= 0)
	{
		errno = EINVAL;
		return	NULL;
	}
	newp = (void *) malloc (length);

	if (newp == NULL)
	{
		errno = ENOMEM;
		return	NULL;
	}
	(void)	memcpy (newp, memoryp, length);


	return	newp;
}
/*==================================================================*/
