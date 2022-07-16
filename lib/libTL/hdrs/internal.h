/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:hdrs/internal.h	1.7.3.1"
/*
	This file contains internal definitions to TLlib.
*/

/* Internal ERRORS */
#define	TLRANGE	-100
#define	TLEOF	(TLRANGE - 1)
#define	TLINTERNAL	(TLRANGE - 2)

/* #DEFINES */
#define TLp_init( p ) strncpy( p, "", sizeof( parse_t ) )
#define TLt_valid(tid) 	(tid >= 0 && tid < TL_MAXTABLES \
		&& (TLtables[tid].status & IN_USE))

#define	E_GROWTH	25	/* Number of entries to grow entry table by */

extern char *malloc(), *realloc();
extern void free();

#ifndef TEST
#define	Malloc(s)	malloc( (unsigned)(s))
#define	Realloc(p,s)	realloc(p,s)
#define Free(a)	(void)free( (char *)(a))
#endif

/* Macros for getting at Table elements */
#define	ETAB(tid)	TLtables[tid].e_table
#define	EINFO(tid)	(&(TLtables[tid].e_info))

#define T_EMPTY(tid)	(EINFO(tid)->nentries == 0) 

/* Get a pointer to an entry in an entry table */
#define ENTRYTAB(tid,entryno)	(&(ETAB(tid)[entryno]))

/* nentries == # of entries in table */
#define T_EEND(tid)	(EINFO(tid)->nentries + 1)

/* Macros for getting at Entry elements */
#define E_SEEKADDR(e)	e->seekaddr
#define	E_FIELDS(e)	(&(e->fields))
#define	E_COMMENT(e)	e->comment
#define	E_NFIELDS(e)	(E_FIELDS(e)->count)
#define	E_VALUES(e)		(E_FIELDS(e)->values)
#define	E_ISCOMMENT(e)	(E_NFIELDS(e) == 0)
#define E_GETFIELD(e,n)	((E_FIELDS(e)->values) + n)


/* Entry Number Manipulations */
#define	IS_FROM_END(eno)	(eno > (1<<TLBITS))
#define	TLe_isnoncomment(e)	(e < TLELIMIT)
#define TLe_intable( t, e )	( e >= 1 && e < T_EEND(t) )
/* e_relative() converts an entryno into a table offset */
#define	TLe_minus( eno ) \
	(((eno ^ ((1<<TLBITS + 1)) - 1) + 1) & ((1<<(TLBITS + 1)) - 1))
#define	TLe_relative( t, eno ) \
	(IS_FROM_END(eno)? (T_EEND(t) - 1 - TLe_minus( eno )): eno )

/* Compare formats */
#define	TLe_diffformat( tid, entry ) \
	( E_NFIELDS(entry) > TLtables[tid].fieldnames.count )

extern char *strcpy();
extern int strcmp();
extern int	strlen();
extern int write();
extern long lseek();
extern int read();
extern char *strncpy();
extern int strncmp();

/* Lint Pacifiers */
#define	Strcpy( x, y )	(void)strcpy( (char *)(x), (char *)(y) )
#define Strcmp( x, y )	strcmp( (char *)(x), (char *)(y) )
#define Strlen( s )	strlen( (char *)(s) )
#define Write( f, b, n )	write( f, (char *)(b), (unsigned)(n) )
#define Lseek( f, o, w )	lseek( f, (long)(o), w )
#define Read( f, b, n )	read( f, (char *)(b), (unsigned)(n) )
#define	Strncpy( a, b, n )	strncpy( (char *)(a), (char *)(b), n )
#define	Strncmp( a, b, n )	strncmp( (char *)(a), (char *)(b), n )

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* STRUCTURES */
typedef	struct field_s {
	long count;
	unsigned char *values[ TL_MAXFIELDS ];
} field_t;

typedef struct file_s {
	unsigned char	*name;
	int fid, mode, oflag;
} file_t;

typedef struct entry_s {
	int status;
	long seekaddr;
	field_t fields;
	unsigned char *comment;
} entry_t;

/* Status field */
#define	IN_FILE	0x1
#define	IS_PARSED	0x2
#define IS_BEGIN	0x4
#define IS_END	0x8

/* Entry Table Info */
typedef struct entryinfo_s {
	int size;	/* Current Size of table */
	int nentries;	/* Number of entries currently in use */
} entryinfo_t;

/*	Internal TABLE Structure. */
typedef struct tbl_s {
	int status;
	file_t	file;
	long	hiwater;	/* last seekaddr we've seen in the file so far */
	TLdesc_t description;
	field_t fieldnames;
	entryinfo_t	e_info;	/* Entry table info */
	entry_t	**e_table;	/* Entry Table */
} tbl_t;

/* Status Field */
#define IN_USE	0x1
#define	MODIFIED	0x2
#define	FOUND_EOF	0x4
#define GOT_FORMAT	0x8

typedef struct parse_s {
	int type;
	unsigned char *comment;
	field_t fields;
	TLdesc_t descr;
} parse_t;

/* Parse Types */
#define PT_COMMENT	0x1
#define	PT_SPECIAL	0x2
