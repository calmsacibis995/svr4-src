/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:next_sector.c	1.3"

/* #define	DEBUG		1	/* */

/*
		next_sector(handle, sector, index)

	Pass this routine a sector number. It will return the
	next sector in the file (assuming that the sector passed is 
	the last one of a cluster).
*/

#include	"MS-DOS.h"

#include	<stdio.h>

long
next_sector(handle, sector, index)
int	handle;
long	sector;
int	index;
{
	long	cluster;
	long	n_cluster;
	long	n_sector;

	cluster = SECT_2_CLUS(sector);

	if ((n_cluster = next_cluster(handle, cluster)) == -1)
		return(-1);

	n_sector = CLUS_2_SECT(n_cluster);

#ifdef DEBUG
	(void) fprintf(stderr, "next_sector(): DEBUG - Sector %ld in cluster %ld is chained\n\tto sector %ld in cluster %ld\n", sector, cluster, n_sector, n_cluster);
#endif
	return(n_sector);
}
