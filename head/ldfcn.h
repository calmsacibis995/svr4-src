/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:ldfcn.h	2.5.1.9"

#ifndef LDFILE

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _FILEHDR_H
#include <filehdr.h>
#endif

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
#define IOPTR(x)	(x)->ioptr
#define OFFSET(x)	(x)->offset
#define TYPE(x)		(x)->type
#define	HEADER(x)	(x)->header
#define LDFSZ		sizeof(LDFILE)

/*
	define various values of TYPE(ldptr)
*/

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

typedef struct
{
	char ar_name[16];
	time_t ar_date;
	uid_t ar_uid;
	gid_t ar_gid;
	mode_t ar_mode;
	off_t ar_size;
} archdr;

#define	ARCHDR	archdr
#define ARCHSZ	sizeof(ARCHDR)


/*
	define some useful symbolic constants
*/

#define SYMTBL	0	/* section nnumber and/or section name of the Symbol Table */

#define	SUCCESS	 1
#define	CLOSED	 1
#define	FAILURE	 0
#define	NOCLOSE	 0
#define	BADINDEX	-1L

#define	OKFSEEK	0

#ifndef _SYMS_H
#include <syms.h>
#endif

#ifndef _LINENUM_H
#include <linenum.h>
#endif

#ifndef _SCNHDR_H
#include <scnhdr.h>
#endif

#if defined(__STDC__)

extern int ldaclose(LDFILE *);
extern int ldahread(LDFILE *, ARCHDR *);
extern LDFILE *ldaopen(const char *, LDFILE *);
extern int ldclose(LDFILE *);
extern int ldfhread(LDFILE *, FILHDR *);
extern char *ldgetname(LDFILE *, const SYMENT *);
extern int ldlinit(LDFILE *, long);
extern int ldlitem(LDFILE *, unsigned int, LINENO *);
extern int ldlread(LDFILE *, long, unsigned int, LINENO *);
extern int ldlseek(LDFILE *, unsigned int);
extern int ldnlseek(LDFILE *, const char *);
extern int ldnrseek(LDFILE *, const char *);
extern int ldnsseek(LDFILE *, const char *);
extern int ldohseek(LDFILE *);
extern LDFILE *ldopen(const char *, LDFILE *);
extern int ldrseek(LDFILE *, unsigned int);
extern int ldshread(LDFILE *, unsigned int, SCNHDR *);
extern int ldnshread(LDFILE *, const char *, SCNHDR *);
extern int ldsseek(LDFILE *, unsigned int);
extern long ldtbindex(LDFILE *);
extern int ldtbread(LDFILE *, long, SYMENT *);
extern int ldtbseek(LDFILE *);
extern long sgetl(const char *);
extern void sputl(long, char *);

#else

extern LDFILE *ldopen();
extern LDFILE *ldaopen();
extern char *ldgetname();
extern long ldtbindex();
extern long sgetl();
extern void sputl();

#endif	/* __STDC__ */


/*
	define macros to permit the direct use of LDFILE pointers with the
	standard I/O library procedures
*/

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
#define STROFFSET(ldptr)	(HEADER(ldptr).f_symptr + HEADER(ldptr).f_nsyms * 18) /* 18 == SYMESZ */
#endif
