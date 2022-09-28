/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:get_label.c	1.3"

/* #define	DEBUG		1	/* */

/*
			get_label(handle)

	Scan a dos root directory for a volume label.
	Store volume label in volume-label, if one exists.

	Return -1 on error;
*/

#include	<stdio.h>
#include	"MS-DOS.h"

char	volume_label[13];

get_label(handle)
int	handle;
{
	int	i;
	int	j;
	int	index;
	long	sector;

	/*
		Ensure we initialized this handle
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "get_label(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Loop over each sector in the root directory
	*/
	for (sector = TABLE.root_base_sector; sector < TABLE.root_base_sector + TABLE.sectors_in_root; sector++) {
		if (read_sector(handle, sector) == -1)
			return(-1);

		/*
			Loop over each directory entry in the sector
		*/
		for (i = 0; i < TABLE.bytes_per_sector && sector_buffer[i] != '\0'; i += BYTES_PER_DIR) {
			/*
				If first byte of volume_label is a '\0', end of 
				directory. If first byte of volume_label is a 0xE5
				the the file has been deleted.
			*/
			if (sector_buffer[i] != '\0' && sector_buffer[i] != 0xE5) {
				/*
					If first byte of volume_label is a 0x05,
					then first character is actually a 0xE5
	
					Per Dos Techincal Reference pp 4-10.
				*/
				if (sector_buffer[i] == 0x05)
					sector_buffer[i] = 0xE5;
	
				/*
					We will strip trailing blanks, get 
					our own copy of the volume_label.
				*/
				for (j = 0; j < 11; j++)
					volume_label[j] = sector_buffer[i + j];
	
				volume_label[j] = '\0';
	
				/*
					If volume_label has trailing blanks -
					drop them.
				*/
				while (volume_label[strlen(volume_label) - 1] == ' ')
					volume_label[strlen(volume_label) - 1] = '\0';
	
				/*
					Is this a label?
				*/
				if (sector_buffer[FILE_ATTRIBUTE + i] & LABEL) {
#ifdef DEBUG
					(void) fprintf(stderr, "get_label(): DEBUG - Found label \"%s\"\n", volume_label);
#endif
					return(0);
				}
			}
		}
	}

	return(-1);
}
