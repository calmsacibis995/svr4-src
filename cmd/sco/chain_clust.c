/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:chain_clust.c	1.3"

/* #define	DEBUG		1	/* */
/* #define	STANDALONE	1	/* */

/*
		chain_cluster(handle, to_cluster, from_cluster)

	Pass this routine a cluster value, and it will return
	the chain_cluster in the chain, or -1 to indicate End Of File.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<fcntl.h>

long
chain_cluster(handle, to_cluster, from_cluster)
int	handle;
long	to_cluster;
long	from_cluster;
{
	int	fat_offset;
	int	index;

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "chain_cluster(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}


	/*
		We must determine which sector in the FAT we want.
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "chain_cluster(): DEBUG - Chain %ld to %ld\n", from_cluster, to_cluster);
#endif

	/*
		Displacement is the number of BITS (NOT BYTES) into
		the FAT that we can expect to find our next entry in
		the cluster list for this file.

		if we are using 12 bit FAT entries then we have some 
		math to do here.
	*/
	if (TABLE.fat16 == 0) {
		/*
			If our current cluster is even - keep 
			low order 12 bits, otherwise high-order
			12 bits.
		*/
		fat_offset = (from_cluster * 3) / 2;

#ifdef DEBUG
		(void) fprintf(stderr, "chain_cluster(): DEBUG - Fat offset = %d\n", fat_offset);
#endif
		
		if ((from_cluster / 2) * 2 == from_cluster) {
			/* Even */
			*(TABLE.our_fat + fat_offset) = to_cluster % 256;
			*(TABLE.our_fat + fat_offset + 1) = (*(TABLE.our_fat + fat_offset + 1) & 0xF0) | ((to_cluster / 256) & 0x0F);

#ifdef DEBUG 
			(void) fprintf(stderr, "chain_cluster(): DEBUG - Cluster %ld is even %x %x\n", from_cluster, *(TABLE.our_fat + fat_offset), *(TABLE.our_fat + fat_offset + 1));
#endif
		}
		else {
			*(TABLE.our_fat + fat_offset) = (*(TABLE.our_fat + fat_offset) & 0x0F) + ((to_cluster % 16) << 4);
			*(TABLE.our_fat + fat_offset + 1) = to_cluster / 16;
#ifdef DEBUG 
			(void) fprintf(stderr, "chain_cluster(): DEBUG - Cluster %ld is odd %x %x\n", from_cluster, *(TABLE.our_fat + fat_offset + 1), *(TABLE.our_fat + fat_offset));
#endif

		}
	} else {
		fat_offset = from_cluster * 2;
		*(TABLE.our_fat + fat_offset) = to_cluster % 256;
		*(TABLE.our_fat + fat_offset+ 1) = to_cluster / 256;
	}

#ifdef DEBUG
	(void) fprintf(stderr, "chain_cluster(): DEBUG - Chained cluster %ld (offset %d) to cluster %ld\n", from_cluster, fat_offset, to_cluster);
#endif

	return(0);
}

#ifdef STANDALONE
main(argc, argv)
int	argc;
char	**argv;
{
	int	handle;
	int	index;
	int	disp;

	if (argc != 4) {
		(void) fprintf(stderr, "Usage: doschain drive from_cluster to_cluster\n\n\t\tUse this command to chain two clusters of an\n\tMS-DOS diskette together. To terminate a chain, chain it to 4095.\n\tTo free a cluster, chain it to zero.\n");
		exit(1);
	}

	/*
		Locate this handle in our device table
	*/
	if ((handle = open_device(argv[1], O_RDWR)) == -1)
		exit(1);

	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "dos_chain: Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Ensure the cluster is within bounds 
		of this volume type
	*/
	if (atoi(argv[2]) > TOTAL_CLUSTERS) {
		(void) fprintf(stderr, "doschain: from cluster is out of range\n");
		exit(1);
	}

	if (chain_cluster(handle, atoi(argv[3]), atoi(argv[2])) == -1)
		(void) fprintf(stderr, "doschain: Error - Failed to chain cluster %d to %d\n", atoi(argv[2]), atoi(argv[3]));

	if (write_fat(handle) == -1)
		(void) fprintf(stderr, "doschain: Error - Failed to update FATs\n");

	(void) close_device(handle);
}
#endif
