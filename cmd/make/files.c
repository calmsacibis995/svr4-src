/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:files.c	1.17"
/*	@(#)make:files.c	1.13 of 5/9/89	*/

#include "defs"
#include <ar.h>
#include <pwd.h>
#include <sys/stat.h>
#include <ccstypes.h>

#define STREQN		!strncmp
#define MAXOPFIL	10	/* number of files that can be open */

extern	PATTERN	firstpat;	/* main.c */
extern	int	ndir;		/* number of directory levels in vpath */
extern	CHARSTAR directs[];     /* pointers to the viewpath members */


/* UNIX VERSION DEPENDENT PROCEDURES */

/*
 * For 6.0, create a make which can understand all three archive
 * formats.  This is kind of tricky, and <ar.h> isn't any help.
 * Note that there is no sgetl() and sputl() on the pdp11, so
 * make cannot handle anything but the one format there.
 */
char	archmem[64];	/* archive file member name to search for */

int	ar_type;	/* to distiguish which archive format we have */
#define ARpdp	1
#define AR5	2
#define ARport	3

long	first_ar_mem,	/* where first archive member header is at */
	sym_begin,	/* where the symbol lookup starts */
	num_symbols,	/* the number of symbols available */
	sym_size;	/* length of symbol directory file */

/*
 * Defines for all the different archive formats.  See next comment
 * block for justification for not using <ar.h>'s versions.
 */
#define ARpdpMAG	0177545	/* old format (short) magic number */

#define AR5MAG		"<ar>"	/* 5.0 format magic string */
#define SAR5MAG		4	/* 5.0 format magic string length */

#define ARportMAG	"!<arch>\n"	/* Port. (6.0) magic string */
#define SARportMAG	8		/* Port. (6.0) magic string length */
#define ARFportMAG	"`\n"		/* Port. (6.0) end of header string */

/*
* These are the archive file headers for the three formats.  Note
* that it really doesn't matter if these structures are defined
* here.  They are correct as of the respective archive format
* releases.  If the archive format is changed, then since backwards 
* compatability is the desired behavior, a new structure is added
* to the list.
*/
struct {	/* pdp11 -- old archive format */
	char	ar_name[14];	/* '\0' terminated */
	long	ar_date;	/* native machine bit representation */
	char	ar_uid;		/* 	"	*/
	char	ar_gid;		/* 	"	*/
	int	ar_mode;	/* 	"	*/
	long	ar_size;	/* 	"	*/
} ar_pdp;

struct {	/* pdp11 a.out header */
	short	a_magic;
	unsigned	a_text;
	unsigned	a_data;
	unsigned	a_bss;
	unsigned	a_syms;		/* length of symbol table */
	unsigned	a_entry;
	char	a_unused;
	char	a_hitext;
	char	a_flag;
	char	a_stamp;
} arobj_pdp;

struct {	/* pdp11 a.out symbol table entry */
	char	n_name[8];	/* null-terminated name */
	int	n_type;
	unsigned	n_value;
} ars_pdp;

struct {	/* UNIX 5.0 archive header format: vaxen and 3b */
	char	ar_magic[SAR5MAG];	/* AR5MAG */
	char	ar_name[16];		/* ' ' terminated */
	char	ar_date[4];		/* sgetl() accessed */
	char	ar_syms[4];		/* sgetl() accessed */
} arh_5;

struct {	/* UNIX 5.0 archive symbol format: vaxen and 3b */
	char	sym_name[8];	/* ' ' terminated */
	char	sym_ptr[4];	/* sgetl() accessed */
} ars_5;

struct {	/* UNIX 5.0 archive member format: vaxen and 3b */
	char	arf_name[16];	/* ' ' terminated */
	char	arf_date[4];	/* sgetl() accessed */
	char	arf_uid[4];	/*	"	*/
	char	arf_gid[4];	/*	"	*/
	char	arf_mode[4];	/*	"	*/
	char	arf_size[4];	/*	"	*/
} arf_5;

struct { 	/* Portable (6.0) archive format: vaxen and 3b */
	char	ar_name[16];	/* '/' terminated */
	char	ar_date[12];	/* left-adjusted; decimal ascii; blank filled */
	char	ar_uid[6];	/*	"	*/
	char	ar_gid[6];	/*	"	*/
	char	ar_mode[8];	/* left-adjusted; octal ascii; blank filled */
	char	ar_size[10];	/* left-adjusted; decimal ascii; blank filled */
	char	ar_fmag[2];	/* special end-of-header string (ARFportMAG) */
} ar_port;


#if 0		/* no longer used! */

/*
 *	New common object version for files.c
 */
char	archmem[64];
struct ar_hdr head;		/* archive file header */
struct ar_sym symbol;		/* archive file symbol table entry */
struct arf_hdr fhead;		/* archive file object file header */

#endif


FILE	*arfd;

int	nopen = 0;

/*
**	Declare local functions and make LINT happy.
*/

static DEPBLOCK	dodir();
static int	umatch();
static time_t	afilescan();
static time_t	entryscan();
static time_t	pdpentrys();
static int	openarch();
static DIR *	getfid();
static time_t	lookarch();


DEPBLOCK
srchdir(pat, mkchain, nextdbl)
register CHARSTAR pat;	/* pattern to be matched in directory */
int	mkchain;	/* nonzero if results to be remembered */
DEPBLOCK nextdbl;       /* final value for chain */
{
	extern	int	errno;
	register PATTERN patp;
	register int	level;
	DIR 	*dirf;
	CHARSTAR dirname, dirpref, filepat, dname(), sname();
	char	temp[MAXPATHLEN], temp2[MAXPATHLEN];
	char	pattemp[MAXPATHLEN], dirtemp[MAXPATHLEN];

	if ( !mkchain )
		for (patp = firstpat; patp; patp = patp->nextpattern)
			if (STREQ(pat, patp->patval))
				return(0);

	patp = ALLOC(pattern);
	patp->nextpattern = firstpat;
	firstpat = patp;
	patp->patval = copys(pat);

	(void)copstr(pattemp, pat);
	CHKARY(srchdir, pattemp, MAXPATHLEN)  /*macro defined in defs*/
	(void)copstr(dirtemp, pat);
	CHKARY(srchdir, dirtemp, MAXPATHLEN)  /*macro defined in defs*/
	(void)dname(dirtemp);
	if (STREQ(dirtemp, ".")) {
		dirpref = "";
		dirname = "";
	} else {
		dirpref = concat(dirtemp, "/", temp);
		CHKARY(srchdir, temp, MAXPATHLEN)  /*macro defined in defs*/
		dirname = concat("/", dirtemp, temp2);
		CHKARY(srchdir, temp2, MAXPATHLEN)  /*macro defined in defs*/
	}

	filepat = sname(pattemp);
	if (*pat == '/') {
		if ( !(dirf = getfid(dirname)) )
			return(nextdbl);
		else if (dirf != (DIR *) -1)
			nextdbl = dodir(dirf, filepat, dirpref, nextdbl, mkchain, ndir);
	} else
		for (level = 1; level <= ndir; level++) {
			(void)concat(directs[level], dirname, dirtemp);
			CHKARY(srchdir, dirtemp, MAXPATHLEN)  /*macro defined in defs*/
#ifdef MKDEBUG
			if (IS_ON(DBUG))
				printf("looking in [%s]\n", dirtemp);
#endif
			if ( !(dirf = getfid(dirtemp)) ) {
				continue;
			} else if (dirf != (DIR *) - 1)
				nextdbl = dodir(dirf, filepat, dirpref, nextdbl, mkchain, level);
		}

	return(nextdbl);
}


FSTATIC char file_name[MAXNAMLEN];


static DEPBLOCK
dodir(dirf, filepat, dirpref, nextdbl, mkchain, level)
DIR	*dirf;
register CHARSTAR filepat;
CHARSTAR dirpref;
DEPBLOCK nextdbl;
int	mkchain, level;
{
	register CHARSTAR p1, p2;
	DEPBLOCK thisdbl;
	register struct dirent * entry;
	char	fullname[MAXPATHLEN];
	int	amatch();
	NAMEBLOCK q;

	while((entry = readdir(dirf)) != NULL) {
		p1 = entry->d_name;
		p2 = file_name;

		CHKARY(dodir, entry->d_name, MAXNAMLEN) /*macro defined in defs*/
		while ( (*p2++ = *p1++) != CNULL ) ;

		if ( amatch(file_name, filepat) ) {
			CHKARY(dodir, file_name, (MAXNAMLEN-strlen(dirpref))) /*macro defined in defs*/
			(void)concat(dirpref, file_name, fullname);
			if ( !(q = SRCHNAME(fullname)) )
				q = makename(copys(fullname));

			/* define file's level if just found */

			if ( !(q->flevel) )
				q->flevel = level;
#ifdef MKDEBUG
			if (IS_ON(DBUG))
				printf("found %o:[%s] level = %d\n", q, q->namep, q->flevel);
#endif
			if (mkchain) {
				thisdbl = ALLOC(depblock);
				thisdbl->nextdep = nextdbl;
				thisdbl->depname = q;
				nextdbl = thisdbl;
			}
		}
	}

	return (nextdbl);
}


amatch(s, p)		/* stolen from glob through find */
register CHARSTAR p;
CHARSTAR s;
{
	register int	cc, scc = *s, k;
	int	c, lc = LRGINT;

	switch (c = *p) {

	case LSQUAR:
		k = 0;
		while (cc = *++p) {
			switch (cc) {

			case RSQUAR:
				if (k)
					return(amatch(++s, ++p));
				return(0);

			case MINUS:
				k |= (((lc <= scc) & (scc <= (cc = p[1]))));
			}
			if (scc == (lc = cc))
				k++;
		}
		return(0);

	case QUESTN:
caseq:		if (scc)
			return(amatch(++s, ++p));
		return(0);

	case STAR:
		return(umatch(s, ++p));

	case 0:
		return(!scc);
	}

	if (c == scc)
		goto caseq;

	return(0);
}


static int
umatch(s, p)
register CHARSTAR s, p;
{
	if ( !(*p) )
		return(1);
	while (*s)
		if (amatch(s++, p))
			return(1);
	return(0);
}


time_t
la(archive, member, flag)
char	*archive, *member;
int	flag;
{
	time_t	date;
	int	aropen = openarch(archive);

	if ((aropen < 0) && arfd)
		(void)fclose( arfd );

	if (aropen == -1)	/* can't open archive */
		return(0L);

/*
 *	The -2 return code for openarch was added so build would be able to
 *	force build an archive using a NULL archive.  A NULL archive
 *	contains only the string "!<arch>\n".
 *	The "make" version of openarch returns -1 if the archive is not
 *	able to be opened, which is correct for build also.  However, the
 *	"make" version also returned -1 for NULL archives, since the symbol
 *	table is not present.  The "build" version of openarch has been
 *	modified to allow distinction between archive open problems and
 *	symbol table problems for 6.0 archives.
 */
	if (aropen == -2)	/* NULL archive */
		return(1L);

	if (flag)
		date = entryscan(archive, member);
	else
		date = afilescan(archive, member);

	/* close the archive file */
	(void)fclose( arfd );

	return(date);
}


static time_t
afilescan(an, name)	/* return date for named archive member file */
char	*an, *name;
{
#ifndef pdp11
	long sgetl();
#endif
	long	ptr;
	size_t	len = strlen(name);

	if (fseek(arfd, first_ar_mem, 0))
seek_error:	fatal1("seek error on archive %s", an);

	/*
	* Hunt for the named file in each different type of
	* archive format.
	*/
	switch (ar_type) {
	case ARpdp:
		for (; ; ) {
			if (fread((char *) &ar_pdp,
				   sizeof(ar_pdp), 1, arfd) != 1) {
				if (feof(arfd))
					return (1L);
				break;
			}
			if (STREQN(ar_pdp.ar_name, name, len))
				return (ar_pdp.ar_date);
			ptr = ar_pdp.ar_size;
			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1))
				goto seek_error;
		}
		break;
#ifndef pdp11
	case AR5:
		for (; ; ) {
			if (fread((char *) &arf_5,
				  sizeof(arf_5), 1, arfd) != 1) {
				if (feof(arfd))
					return (1L);
				break;
			}
			if (STREQN(arf_5.arf_name, name, len))
				return (sgetl(arf_5.arf_date));
			ptr = sgetl(arf_5.arf_size);
			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1))
				goto seek_error;
		}
		break;
	case ARport:
		for (; ; ) {
			if ((fread((char *) &ar_port,
				    sizeof(ar_port), 1, arfd) != 1) ||
			    !(STREQN(ar_port.ar_fmag, ARFportMAG,
				      sizeof(ar_port.ar_fmag)))) {
				if (feof(arfd))
					return (1L);
				break;
			}
			if (STREQN(ar_port.ar_name, name, len) &&
			    (len == sizeof ar_port.ar_name ||
			     ar_port.ar_name[len] == '/' ||
			     ar_port.ar_name[len] == ' ' ||
			     ar_port.ar_name[len] == CNULL )) {
				long	date;

				if (sscanf(ar_port.ar_date, "%ld", &date) != 1)
					fatal1("Bad date field for %.14s in %s",
					    name, an);

				return (date);
			}
			if (sscanf(ar_port.ar_size, "%ld", &ptr) != 1)
				fatal1("Bad size field for %.14s in archive %s", name, an);

			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1))
				goto seek_error;
		}
		break;
#endif
	}

	/* Only here if fread() [or STREQN()] failed and not at EOF */

	fatal1("read error on archive %s", an);
	/*NOTREACHED*/
#if 0
	long	date, nsyms = sgetl(head.ar_syms), ptr;

	if (fseek(arfd, (long)(nsyms * sizeof(symbol) + sizeof(head) ), 0) == -1)
		fatal("seek error on archive %s", an);
	for (; ; ) {
		if (fread(&fhead, sizeof(fhead), 1, arfd) != 1)
		    	if (feof(arfd))
				break;
			else
				fatal("read error on archive %s", an);
		if (STREQN(fhead.arf_name, name, 14))
			return( sgetl(fhead.arf_date) );

		ptr = sgetl(fhead.arf_size);
		ptr = (ptr + 1) & (~1);
		(void)fseek(arfd, ptr, 1);
	}
	return(1L);
#endif
}


static time_t
entryscan(an, name)	/* return date of member containing global var named */
char	*an, *name;
{
	long 	sgetl();

	/*
	* Hunt through each different archive format for the named
	* symbol.  Note that the old archive format does not support
	* this convention since there is no symbol directory to scan
	* through for all defined global variables. 
	*/
	if (ar_type == ARpdp)
		return (pdpentrys(an, name));
	if (sym_begin == 0L || num_symbols == 0L)
no_such_sym:	fatal1("cannot find symbol %s in archive %s", name, an);

	if (fseek(arfd, sym_begin, 0))
seek_error:	fatal1("seek error on archive %s", an);

#ifndef pdp11
	if (ar_type == AR5) {
		register int	i;
		unsigned int	len = strlen(name);

		if (len > 8)
			len = 8;
		for (i = 0; i < num_symbols; i++) {
			if (fread((char *) &ars_5, sizeof(ars_5), 1, arfd) != 1)
read_error:			fatal1("read error on archive %s", an);
			if (STREQN(ars_5.sym_name, name, len)) {
				if (fseek(arfd, sgetl(ars_5.sym_ptr), 0))
					goto seek_error;
				if (fread((char *)&arf_5,
					  sizeof(arf_5), 1, arfd) != 1)
					goto read_error;

				/* replace symbol name w/ member name */
				(void)strncpy(archmem, arf_5.arf_name,
					sizeof(arf_5.arf_name));

				return (sgetl(arf_5.arf_date));
			}
		}
	} else {	/* ar_type == ARport */
		register CHARSTAR offs,		/* offsets table */
			 	  syms,		/* string table */
				strend;		/* end of string table */
		void	free();
		int	strtablen;
		CHARSTAR strbeg;

		/*
		* Format of the symbol directory for this format is
		* as follows:	[sputl()d number_of_symbols]
		*		[sputl()d first_symbol_offset]
		*			...
		*		[sputl()d number_of_symbols'_offset]
		*		[null_terminated_string_table_of_symbols]
		*/
		if ( !(offs = (char *) malloc( (unsigned) (num_symbols * sizeof(long)))) )
			fatal1("cannot alloc offsets table for archive %s", an);

		if (fread(offs, sizeof(long), (int) num_symbols, arfd) != num_symbols)
			goto read_error;

		strtablen = sym_size - ((num_symbols + 1L) * sizeof(long));
		if ( !(syms = (char *) malloc((unsigned) strtablen)) )
			fatal1("cannot alloc string table for archive %s",
			    an);

		if (fread(syms, sizeof(char), strtablen, arfd) != strtablen)
			goto read_error;
		strbeg = syms;
		strend = &syms[strtablen];
		/* while less than end of string table */
		while (syms < strend) {
			if (STREQ(syms, name)) {
				long	date;
				register char *ap, *hp;

				if (fseek(arfd, sgetl(offs), 0))
					goto seek_error;
				if ((fread((char *) &ar_port,
					sizeof(ar_port), 1, arfd) != 1) ||
				    !(STREQN(ar_port.ar_fmag,
				    		ARFportMAG,
						sizeof(ar_port.ar_fmag))))
					goto read_error;

				if (sscanf(ar_port.ar_date, "%ld", &date) != 1)
					fatal1("Bad date for %.14s, archive %s", ar_port.ar_name, an);

				/* replace symbol name w/ member name */
				ap = archmem;
				hp = ar_port.ar_name;
				while (*hp && *hp != '/' &&
				       (ap < archmem + sizeof(archmem)))
					*ap++ = *hp++;

				free(strbeg);
				return (date);
			}
			syms += strlen(syms) + 1;
			offs += sizeof(long);
		}
		free(strbeg);
	}
#endif
	goto no_such_sym;
#if 0
	register int	i;
	long	date, nsyms = sgetl(head.ar_syms);

	for (i = 0; i < nsyms; i++) {
		if (fread(&symbol, sizeof(symbol), 1, arfd) != 1)
badread:		fatal("read error on archive %s", an);

		if (STREQN(symbol.sym_name, name, 8)) {
			if (fseek(arfd, sgetl(symbol.sym_ptr), 0) == -1)
				fatal("seek error on archive %s", an);
			if (fread(fhead, sizeof(fhead), 1, arfd) != 1)
				goto badread;
			return( sgetl(fhead.arf_date) );
		}
	}
	fatal("cannot find symbol %s in archive %s", name , an);
#endif
}


static time_t
pdpentrys(an, name)
char	*an, *name;
{
	register int	i;
	long 	ftell(), skip, last, len = strlen(name);

#ifndef pdp11
	fatal("Cannot do global variable search in pdp11 or old object file.");
#endif
	if (len > 8L)
		len = 8L;
	/*
	* Look through archive, an object file entry at a time.  For each
	* object file, jump to its symbol table and check each external
	* symbol for a match.  If found, return the date of the module
	* containing the symbol.
	*/
	if (fseek(arfd, sizeof(short), 0))
seek_error:	fatal1("seek error on archive %s", an);

	while (fread((char *) & ar_pdp, sizeof(ar_pdp), 1, arfd) == 1) {
		last = ftell(arfd);
		if (ar_pdp.ar_size < sizeof(arobj_pdp) || 
		    fread((char *) & arobj_pdp, sizeof(arobj_pdp),
			  1, arfd) != 1 || 
		    (arobj_pdp.a_magic != 0401 && 	/* A_MAGIC0 */
		     arobj_pdp.a_magic != 0407 && 	/* A_MAGIC1 */
		     arobj_pdp.a_magic != 0410 && 	/* A_MAGIC2 */
		     arobj_pdp.a_magic != 0411 && 	/* A_MAGIC3 */
		     arobj_pdp.a_magic != 0405 && 	/* A_MAGIC4 */
		     arobj_pdp.a_magic != 0437)) 	/* A_MAGIC5 */
			fatal1("%s is not an object module (bu42)", ar_pdp.ar_name);

		skip = arobj_pdp.a_text + arobj_pdp.a_data;
		if (!arobj_pdp.a_flag)
			skip *= 2L;
		if (skip >= ar_pdp.ar_size || fseek(arfd, skip, 1))
			goto seek_error;
		skip = ar_pdp.ar_size;
		skip += (skip & 01) + last;
		i = (arobj_pdp.a_syms / sizeof(ars_pdp)) + 1;
		while ( --i ) {	/* look through symbol table */
			if (fread((char *) &ars_pdp, sizeof(ars_pdp), 1, arfd) != 1)
				fatal1("read error on archive %s", an);

			if ((ars_pdp.n_type & 040)	/* N_EXT for pdp11 */
			     && STREQN(ars_pdp.n_name, name, (int) len)) {
				(void)strncpy(archmem, ar_pdp.ar_name, (size_t)14);
				archmem[14] = CNULL;
				return (ar_pdp.ar_date);
			}
		}
		if (fseek(arfd, skip, 0))
			goto seek_error;
	}
	return (0L);
}


static int
openarch(f)
register CHARSTAR f;
{
	long	sgetl(), ftell();
	unsigned short	mag_pdp;	/* old archive format */
	char	mag_5[SAR5MAG],		/* 5.0 archive format */
		mag_port[SARportMAG];	/* port (6.0) archive format */

	
	if ( !(arfd = fopen(f, "r")) )
		return(-1);
	/*
	* More code for three archive formats.  Read in just enough to
	* distinguish the three types and set ar_type.  Then if it is
	* one of the newer archive formats, gather more info.
	*/
	if (fread((char *) & mag_pdp, sizeof(mag_pdp), 1, arfd) != 1)
		return (-1);
	if (mag_pdp == (unsigned short)ARpdpMAG) {
		ar_type = ARpdp;
		first_ar_mem = ftell(arfd);
		sym_begin = num_symbols = sym_size = 0L;
		return (0);
	}
	if (fseek(arfd, 0L, 0) || fread(mag_5, SAR5MAG, 1, arfd) != 1)
		return (-1);
	if (STREQN(mag_5, AR5MAG, SAR5MAG)) {
		ar_type = AR5;

		/* Must read in header to set necessary info */

		if (fseek(arfd, 0L, 0) || 
		    fread((char *) & arh_5, sizeof(arh_5), 1, arfd) != 1)
			return (-1);

#ifdef pdp11
		fatal1("cannot handle 5.0 archive format for %s", f);
		/*NOTREACHED*/
#else
		sym_begin = ftell(arfd);
		num_symbols = sgetl(arh_5.ar_syms);
		first_ar_mem = sym_begin + sizeof(ars_5) * num_symbols;
		sym_size = 0L;
		return (0);
#endif
	}
	if (fseek(arfd, 0L, 0) ||
	    fread(mag_port, SARportMAG, 1, arfd) != 1)
		return (-1);

	if (STREQN(mag_port, ARportMAG, SARportMAG)) {
		ar_type = ARport;
		/*
		* Must read in first member header to find out
		* if there is a symbol directory
		*/
		if (fread((char *) & ar_port,
			  sizeof(ar_port), 1, arfd) != 1 || 
		    !STREQN(ARFportMAG, ar_port.ar_fmag,
		   	    sizeof(ar_port.ar_fmag)))
			return (-2);

#ifdef pdp11
		fatal1("cannot handle portable archive format for %s", f);
		/*NOTREACHED*/
#else
		if (ar_port.ar_name[0] == '/') {
			char	s[4];

			if (sscanf(ar_port.ar_size, "%ld", &sym_size) != 1)
				return (-2);
			sym_size += (sym_size & 01);	/* round up */
			if (fread(s, sizeof(s), 1, arfd) != 1)
				return (-2);
			num_symbols = sgetl(s);
			sym_begin = ftell(arfd);
			first_ar_mem = sym_begin + sym_size - sizeof(s);
		} else {
			/* there is no symbol directory */
			sym_size = num_symbols = sym_begin = 0L;
			first_ar_mem = ftell(arfd) - sizeof(ar_port);
		}
		return (0);
#endif
	}
	fatal1("%s is not an archive", f);
	/*NOTREACHED*/
#if 0
	if (fread(&head, sizeof(head), 1, arfd) != 1)
		return(-1);
	if (!STREQN(head.ar_magic, ARMAG, 4))
		fatal1("%s is not an archive (bu28)", f);
	return(0);
#endif
}


static DIR *
getfid(dirname)
char *dirname;
{
	char	temp[MAXPATHLEN];
	register DIR * dirf;
	register OPENDIR fod, od = firstod;
	OPENDIR cod, odpred = NULL;
	void cat();

	cat(temp, dirname, 0);
	CHKARY(getfid, temp, MAXPATHLEN)  /*macro defined in defs*/
	(void) compath(temp);

	while ( od ) {
		if (STREQ(temp, od->dirn)) {
			if ( !(dirf = od->dirfc) ) {
				if (nopen >= MAXOPFIL) {
					for (fod = firstod; fod; fod = fod->nextopendir)
						if ( fod->dirfc )
							cod = fod;
					(void)closedir (cod->dirfc);
					cod->dirfc = NULL;
					nopen--;
				}
				if ( !(dirf = opendir(od->dirn)) )
					return((DIR * ) -1);
				else {
					od->dirfc = dirf;
					nopen++;
				}
			} else	/* start over at the beginning  */
				(void)rewinddir(dirf);

			if (odpred) {
				odpred->nextopendir = od->nextopendir;
				od->nextopendir = firstod;
				firstod = od;
			}
			return(dirf);
		}

		odpred = od;
		od = od->nextopendir;
	}

	od = ALLOC(s_opendir);
	od->nextopendir = firstod;
	firstod = od;
	od->dirn = copys(temp);
	if (nopen >= MAXOPFIL) {
		for (fod = firstod; fod; fod = fod->nextopendir)
			if ( fod->dirfc )
				cod = fod;
		(void)closedir(cod->dirfc);
		cod->dirfc = NULL;
		nopen--;
	}
	
	if ( dirf = opendir(temp) )
		nopen++;

	od->dirfc = dirf;

	return(dirf);
}
