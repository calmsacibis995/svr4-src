/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/merge386.h	1.1.2.1"

/***************************************************************************

       Copyright (c) 1988 Locus Computing Corporation.
       All rights reserved.
       This is an unpublished work containing CONFIDENTIAL INFORMATION
       that is the property of Locus Computing Corporation.
       Any unauthorized use, duplication or disclosure is prohibited.

***************************************************************************/

/*
**  merge386.h
**		Only the structure definitions and defines needed
**		for the Merge hooks in the kernel files.
*/

#define	DOSSPSZ	0x100000	/* size of DOS space = 1 meg */
#define	NPPEMS 4		/* number of standard pages per EMS page */
#define	EMSPGSZ	0x4000		/* number of bytes in EMS page */
#define	NEMS_1MEG (DOSSPSZ/EMSPGSZ) /* number of EMS pages in 1 meg */

/* Constants used for validation of offsets for mapped rom files. */
#define	MIN_ROM_OFFSET	0x000c0000
#define	MAX_ROM_OFFSET	0x000f0000
#define	REGION_OFFSET	0x00ffffff

typedef struct	mproc {
	struct	vm86	*p_vm86p;
} mproc_t;

/* max number of files allowed to be mapped into a vm region */
#define	VMF	5

typedef struct mregion {
	struct	vmipage	*mr_vm86pgp;	/* ptr to information page */
	struct	inode	*mr_ip[VMF+1];	/* list of inode ptrs to mapped files*/
					/* Add one null ptr at end to make */
					/* searches fast. */
	int		mr_Bva[VMF];	/* offset into region file begins at */
	int		mr_Eva[VMF];	/* offset into region file ends at */
	int		mr_filesz[VMF];	/* our alternate for rp->r_filesz */
					/* i.e. the size in bytes of section */
					/* of file from which this region is */
					/* loaded. */
	int		linkcount[NEMS_1MEG];/* link count for each 16K page */
					/* in a 1M DOS space */
} mreg_t;


/*
** devfunc allows us to find the device handler
** related to a virtual machine port.
*/

struct	devfunc	{
	int	df_loport;		/* Lowest port assoc with that	*/
					/* device handler.		*/
	int	df_hiport;		/* Highest port.		*/
	struct vpistruct *df_vpistr;	/* Pointer to vpistruct.	*/
};
