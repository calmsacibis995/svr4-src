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

#ident	"@(#)mbus:cmd/disksetup/boot.c	1.3"

/* #include <fcntl.h> */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <sys/bbh.h>

extern int errno;
extern char * devname;
extern char * bootname;
extern struct disk_parms dp;
extern int diskfd;

/*
 * Write the boot loader out to disk.  The IVLAB is also written
 * since it is surrounded by boot loader code.  The boot loader is
 * actually written by sgib. 
 */
loadboot()
{
	static char	buffer[256];

	/*
	 * Collect the required information.
	 */
#ifdef MB1
	sprintf( buffer,
		"sgib -o%d -c%d -s%d -d%d -f%d -M%s -N /etc/boot -r0 -i1 %s",
		dp.dp_sectors * dp.dp_heads * 2, dp.dp_cyls, dp.dp_sectors,
		dp.dp_secsiz, dp.dp_heads, bootname, devname);
#else
	sprintf( buffer,
		"sgib -o%d -c%d -s%d -d%d -f%d -M%s -r0 -i1 %s",
		dp.dp_sectors * dp.dp_heads * 2, dp.dp_cyls, dp.dp_sectors,
		dp.dp_secsiz, dp.dp_heads, bootname, devname);
#endif

#ifdef DEBUG
	printf("\n\nBuffer: %s\n\n\n",buffer); 
#endif

	if (system(buffer)){
		perror("Write of Boot Block failed\n");
		exit (errno);
	}

	/*
	 * update the disks version of the vlab
	 */

	if (ioctl(diskfd, V_R_VLAB, NULL) == -1){
		perror("Failed to update Drivers copy of Ivlab\n");
		exit (errno);
	}
	return(0);
}
