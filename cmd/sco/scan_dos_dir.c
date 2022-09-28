/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:scan_dos_dir.c	1.3"

/* #define	DEBUG		1 	/* */

/*
	This routine uses code which is needed in two cases,
	searching and printing of directories. In an effort to
	minimize the amount of code, both facilities are provided
	in the same routine. If a 4th argument (a character string)
	has length, then we search for that file, otherwise we dump
	the directory. Here are the two formats:

		scan_dos_dir(handle, sector, displacement, file)

				or

		scan_dos_dir(handle, sector, displacement, "")



	Format 1:
	=========

	Scan a dos directory starting at the passed sector, for the
	passed target_file. If found, return the displacement within
	the sector_buffer of its directory entry.



	Format 2:
	=========

	We traverse the directory, starting at the passed sector, and
	print all entries in that directory. As we are not looking for 
	a particular entry, the file parameter is a string of length 
	zero. Return the number of file entries printed.


	Either format returns -1 on error.
*/

#include	<stdio.h>

#include	"MS-DOS.h"

static	char	dosdir_filename[13];

#define	DISPLAYING	*target_file == '\0'

void
make_filename(string, delimiter)
unsigned	char	*string;
char	delimiter;
{
	int	i;
	int	index;

	for (i = 0, index = 0; index < 8 && (*(string + i) != ' ' || we_are_dosdir); i++)
		dosdir_filename[index++] = *(string + i);

	dosdir_filename[index] = '\0';

	for (i = 8; (we_are_dosdir || *(string + i) != ' ') && i < 11; i++) {
		if (i == 8)
			dosdir_filename[index++] = delimiter;

		dosdir_filename[index++] = *(string + i);
	}

	dosdir_filename[index] = '\0';

#ifdef DEBUG
	(void) fprintf(stderr, "disp_dos_dir(): DEBUG - Filename \"%s\"\n", dosdir_filename);
#endif
}

scan_dos_dir(handle, passed_sector, displacement, target_file)
int	handle;
long	passed_sector;
int	displacement;
char	*target_file;
{
	long	sector;
	int	i;
	int	j;
	int	k;
	int	index;
	int	starting_sector = passed_sector;
	int	total_files = 0;

#ifdef DEBUG
	(void) fprintf(stderr, "scan_dos_dir(): DEBUG - starting_sector = %ld, displacement = %d, target_file = \"%s\"\n", starting_sector, displacement, target_file);
#endif

	/*
		Ensure we initialized this handle
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "scan_dos_dir(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		For each sector in the current cluster...
	*/
	i = displacement;

	do {
		for (sector = starting_sector; sector < starting_sector + TABLE.sectors_per_cluster; sector++) {
#ifdef DEBUG
			(void) fprintf(stderr, "scan_dos_dir(): DEBUG - Reading sector %ld\n", sector);
#endif
			if (read_sector(handle, sector) == -1)
				return(-1);

			/*
				Loop over each directory entry 
				in the sector
			*/
			for (; i < TABLE.bytes_per_sector; i += BYTES_PER_DIR) {
				/*
					If first byte of sdd_filename is
					a '\0', end of target_file.
				*/
				if (sector_buffer[i] == 0x00) {
					if (DISPLAYING)
						return(total_files);
					else
						return(-1);
				}

				/*
					Disregard HIDDEN, SYSTEM, or LABEL
					files.
				*/
				if (DISPLAYING) {
					if ((sector_buffer[FILE_ATTRIBUTE + i] & (LABEL | HIDDEN | SYSTEM)) != 0) {
#ifdef DEBUG
						(void) fprintf(stderr, "disp_dos_dir(): DEBUG - Skipping HIDDEN | SYSTEM | LABEL file\n");
#endif
	
						continue;
					}
				}

				/*
					If the first byte of 
					sdd_filename is a 0xE5 then 
					the file has been deleted.
				*/
				if (sector_buffer[i] != 0xE5) {
					char	*c_ptr;
					char	sdd_filename[13];

					/*
						If first byte of name 
						is a 0x05, then first 
						character is actually 
						a 0xE5

						Per Dos Techincal 
						Reference pp 4-10.
					*/
					if (sector_buffer[i] == 0x05)
						sector_buffer[i] = 0xE5;

					/*
						We will strip blanks, 
						get our own copy of 
						the filename.
					*/
					for (j = 0; j < 8; j++)
						sdd_filename[j] = sector_buffer[i + j];

					sdd_filename[j] = '\0';

					/*
						If filename has trailing
						blanks - drop them.
					*/
					if ((c_ptr = strchr(sdd_filename, ' ')) != NULL)
						*c_ptr = '\0';

					/*
						Handle extension
					*/
					if (sector_buffer[EXTENSION + i] != ' ') {
						j = strlen(sdd_filename);
						sdd_filename[j++] = '.';

						for (k = 0; sector_buffer[EXTENSION + i + k] != ' ' && k < 3; k++)
							sdd_filename[j++] = sector_buffer[EXTENSION + i + k];

						sdd_filename[j] = '\0';
					}

					if (DISPLAYING) {
						make_filename(&sector_buffer[FILENAME + i], we_are_dosdir ? ' ' : '.');
						/*
							Output the information.
						*/
						if (we_are_dosdir) {
							if (sector_buffer[FILE_ATTRIBUTE + i] == SUB_DIRECTORY) {
								(void) printf("%-13s <DIR>     %s  %s\n", dosdir_filename, dos_mod_date(i), dos_mod_time(i));
							}
							else 
								(void) printf("%-13s %8ld  %s  %s\n", dosdir_filename, dos_fil_size(i), dos_mod_date(i), dos_mod_time(i));
						} else if (*dosdir_filename != '.') {
							(void) printf("%s\n", dosdir_filename);
#ifdef DEBUG
							(void) fprintf(stderr, "disp_dos_dir(): DEBUG - Starting cluster: %ld\n", GET_CLUS(i));
#endif
						}

						total_files++;
					}
					else {
						/*
							Compare the
							information.
						*/
#ifdef DEBUG
						(void) fprintf(stderr, "scan_dos_dir(): DEBUG - Comparing \"%s\" to \"%s\"\n", target_file, sdd_filename);
#endif
						if (strcmp(target_file, sdd_filename) == 0) {
#ifdef DEBUG
							(void) fprintf(stderr, "scan_dos_dir(): DEBUG - Located file \"%s\" at displacement %d in sector %ld\n", target_file, i, last_sector_read);
#endif
							return(i);
						}
					}
				}
			}

			i = 0;
#ifdef DEBUG
			(void) fprintf(stderr, "scan_dos_dir(): DEBUG - End of for loop - sector = %ld\n", sector);
#endif
		}

#ifdef DEBUG
		(void) fprintf(stderr, "scan_dos_dir(): DEBUG - Reached end of cluster - sector %ld in directory (%d)\n", sector, TABLE.root_base_sector + TABLE.sectors_in_root);
#endif

		if ((sector > (TABLE.root_base_sector + TABLE.sectors_in_root)) && passed_sector == TABLE.root_base_sector)
			starting_sector = -1;

		else if (sector > (TABLE.root_base_sector + TABLE.sectors_in_root)) {
			/*
				Fake being dosrm, as we dont complain
				about terminated chains when dosrm.
				Directory does not preallocate next
				cluster - so a broken chain is ok.
			*/
			we_are_dosrm = 1;
			starting_sector = next_sector(handle, sector - 1, index);
			we_are_dosrm = 0;
		}
		else starting_sector++;

		if (passed_sector == TABLE.root_base_sector && (total_files >= TABLE.root_dir_ent || starting_sector >= TABLE.root_base_sector + TABLE.sectors_in_root)) {
			if (DISPLAYING)
				return(total_files);
			else
				return(-1);
		}
	} while (starting_sector != -1);

#ifdef DEBUG
	(void) fprintf(stderr, "scan_dos_dir(): DEBUG - Exiting scan_dos_dir()\n");
#endif
	if (DISPLAYING)
		return(total_files);
	else
		return(-1);
}
