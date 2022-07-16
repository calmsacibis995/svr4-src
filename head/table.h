/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)table.h:table.h	1.2"
/*
	This file contains all structure definitions for the table
	management library.
*/

/*
	#DEFINES
*/
/* Maximum values of various things */
#define TL_MAXTABLES 20
#define	TL_MAXFIELDS 32
#define	TL_MAXLINESZ 512
#define	TLCOMMENT	(unsigned char *)"<COMMENT>"
#define TLTRAILING	(unsigned char *)"<TRAILING>"

/* Search Operations */
#define	TLEQ	-1
#define	TLNE	-2
#define	TLLT	-3
#define TLGT	-4
#define	TLLE	-5
#define	TLGE	-6
#define TLMATCH	-7
#define	TLNMATCH	-8
#define	TLNUMEQ	-9
#define	TLNUMNE	-10
#define	TLNUMLT	-11
#define TLNUMGT	-12
#define	TLNUMLE	-13
#define	TLNUMGE	-14

/* Search how_to_match argument values */
#define	TL_AND	1
#define	TL_NAND	2
#define	TL_OR	3
#define	TL_NOR	4

/* NOTE: Entryno must have at least TLBITS + 2 number of bits */
typedef	int entryno_t;

#define	TLBITS		14
#define	TLBEGIN		0
#define	TLEND		(1<<(TLBITS + 2))	/* This is NOT portable */

/* 
	RETURN CODES
*/
#define	TLSUBSTITUTION -11 /* Text to be used in a field contains one or more
	field separatiors, end-of-entry characters, or comment characters */
#define	TLBADFIELD	-10	/* Non-existent field */
#define	TLNOMEMORY	-9	/* A malloc() failed */
#define	TLBADENTRY	-8	/* Entry does not exist */
#define	TLBADFORMAT	-7	/* Format in table description has bad syntax */
#define	TLBADID	-6	/* Invalid tid given to a library routine */
#define	TLTOOMANY	-5	/* Attempting to open too many tables */
#define TLDESCRIPTION	-4	/* Semantic ambiguity in a table description */
#define TLTOOLONG	-3	/* An Entry in the table is too long */
#define	TLARGS	-2
#define	TLFAILED	-1
#define	TLOK	0
#define	TLBADFS	1	/* Field Separator in TLopen() call differs from
					that in the table */
#define	TLDIFFFORMAT	2	/* Format in TLopen() call or entry argument
							differs from that in the table */

/*
	Typedefs
*/

/*
	This structure describes what a table is supposed to look like.
*/
typedef struct TLdesc {
	unsigned char td_fs;	/* field separator - default ":" */
	unsigned char td_eoe;	/* End of entry - default "\n" */
	unsigned char td_comment;	/* comment character - default "#" */
	unsigned char *td_format;	/* entry format */
} TLdesc_t;

/* This hides the implementation of an ENTRY from applications */
typedef	long ENTRY;

/*
	This structure describes search operations.
*/
typedef	struct TLsearch	{
	unsigned char *ts_fieldname;
	unsigned char *ts_pattern;
	int	(*ts_operation)();
} TLsearch_t;

/* Declarations */
int TLopen(), TLsync(), TLclose(), TLread(), TLwrite(), TLdelete();
int TLappend(), TLassign(), TLsubst(), TLfreeentry(), TLsearch1();
ENTRY TLgetentry();
unsigned char *TLgetfield();

