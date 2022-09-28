/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cflow:lpfx.c	1.7.2.5"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <sgs.h>
#ifdef __STDC__
#include <stdlib.h>
#else
extern int exit();
extern int getopt();
extern char *malloc();
extern char *realloc();
#endif


/*
** This is ugly defining T1WORD here - if it ever changes this
** is sure to break.
*/
typedef long T1WORD;
#include "lnstuff.h"

#define FNSIZE LFNM
#define NSIZE LCHNM

typedef struct {
	union rec r;
	char *name;
	char *fname;
} funct;

typedef struct LI {
	struct LI *next;
	funct fun;
} li;


#ifdef __STDC__
static char *getstr(void);
static void rdargs(funct *);
static void insert(funct *);
static void putout(void);
static char *typr(long);
static char *prtype(li *);
static char *shift(char *);
#else
static char *getstr();
static void rdargs();
static void insert();
static void putout();
static char *typr();
static char *prtype();
static char *shift();
#endif

#define FMASK (LDI | LIB | LUV | LUE | LDS)

/*
 * lpfx - read lint1 output, sort and format for dag
 *
 *	options -i_ (inclusion)
 *
 *	while (read lint1 output into "funct" structures)
 *		if (this is a filename record)
 *			save filename
 *		else
 *			read arg records and throw on floor
 *			if (this is to be included)
 *				copy filename into "funct"
 *				insert into list
 *	format and print
 */


main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	funct fu;
	int uscore, stavar, c;
	char *filename;
	int start, pass;
	long curpos;
	FLENS lout;

	uscore = 0;
	stavar = 0;
	while ((c = getopt(argc, argv, "i:V")) != EOF)
		switch (c) {
		case 'i':
			switch (*optarg) {
			case 'x': 
				stavar = 1;
				break;
			case '_': 
				uscore = 1;
				break;	
			default:
				(void)fprintf(stderr, 
					"lpfx: bad option %c ignored\n", c);
				break;
			}
			break;
		case 'V':
			(void) fprintf(stderr, "cflow: %s %s\n", ESG_PKG, ESG_REL);
		    exit(0);
		}

	start = 1;
	pass = 1;

	for (;;) {
		if (start) {
			curpos = ftell(stdin);
			if (fread((char *)&lout, sizeof(FLENS), 1, stdin) != 1) {
				rewind(stdin);	
				break;
			}
			start = 0;
		}

		if ((int)fread((char *)&fu.r, sizeof(union rec), 1, stdin) < 1)
			break;

		if (fu.r.l.decflag & LND) {
			switch (pass) {
				case 1:
				case 2:
					pass++;
					continue;
				case 3:
					start = 1;
					pass = 1;
					curpos += lout.f1 + lout.f2 + lout.f3 + lout.f4;
					(void) fseek(stdin, curpos, 0);
					continue;
			}
		}

		if (fu.r.l.decflag & LFN)
			filename = fu.r.f.fn = getstr();
		else {
			int name_;
			fu.name = getstr();
			name_ = *fu.name == '_';
			rdargs(&fu);

			if (((fu.r.l.decflag & FMASK) && LN_ISFTN(fu.r.l.type.dcl_mod))
			    || (stavar && !LN_ISFTN(fu.r.l.type.dcl_mod)))
				if ((name_ && uscore) || !name_) {
					fu.fname = filename;
					insert(&fu);
				}
		}
	}
	putout();
	exit(0);
	/* NOTREACHED */
}

/* getstr - get strings from intermediate file
 *
 * simple while loop reading into static buffer
 * transfer into malloc'ed buffer
 * panic and die if format or malloc error
 *
 */
static char *
getstr()
{
	static char tab[BUFSIZ];
	static char *buf = tab;
	static int size = BUFSIZ;
	register int c;
	register char *p = buf;

	while ((c = getchar()) != EOF) {
		*p++ = (char) c;
		if (c == '\0' || !isascii(c))
			break;
		if (p >= buf + size) {
			if (buf == tab){
				buf = (char *)malloc((size += BUFSIZ));
				if (buf == NULL) {
					(void)fprintf(stderr, "out of heap space\n");
					exit (1);
				}
				(void)memcpy(buf, tab, (size - BUFSIZ));
			} else	{
				buf = (char *)realloc((char *)buf, (size += BUFSIZ));
				if (buf == NULL) {
					(void)fprintf(stderr, "out of heap space\n");
					exit (1);
				}
			}
			p = buf + (size - BUFSIZ);
		}
	}
	if (c != '\0') {
	    (void)fprintf(stderr,"lpfx: Intermediate file string format error\n");
		exit(1);
	}
	if (!(p = (char *)malloc(strlen(buf) + 1))) {
		(void) fprintf(stderr,"lpfx: out of heap space\n");
		exit(1);
	}
	return (strcpy(p, buf));
}

/*
 * rdargs - read arg records and throw on floor
 *
 *	if ("funct" has args)
 *		get absolute value of nargs
 *		read args 1 at a time and throw away
 */

static void
rdargs(pf)
register funct *pf;
{
	ATYPE atype;
	int n;

	if (pf->r.l.nargs) {
		if (pf->r.l.nargs < 0)
			pf->r.l.nargs = -pf->r.l.nargs - 1;

		if (pf->r.l.decflag & LSU) {
			for (n=0;n<pf->r.l.nargs;n++) {
				(void) fread((char *)&atype, sizeof(ATYPE), 1, stdin);
				(void) getstr();
			}
		} else for (n=0;n<pf->r.l.nargs;n++)
			if((int)fread((char *)&atype, sizeof(ATYPE), 1, stdin) <= 0){
				perror("lpfx.rdargs");
				exit(1);
			}
	}
}

/*
 * insert - insertion sort into (singly) linked list
 *
 *	stupid linear list insertion
 */

static li *head = NULL;

static void 
insert(pfin)
register funct *pfin;
{
	register li *list_item, *newf;

	/*
	** Allocate space, and set pfin in that space.
	*/
	if ((newf = (li *)malloc(sizeof(li))) == NULL) {
		(void)fprintf(stderr, "lpfx: out of heap space\n");
		exit(1);
	}
	newf->fun = *pfin;

	/*
	** Search list for place to insert pfin.
	*/
	list_item = head;
	if (list_item)
		/*
		** Lint of current is less than that in the linked list.
		** Make the current one at the head of the list.
		*/
		if (newf->fun.r.l.fline < list_item->fun.r.l.fline) {
			newf->next = head;
			head = newf;
		} 
		/*
		** Else search list until an item with a larger line number
		** is found, or we get to the end of the list.
		*/
		else {
			while (list_item->next &&
			  list_item->next->fun.r.l.fline < newf->fun.r.l.fline)
				list_item = list_item->next;
			/*
			** Line number is the same - keep going until end of list.
			*/
			while (list_item->next &&
			  list_item->next->fun.r.l.fline == newf->fun.r.l.fline &&
			  list_item->next->fun.r.l.decflag < newf->fun.r.l.decflag)
					list_item = list_item->next;

			/*
			** Append newf to the list.
			*/
			newf->next = list_item->next;
			list_item->next = newf;
		} 
	else {
		head = newf;
		newf->next = NULL;
	}
}

/*
 * putout - format and print sorted records
 *
 *	while (there are records left)
 *		copy name and null terminate
 *		if (this is a definition)
 *			if (this is a function**)
 *				save name for reference formatting
 *			print definition format
 *		else if (this is a reference)
 *			print reference format
 *
 *	** as opposed to external/static variable
 */

static void 
putout()
{
	register li *pli;
	char lnbuf[BUFSIZ], nbuf[BUFSIZ];
	char *lname = lnbuf, *name = nbuf;
	register int bufsiz = BUFSIZ, size;
	
	pli = head;
	name[0] = lname[0] = '\0';

	/*
	** Still records left?
	*/
	while (pli != NULL) {
		if ((size = strlen(pli->fun.name) + 1) > bufsiz) {
			if (lname == lnbuf) {
				lname = (char *)malloc(size);
				name = (char *)malloc(size);
				if (lname == NULL || name == NULL) {
					(void)fprintf(stderr,"lpfx: out of space\n");
					exit(1);
				}
				(void)memcpy(lname, lnbuf, bufsiz);
				(void)memcpy(name, nbuf, bufsiz);
			} else {
				lname = (char *)realloc((char *)lname, size);
				name = (char *)realloc((char *)name, size);
				if (lname == NULL || name == NULL) {
					(void)fprintf(stderr,"lpfx: out of space\n");
					exit(1);
				}
			}
			bufsiz = size;
		}

		(void) strcpy(name, pli->fun.name);
		if (pli->fun.r.l.decflag & (LDI | LDC | LDS)) {
			(void)strcpy(lname, name);
			(void)printf("%s = %s, <%s %d>\n", name, prtype(pli),
			    pli->fun.fname, pli->fun.r.l.fline);
		} else if (pli->fun.r.l.decflag & (LUV | LUE | LUM))
			(void)printf("%s : %s\n", lname, name);
		pli = pli->next;
	}
}

static char *
typr(ty)
long ty;
{
    switch (ty) {
	case LN_CHAR:	return "char";
	case LN_UCHAR:	return "unsigned char";
	case LN_SCHAR:	return "signed char";
	case LN_SHORT:	return "short";
	case LN_USHORT:	return "unsigned short";
	case LN_SSHORT:	return "signed short";
	case LN_INT:	return "int";
	case LN_UINT:	return "unsigned int";
	case LN_SINT:	return "signed int";
	case LN_LONG:	return "long";
	case LN_ULONG:	return "unsigned long";
	case LN_SLONG:	return "signed long";
	case LN_LLONG:	return "long long";				/* unused */
	case LN_ULLONG:	return "unsigned long long";	/* unused */
	case LN_SLLONG:	return "signed long long";		/* unused */
	case LN_ENUM:	return "enum";
	case LN_FLOAT:	return "float";
	case LN_DOUBLE:	return "double";
	case LN_LDOUBLE:return "long double";
	case LN_VOID:	return "void";
	case LN_STRUCT:	return "struct";
	case LN_UNION:	return "union";
	default:
		(void)fprintf(stderr,"bad type to typr: %ld\n", ty);
		return "int";
	}
}


/*
 * prtype - decode type fields
 *
 *	strictly arcana
 */

static char *
prtype(pli)
register li *pli;
{
	static char bigbuf[64];
	char buf[32];
	register char *bp;
	register long typ, mod;

	typ = pli->fun.r.l.type.aty;
	mod = pli->fun.r.l.type.dcl_mod;

	(void)strcpy(bigbuf, typr(typ&LNQUAL));
	*(bp = buf) = '\0';
	while (mod) {
		if (LN_ISPTR(mod)) {
			bp = shift(buf);
			buf[0] = '*';
		} else if (LN_ISFTN(mod)) {
			*bp++ = '(';
			*bp++ = ')';
			*bp = '\0';
		} else if (LN_ISARY(mod)) {
			*bp++ = '[';
			*bp++ = ']';
			*bp = '\0';
		}
		mod = mod >> 2;
	}
	(void)strcat(bigbuf, buf);
	return(bigbuf);
}

static char *
shift(s)
register char *s;
{
	register char *p1, *p2;
	char *rp;

	for (p1 = s; *p1; ++p1)
		;
	rp = p2 = p1++;
	while (p2 >= s)
		*p1-- = *p2--;
	return(++rp);
}
