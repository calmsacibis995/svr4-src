/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/diskstat.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)diskstat.c	3.8	LCC);	/* Modified: 16:24:12 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#if	defined(BERK42FILE) && !defined(CCI) && !defined(LOCUS) && !defined(DGUX)
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fstab.h>
#include	<fcntl.h>
#include	"const.h"
#include	<string.h>

/*			External Functions & Variables			*/

static  int
	fstbl_entries;			/* Number of entries in table */

struct	dev_entry {
	dev_t	rdev;			/* Major/mior device number */
	long	disksize;		/* Size of file system blocks */
	long	diskfree;		/* free blocks in file system */
};

struct	dev_entry
	*disktab = NULL;		/* Pointer to block-size table */

extern char
	*malloc();

extern int
	errno;

#endif	  /* BERK42FILE and not CCI and not LOCUS */


void
diskstat(device,totlDisk,freeDisk)
int
	device;
long  
	*totlDisk,
	*freeDisk;
{
#if	defined(BERK42FILE) && !defined(CCI) && !defined(LOCUS) && !defined(DGUX)
    register int
	i,
	fstabdesc,			/* File descriptor of /etc/fstab */
	devicedesc;			/* File descriptor of device */

	static int diskTabDesc = -1; 
	static int disktabsiz;

    char 
	devicename[MAX_PATH];

    struct fstab
	*fstab_entry;			/* Entry of /etc/fstab */

    struct stat
	filstat;			/* Contains data from stat() */

    *totlDisk = 32000000;
    *freeDisk = 32000000;

    if (disktab == NULL) {

    	if (stat(FS_TABLE, &filstat) < 0)
		return;

    	if ((fstabdesc = open(FS_TABLE, O_RDONLY)) < 0)
		return;

    	/* Calculate the number of entries in /etc/fstab */
    	fstbl_entries = filstat.st_size/sizeof(struct fstab);

    	/* Get enough memory for blocksize table */
    	disktabsiz = fstbl_entries * (int) sizeof(struct fstab);
    	if ((disktab = (struct dev_entry *)malloc(disktabsiz)) == NULL)
		return;
    }

    if (diskTabDesc < 0) {
    	if ((diskTabDesc = open(DISKTAB, O_RDONLY)) < 0) {
			log("Cound not open disktab.\n");
			return;
    	}
    }
    else
	lseek(diskTabDesc, 0L, 0);

    read(diskTabDesc,disktab,disktabsiz);

    for (i = 0; i < fstbl_entries; i++) {
	if (device == disktab[i].rdev) {
	    *totlDisk = disktab[i].disksize;
	    *freeDisk = disktab[i].diskfree;
	}
    }
#endif	  /* BERK42FILE and not CCI and not LOCUS */

}
