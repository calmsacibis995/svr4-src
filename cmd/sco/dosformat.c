/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dosformat.c	1.3"

/* #define		DEBUG		1	/* */

/*
	MORE_SPACE and VERIFY are used to control the degree
	of verification performed. If MORE_SPACE is defined,
	we check each sector within a bad track for usability.
	This makes the command run MUCH SLOWER, but always yields
	a more accurate report on bad sectors, and thus more usable
	space on the volume. If MORE_SPACE is defined, XENIX 
	compatability no longer exists. We do not define it by default.

	If MORE_SPACE is not defined, an error on a track causes the 
	entire track to be flagged as bad in the FAT(s). This is the
	way real MS-DOS does it, and XENIX as well.

	VERIFY says to verify the disk. We define it by default.
	If VERIFY is not defined, MORE_SPACE has no effect.
*/

/* #define		MORE_SPACE	1	/* Default is OFF */
#define			VERIFY		1	/* Default is ON */

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<sys/vtoc.h>

#include	<fcntl.h>

#include	<signal.h>

#include	"MS-DOS_boot.h"

unsigned	char	zero[MAX_SECTOR_SIZE];

struct	{
	int		sectors_per_cluster;
	int		total_sectors;
	int		sectors_per_fat;
	int		root_dir_ent;
	unsigned	media_descriptor;
}
drive_table[] = {{1, 320, 1, 64, 0xFE},
		{2, 640, 1, 112, 0xFF},
		{1, 360, 2, 64, 0xFC},
		{2, 720, 2, 112, 0xFD},
		{1, 2400, 7, 224, 0xF9},
		{2, 1440, 3, 112, 0xF9},
		{1, 2880, 9, 224, 0XF0},
		{0, 0, 0, 0}};

#ifdef SIGPOLL
void
#else
int
#endif
interrupt()
{
	(void) fprintf(stderr, "\ndosformat: Signal caught - Terminating\n\tDisk is unusable\n");
	exit(2);

#ifndef SIGPOLL
	return(0);	/* NOTREACHED */
#endif
}

static	char	*usage = "Usage: dosformat [-fqv2] drive\n";

main(argc, argv)
int	argc;
char	**argv;
{
	char	buffer[30];
	long	c;
	char	device[MAX_FILENAME];
	int	dindex;
	int	handle;
	int	index;
	union	io_arg		io_arg_union;
	int	label_volume = 0;
	struct	disk_parms	parms_struct;
	int	quiet = 0;
	int	silent = 0;
	int	trk;
	char	verify_buffer[MAX_TRACK];


	while ((c = getopt(argc, argv, "fqv2")) != EOF)
		switch(c) {
		case 'f':
			quiet = 1;
			break;

		case 'q':
			silent = 1;
			break;

		case 'v':
			label_volume = 1;
			break;

		case '2':
			break;

		default:
			(void) fprintf(stderr, usage);
			exit(1);
		}

	/*
		Must have at least one argument, drive to format
	*/
	if (optind != argc - 1) {
		(void) fprintf(stderr, usage);
		exit(1);
	}

	/*
		Load up assignments file so we can do drive translation
	*/
	if (get_assignments() == -1) {
		(void) fprintf(stderr, "dosformat: Error - Failed to process assignments file - Terminating\n");
		exit(1);
	}

	/*
		If the passed device does not end in a colon,
		tack one on (as dosformat cannot be passed a filename)
	*/
	(void) strcpy(device, argv[optind]);

	if (device[strlen(device) - 1] != ':')
		(void) strcat(device, ":");

	/*
		Must parse the filename before opening the device
		(Sets drive)
	*/
	if (parse_name(device) == -1)
		return(-1);

	if ((index = lookup_drive(drive)) == -1) {
		(void) fprintf(stderr, "dosformat: Error - Drive '%c' not found in assignments file\n", drive);
		exit(1);
	}

	/*
		Prompt before we begin - if requested
	*/
	if (quiet == 0) {
		(void) printf("Insert new diskette for %s\n", device_pathname);
		(void) printf("and press <RETURN> when ready ");
		(void) fflush(stdout);

		(void) my_fgets(buffer, 20, stdin);
	}

	if (silent == 0) {
		(void) printf("\nFormatting...");
		(void) fflush(stdout);
	}

	/*
		Issue "interrupted" message, when signals are caught
	*/
	(void) signal(SIGHUP, interrupt);
	(void) signal(SIGINT, interrupt);
	(void) signal(SIGQUIT, interrupt);

	/*
		Open device 
	*/
	if ((handle = open(device_pathname, O_RDWR)) == -1) {
		(void) fprintf(stderr, "dosformat: open(\"%s\", O_RDWR | O_EXCL) failed\n", device_pathname);
		exit(1);
	}

	/*
		Get parameters for this device
	*/
	if (ioctl(handle, V_GETPARMS, &parms_struct) == -1) {
		(void) fprintf(stderr, "dosformat: Error - V_GETPARMS failed\n");
		perror("	Reason");
		exit(1);
	}

#ifdef DEBUG
	(void) fprintf(stderr, "dp_type: %c\n", parms_struct.dp_type);
	(void) fprintf(stderr, "dp_heads: %u\n", parms_struct.dp_heads);
	(void) fprintf(stderr, "dp_cyls: %u\n", parms_struct.dp_cyls);
	(void) fprintf(stderr, "dp_sectors: %u\n", parms_struct.dp_sectors);
	(void) fprintf(stderr, "dp_secsiz: %u\n", parms_struct.dp_secsiz);
#endif

	/*
		Locate this device type in our table of 
		known formats
	*/
	for (dindex = -1, c = 0; drive_table[c].total_sectors; c++)
		if (drive_table[c].total_sectors == (parms_struct.dp_heads * parms_struct.dp_cyls * parms_struct.dp_sectors)) {
			dindex = c;
			break;
		}

	/*
		Make sure we found this type
	*/
	if (dindex == -1) {
		(void) fprintf(stderr, "Invalid media type\n");
		exit(1);
	}

	/*
		Format the volume, track by track
	*/
	for (trk = 0; trk < parms_struct.dp_heads * parms_struct.dp_cyls; trk++) {
		io_arg_union.ia_fmt.start_trk = trk;
		io_arg_union.ia_fmt.num_trks = 1;
		io_arg_union.ia_fmt.intlv = 2;

		(void) ioctl(handle, V_FORMAT, &io_arg_union);
	}

	/*
		Load sector zero
	*/
	if (lseek(handle, 0, 0) == -1) {
		(void) fprintf(stderr, "dosformat: Failed to seek to sector zero\n");
		exit(1);
	}

	/*
		Load up original 360 K format.
	*/
	for (c = 0; c < MAX_SECTOR_SIZE; c++) {
		zero[c] = (unsigned char) sector_zero[c];
	}

	/*
		Patch the format for our particular type
	*/
	zero[LOW_SECTOR_SIZE] = parms_struct.dp_secsiz % 256;
	zero[HI_SECTOR_SIZE] = parms_struct.dp_secsiz / 256;
	zero[SECTORS_PER_CLUSTER] = drive_table[dindex].sectors_per_cluster;
	zero[LOW_RESERVED_SECTORS] = 0x01;
	zero[HI_RESERVED_SECTORS] = 0x00;
	zero[NUMBER_OF_FATS] = 0x02;
	zero[LOW_ROOT_DIR_ENT] = drive_table[dindex].root_dir_ent % 256;
	zero[HI_ROOT_DIR_ENT] = drive_table[dindex].root_dir_ent / 256;
	zero[LOW_TOTAL_SECTORS] = drive_table[dindex].total_sectors % 256;
	zero[HI_TOTAL_SECTORS] = drive_table[dindex].total_sectors / 256;
	zero[MEDIA_DESCRIPTOR] = drive_table[dindex].media_descriptor;
	zero[LOW_SECTORS_PER_FAT] = drive_table[dindex].sectors_per_fat % 256;
	zero[HI_SECTORS_PER_FAT] = drive_table[dindex].sectors_per_fat / 256;
	zero[LOW_SECTORS_PER_TRACK] = parms_struct.dp_sectors % 256;
	zero[HI_SECTORS_PER_TRACK] = parms_struct.dp_sectors / 256;
	zero[LOW_NUMBER_OF_HEADS] = parms_struct.dp_heads % 256;
	zero[HI_NUMBER_OF_HEADS] = parms_struct.dp_heads / 256;

	/*
		Write out patched boot sector
	*/
	if (write(handle, zero, (unsigned) parms_struct.dp_secsiz) != parms_struct.dp_secsiz) {
		(void) fprintf(stderr, "dosformat: Error - Failed to write patched boot sector\n");
		perror("	Reason");
	}

	/*
		We have written a boot sector for 360K
		MS-DOS diskette. Fix it up for other types.
		close down this handle, so we can re-open
		using libdos.
	*/
	(void) close(handle);

	/*
		Now we must create the root directory and FATs
		We use libdos now. We could not use it before
		because we did not have a reasonably valid 
		MS-DOS filesystem.
	*/
	if ((handle = open_device(device, O_RDWR)) == -1)
		exit(1);

	/*
		Make sure we are all setup properly
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "dosformat: Failed to locate handle %d in device_table\n", handle);
		exit(1);
	}

	/*
		Chain each cluster to zero (indicating available)
	*/
	for (c = 2; c < TOTAL_CLUSTERS; c++)
		(void) chain_cluster(handle, (long) 0, c);

#ifdef VERIFY
	/*
		Verify track by track

		Read each track. If read fails, re-read track sector
		by sector flagging failures in the FAT
	*/
	if (MAX_TRACK < parms_struct.dp_secsiz * parms_struct.dp_sectors) {
		(void) fprintf(stderr, "dosformat: Warning - Internal static buffer size\n\tinsufficient - Increase MAX_TRACK\n\tSkipping verify\n");
		goto no_verify;
	}

	if (lseek(handle, 0, 0) == -1) {
		(void) fprintf(stderr, "dosformat: Error - Failed to seek to track 0\n\tDisk is unusable\n");
		exit(1);
	}

	for (trk = 0; trk < parms_struct.dp_heads * parms_struct.dp_cyls; trk++) {
		int	read_failed = 0;

#ifdef DEBUG
		(void) fprintf(stderr, "dosformat: DEBUG - Track %d ", trk);
#endif

		if (read(handle, verify_buffer, parms_struct.dp_secsiz * parms_struct.dp_sectors) == -1)
			read_failed = 1;

#ifdef DEBUG
		(void) fprintf(stderr, "Read %s\n", read_failed ? "failed" : "ok");
#endif

		/*
			Look for the bad sector(s). Flag each one in the FAT
		*/
		if (read_failed) {
			long	sector;

			for (sector = trk * parms_struct.dp_sectors; sector < (trk + 1) * parms_struct.dp_sectors; ) {
#ifdef DEBUG
				(void) fprintf(stderr, "dosformat: DEBUG - Checking sector %ld\n", sector);
#endif

#ifdef MORE_SPACE
				if (lseek(handle, sector * TABLE.bytes_per_sector, 0) == -1)
					(void) chain_cluster(handle, TABLE.fat16 ? 0xFFF7 : 0xFF7, SECT_2_CLUS(sector));
				else if (read(handle, verify_buffer, TABLE.bytes_per_sector) == -1)
					(void) chain_cluster(handle, TABLE.fat16 ? 0xFFF7 : 0xFF7, SECT_2_CLUS(sector));

				sector++;
#else
				(void) chain_cluster(handle, (long) (TABLE.fat16 ? 0xFFF7 : 0xFF7), (long) (SECT_2_CLUS(sector)));

				sector += TABLE.sectors_per_cluster;
#endif
			}
		}
	}
#endif

no_verify:
	/*
		Chain cluster zero to a cluster number resulting
		from ORing the media descriptor with a 0xF00.
	*/
	if (chain_cluster(handle, (long) (0xF00 | drive_table[dindex].media_descriptor), (long) 0) == -1) {
		(void) fprintf(stderr, "dosformat: Error - Failed to initialize cluster 0\n");
	}

	/*
		Cluster one is chained to a 0xFFF, as a 
		delimeter for cluster zero and the data space.
	*/
	if (chain_cluster(handle, (long) (TABLE.fat16 ? 0xFFFF : 0xFFF), (long) 1) == -1) {
		(void) fprintf(stderr, "dosformat: Error - Failed to initialize cluster 1\n");
	}

	/*
		Write out updated FATs to disk
	*/
	if (write_fat(handle) == -1) {
		(void) fprintf(stderr, "dosformat: Error - Failed to write out FATs\n\tDisk may be unusable\n");
	}

	/*
		Set up the ROOT directory
	*/
	for (c = TABLE.root_base_sector; c < TABLE.root_base_sector + TABLE.sectors_in_root; c++) {
		int	j;

		if (read_sector(handle, c) == -1) 
			exit(1);

		for (j = 0; j < TABLE.bytes_per_sector; j++)
			sector_buffer[j] = '\0';

		if (write_sector(handle, c) == -1) {
			(void) fprintf(stderr, "dosformat: Error - write_sector() failed\n\tDisk may be unusable\n");
			exit(1);
		}
	}

	if (silent == 0) {
		(void) printf("Format complete\n");
		(void) fflush(stdout);
	}

	/*
		If user wants to label the volume, do it now
	*/
	if (label_volume) {
		(void) printf("\nVolume label (11 characters, <RETURN> for none)? ");

		if (my_fgets(buffer, 20, stdin) != NULL) 
			(void) make_label(handle, buffer);
	}

	/*
		No longer critical, disk is usable as is.
	*/
	critical(0);

	/*
		All done with our interface through libdos
	*/
	if (silent == 0) {
		long	bad;

		(void) printf("\n%10ld bytes total disk space\n", TABLE.bytes_per_sector * (TABLE.total_sectors - (TABLE.root_base_sector + TABLE.sectors_in_root)));

		bad = (TABLE.bytes_per_sector * (TABLE.total_sectors - (TABLE.root_base_sector + TABLE.sectors_in_root))) - free_space(handle);

		if (bad)
			(void) printf("%10ld bytes in bad sectors\n", bad);

		(void) printf("%10ld bytes available on disk\n", free_space(handle));
		(void) fflush(stdout);
	}

	(void) close_device(handle);

	exit(0);	/* NOTREACHED */
}
