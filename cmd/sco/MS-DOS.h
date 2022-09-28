/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:MS-DOS.h	1.3"

/*
	MS-DOS is a trademark of Microsoft Corporation
*/

/*
	Definitions for MS-DOS file systems
*/
#include	<sys/types.h>

/*
	Implementation Constants
*/

/*	ASSIGNMENTS is the pathname fo the default msdos configuration
	file. It should be set to "/etc/default/msdos"
*/
#define	ASSIGNMENTS		"/etc/default/msdos"

/*
	DEFAULT_[AB] specifies the default path to be used for drives
	A & B respectively.
*/
#define	DEFAULT_A		"/dev/rdsk/f0t"
#define	DEFAULT_B		"/dev/rdsk/f1t"

/*
	How many definitions will we allow for in the msdos 
	configuration file?
*/
#define	MAX_ASSIGNMENTS		26

/*
	How many unique devices will we support?
*/
#define MAX_DEVICES		26

/*
	Maximum size of a filename 
*/
#define	MAX_FILENAME		256

/*
	Maximum sector size of any device to be supported.
*/
#define	MAX_SECTOR_SIZE		1024

/*
	Maximum track size. This is used in dosformat.c when
	we need a buffer to hold the contents of a track. malloc(3[sc])
	memory faults when trying to allocate these BIG chunks of
	memory, so we make it static.
*/
#define	MAX_TRACK		20480

/*
	If MSDOSENV is defined, then it will be used to determine
	the location of the MS-DOS device control file.
*/
/* #define	MSDOSENV		"MSDOS"		/* */

/*
	MS-DOS Constants
*/
#define	BYTES_PER_DIR		32

/*
	MS-DOS Directory fields and their displacements
*/
#define	FILENAME		0	/* 8 bytes */
#define	EXTENSION		8	/* Filename Extension 3 bytes */
#define	FILE_ATTRIBUTE		11	/* File's attribute */
#define	TIME			22	/* Modification Time */
#define	DATE			24	/* Modification Date */
#define	STARTING_CLUSTER	26	/* Starting Cluster,  LSB 1st */
#define	FILE_SIZE		28	/* 4 Bytes */

/*
	Boot sector fields under MS-DOS
*/
#define	LOW_SECTOR_SIZE		11
#define	HI_SECTOR_SIZE		12

#define	SECTORS_PER_CLUSTER	13

#define	LOW_RESERVED_SECTORS	14
#define	HI_RESERVED_SECTORS	15

#define	NUMBER_OF_FATS		16

#define	LOW_ROOT_DIR_ENT	17
#define	HI_ROOT_DIR_ENT		18

#define	LOW_TOTAL_SECTORS	19
#define	HI_TOTAL_SECTORS	20

#define	MEDIA_DESCRIPTOR	21

#define	LOW_SECTORS_PER_FAT	22
#define	HI_SECTORS_PER_FAT	23

#define	LOW_SECTORS_PER_TRACK	24
#define	HI_SECTORS_PER_TRACK	25

#define	LOW_NUMBER_OF_HEADS	26
#define	HI_NUMBER_OF_HEADS	27

#define	LOW_HIDDEN_SECTORS	28
#define	HI_HIDDEN_SECTORS	29

/*
	MS-DOS FIle attriubute definitions
*/
#define	READ_ONLY	0x01
#define	HIDDEN		0x02
#define	SYSTEM		0x04
#define	LABEL		0x08
#define	SUB_DIRECTORY	0x10
#define	ARCHIVE		0x20

/*
	Our open device structure
*/
struct	table_struct {
	int	handle;
	char	*our_fat;
	int	sectors_per_cluster;
	unsigned	bytes_per_sector;
	int	reserved_sectors;
	int	number_of_fats;
	long	root_dir_ent;
	long	total_sectors;
	int	fat16;
	int	media_descriptor;
	long	sectors_per_fat;
	long	root_base_sector;
	long	sectors_in_root;
};

/*
	Our hardware structures
*/
struct	hardware_struct {
	char	device_letter;		/* MS-DOS Device letter */
	char	device_path[MAX_FILENAME]; /* UNIX Pathname */
};

/*
	Different type of magnetic media available
*/
#define	HW_525			0x00
#define	HW_35			0x01
#define	HW_HD			0x02

#define	del_cluster(handle, cluster)	chain_cluster(handle, 0, cluster)	/* */
#define	disp_dos_dir(a, b, c)		scan_dos_dir(a, b, c, "") /* */

extern	char	*basename();
extern	struct	table_struct	device_table[];
extern	char	filename[];
extern	struct	hardware_struct	hardware_table[];
extern	unsigned	char	sector_buffer[];
extern	char	volume_label[];
extern	int	we_are_dosdir;
extern	int	we_are_dosinfo;
extern	int	we_are_dosrm;
extern	int	we_are_dosrmdir;

extern	int	assignments_loaded;
extern	long	chain_cluster();
extern	void	critical();
extern	char	device_pathname[];
extern	long	dir_sector;
extern	long	dos_fil_size();
extern	char	*dos_mod_date();
extern	char	*dos_mod_time();
extern	char	drive;
extern	char	filename[];
extern	long	free_space();
extern	long	last_sector_read;
extern	long	next_cluster();
extern	long	next_sector();
extern	char	*my_fgets();
extern	long	ud_copy();

extern	int	errno;
extern	void	exit();
extern	void	free();
extern	char	*getenv();
extern	long	lseek();
extern	char	*malloc();
extern	int	optind;
extern	void	perror();
extern	char	*strcat();
extern	char	*strchr();
extern	char	*strrchr();
extern	char	*strcpy();
extern	char	*strtok();
extern	long	time();

#define		TABLE		device_table[index]
#define		HARDWARE	hardware_table[index]


/*
	Convert a passed cluster to a sector 
*/
#define	CLUS_2_SECT(cluster)	(((cluster - 2) * TABLE.sectors_per_cluster) + TABLE.root_base_sector + TABLE.sectors_in_root)

/*
	Convert a passed sector to a cluster
*/
#define	SECT_2_CLUS(sector)	(((sector - (TABLE.root_base_sector + TABLE.sectors_in_root)) / TABLE.sectors_per_cluster) + 2)

/*
	Determine total cluster on volume
*/
#define	TOTAL_CLUSTERS		(2 + ((TABLE.total_sectors - (TABLE.root_base_sector + TABLE.sectors_in_root)) / TABLE.sectors_per_cluster))

/*
	Retreive starting cluster value from the current sector which
	contains a directory entry at displacement disp.
*/
#define	GET_CLUS(disp)		((unsigned char) sector_buffer[STARTING_CLUSTER + disp] + (256 * (unsigned char) sector_buffer[STARTING_CLUSTER + disp + 1]))
