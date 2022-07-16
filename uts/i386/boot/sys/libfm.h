/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/sys/libfm.h	1.1.3.1"

struct bootattr {
	ulong size;		/* size of file */
	time_t mtime;		/* modification time */
	time_t ctime;		/* creation time */
	ulong iosize;		/* I/O size most suited for FS */
	ulong blksize;		/* blksize of FS */
};

enum lfsect { TLOAD, DLOAD, NOLOAD, BKI };
typedef enum lfsect lfsect_t;

/*  common section for ELF or COFF */
struct bootsect {
	lfsect_t type;		/* type of section */
	ulong addr;		/* virtual address to load section */
	ulong size;		/* size of section */
	off_t offset;		/* offset in file */
};

enum lfhdr { COFF, ELF, NONE};
typedef enum lfhdr lfhdr_t;

struct boothdr {
	lfhdr_t type;		/* type of file */
	int nsect;		/* number of sections or segments */
	ulong entry;		/* entry point virtual */
};


struct nfso {
	long start;
	long size;
};

enum bfstyp { s5, BFS, UNKNOWN };
typedef enum bfstyp bfstyp_t;

#define	SECTSIZE dev_gran
#define NBSECT	13	/* number of sec/segments */
