/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:rm_file.c	1.3"

/* #define	DEBUG		1		/* */

/*
		rm_file(handle, displacement)

	Removes the file whose directory entry is at displacement
	within the current sector_buffer. Expects the extern 
	dir_sector to have the value of the directory sector.

	Return -1 on failure.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

extern	long	dir_sector;

/*
	We need to build a chain of all clusters in the file.
	As we must delete from the end of the chain up.
	This is our structure.
*/
struct	cluster_struct	{
	struct	cluster_struct	*forward_pointer;
	struct	cluster_struct	*backward_pointer;
	long	cluster;
};

extern	long	dir_sector;	/* in dosrm.c */

rm_file(handle, disp)
int	handle;
long	disp;
{
	long	cluster;
	struct	cluster_struct	base_struct;
	struct	cluster_struct	*struct_pointer;

	base_struct.forward_pointer = NULL;
	base_struct.backward_pointer = NULL;

	if (lookup_device(handle) == -1) {
		(void) fprintf(stderr, "rm_file(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Determine starting cluster number. Remember that data
		always starts in the SECOND cluster per DOS Tech.
		Manual pp. 4-13. Least significant byte is first.
	*/
	cluster = GET_CLUS(disp);

#ifdef DEBUG
	(void) fprintf(stderr, "rm_file(): DEBUG - starting cluster %ld\n", cluster);
#endif

	struct_pointer = &base_struct;

	/*
		Loop over each cluster in the chain, building
		our data structure as we go.
	*/
	we_are_dosrm = 1;

	if (cluster) {
		do {
#ifdef DEBUG
			(void) fprintf(stderr, "rm_file(): DEBUG - Adding cluster %ld to chain\n", cluster);
#endif
	
			/*
				Clusters zero and one should never appear in 
				a file's chain
			*/
			if (cluster < 2) {
				(void) fprintf(stderr, "rm_file(): Error - Invalid chain detected\n");
				we_are_dosrm = 0;
				return(-1);
			}
	
			/*
				Store the cluster
			*/
			struct_pointer->cluster = cluster;
	
			/*
				Malloc next structure in the chain
			*/
			if ((struct_pointer->forward_pointer = (struct cluster_struct *) malloc((unsigned) sizeof(struct cluster_struct))) == NULL) {
				(void) fprintf(stderr, "rm_file(): Failed to malloc cluster_struct\n");
				exit(1);
			}
	
			/*
				Set backwards pointer in newly malloc'd
				adta structure
			*/
			(struct_pointer->forward_pointer)->backward_pointer = struct_pointer;
	
			/*
				Switch to the newly malloc'd structure
			*/
			struct_pointer = struct_pointer->forward_pointer;
	
			/*
				Initialize newly malloc'd structure's forward
				pointer to NULL (last in chain).
	
				NOTE: The last structure has no valid
					cluster value.
			*/
			struct_pointer->forward_pointer = NULL;
			struct_pointer->cluster = -1;
		} while ((cluster = next_cluster(handle, cluster)) != -1);
	}

	we_are_dosrm = 0;

	/*
		Allow delete of directory to continue only if
		the directory is empty. Reload the directory
		before the test.
	*/
	if (last_sector_read != dir_sector)
		if (read_sector(handle, dir_sector) == -1)
			return(-1);

	/*
		Now we are in critical code.
		We disllow SIGNALS here. We never re-allow them,
		as we are in critical code until exit().
	*/
	critical(1);

	if (we_are_dosrmdir && is_dir_empty(handle, disp) == 0)
		return(-1);

	/*
		Do the actual deletion of the data space of the file,
		freeing our malloc'd data structures as we go along.
	*/
	if (cluster) {
		do {
			struct_pointer = struct_pointer->backward_pointer;
	
			(void) free(struct_pointer->forward_pointer);
	
#ifdef DEBUG
			(void) fprintf(stderr, "rm_file(): DEBUG - Cluster in chain: %ld\n", struct_pointer->cluster);
#endif
			if (del_cluster(handle, struct_pointer->cluster) == -1)
				break;
	
		} while (struct_pointer->backward_pointer != NULL);
	}

	/*
		Update the directory entry to indicate deleted
	*/
	if (last_sector_read != dir_sector)
		if (read_sector(handle, dir_sector) == -1)
			return(-1);

	sector_buffer[disp] = 0xE5;

	if (write_sector(handle, dir_sector) == -1) {
		(void) fprintf(stderr, "rm_file(): Error - Failed to update directory entry - Terminating\n");
		exit(1);
	}

	if (cluster) {
		if (write_fat(handle) == -1) {
			(void) fprintf(stderr, "rm_file(): Write of FAT failed - Disk amy be unusable\n");
			exit(1);
		}
	}

	return(0);
}
