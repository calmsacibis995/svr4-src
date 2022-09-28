/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:i286sys/exec.h	1.1"

/*
 * exec.h
 *
 * Executable file info used by exec.c and text.c
 */

#define XNSCNMAX        16      /* maximum number of sections */

struct exdata {
	struct inode  *x_ip;    /* pointer to executable file inode */
	short   x_fmagic;       /* file header magic number */
	short   x_omagic;       /* "optional" unix header magic number */
	long    x_entloc;       /* entry location */
	long    x_tstart;       /* text start location */
	long    x_dstart;       /* data start location */
	unsigned short  x_nscns;        /* number of sections in x_scn array */
	unsigned short  x_ntscns;       /* number of text sections in x_scn array */

	/* shared library section info */
	long    x_loffset;      /* file offset of lib info section */
	unsigned short  x_lsize;        /* lib info section size */
	unsigned short  x_nshlibs;      /* number of shared libs */

	struct xscn {           /* COFF section info */
		long    xs_vaddr;       /* virtual addr in mem */
		long    xs_offset;      /* file offset */
		long    xs_size;        /* size in bytes */
		unsigned short  xs_flags;       /* type flags */
	} x_scn[XNSCNMAX];
};


/*
 *   Common object file header
 */

/* f_magic (magic number) distinguishes small and large model
 */

#define  I286SMAGIC       0512
#define  I286LMAGIC       0522

/* f_flags
 *
 *	F_EXEC  	file is executable (i.e. no unresolved 
 *	        	  externel references).
 *	F_AR16WR	this file created on AR16WR machine
 *	        	  (e.g. 11/70).
 *	F_AR32WR	this file created on AR32WR machine
 *	        	  (e.g. vax).
 *	F_AR32W		this file created on AR32W machine
 *	        	  (e.g. 3b,maxi).
 */
#define  F_EXEC		0000002
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000

struct filehdr {
	unsigned short	f_magic;	
	unsigned short	f_nscns;	/* number of sections */
	long	f_timdat;	/* time & date stamp */
	long	f_symptr;	/* file pointer to symtab */
	long	f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;
};

/*
 *  Common object file section header
 */

/*
 *  s_name
 */
#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"
#define _LIB  ".lib"

/*
 * s_flags
 */
#define	STYP_TEXT	0x0020	/* section contains text only */
#define STYP_DATA	0x0040	/* section contains data only */
#define STYP_BSS	0x0080	/* section contains bss only  */
#define STYP_LIB	0x0800	/* section contains lib only  */

struct scnhdr {
	char	s_name[8];	/* section name */
	long	s_paddr;	/* physical address */
	long	s_vaddr;	/* virtual address */
	long	s_size;		/* section size */
	long	s_scnptr;	/* file ptr to raw	*/
				/* data for section	*/
	long	s_relptr;	/* file ptr to relocation */
	long	s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation	*/
				/* entries		*/
	unsigned short	s_nlnno;	/* number of line	*/
				/* number entries	*/
	long	s_flags;	/* flags */
};

/*
 * Common object file optional unix header
 */

struct aouthdr {
	short	o_magic;	/* magic number */
	short	o_stamp;	/* stamp */
	long	o_tsize;	/* text size */
	long	o_dsize;	/* data size */
	long	o_bsize;	/* bss size */
	long	o_entloc;	/* entry location */
	long	o_tstart;
	long	o_dstart;
};
