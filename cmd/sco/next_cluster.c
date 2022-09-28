/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:next_cluster.c	1.3"

/* #define	DEBUG		1	/* */

/*
		next_cluster(handle, cluster)

	Pass this routine a cluster value, and it will return
	the next_cluster in the chain, or -1 to indicate End Of File.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

long
next_cluster(handle, cluster)
int	handle;
long	cluster;
{
	int	fat_offset;
	long	fat_entry;
	int	index;
	unsigned	char	low_char;
	unsigned	char	high_char;

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "next_cluster(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "next_cluster(): DEBUG - Fat cluster %ld\n", cluster);
#endif

	/*
		if we are using 12 bit FAT entries then we have some 
		math to do here.
	*/
	if (TABLE.fat16 == 0) {
		/*
			If our current cluster is even - keep 
			low order 12 bits, otherwise high-order
			12 bits.
		*/
		fat_offset = (cluster * 3) / 2;

#ifdef DEBUG
		(void) fprintf(stderr, "next_cluster(): DEBUG - Fat offset = %d\n", fat_offset);
#endif
		
		low_char = *(TABLE.our_fat + fat_offset);
		high_char = *(TABLE.our_fat + fat_offset + 1);

		if ((cluster / 2) * 2 == cluster) {
			/* Even */
			fat_entry = low_char + (256 * (high_char & 0x0F));

#ifdef DEBUG 
			(void) fprintf(stderr, "next_cluster(): DEBUG - Cluster %ld is even %x %x\n", cluster, low_char, high_char);
#endif
		}
		else {
			fat_entry = ((low_char & 0xF0) >> 4) + (high_char * 16);
#ifdef DEBUG 
			(void) fprintf(stderr, "next_cluster(): DEBUG - Cluster %ld is odd %x %x\n", cluster, low_char, high_char);
#endif
		}
	} else {
		fat_offset = cluster * 2;
		fat_entry = low_char + (256 * high_char);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "next_cluster(): DEBUG - fat_entry = %x\n", fat_entry);
#endif

	/*
		Check validity of FAT entry
	*/
	if (TABLE.fat16) {
		if (fat_entry >= 0xFFF8 && fat_entry <= 0xFFFF) {
			if (we_are_dosrm == 0)
				(void) fprintf(stderr, "next_cluster(): Expected cluster %ld to be chained to another - it is not\n", cluster);

			return(-1);
		}
	}
	else {
		if (fat_entry >= 0x0FF8 && fat_entry <= 0x0FFF) {
			if (we_are_dosrm == 0)
				(void) fprintf(stderr, "next_cluster(): Expected cluster %ld to be chained to another - it is not\n", cluster);

			return(-1);
		}
	}

	/*
		Ok - return cluster
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "next_cluster(): DEBUG - Cluster %ld is chained to cluster %ld\n", cluster, fat_entry);
#endif
	return(fat_entry);
}
