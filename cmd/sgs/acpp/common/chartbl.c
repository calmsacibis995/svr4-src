/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/chartbl.c	1.4"
#include <memory.h>
#include "cpp.h"

#define TBLSIZE	10240
#define ALLOCSZ	TBLSIZE
/*
** a chracter table to keep character strings.
** When the static array runs out of space a new chunk of
** TBLSIZE bytes is allocated.
*/
static char chartbl[TBLSIZE];
static char * tablep;
static char * eotbl;

void
ch_init()
/*
** Initializes data structures in chartbl.c
*/
{
	tablep = chartbl;
	eotbl = chartbl + TBLSIZE;
}

char *
ch_alloc(len)
	unsigned int len;
/*
** Given the number of byte needed, this routine checks if there is enough
** space in the current table. If not a new table is allocated.
** A pointer to len allocated chars is returned.
*/
{
	register char * cp;

	if ((cp = eotbl - len) < tablep)
		if (len >= ALLOCSZ)
			cp = pp_malloc(len);
		else
		{
			tablep = pp_malloc(ALLOCSZ);
			eotbl = cp = tablep + ALLOCSZ - 1 - len;
		}
	else
		eotbl = cp;
	return cp;
}

char * 
ch_saven(fromp, len)
	char * fromp;
	unsigned int len;
/*
** Given a pointer to a string to copy from and the length, this routine
** allocates space and copies the string into the new location.
*/
{
	register char * cp;

	len++;		/* one moe byte for the null at the end */
	if ((cp = eotbl - len) < tablep)
		if (len >= ALLOCSZ)
			cp = pp_malloc(len);
		else
		{
			tablep = pp_malloc(ALLOCSZ);
			eotbl = cp = tablep +  ALLOCSZ  - 1 - len;
		}
	else
		eotbl = cp;
	len--;		/* original length */
	(void) memcpy(cp, fromp, len);
	return cp;
}
