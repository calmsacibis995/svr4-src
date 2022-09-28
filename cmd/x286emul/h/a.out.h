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

#ident	"@(#)x286emul:h/a.out.h	1.1"

/*
 *	<a.out.h> - Object file structure declarations.
 */


/*
 *	The main and extended header structures.
 *	For x.out segmented (XE_SEG):
 *	    1) fields marked with (s) must contain sums of xs_psize for
 *		non-memory images, or xs_vsize for memory images.
 *	    2) the contents of fields marked with (u) are undefined.
 */

/*
 *	For the 386 all these fields assume their full 32 bit meaning
 *	
 *	This is still sufficient for 386 large model so long as the
 *	sum of the virtual sizes of the segments does not exceed 4Gb.
 *	
 *	The linker must generate some new data here, specifically:
 *	
 *		x_cpu == XC_386 
 *		x_renv	has new bits interpretations controlled by
 *			the -V option of the linker (see below).
 */	

struct xexec {			    /* x.out header */
	unsigned short	x_magic;	/* magic number */
	unsigned short	x_ext;		/* size of header extension */
	long		x_text;		/* size of text segment (s) */
	long		x_data;		/* size of initialized data (s) */
	long		x_bss;		/* size of uninitialized data (s) */
	long		x_syms;		/* size of symbol table (s) */
	long		x_reloc;	/* relocation table length (s) */
	long		x_entry;	/* entry point, machine dependent */
	char		x_cpu;		/* cpu type & byte/word order */
	char		x_relsym;	/* relocation & symbol format (u) */
	unsigned short	x_renv;		/* run-time environment */
};

/*
 *	For the 386 the following fields are affected:
 *
 *	xe_pagesize	set by the linker option -R this
 *			is currently 0 and should be set to the pagesize
 *			of the 386 XENIX implementation modulo 512.
 */

struct xext {			    /* x.out header extension */
	/* The following 4 fields are UNUSED currently */
	long		xe_trsize;	/* size of text relocation (s) */
	long		xe_drsize;	/* size of data relocation (s) */
	long		xe_tbase;	/* text relocation base (u) */
	long		xe_dbase;	/* data relocation base (u) */
	/* End of unused fields */
	long		xe_stksize;	/* stack size (if XE_FS set) */
				/* the following must be present if XE_SEG */
	long		xe_segpos;	/* segment table position */
	long		xe_segsize;	/* segment table size */
	long		xe_mdtpos;	/* machine dependent table position */
	long		xe_mdtsize;	/* machine dependent table size */
	char		xe_mdttype;	/* machine dependent table type */
	char		xe_pagesize;	/* file pagesize, in multiples of 512 */
	char		xe_ostype;	/* operating system type */
	char		xe_osvers;	/* operating system version */
	unsigned short	xe_eseg;	/* entry segment, machine dependent */
	unsigned short	xe_sres;	/* reserved */
};


/*
 *	for the 386:
 *
 *	xs_filpos	set to a multiple of xe_pagesize*512 to allow
 *			demand loading from the file system.
 *	xs_rbase	modulo xe_pagesize*512 and set using the -D and
 *			-T options of the linker.
 */

struct xseg {			    /* x.out segment table entry */
	unsigned short	xs_type;	/* segment type */
	unsigned short	xs_attr;	/* segment attributes */
	unsigned short	xs_seg;		/* segment number */
	char		xs_align;	/* log base 2 of alignment */
	char		xs_cres;	/* unused */
	long		xs_filpos;	/* file position */
	long		xs_psize;	/* physical size (in file) */
	long		xs_vsize;	/* virtual size (in core) */
	long		xs_rbase;	/* relocation base address/offset */
	unsigned short	xs_noff;	/* segment name string table offset */
	unsigned short	xs_sres;	/* unused */
	long		xs_lres;	/* unused */
};


struct xiter {			    /* x.out iteration record */
	long		xi_size;	/* source byte count */
	long		xi_rep;		/* replication count */
	long		xi_offset;	/* destination offset in segment */
};


struct xlist { 			    /* xlist structure for xlist(3). */
	unsigned short	xl_type;	/* symbol type */
	unsigned short	xl_seg;		/* file segment table index */
	long		xl_value;	/* symbol value */
	char		*xl_name;	/* pointer to asciz name */
};


/*
 *      Definitions for xexec.x_magic, HEX (short).
 */

#define ARCMAGIC	0xff65	/* 0177545, archive, same as always */
#define X_MAGIC		0x0206	/* indicates x.out header */


/*
 *      Definitions for xexec.x_cpu, cpu type (char).
 *
 *	b       	set if high byte first in short
 *	 w              set if low word first in long
 *	  cccccc	cpu type
 */

	/* bytes/words are "swapped" if not stored in pdp11 ordering */
#define XC_BSWAP	0x80	/* bytes swapped */
#define XC_WSWAP	0x40	/* words swapped */

#define XC_NONE		0x00	/* none */
#define XC_PDP11	0x01	/* pdp11 */
#define XC_23		0x02	/* 23fixed from pdp11 */
#define XC_Z8K		0x03	/* Z8000 */
#define XC_8086		0x04	/* I8086 */
#define XC_68K		0x05	/* M68000 */
#define XC_Z80		0x06	/* Z80 */
#define XC_VAX		0x07	/* VAX 780/750 */
#define XC_16032	0x08	/* NS16032 */
#define XC_286		0x09	/* iAPX 80286 */
#define XC_286V		0x29	/* iAPX 80286, use xe_osver for version */
#define XC_386		0x0a	/* iAPX 80386 */
#define XC_186		0x0b	/* iAPX 80186 */
#define XC_CPU		0x3f	/* cpu mask */


/*
 *	Definitions for xexec.x_relsym (char), valid only if !XE_SEG.
 *
 *	rrrr            relocation table format
 *	    ssss        symbol table format
 */

	/* relocation table format */
#define XR_RXOUT	0x00	/* x.out long form, linkable */
#define XR_RXEXEC	0x10	/* x.out short form, executable */
#define XR_RBOUT	0x20	/* b.out format */
#define XR_RAOUT	0x30	/* a.out format */
#define XR_R86REL	0x40	/* 8086 relocatable format */
#define XR_R86ABS	0x50	/* 8086 absolute format */
#define XR_R286ABS	0x60	/* 80286 absolute format */
#define XR_R286REL	0x70	/* 80286 relocatable format */
#define XR_REL		0xf0	/* relocation format mask */

	/* symbol table format */
#define XR_SXOUT	0x00	/* trailing string, struct sym */
#define XR_SBOUT	0x01	/* trailing string, struct bsym */
#define XR_SAOUT	0x02	/* struct asym (nlist) */
#define XR_S86REL	0x03	/* 8086 relocatable format */
#define XR_S86ABS	0x04	/* 8086 absolute format */
#define XR_SUCBVAX	0x05	/* separate string table */
#define XR_S286ABS	0x06	/* 80286 absolute format */
#define XR_S286REL	0x07	/* 80286 relocatable format */
#define XR_SXSEG	0x08	/* segmented format */
#define XR_SYM		0x0f	/* symbol format mask */


/*
 *      Definitions for xexec.x_renv (short).
 *
 *	vv                  version compiled for
 *	  xx                extra (zero)
 *	    s               set if segmented x.out
 *	     a              set if absolute (set up for physical address)
 *	      i             set if segment table contains iterated text/data
 *	       v            set if virtual kernel module or shared library
 *				   was (h) but this was never used.
 *	        f           set if floating point hardware required
 *	         t          set if large model text
 *	          d         set if large model data
 *	           o        set if text overlay
 *	            f       set if fixed stack
 *	             p      set if text pure
 *	              s     set if separate I & D
 *	               e    set if executable
 */

/*
 * 	On the 386 the validity of a module and its type
 *	is determined by the settings of the XE_ABS, XE_VMOD and XE_EXEC
 *	bits as follows:
 *
 *	XE_ABS	XE_VMOD	XE_EXEC			Meaning
 *	  0	   0       0		BAD x.out (error in linking)
 *	  0	   0	   1		Old x.out, no shared libraries used.
 *	  0	   1	   0		Shared Library Module.
 *	  0	   1	   1		x.out executable that uses shared libs.
 *	  1	   0	   0		BAD (not possible)
 *	  1	   0	   1		Standalone Program (e.g. kernel)
 *	  1	   1	   0		Virtual Kernel Module (e.g. IDD)
 *	  1	   1	   1		BAD (not possible)
 *
 *	The setting of the XE_VMOD bit is controlled by the -V option of
 * 	the linker.
 */

#define XE_V2		0x4000		/* version 2.x */
#define XE_V3		0x8000		/* version 3.x */
#define XE_OSV		0xc000		/* if XE_SEG use xe_osvers ... */
#define XE_V5		XE_OSV		/* else assume v5.x */
#define XE_VERS		0xc000		/* version mask */

#define XE_res1		0x2000		/* reserved */
#define XE_res2		0x1000		/* reserved */
#define XE_SEG		0x0800		/* segment table present */
#define XE_ABS		0x0400		/* absolute memory image (standalone) */
#define XE_ITER		0x0200		/* iterated text/data present */
/* #define XE_HDATA	0x0100		/* huge model data (never used) */
#define XE_VMOD		0x0100		/* virtual module */
#define XE_FPH		0x0080		/* floating point hardware required */
#define XE_LTEXT	0x0040		/* large model text */
#define XE_LDATA	0x0020		/* large model data */
#define XE_OVER		0x0010		/* text overlay */
#define XE_FS		0x0008		/* fixed stack */
#define XE_PURE		0x0004		/* pure text */
#define XE_SEP		0x0002		/* separate I & D */
#define XE_EXEC		0x0001		/* executable */


/*
 *	Definitions for xe_mdttype (char).
 */

#define	XE_MDTNONE	0	/* no machine dependent table */
#define	XE_MDT286	1	/* iAPX286 LDT */


/*
 *	Definitions for xe_ostype (char).
 */

#define	XE_OSNONE	0
#define	XE_OSXENIX	1	/* Xenix */
#define	XE_OSRMX	2	/* iRMX */
#define	XE_OSCCPM	3	/* Concurrent CP/M */


/*
 *	Definitions for xe_osvers (char).
 */

#define	XE_OSXV2	0	/* Xenix V3.x */
#define	XE_OSXV3	1	/* Xenix V3.x */
#define	XE_OSXV5	2	/* Xenix V5.x */


/*
 *	Definitions for xs_type (short).
 *	  Values from 64 to 127 are reserved.
 */

#define	XS_TNULL	0	/* unused segment */
#define	XS_TTEXT	1	/* text segment */
#define	XS_TDATA	2	/* data segment */
#define	XS_TSYMS	3	/* symbol table segment */
#define	XS_TREL		4	/* relocation segment */
#define	XS_TSESTR	5	/* segment table's string table segment */
#define	XS_TGRPS	6	/* group definitions segment */

#define	XS_TIDATA	64	/* iterated data */
#define	XS_TTSS		65	/* tss */
#define	XS_TLFIX	66	/* lodfix */
#define	XS_TDNAME	67	/* descriptor names */
#define	XS_TDTEXT	68	/* debug text segment */
#define	XS_TIDBG	XS_TDTEXT
#define	XS_TDFIX	69	/* debug relocation */
#define	XS_TOVTAB	70	/* overlay table */
#define	XS_T71		71
#define	XS_TSYSTR	72	/* symbol string table */


/*
 *	Definitions for xs_attr (short).
 *	The top bit is set if the file segment represents a memory image.
 *	The low 15 bits' definitions depend on the type of file segment.
 */

#define XS_AMEM		0x8000	/* segment represents a memory image */
#define XS_AMASK	0x7fff	/* type specific field mask */

    /* For XS_TTEXT and XS_TDATA segments, bit definitions. */
#define XS_AITER	0x0001	/* contains iteration records */
#define XS_AHUGE	0x0002	/* contains huge element */
#define XS_ABSS		0x0004	/* contains implicit bss */
#define XS_APURE	0x0008	/* read-only, may be shared */
#define XS_AEDOWN	0x0010	/* segment expands downward (stack) */
#define XS_APRIV	0x0020	/* segment may not be combined */
#define	XS_A32BIT	0x0040	/* segment is 32 bits */

    /* For XS_TSYMS segments, enumerated symbol table types. */
#define XS_S5BELL	0	/* Bell 5.2 format */
#define XS_SXSEG	1	/* x.out segmented format */
#define	XS_SISLAND	2	/* island debugger support */

    /* For XS_TREL segments, enumerated relocation table types. */
#define XS_RXSEG	1	/* x.out segmented format */
#define XS_R86SEG	2	/* 8086 x.out segmented relocation */


/*
 *	File position macros, valid only if !XE_SEG.
 */

#define XEXTPOS(xp)	((long) sizeof(struct xexec))
#define XTEXTPOS(xp)	(XEXTPOS(xp) + (long) (xp)->x_ext)
#define XDATAPOS(xp)	(XTEXTPOS(xp) + (xp)->x_text)
#define XSYMPOS(xp)	(XDATAPOS(xp) + (xp)->x_data)
#define XRELPOS(xp)	(XSYMPOS(xp) + (xp)->x_syms)
#define XENDPOS(xp)	(XRELPOS(xp) + (xp)->x_reloc)

#define XRTEXTPOS(xp, ep)	(XRELPOS(xp))
#define XRDATAPOS(xp, ep)	(XRELPOS(xp) + (ep)->xe_trsize)


/*
 *	byte/word swapping macros:
 */

#define SBSWAP(x) ((((x) >> 8) & 0x00ff) | \
			(((x) << 8) & 0xff00))

#define LBSWAP(x)  ((((long) (x) >> 8) & 0x00ff00ffL) | \
			(((long) (x) << 8) & 0xff00ff00L))

#define LWSWAP(x)   ((((long) (x) >> 16) & 0x0000ffffL) | \
			   (((long) (x) << 16) & 0xffff0000L))

/* This hack avoids the collision of STRUCTOFF in sys/emap.h */
#ifdef STRUCTOFF
#undef STRUCTOFF
#endif
#define	STRUCTOFF(structure, field)	(int) &(((struct structure *) 0)->field)


#define H_NONE	0		/* not an object file */
#define H_AOUT	1		/* a.out */
#define H_BOUT	2		/* b.out */
#define H_ROUT	3		/* 8086 rel */
#define H_XROUT	4		/* 8086 rel with x.out header */
#define H_ZAOUT	5		/* z8000 a.out */
#define H_XOUT	6		/* x.out */
#define H_XSEG	7		/* segmented x.out */

#ifndef AT386
#define S_NONE	0		/* not an object file */
#endif	/* AT386 */
#define S_ASYM	1		/* a.out */
#define S_BSYM	2		/* b.out */
#define S_RSYM	3		/* 8086 rel */
#define S_XSYM	4		/* x.out symbols */
#define S_XSEG	5		/* x.out segmented symbols */
#define S_86ABS	6		/* 8086 abs symbols */
#define S_IDBG	7		/* debug symbols */
#define S_5BELL	8		/* Bell 5.2 symbols */
#define S_ISLAND 9		/* island debugger support */




/*
 *	All of the following are provided for compatibility only.
 */

struct aexec {			    /* a.out header */
	unsigned short	xa_magic;       /* magic number */
	unsigned short	xa_text;        /* size of text segment */
	unsigned short	xa_data;        /* size of initialized data */
	unsigned short	xa_bss;         /* size of unitialized data */
	unsigned short	xa_syms;        /* size of symbol table */
	unsigned short	xa_entry;       /* entry point */
	unsigned short	xa_unused;      /* not used */
	unsigned short	xa_flag;        /* relocation info stripped */
};


struct nlist {			    /* nlist structure for nlist(3). */
	char		n_name[8];	/* symbol name */
	int		n_type;		/* type flag */
	unsigned	n_value;	/* value */
};


/*
 *	Definitions for aexec.xa_magic, OCTAL, obsolete (short).
 */

#define FMAGIC		0407	/* normal */
#define NMAGIC		0410	/* pure, shared text */
#define IMAGIC		0411	/* separate I & D */
#define OMAGIC		0405	/* text overlays */
#define ZMAGIC		0413	/* demand load format */

#define	A_MAGIC1	FMAGIC
#define	A_MAGIC2	NMAGIC
#define	A_MAGIC3	IMAGIC
#define	A_MAGIC4	OMAGIC

#define Z_MAGIC1	0164007	/* normal 	    0xe807 */
#define Z_MAGIC2	0164010	/* pure only text   0xe808 */
#define Z_MAGIC3	0164011	/* separate I & D   0xe809 */
#define Z_MAGIC4	0164005	/* overlay	    0xe805 */


#define ATEXTPOS(ap)	((long) sizeof(struct aexec))
#define ADATAPOS(ap)	(ATEXTPOS(ap) + (long) (ap)->xa_text)
#define ARTEXTPOS(ap)	(ADATAPOS(ap) + (long) (ap)->xa_data)
#define ARDATAPOS(ap)	(ARTEXTPOS(ap) + ((long) \
			    ((ap)->xa_flag? 0 : (ap)->xa_text)))
#define ASYMPOS(ap)	(ATEXTPOS(ap) + \
			    (((ap)->xa_flag? 1L : 2L) * \
			((long) (ap)->xa_text + (long) (ap)->xa_data)))
#define AENDPOS(ap)	(ASYMPOS(ap) + (long) (ap)->xa_syms)




struct bexec {		    /* b.out header */
	long	xb_magic;	/* magic number */
	long	xb_text;	/* text segment size */
	long	xb_data;	/* data segment size */
	long	xb_bss;		/* bss size */
	long	xb_syms;	/* symbol table size */
	long	xb_trsize;	/* text relocation table size */
	long	xb_drsize;	/* data relocation table size */
	long	xb_entry;	/* entry point */
};


#define BTEXTPOS(bp)	((long) sizeof(struct bexec))
#define BDATAPOS(bp) 	(BTEXTPOS(bp) + (bp)->xb_text)
#define BSYMPOS(bp)	(BDATAPOS(bp) + (bp)->xb_data)
#define BRTEXTPOS(bp)	(BSYMPOS(bp) + (bp)->xb_syms)
#define BRDATAPOS(bp)	(BRTEXTPOS(bp) + (bp)->xb_trsize)
#define BENDPOS(bp)	(BRDATAPOS(bp) + (bp)->xb_drsize)

