/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/disk.c	1.1.3.1"

#include "../sys/boot.h"
#include "../sys/libfm.h"
#include "sys/hd.h"
#include "sys/vtoc.h"
#include "sys/alttbl.h"
#include "sys/fdisk.h"

/*
 * the following must be set up for the use of the high level filesystem 
 * routines
 */

struct 		alt_info Alt_tbl;	/* alternate sector table */
bfstyp_t	boot_fs_type=UNKNOWN;	/* boot file system,BFS,s5 or UNKNOWN */
bfstyp_t	root_fs_type=UNKNOWN;	/* root file system type */
off_t 		root_delta;		/* root starting sector on disk */
extern 		scsidiskflag;		/* is the hard disk a scsi disk? */

/* foll. is declared in bootlib/blfile.c */
extern	off_t 	boot_delta;		/* boot starting sector on disk */

/*
 * get_fs: 	initialize the driver; open the disk, find
 *		the root slice and fill in the global 
 *		variables and tables.
 *	returns the begining offset of BFS or ROOT.
 */

off_t
get_fs()
{
#if WINI
	struct pdinfo	*pip;
	struct vtoc	*vtp;
	daddr_t		secno;
	int		i;
	ushort		ptag;
	char		buf[SECSIZE];

	/* read in pdinfo to find where the bad block tables are */

	secno = unix_start + HDPDLOC;

	debug(printf("\nReading disk info (pdinfo) from sector %ld\n", \
			secno));

	disk(secno, physaddr(buf), (short)1);
	pip = (struct pdinfo *)buf;

	if (scsidiskflag)
	{
		Alt_tbl.alt_sec.alt_used = Alt_tbl.alt_trk.alt_used = 0;
	}
	else
	{
		/*
	 	* read in alternate sector table
	 	* (Assumes alt tbl starts on sector boundary).
	 	*/

		secno = unix_start + (pip->alt_ptr >> 9);
		i = (unsigned long) (pip->alt_len + (SECSIZE - 1)) >> 9;

		debug(printf("alts at sector(s) %ld to %ld\n", \
			secno, secno + i - 1));

		if (i > 0)
			disk( secno++, physaddr(&Alt_tbl), (short)i );

		if (Alt_tbl.alt_sanity != ALT_SANITY || 
				Alt_tbl.alt_version != ALT_VERSION)

			Alt_tbl.alt_sec.alt_used = Alt_tbl.alt_trk.alt_used = 0;
	}

	/*
	 * look in vtoc to find start of root partition
	 *	(Assumes vtoc is in same sector as pdinfo.)
	 */

	vtp = (struct vtoc *)&buf[pip->vtoc_ptr % SECSIZE];

	for (i = 0; i < (int)vtp->v_nparts; i++) {

		switch (vtp->v_part[i].p_tag)  {

		case V_STAND: 
			debug(printf("\nfound V_STAND partition\n"));
			boot_delta=vtp->v_part[i].p_start;
			boot_fs_type = BFS;		/* originally UNKNOWN */
			break;

		case V_ROOT: 
			debug(printf("\nfound V_ROOT partition\n"));
			root_delta=vtp->v_part[i].p_start;
			break;
		default:
			debug(printf("\ngetfs:pts=%d,i=%d:no match i/vtoc\n", \
				(int)vtp->v_nparts, i));
			break;
		}
	}

	if ( !boot_delta && !root_delta ) {
		printf("\nboot:No file system (stand or root) to boot from.\n");
		halt();
	}

#else	/* WINI */

	/* boot_fs_type is unknown at this point */
	binfo.bootflags = BF_FLOPPY;
	root_delta = spc;

#endif /* WINI */

	return (root_delta);
}
