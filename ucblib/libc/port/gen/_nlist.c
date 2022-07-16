/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/_nlist.c	1.1.3.1"


/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*LINTLIBRARY*/
#if vax
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)VAXROMAGIC) || \
			  (((unsigned short)x)==(unsigned short)VAXWRMAGIC))
#endif
#if u3b
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)N3BMAGIC) || \
			  (((unsigned short)x)==(unsigned short)NTVMAGIC))
#endif
#if m32 || i386
#define ISMAGIC(x)	((x)==FBOMAGIC)
#endif
#if u3b || vax || m32 || i386
#define BADMAG(x)	(!ISMAGIC(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default : readonly sharable text segment
 *		AOUT2MAGIC :	     : writable text segment
 *		PAGEMAGIC  :	     : directly paged object file
 */

#define AOUT1MAGIC 0410
#define AOUT2MAGIC 0407
#define PAGEMAGIC  0413

#define	SGSNAME	""
#define SGS ""
#define RELEASE "Release 6.0 6/1/82"
#include <a.out.h>

#if vax || u3b || m32 || i386
#	ifndef FLEXNAMES
#		define FLEXNAMES 1
#	endif
#	undef n_name		/* this patch causes problems here */
#endif

#define SPACE 100		/* number of symbols read at a time */

#if vax || u3b || m32 || i386
static char sym_buf[SPACE * SYMESZ];
static int num_in_buf = 0;
static char *next_entry = (char *)0;
static int refill = 0;
#endif

extern long _lseek();
extern int _open(), _read(), _close(), strncmp(), strcmp();

int
_nlist(fd, list)
	int fd;
	struct nlist *list;
{
#if u3b || vax || m32 || i386
	int sym_read(int, struct syment *, int);
	extern char *malloc();
	extern void free();
	struct	filehdr	buf;
	struct	syment	sym;
	long n;
	int nreq;
	int bufsiz=FILHSZ;
#if FLEXNAMES
	char *strtab = (char *)0;
	long strtablen;
#endif
	register struct nlist *p;
	register struct syment *q;
#else
	struct nlist space[SPACE];
	struct exec buf;
	int	nlen=sizeof(space[0].n_name), bufsiz=(sizeof(buf));
	unsigned n, m; 
	register struct nlist *p, *q;
#endif
	long	sa;
	void	sym_close();

	for (p = list, nreq = 0; p->n_name && p->n_name[0]; 
		p++, nreq++)	/* n_name can be ptr */
	{
		p->n_type = 0;
#if u3b || vax || m32 || i386
		p->n_value = 0L;
		p->n_scnum = 0;
		p->n_sclass = 0;
		p->n_numaux = 0;
#else
		p->n_value = 0;
#endif
	}
	
	if(_read(fd, (char *)&buf, bufsiz) == -1) {
		(void) _close(fd);
		return(-1);
	}

#if u3b || vax || m32 || i386
	if (BADMAG(buf.f_magic))
#else
	if (BADMAG(buf))
#endif
	{
		(void) _close(fd);
		return (-1);
	}
#if u3b || vax || m32 || i386
	sa = buf.f_symptr;	/* direct pointer to sym tab */
	_lseek(fd, sa, 0);
	q = &sym;
	n = buf.f_nsyms;	/* num. of sym tab entries */
#else
	sa = buf.a_text;
	sa += buf.a_data;
#if u370
	sa += (long)(buf.a_trsize + buf.a_drsize);
#endif
#if pdp11
	if (buf.a_flag != 1)
		sa += sa;
	else if ( buf.a_magic == A_MAGIC5 )
		sa += (long)buf.a_hitext << 16; /* remainder of text size for  */
						/*	system overlay a.out   */
#endif
	sa += (long)sizeof(buf);
	(void) _lseek(fd, sa, 0);
	n = buf.a_syms;
#endif

	while (n)
	{
#if u3b || vax || m32 || i386
		if(sym_read(fd, &sym, SYMESZ) == -1) {
			sym_close(fd);
			return(-1);
		}
		n -= (q->n_numaux + 1L);
#else
		m = (n < sizeof(space))? n: sizeof(space);
		if(_read(fd, (char*)space, m) == -1) return(-1);
		n -= m;
		for (q=space; ((int)(m -= sizeof(space[0])) >= 0); ++q)
		{
#endif
			for (p = list; p->n_name && p->n_name[0]; ++p)
			{
#if u3b || vax || m32 || i386
				if (p->n_value != 0L && p->n_sclass == C_EXT)
					continue;
				/*
				* For 6.0, the name in an object file is
				* either stored in the eight long character
				* array, or in a string table at the end
				* of the object file.  If the name is in the
				* string table, the eight characters are
				* thought of as a pair of longs, (n_zeroes
				* and n_offset) the first of which is zero
				* and the second is the offset of the name
				* in the string table.
				*/
#if FLEXNAMES
				if (q->n_zeroes == 0L)	/* in string table */
				{
					if (strtab == (char *)0) /* need it */
					{
						long home = _lseek(fd, 0L, 1);
						if (_lseek(fd, buf.f_symptr +
							buf.f_nsyms * SYMESZ,
							0) == -1 || _read(fd,
							(char *)&strtablen,
							sizeof(long)) !=
							sizeof(long) ||
							(strtab = malloc(
							(unsigned)strtablen))
							== (char *)0 ||
							_read(fd, strtab +
							sizeof(long),
							strtablen -
							sizeof(long)) !=
							strtablen -
							sizeof(long) ||
							strtab[strtablen - 1]
							!= '\0' ||
							_lseek(fd, home, 0) ==
							-1)
						{
							(void) _lseek(fd,home,0);
							sym_close(fd);
							if (strtab != (char *)0)
								free(strtab);
							return (-1);
						}
					}
					if (q->n_offset < sizeof(long) ||
						q->n_offset >= strtablen)
					{
						sym_close(fd);
						if (strtab != (char *)0)
							free(strtab);
						return (-1);
					}
					if (strcmp(&strtab[q->n_offset],
						p->n_name))
					{
						continue;
					}
				}
				else
#endif /*FLEXNAMES*/
				{
					if (strncmp(q->_n._n_name,
						p->n_name, SYMNMLEN))
					{
						continue;
					}
				}
#else
				if (strncmp(p->n_name, q->n_name, nlen))
					continue;
#endif
				p->n_value = q->n_value;
				p->n_type = q->n_type;
#if u3b || vax || m32 || i386
				p->n_scnum = q->n_scnum;
				p->n_sclass = q->n_sclass;
#endif
				if (--nreq == 0)
					goto alldone;
				break;
			}
#if !(u3b || vax || m32 || i386)
		}
#endif
	}
#if (vax || u3b || m32 || i386) && FLEXNAMES
	sym_close(fd);
	if (strtab != (char *)0)
		free(strtab);
#else
	(void) _close(fd);
#endif

alldone:
	return (nreq);
}

void sym_close(fd)
int fd;
{
	num_in_buf = 0;
	next_entry = (char *)0; 
	refill = 0;

	(void) _close(fd);
}


#if u3b || vax || m32 || i386

int sym_read(fd, sym, size)	/* buffered read of symbol table */
int fd;
struct syment *sym;
int size; 
{
	extern char *memcpy();
	int fill_sym_buf(int, int);
	long where = 0L;

	where = _lseek(fd, 0L, 1);

	if(!num_in_buf) {
		if(fill_sym_buf(fd, size) == -1) return(-1);
	}
	(void) memcpy((char *)sym, next_entry, size);
	num_in_buf--;

	if(sym->n_numaux && !num_in_buf) {
		if(fill_sym_buf(fd, size) == -1) return(-1);
	}
	_lseek(fd, where + SYMESZ + (AUXESZ * sym->n_numaux), 0); 
	size += AUXESZ * sym->n_numaux;
	num_in_buf--;

	next_entry += size;
	return(0);
}

int fill_sym_buf(int fd, int size)
{
	if((num_in_buf = _read(fd, sym_buf, size * SPACE)) == -1) 
		return(-1);
	num_in_buf /= size;
	next_entry = &(sym_buf[0]);
	refill++;
	return(0);
}

#endif
