/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)unix_conv:common/old.a.out.h	2.8.1.2"
/*
 *	layout of a.out file:
 *
 *	header of 8 words	magic number 0410:
 *					data starts at 1st 0777
 *					boundary above text
 *				magic number 0407:
 *					data starts immediately after
 *					text
 *				text size	)
 *				data size	) in bytes
 *				bss size	)
 *				symbol table size
 *				entry point
 *				size of text relocation info
 *				size of data relocation info
 *
 *  'segment'   origin   comments
 *	header:		0
 *	text:		32       0 padded to multiple of 4 bytes
 *	data:		32+textsize     0 padded to multiple of 4 bytes
 *	text relocation:	32+textsize+datasize
 *	data relocation:	32+textsize+datasize+textrelocationsize
 *	symbol table:	32+textsize+datasize+textrelocationsize+datarelocationsize
 *
 */



/*
 * Format of an a.out header
 */
 
struct	exec {	/* a.out header */
	short		a_magic;	/* magic number */
	short		a_stamp;	/* version stamp */
	unsigned	a_text;		/* size of text segment */
	unsigned	a_data;		/* size of initialized data */
	unsigned	a_bss;		/* size of uninitialized data */
	unsigned	a_syms;		/* size of symbol table */
	unsigned	a_entry;	/* entry point */
	unsigned	a_trsize;	/* size of text relocation */
	unsigned	a_drsize;	/* size of data relocation */
};

/* relocation table entry (text and data) structure */

struct relocation_info {
	  unsigned long	r_address;	/* relative to current segment */
	  unsigned long	r_symbolnum:24,
					/* if extern then symbol table */
					/* ordinal (0, 1, 2, ...) else */
					/* segment nbr (same as symbol types) */
	        	r_pcrel:1, 	/* if so, segment offset has already */
					/* been subtracted */
	  		r_length:2,	/* 0=byte, 1=word, 2=long */
	  		r_extern:1,	/* does not include value */
					/* of symbol referenced */
			r_pad:4;	/* unused bits */
};



/* symbol table entry structure */

struct	onlist {
	char	n_oname[8];	/* symbol name */
	char	n_type;		/* type flag */
	char	n_other;
	short	n_desc;
	unsigned n_value;	/* value */
};


/* "Magic" Defines */

#define	A_MAGIC1	0407		/* normal */
#define	A_MAGIC2	0410		/* read-only text */



/* values for type flag */


#define	ON_UNDF	0		/* undefined */
#define	ON_ABS	02		/* absolute */
#define	ON_TEXT	04		/* text */
#define	ON_DATA	06		/* data */
#define	ON_BSS	010
#define	ON_TYPE	037
#define	ON_FN	037		/* file name symbol */

#define	ON_GSYM	0040		/* global sym: name,,type,0 */
#define	ON_FNAME 0042		/* procedure name (f77 kludge): name,,,0 */
#define	ON_FUN	0044		/* procedure: name,,linenumber,address */
#define	ON_STSYM 0046		/* static symbol: name,,type,address */
#define	ON_LCSYM 0048		/* .lcomm symbol: name,,type,address */
#define	ON_BSTR  0060		/* begin structure: name,,, */
#define	ON_RSYM	0100		/* register sym: name,,register,offset */
#define	ON_SLINE	0104		/* src line: ,,linenumber,address */
#define	ON_ESTR  0120		/* end structure: name,,, */
#define	ON_SSYM	0140		/* structure elt: name,,type,struct_offset */
#define	ON_SO	0144		/* source file name: name,,,address */
#define	ON_BENUM 0160		/* begin enum: name,,, */
#define	ON_LSYM	0200		/* local sym: name,,type,offset */
#define	ON_SOL	0204		/* #line source filename: name,,,address */
#define	ON_ENUM	0220		/* enum element: name,,,value */
#define	ON_PSYM	0240		/* parameter: name,,type,offset */
#define	ON_ENTRY	0244		/* alternate entry: name,,linenumber,address */
#define	ON_EENUM 0260		/* end enum: name,,, */
#define	ON_LBRAC	0300		/* left bracket: ,,nesting level,address */
#define	ON_RBRAC	0340		/* right bracket: ,,nesting level,address */
#define	ON_BCOMM 0342		/* begin common: name,,, */
#define	ON_ECOMM 0344		/* end common: name,,, */
#define	ON_ECOML 0348		/* end common (local name): ,,,address */
#define	ON_STRU	0374		/* 2nd entry for structure: str tag,,,length */
#define	ON_LENG	0376		/* additional entry with length: ,,,length */

#define	ON_EXT	01		/* external bit, or'ed in */

#define	STABTYPES	0340
