/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:alloc_clust.c	1.3"

/* #define	DEBUG		1	/* */

/*
		alloc_cluster(handle)

	Allocates a structure. Returns value of allocated cluster
	on success, -1 on error. Sets cluster entry in FAT to FFF.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

alloc_cluster(handle)
int	handle;
{
	int	index;
	long	cluster;
	int	fat_offset;
	long	fat_entry;

	/*
		Locate this handle in our device table
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "alloc_cluster(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Loop over each cluster in the FAT
	*/
	for (cluster = 2; cluster < TOTAL_CLUSTERS; cluster++) {

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
			(void) fprintf(stderr, "alloc_cluster(): DEBUG - Fat offset = %d\n", fat_offset);
#endif
	
			if ((cluster / 2) * 2 == cluster) {
				/* Even */
				fat_entry = (*(TABLE.our_fat + fat_offset) + (*(TABLE.our_fat + fat_offset + 1) * 256)) & 0x0FFF;

#ifdef DEBUG_FREE
				(void) fprintf(stderr, "alloc_cluster(): DEBUG - Cluster %ld is even %x %x\n", cluster, (unsigned) *(TABLE.our_fat + fat_offset), (unsigned) *(TABLE.our_fat + fat_offset + 1));
#endif
			}
			else {
				fat_entry = (*(TABLE.our_fat + fat_offset) + (*(TABLE.our_fat + fat_offset + 1) * 256) & 0xFFF0) >> 4;

#ifdef DEBUG_FREE
				(void) fprintf(stderr, "alloc_cluster(): DEBUG - Cluster %ld is odd %x %x\n", cluster, *(TABLE.our_fat + fat_offset), *(TABLE.our_fat +fat_offset + 1));
#endif
			}
		} else {
			fat_offset = cluster * 2;
			fat_entry = (*(TABLE.our_fat + fat_offset + 1) * 256) + *(TABLE.our_fat + fat_offset);
		}

#ifdef DEBUG_FREE
		(void) fprintf(stderr, "alloc_cluster(): DEBUG - cluster = %ld fat_entry = %x\n", cluster, fat_entry);
#endif

		if (fat_entry == 0)
			break;
	}

	/*
		Check for no space condition
	*/
	if (cluster == TOTAL_CLUSTERS) {
		(void) fprintf(stderr, "alloc_cluster(): Error - No space left on device \"%s\"\n", device_pathname);
		return(-1);
	}

	/*
		Mark this cluster as the last in chain
	*/
	if (chain_cluster(handle, (long) (TABLE.fat16 ? 0xFFFF : 0xFFF), cluster) == -1)
		return(-1);

	/*
		Ok - return the cluster.
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "alloc_cluster(): DEBUG - Allocated cluster %ld sector %ld\n", cluster, CLUS_2_SECT(cluster));
#endif
	return(cluster);
}
