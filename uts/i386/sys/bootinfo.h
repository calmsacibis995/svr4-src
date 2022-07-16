/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_BOOTINFO_H
#define _SYS_BOOTINFO_H

#ident	"@(#)head.sys:sys/bootinfo.h	1.1.2.1"

/*
 *	Definition of bootinfo structure.  This is used to pass
 *	information between the bootstrap and the kernel.
 */

#define BKI_MAGIC	0xff1234ff

#define B_MAXARGS	15		/* max. number of boot args */
#define B_STRSIZ	100		/* max length of boot arg. string */

struct bootmem {
	paddr_t		base;
	long		extent;
	ushort		flags;
	};

struct bootinfo {
	ulong	bootflags;		/* miscellaneous flags */

	struct hdparams { 		/* hard disk parameters */
		ushort	hdp_ncyl;	/* # cylinders (0 = no disk) */
		unchar	hdp_nhead;	/* # heads */
		unchar	hdp_nsect;	/* # sectors per track */
		ushort	hdp_precomp;	/* write precomp cyl */
		ushort	hdp_lz;		/* landing zone */
	} hdparams[2];			/* hard disk parameters */

	int	memavailcnt;
	struct	bootmem	memavail[B_MAXARGS];

	int	memusedcnt;
	struct	bootmem	memused[B_MAXARGS];

	int	bargc;			    /* count of boot arguments */
	char	bargv[B_MAXARGS][B_STRSIZ]; /* argument strings */

	char	id[5];			/* Contents of F000:E000 */
#ifdef MBUS
	caddr_t	bpsloc;
#endif
	int	checksum;
};

#if defined(MB1) || defined(MB2)
	unsigned long ramfsloc;
	unsigned long ramfssiz;
#endif

/* flags for struct mem flags */

#define B_MEM_NODMA	0x01
#define B_MEM_KTEXT	0x02
#define	B_MEM_KDATA	0x04
#define B_MEM_BOOTSTRAP	0x8000	/* Used internally by bootstrap */

#define BF_FLOPPY	0x01		/* booted from floppy */
#define BF_MB2SA	0x20000000	/* Kernel booted from MSA */
#define BF_TAPE		0x40000000	/* Kernel booted from tape */
#define BF_DEBUG	0x80000000	/* Bootloader  debug flag set by user */

#ifdef MB1
#define BOOTINFO_LOC	((paddr_t)0x1700)
#define KPTBL_LOC	((paddr_t)0x2000)  /* 8K Reserved for kernel pg table */
#endif

#if defined (MB2) 
#define BOOTINFO_LOC	((paddr_t)0xd000)
#define KPTBL_LOC	((paddr_t)0xe000)  /* 8K Reserved for kernel pg table */
#endif

#if defined (MB1) || defined (MB2) 
extern char * bpsinfo;			   /* defined by vuifile */
#endif

#if defined (AT386)
#define BOOTINFO_LOC	((paddr_t)0x600)
#define KPTBL_LOC	((paddr_t)0x1000)  /* Reserved for kernel page table */
#endif

extern struct bootinfo bootinfo;

#endif	/* _SYS_BOOTINFO_H */
