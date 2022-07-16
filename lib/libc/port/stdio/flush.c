/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/flush.c	1.22"
/*LINTLIBRARY*/		/* This file always part of stdio usage */

#include "synonyms.h"
#include "shlib.h"
#include <stdlib.h>
#include <stdio.h>
#include "stdiom.h"

#undef _cleanup
#undef end

#define FILE_ARY_SZ	8 /* a nice size for FILE array & end_buffer_ptrs */

/* initial array of end-of-buffer ptrs  */
static Uchar *_cp_bufendtab[_NFILE + 1] = {0}; /* for alternate system - original */

typedef struct _link_ Link;	/* list of iob's */

struct _link_	/* manages a list of streams */
{
	FILE	*iobp;	/* the array of FILE's */
	Uchar 	**endbuf; /* the array of end buffer pointers */
	int	niob;	/* length of the arrays */
	Link	*next;	/* next in the list */
};

/* With dynamic linking, iob may be in either the library or in the user's
 * a.out, so the run time linker fixes up the first entry in __first_link at
 * process startup time.
 */
Link __first_link =	/* first in linked list */
{
#if DSHLIB
	0,
#else
	&_iob[0],
#endif
	&_cp_bufendtab[0],
	_NFILE,
	0
};

/*
* All functions that understand the linked list of iob's follow.
*/

void
_cleanup()	/* called at process end to flush ouput streams */
{
	fflush(NULL);
}

void
_flushlbf()	/* fflush() all line-buffered streams */
{
	register FILE *fp;
	register int i;
	register Link *lp;

	lp = &__first_link;

	do {
		fp = lp->iobp;
		for (i = lp->niob; --i >= 0; fp++)
		{
			if ((fp->_flag & (_IOLBF | _IOWRT)) == (_IOLBF | _IOWRT))
				(void)fflush(fp);
		}
	} while ((lp = lp->next) != 0);
}

FILE *
_findiop()	/* allocate an unused stream; 0 if cannot */
{
	register Link *lp, **prev;
	typedef struct		/* used so there only needs to be one malloc() */
	{
		Link	hdr;
		FILE	iob[FILE_ARY_SZ];
		Uchar	*nbuf[FILE_ARY_SZ]; /* array of end buffer pointers */
	} Pkg;
	register Pkg *pkgp;
	register FILE *fp;

	lp = &__first_link;

	do {
		register int i;

		prev = &lp->next;
		fp = lp->iobp;
		for (i = lp->niob; --i >= 0; fp++)
		{
			if (fp->_flag == 0)	/* unused */
			{
				fp->_cnt = 0;
				fp->_ptr = 0;
				fp->_base = 0;
				return fp;
			}
		}
	} while ((lp = lp->next) != 0);
	/*
	* Need to allocate another and put it in the linked list.
	*/
	if ((pkgp = (Pkg *)malloc(sizeof(Pkg))) == 0)
		return 0;
	(void)memset(pkgp, 0, sizeof(Pkg));
	pkgp->hdr.iobp = &pkgp->iob[0];
	pkgp->hdr.niob = sizeof(pkgp->iob) / sizeof(FILE);
	pkgp->hdr.endbuf = &pkgp->nbuf[0];
	*prev = &pkgp->hdr;
	return &pkgp->iob[0];
}

void
_setbufend(iop, end)	/* make sure _bufendtab[] big enough, set end ptr */
	register FILE *iop;
	Uchar *end;
{
	register Link *lp;

	lp = &__first_link;

	if (iop->_file < _NFILE)
                _bufendtab[iop->_file] = end;
        else
        {
		do {
			if ((lp->iobp <= iop)  && (iop < ( lp->iobp + lp->niob)))
			{
				lp->endbuf[iop - lp->iobp] = end;
			}
		} while ((lp = lp->next) != 0);
	}
}

Uchar *
_realbufend(iop) 	/* get the end pointer for this iop */
FILE * iop;
{
	register Link *lp;

	lp = &__first_link;

	do {
		if ((lp->iobp <= iop)  && (iop < ( lp->iobp + lp->niob)))
		{
			return lp->endbuf[iop - lp->iobp];
		}
	} while ((lp = lp->next) != 0);
	return 0;
}


void
_bufsync(iop, bufend)	/* make sure _cnt, _ptr are correct */
	register FILE *iop;
	register Uchar *bufend;
{
	register int spaceleft;

	if ((spaceleft = bufend - iop->_ptr) < 0)
	{
		iop->_ptr = bufend;
		iop->_cnt = 0;
	}
	else if (spaceleft < iop->_cnt)
		iop->_cnt = spaceleft;
}

extern int write(	/* int fd, char *buf, unsigned len */	);

int
_xflsbuf(iop)	/* really write out current buffer contents */
	register FILE *iop;
{
	register int n;
	register Uchar *base = iop->_base;
	register Uchar *bufend;
	int num_wrote;

	/*
	* Hopefully, be stable with respect to interrupts...
	*/
	n = iop->_ptr - base;
	iop->_ptr = base;
	bufend = _bufend(iop);
	if (iop->_flag & (_IOLBF | _IONBF))
		iop->_cnt = 0; /* always go to a flush */
	else
		iop->_cnt = bufend - base;
	if (_needsync(iop, bufend))   /* recover from interrupts */
		_bufsync(iop, bufend);
	while((num_wrote = write(iop->_file, (char *)base,(unsigned)n)) != n) {
		if(num_wrote <= 0) {
			iop->_flag |= _IOERR;
			return EOF;
		}
		n -= num_wrote;
		base += num_wrote;
	}
	return 0;
}

int
fflush(iop)	/* flush (write) buffer */
	register FILE *iop;
{
	int res = 0;

	if (iop == NULL) {
		register int i;
		register Link *lp;

		lp = &__first_link;

		do {
			iop = lp->iobp;
			for (i = lp->niob; --i >= 0; iop++) {
				if (iop->_flag & _IOWRT)
					 res |= fflush(iop);
			}
		} while ((lp = lp->next) != 0);
		return res;
	}

	if (!(iop->_flag & _IOWRT))
	{
		lseek(iop->_file, -iop->_cnt, SEEK_CUR);
		iop->_cnt = 0;
		iop->_ptr = iop->_base;	/* needed for ungetc & mulitbyte pushbacks */
		if (iop->_flag & _IORW) {
			iop->_flag &= (unsigned short)~_IOREAD;
		}
		return 0;
	}
	if (iop->_base != 0 && iop->_ptr > iop->_base)
		res = _xflsbuf(iop);
	if (iop->_flag & _IORW) {
		iop->_flag &= (unsigned short)~_IOWRT;
		iop->_cnt = 0;
	}
	return res;
}

extern int close(	/* int fd */	);

int
fclose(iop)	/* flush buffer and close stream */
	register FILE *iop;
{
	register int res = 0;

	if (iop == 0 || iop->_flag == 0)
		return EOF;
	/* Is it writeable and not unbuffered? */
	if ((iop->_flag & (_IOWRT | _IONBF)) == _IOWRT)
		res = fflush(iop);
	if (close(iop->_file) < 0)
		res = EOF;
	if (iop->_flag & _IOMYBUF)
	{
		free((char *)iop->_base - PUSHBACK);
		/* free((VOID *)iop->_base); */
	}
	iop->_base = 0;
	iop->_ptr = 0;
	iop->_cnt = 0;
	iop->_flag = 0;			/* marks it as available */
	return res;
}
