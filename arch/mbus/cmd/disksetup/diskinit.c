/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/disksetup/diskinit.c	1.3"

/* The diskinit.c file contains Multibus specific routines used by disksetup to */
/* initialize the disk. 						  */

#include <sys/types.h>
#include <sys/fdisk.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/alttbl.h>
#include <sys/vtoc.h>
#include <sys/ivlab.h>
#include <sys/bbh.h>

#define TRUE		1
#define RESERVED        34	/* reserved sectors at start of drive */
#define ROOTSLICE	1
#define SWAPSLICE	2
#define DOSSLICE	5
#define BOOTSLICE	7
#define ALTSSLICE	8
#define TALTSLICE	9

#define MBUS_RESERVED 3		/*cyls reserved for mfg defects */

extern int	diskfd;        	/* file descriptor for raw wini disk */
extern int	scsi_flag;   	/* flag indicating drive is scsi */
extern struct	disk_parms dp;  /* Disk parameters returned from driver */
extern struct	vtoc 	vtoc;	/* struct containing slice info */
extern struct	pdinfo 	pdinfo; /* struct containing disk param info */
extern char	replybuf[80];   /* used for user replies to questions */
extern char	*devname;	/* pointer to device name */
extern int	cylsecs;        /* number of sectors per cylinder */
extern long	cylbytes;       /* number of bytes per cylinder */
extern daddr_t	unix_base;	/* first sector of UNIX System partition */
extern daddr_t	unix_size;	/* # sectors in UNIX System partition */
extern int	pstart;		/* next slice start location */
extern struct	absio absio;
extern struct	alt_info alttbls; /* struct contains bad sec & track info */
struct ivlab	*vlab;

/* get_unix_partition will read in partition table from sector 0 of the disk  */
/* it will verify its sane and then search for an active unix partition. If   */
/* found it will save the start and size of the partition. Next it will check */
/* for a Dos partition and will place it in slice 5 of the vtoc for vpix use  */
get_unix_partition()
{
	int i;

	int nalts;
	absio.abs_sec = 0;
	absio.abs_buf = (char *)vlab;
	if (ioctl(diskfd, V_RDABS, &absio) < 0) {
		sprintf(replybuf,
		"Invalid Volume label on%s\nPlease do a low-level format before proceding",devname);
		perror(replybuf);
		exit(70);
	}
	cylsecs = (int)dp.dp_heads * dp.dp_sectors;
	cylbytes = (long)cylsecs * dp.dp_secsiz;
	unix_base = 0;
		nalts = NALTS((int)dp.dp_heads,(int)dp.dp_cyls);
	unix_size = 
		cylsecs * (dp.dp_cyls - nalts - MBUS_RESERVED );


	/* Initialize vtoc */
	memset((char *)&vtoc, 0, sizeof(vtoc));

/*
	printf("Heads %d\n",dp.dp_heads);
	printf("Cyls %d\n",dp.dp_cyls);
	printf("Sec %d\n",dp.dp_sectors);
	printf("Hardware Alternates %d\n",nalts);
	printf("Unix size %d\n",unix_size);
*/
}

init_structs()
{
	
	/* Initialize alt_info struct alttbls */
	memset((char *)&alttbls, 0, sizeof(alttbls));
	alttbls.alt_sanity = ALT_SANITY;
	alttbls.alt_version = ALT_VERSION;

	/* Initialize pdinfo structure */
	pdinfo.driveid = 0;		/* reasonable default value	*/
	pdinfo.sanity = VALID_PD;
	pdinfo.version = V_VERSION;
	strncpy(pdinfo.serial, "            ", sizeof(pdinfo.serial));
	pdinfo.cyls = dp.dp_cyls;
	pdinfo.tracks = dp.dp_heads;
	pdinfo.sectors = dp.dp_sectors;
	pdinfo.bytes = dp.dp_secsiz;
	pdinfo.logicalst = dp.dp_pstartsec;
	pdinfo.vtoc_ptr = dp.dp_secsiz * VTOC_SEC + sizeof(pdinfo);
	pdinfo.vtoc_len =  sizeof(vtoc);
	if (scsi_flag == TRUE) {
		pdinfo.alt_ptr = 0;
		pdinfo.alt_len = 0;
	}
	else {
		pdinfo.alt_ptr = dp.dp_secsiz * (VTOC_SEC + 1);
		pdinfo.alt_len = sizeof(struct alt_info);
	}
	pdinfo.mfgst = (dp.dp_cyls - 3) * dp.dp_heads * dp.dp_sectors;
	pdinfo.mfgsz = cylsecs * 2;
	pdinfo.relst = START_ALT(
		(int)dp.dp_heads,(int)dp.dp_cyls,(int)dp.dp_sectors);
	pdinfo.relsz = NALTS((int)dp.dp_heads,(int)dp.dp_cyls) * cylsecs;
	pdinfo.relnext = pdinfo.relst;
	pdinfo.relno = 0;

	/* Initialize vtoc */
	vtoc.v_sanity = VTOC_SANE;
	vtoc.v_version = V_VERSION;
	strncpy(vtoc.v_volume, devname, sizeof(vtoc.v_volume));
	vtoc.v_nparts = 1;
	vtoc.v_part[0].p_tag = V_BACKUP;
	vtoc.v_part[0].p_flag = V_UNMNT | V_VALID;
	vtoc.v_part[0].p_start = unix_base;
	vtoc.v_part[0].p_size = unix_size;
	vtoc.v_part[BOOTSLICE].p_tag = V_BOOT;
	vtoc.v_part[BOOTSLICE].p_flag = V_UNMNT | V_VALID;
	vtoc.v_part[BOOTSLICE].p_start = unix_base;
	vtoc.v_part[BOOTSLICE].p_size = RESERVED;
	pstart = unix_base + RESERVED;
}

disk_sync(){

	if (ioctl(diskfd, V_R_VLAB, NULL) == -1)
		fprintf(stderr,"Error in reading the IVLAB\n");
	if (ioctl(diskfd, V_R_PDIN, NULL)  == -1)
		fprintf(stderr,"Error in reading the PIDIN\n");
	if (ioctl(diskfd, V_R_VTOC, NULL)  == -1)
		fprintf(stderr,"Error in reading the VTOC\n");
	if (ioctl(diskfd, V_R_SWALT, NULL) == -1)
		fprintf(stderr,"Error in reading the SWALT\n");
	if(ioctl(diskfd, V_R_MDL, NULL)   == -1)
		fprintf(stderr,"Error in reading the MDL\n");
}
fsgap(){
	return(1); /* mbus controlers always use gap of 1 */
}
