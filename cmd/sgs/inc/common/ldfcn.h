/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:common/ldfcn.h	1.9"
/*
 */
#include "syms.h"
#include "linenum.h"
#include "scnhdr.h"

#ifndef LDFILE
struct	ldfile {
	int	_fnum_;		/* so each instance of an LDFILE is unique */
	FILE	*ioptr;		/* system I/O pointer value */
	long	offset;		/* absolute offset to the start of the file */
	FILHDR	header;		/* the file header of the opened file */
	unsigned short	type;		/* indicator of the type of the file */
};


/*
	provide a structure "type" definition, and the associated
	"attributes"
*/

#define	LDFILE		struct ldfile
#define IOPTR(x)	x->ioptr
#define OFFSET(x)	x->offset
#define TYPE(x)		x->type
#define	HEADER(x)	x->header
#define LDFSZ		sizeof(LDFILE)

/*
	define various values of TYPE(ldptr)
*/

#define TVTYPE	TVMAGIC	
#if u3b5
#define USH_ARTYPE	ARTYPE
#else
#define USH_ARTYPE	(unsigned short) ARTYPE
#endif
#define ARTYPE 	0177545

/*
	define symbolic positioning information for FSEEK (and fseek)
*/

#define BEGINNING	0
#define CURRENT		1
#define END		2

/*
	define a structure "type" for an archive header
*/

#if defined(PORTAR) || defined(PORT5AR)
typedef struct
{
	char ar_name[16];
	long ar_date;
	int ar_uid;
	int ar_gid;
	long ar_mode;
	long ar_size;
} archdr;

#define	ARCHDR	archdr
#else
#define	ARCHDR	struct ar_hdr	/* ARCHIVE is defined in ar.h */
#endif
#define ARCHSZ	sizeof(ARCHDR)


/*
	define some useful symbolic constants
*/

#define SYMTBL	0	/* section nnumber and/or section name of the Symbol Table */
#define SYMESZ 18  	/* Size of symbol table entry */

#define	SUCCESS	 1
#define	CLOSED	 1
#define	FAILURE	 0
#define	NOCLOSE	 0
#define	BADINDEX	-1L

#define	OKFSEEK	0

/*
	define macros to permit the direct use of LDFILE pointers with the
	standard I/O library procedures
*/

#if defined(__STDC__)
extern int 	ldaclose(LDFILE *);
extern int 	ldahread(LDFILE *, ARCHDR *);
extern LDFILE 	*ldaopen(const char *, LDFILE *);
extern int 	ldclose(LDFILE *);
extern int 	ldfhread(LDFILE *, FILHDR *);
extern char 	*ldgetname(LDFILE *, const SYMENT *);
extern int 	ldlinit(LDFILE *, long);
extern int 	ldlitem(LDFILE *, unsigned int, LINENO *);
extern int 	ldlread(LDFILE *, long, unsigned int, LINENO *);
extern int 	ldlseek(LDFILE *, unsigned int);
extern int 	ldnlseek(LDFILE *, const char *);
extern int 	ldnrseek(LDFILE *, const char *);
extern int 	ldnsseek(LDFILE *, const char *);
extern int 	ldohseek(LDFILE *);
extern LDFILE 	*ldopen(const char *, LDFILE *);
extern int 	ldrseek(LDFILE *, unsigned int);
extern int 	ldshread(LDFILE *, unsigned int, SCNHDR *);
extern int 	ldnshread(LDFILE *, const char *, SCNHDR *);
extern int 	ldsseek(LDFILE *, unsigned int);
extern long 	ldtbindex(LDFILE *);
extern int 	ldtbread(LDFILE *, long, SYMENT *);
extern int 	ldtbseek(LDFILE *);
extern long 	sgetl(const char *);
extern void 	sputl(long, char *);
#else
extern int      ldaclose();
extern int      ldahread();
extern LDFILE   *ldaopen();
extern int      ldclose();
extern int      ldfhread();
extern char     *ldgetname();
extern int      ldlinit();
extern int      ldlitem();
extern int      ldlread();
extern int      ldlseek();
extern int      ldnlseek();
extern int      ldnrseek();
extern int      ldnsseek();
extern int      ldohseek();
extern LDFILE   *ldopen();
extern int      ldrseek();
extern int      ldshread();
extern int      ldnshread();
extern int      ldsseek();
extern long     ldtbindex();
extern int      ldtbread();
extern int      ldtbseek();
extern long     sgetl();
extern void     sputl();
#endif

#define GETC(ldptr)	getc(IOPTR(ldptr))
#define GETW(ldptr)	getw(IOPTR(ldptr))
#define FEOF(ldptr)	feof(IOPTR(ldptr))
#define FERROR(ldptr)	ferror(IOPTR(ldptr))
#define FGETC(ldptr)	fgetc(IOPTR(ldptr))
#define FGETS(s,n,ldptr)	fgets(s,n,IOPTR(ldptr))
#define FILENO(ldptr)	fileno(IOPTR(ldptr))
#define FREAD(p,s,n,ldptr)	fread(p,s,n,IOPTR(ldptr))
#define FSEEK(ldptr,o,p)	fseek(IOPTR(ldptr),(p==BEGINNING)?(OFFSET(ldptr)+o):o,p)
#define FTELL(ldptr)	ftell(IOPTR(ldptr))
#define FWRITE(p,s,n,ldptr)       fwrite(p,s,n,IOPTR(ldptr))
#define REWIND(ldptr)	rewind(IOPTR(ldptr))
#define SETBUF(ldptr,b)	setbuf(IOPTR(ldptr),b)
#define UNGETC(c,ldptr)		ungetc(c,IOPTR(ldptr))
#define STROFFSET(ldptr) (HEADER(ldptr).f_symptr + HEADER(ldptr).f_nsyms * SYMESZ) 
#endif
