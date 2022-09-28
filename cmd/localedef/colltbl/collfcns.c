/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)localedef:colltbl/collfcns.c	1.1.4.1"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include "colltbl.h"

#ifdef REGEXP
#define INIT		register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return(c)
#define ERROR(c)	{ regerr(c); return((char *) NULL); }
#include <regexp.h>
#endif

extern int	regexp_flag;

#define SZ_COLLATE	256
#define SUBFLG		0x80
#define ORD_LST		1
#define PAR_LST		2
#define BRK_LST		3

/* entry in the collation table */
typedef struct collnd {
#ifdef i386
	unsigned char		ch;	/* character or number of followers */
	unsigned char		pwt;	/* primary weight */
	unsigned char		swt;	/* secondary weight */
#else
	char		ch;	/* character or number of followers */
	char		pwt;	/* primary weight */
	char		swt;	/* secondary weight */
#endif
	struct collnd	*next;	/* beginning of follower state list or
				 * next follower state */
} collnd;

/* entry in the substitution table */
typedef struct subnd {
	char		*exp;	/* expression to be replaced */
	int		explen; /* length of expression */
	char		*repl;	/* replacement string */
	int		repllen;/* length of replacement string */
	struct subnd	*next;
} subnd;

int	curprim = 0;
int	cursec = 0;
int	maxsec = 0;

static collnd	colltbl[SZ_COLLATE];	/* collation table */
static long	coll_cnt = SZ_COLLATE;	/* number of entries in colltbl */
static subnd	*sublst;	/* substitution table (list) */
static subnd	*endsublst;	/* pointer to last entry in substitution table */
static long	sub_cnt = 0;	/* number of entries in substitution table */
static long	sub_sz = 0;	/* total size of substitution strings */

#ifdef __STDC__
collnd	*newcollnd(int, int, int, collnd *);
void	putzeros(FILE *, int);
#else
collnd	*newcollnd();
void	putzeros();
#endif

void
mkord(sym, type)
#ifdef i386
unsigned char *sym;
#else
char *sym;
#endif
int type;
{
	int	prim, sec;
	collnd	*cptr;
#ifdef i386
	unsigned int	i;
#else
	int	i;
#endif


	switch (type) {
		case ORD_LST:
			prim = ++curprim;
			sec = 0;
			break;
		case PAR_LST:
			prim = curprim;
			sec = cursec++;
			if (maxsec < cursec)
				maxsec = cursec;
			break;
		case BRK_LST:
			prim = curprim;
			sec = 0;
			break;
	}
	if (sym[1] == '\0') {
		if (colltbl[*sym].pwt != 0)
			error(DUPLICATE, "symbol", sym);
		colltbl[*sym].pwt = prim;
		colltbl[*sym].swt |= sec;
	} else {
		cptr = colltbl[*sym].next;
		for (i=0; i<colltbl[*sym].ch; i++)
			if (cptr->ch == sym[1])
				error(DUPLICATE, "symbol", sym);
		colltbl[*sym].next = newcollnd(sym[1],prim,sec,colltbl[*sym].next);
		colltbl[*sym].ch++;
		coll_cnt++;
	}
}

collnd *
newcollnd(ch, pwt, swt, next)
int ch;
int pwt;
int swt;
collnd *next;
{
	collnd	*cptr;

	if ((cptr = (collnd *)malloc(sizeof(collnd))) == NULL) {
		fprintf(stderr, "Out of space\n");
		exit(-1);
	}
	cptr->ch = ch;
	cptr->pwt = pwt;
	cptr->swt = swt;
	cptr->next = next;
	return(cptr);
}

void
substitute(exp, repl)
char *exp;
char *repl;
{
	subnd	*psubnd;

	if ((psubnd = (subnd *)malloc(sizeof(subnd))) == NULL) {
		fprintf(stderr, "Out of space\n");
		exit(-1);
	}
#ifdef REGEXP
	if (regexp_flag) 
	{
		static char	ebuf[BUFSIZ];

		compile(exp, &ebuf[0], &ebuf[BUFSIZ], '\0');
		free(exp);
		psubnd->explen = (char *)memchr(ebuf,'\026',BUFSIZ) - ebuf + 1;
		if ((psubnd->exp = malloc(psubnd->explen)) == NULL) {
			fprintf(stderr, "Out of space\n");
			exit(-1);
		}
		(void)memcpy(psubnd->exp, ebuf, psubnd->explen);
	}
	else
#endif
	{
		psubnd->explen = strlen(exp) + 1;
		psubnd->exp = exp;
#ifdef i386
		colltbl[(unsigned char)*exp].swt |= SUBFLG;
#else
		colltbl[*exp].swt |= SUBFLG;
#endif
	}
	psubnd->repllen = strlen(repl) + 1;
	psubnd->repl = repl;

	if (sublst == NULL) {
		sublst = endsublst = psubnd;
	} else {
		endsublst->next = psubnd;
		endsublst = psubnd;
	}

	sub_cnt++;
	sub_sz += psubnd->explen + psubnd->repllen;
}


/* Create an LC_COLLATE database.  The database will consist
 * of a header, followed by the collation table, followed by the
 * substitution table, followed by the substitution strings.  
 * /
/* database header */
typedef struct {
	long	coll_offst;	/* offset of collation table from beg. of file */
	long	sub_cnt;	/* number of substitution table entries */
	long	sub_offst;	/* offset of substitution table from beg. of file */
	long	str_offst;	/* offset of substitution strings from beg. of file*/
	long	flags;		/* allows future flexibility in database */
} hd;

/* collation table entry in database */
typedef struct {
#ifdef i386
	unsigned char		ch;	/* character or number of followers */
	unsigned char		pwt;	/* primary weight */
	unsigned char		swt;	/* secondary weight */
	unsigned char		ns;	/* index of follower state list */
#else
	char		ch;	/* character or number of followers */
	char		pwt;	/* primary weight */
	char		swt;	/* secondary weight */
	char		ns;	/* index of follower state list */
#endif
} xnd;

/* subsitution table entry in database */
typedef struct {
	char	*exp;
	long	explen;
	char	*repl;
} subtent;

/* structure representing the padding between sections of the database */
struct db {
	hd	collhdr;
	xnd	cdata;
	subtent	subtbl;
	char	*substrs;
};

void
setdb(outname)
char *outname;
{
	FILE	*outfile;
#ifdef i386
	unsigned char	last = 0;
#else
	char	last = 0;
#endif
	hd	collhdr;
	xnd 	*cdata;
	subtent	*subtbl;
	char	*substrs;
	collnd 	*cptr;
	char	*sptr;
	int	i, j;
	long	offset;

	/* initialize header */
	collhdr.coll_offst = offsetof(struct db, cdata);
	collhdr.sub_cnt = sub_cnt;
	collhdr.sub_offst = offsetof(struct db, subtbl) + (coll_cnt-1) * sizeof(xnd);
	collhdr.str_offst = offsetof(struct db, substrs)+ (coll_cnt-1) * sizeof(xnd)
		+ (sub_cnt-1) * sizeof(subtent);
	collhdr.flags = regexp_flag;

	/* initialize collation table */
	if ((cdata = (xnd *)malloc(coll_cnt * sizeof(xnd))) == NULL) {
		fprintf(stderr, "Out of space\n");
		exit(-1);
	}
	for (i = 0; i < SZ_COLLATE; i++) {
		cdata[i].ch = colltbl[i].ch;
		if (colltbl[i].pwt)
			colltbl[i].pwt += maxsec;
		cdata[i].pwt = colltbl[i].pwt;
		cdata[i].swt = colltbl[i].swt;
		if (colltbl[i].ch != 0) {
			cdata[i].ns = last;
			last += colltbl[i].ch;
		} else
			cdata[i].ns = 0;
	}
	j = SZ_COLLATE;
	for (i = 0; i < SZ_COLLATE; i++) {
		if ((cptr = colltbl[i].next) == NULL)
			continue;
		while (cptr) {
			cdata[j].ch = cptr->ch;
			if (cptr->pwt)
				cptr->pwt += maxsec;
			cdata[j].pwt = cptr->pwt;
			cdata[j].swt = cptr->swt;
			cdata[j].ns = 0;
			cptr = cptr->next;
			j++;
		}
	}

	/* initialize substitution table and substitution strings */
	if ((substrs = malloc(sub_sz)) == NULL) {
		fprintf(stderr, "Out of space\n");
		exit(-1);
	}
	if ((subtbl = (subtent *)malloc(sub_cnt * sizeof(subtent))) == NULL) {
		fprintf(stderr, "Out of space\n");
		exit(-1);
	}
	i = 0;
	offset = 0;
	sptr = substrs;
	while (sublst) {
		subtbl[i].exp = (char *)offset;
		subtbl[i].explen = sublst->explen - 1;
		offset += sublst->explen;
		subtbl[i].repl = (char *)offset;
		offset += sublst->repllen;
		i++;

		memcpy(sptr, sublst->exp, sublst->explen);
		sptr += sublst->explen;
		memcpy(sptr, sublst->repl, sublst->repllen);
		sptr += sublst->repllen;
		sublst = sublst->next;
	}

	/* write out database, making sure there is appropriate
	 * padding between database members */
	if ((outfile = fopen(outname, "w")) == NULL) {
		fprintf(stderr,"Cannot open output file, %s\n", outname);
		exit(-1);
	}
	if (fwrite(&collhdr, sizeof(hd), 1, outfile) != 1)
		goto err;
	putzeros(outfile, offsetof(struct db, cdata) - sizeof(hd));

	if (fwrite(cdata, sizeof(xnd), coll_cnt, outfile) != coll_cnt)
		goto err;
	putzeros(outfile, offsetof(struct db, subtbl) -
			offsetof(struct db, cdata) - sizeof(hd));
	if (fwrite(subtbl, sizeof(subtent), sub_cnt, outfile) != sub_cnt)
		goto err;
	putzeros(outfile, offsetof(struct db, substrs) -
			offsetof(struct db, subtbl) - sizeof(subtent));
	if (fwrite(substrs, sizeof(char), sub_sz, outfile) != sub_sz)
		goto err;
	fclose(outfile);
	return;
err:
	fprintf(stderr,"Cannot write to output file\n");
	fclose(outfile);
	exit(-1);
}

void
putzeros(outfile, num)
FILE *outfile;
int num;
{
	register int i;

	for (i=0; i<num; i++)
		putc('\0', outfile);
}
