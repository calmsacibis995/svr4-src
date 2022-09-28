/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:free_space.c	1.3"

/* #define	DEBUG		1	/* */
/* #define	DEBUG_FREE	1	/* */

/*
		free_space(handle)

	Pass this routine a cluster value, and it will return
	the free_space in the chain, or -1 to indicate End Of File.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

long
free_space(handle)
int	handle;
{
	long	cluster;
	int	fat_offset;
	long	fat_entry;
	long	free_clusters = 0;
	int	index;

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "free_space(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Loop over each cluster in the FAT
	*/
	for (cluster = 2; cluster < TOTAL_CLUSTERS; cluster++) {
		/*
			Determine which sector in the FAT to use
		*/
#ifdef DEBUG_FREE
		(void) fprintf(stderr, "free_space(): DEBUG - Fat cluster %ld\n", cluster);
#endif

		/*
			Displacement is the number of BITS (NOT BYTES) 
			into the FAT that we can expect to find our 
			next entry in the cluster list for this file.

			if we are using 12 bit FAT entries then we 
			have some math to do here.
		*/
		if (TABLE.fat16 == 0) {
			/*
				If our current cluster is even - keep 
				low order 12 bits, otherwise high-order
				12 bits.
			*/
			fat_offset = (cluster * 3) / 2;

#ifdef DEBUG_FREE
			(void) fprintf(stderr, "free_space(): DEBUG - Fat offset = %d\n", fat_offset);
#endif
	
			if ((cluster / 2) * 2 == cluster) {
				/* Even */
				fat_entry = (*(TABLE.our_fat + fat_offset) + (*(TABLE.our_fat + fat_offset + 1) * 256)) & 0x0FFF;

#ifdef DEBUG_FREE
				(void) fprintf(stderr, "free_space(): DEBUG - Cluster %ld is even %x %x\n", cluster, (unsigned) *(TABLE.our_fat + fat_offset), (unsigned) *(TABLE.our_fat + fat_offset + 1));
#endif
			}
			else {
				fat_entry = (*(TABLE.our_fat + fat_offset) + (*(TABLE.our_fat + fat_offset + 1) * 256) & 0xFFF0) >> 4;

#ifdef DEBUG_FREE
				(void) fprintf(stderr, "free_space(): DEBUG - Cluster %ld is odd %x %x\n", cluster, *(TABLE.our_fat + fat_offset), *(TABLE.our_fat +fat_offset + 1));
#endif
			}
		} else {
			fat_offset = cluster * 2;
			fat_entry = (*(TABLE.our_fat + fat_offset + 1) * 256) + *(TABLE.our_fat + fat_offset);
		}

#ifdef DEBUG_FREE
		(void) fprintf(stderr, "free_space(): DEBUG - cluster = %ld fat_entry = %x\n", cluster, fat_entry);
#endif

		/*
			Check FAT entry
		*/
		if (fat_entry == 0) {
			free_clusters++;
		}
	}

	/*
		Ok - return free_space
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "free_space(): DEBUG - Free clusters %ld\n", free_clusters);
#endif
	return((free_clusters * TABLE.sectors_per_cluster) * TABLE.bytes_per_sector);
}
