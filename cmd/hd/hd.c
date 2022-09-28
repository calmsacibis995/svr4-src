/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)hd:hd.c	1.1.1.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) hd.c 1.1 88/03/30 hd:hd.c
 */
/***	hd -- hex (also decimal, octal) dump
 *
 *	hd [-abwlxdoc] [-t] [-A] [-[sn] count[*][wlbk]] [file] ...
 *
 *	a	ascii
 *	b	bytes
 *	w	words (shorts)
 *	l	longs
 *	x	hex
 *	d	decimal
 *	o	octal
 *	c	character
 *	t	text
 *	A	ascii (in right margin)
 *	s	starting offset [word/long/block=512/k=1024]
 *	n	count [word/long/block=512/k=1024]
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifndef M_SYS3
#  define _tolower	tolower
#endif

#define LOWER(c)	(isupper(c)? _tolower(c) : (c))

#define	SBIT	1
#define NBIT	2

#define IHEX	0		/* index defines */
#define IDEC	1
#define IOCT	2
#define NFMTS	3

#define FHEX	(1 << IHEX)	/* output format bits */
#define FDEC	(1 << IDEC)
#define FOCT	(1 << IOCT)


/*
 *	IBYTE, IWORD & ILONG must be adjacent and in ascending order
 */
#define ICHAR	0
#define IBYTE	1
#define IWORD	2
#define ILONG	3
#define IADDR	4
#define NSIZES	5
#define NOSIZES	(NSIZES - 1)	/* assumes IADDR is last! */

/*
 *	The following defines indicate bits within a word to record
 *	occurrences of the -cbwla flags, respectively.
 */
#define CHAR	(1 << ICHAR)
#define BYTE	(1 << IBYTE)
#define WORD	(1 << IWORD)
#define LONG	(1 << ILONG)
#define ADDR	(1 << IADDR)

char *fmtstr[NFMTS] =	  { "%0*lx", "%*lu",  "%0*lo" };
char *addrfmtstr[NFMTS] = { "%04lx", "%05lu", "%06lo" };


struct finfo {				/* formatting info */
	int	f_cwidths[NFMTS];	/* composite width information */
	int	f_rwidths[NFMTS];	/* raw width information */
	int	f_size;			/* # of bytes per value */
	int	f_wshift;		/* # of bits to shift width right */
};

struct finfo byteinfo = {
	{ 12,	16,	16 },	/* composite IHEX, IDEC, IOCT widths */
	{  2,	 3,	 3 },	/* raw widths */
	   1,			/* # of bytes per value */
	   2			/* shifts to map width into 1 value width */
};

struct finfo wordinfo = {
	{ 10,	12,	14 },	/* composite IHEX, IDEC, IOCT widths */
	{  4,	 5,	 6 },	/* raw widths */
	   2,			/* # of bytes per value */
	   1			/* shifts to map width into 1 value width */
};

struct finfo longinfo = {
	{  9,	11,	12 },	/* composite IHEX, IDEC, IOCT widths */
	{  8,	10,	11 },	/* raw widths */
	   4,			/* # of bytes per value */
	   0			/* shifts to map width into 1 value width */
};


struct finfo *finfo[NOSIZES] = {
	&byteinfo,	/* ICHAR */
	&byteinfo,	/* IBYTE */
	&wordinfo,	/* IWORD */
	&longinfo	/* ILONG */
};

int	fmtmask[NSIZES];	/* format bit masks */


char	*Prog;			/* argv[0] */
char	cbuf[16];		/* current dump buffer */
char	lastcbuf[16];		/* previous dump buffer */
long	startaddr = 0L;		/* offset to begin dump */
long	endaddr = 0L;		/* address to stop before */
int	tflag = 0;		/* dump as text */
int	Aflag = 0;		/* add ascii format to first line */

int	width = 0;		/* columns required for 4 byte display */
char	*addrfs;		/* address printf format string */

long	atolm();




main(argc, argv)
int argc;
char **argv;
{
	register char *cp;
	register int tfmt;
	register int tsize;
	register int w;
	register int i;
	register int printname;
	register int snmask;
	
	for (cp = Prog = argv[0]; *cp; ) {   /* Prog = tail(argv[0]); */
		if (*cp++ == '/') {
			Prog = cp;
		}
	}
	while (argc > 1 && argv[1][0] == '-') {
		argc--;
		cp = *++argv;
		snmask = tfmt = tsize = 0;
		while (*++cp != '\0') {
			switch (*cp) {
				case 'c':	tsize |= CHAR;	break;
				case 'b':	tsize |= BYTE;	break;
				case 'w':	tsize |= WORD;	break;
				case 'l':	tsize |= LONG;	break;
				case 'a':	tsize |= ADDR;	break;

				case 'x':	tfmt |= FHEX;	break;
				case 'd':	tfmt |= FDEC;	break;
				case 'o':	tfmt |= FOCT;	break;

				case 't':	tflag = 1;	break;
				case 'A':	Aflag = 1;	break;
				case 's':	snmask |= SBIT;	break;
				case 'n':	snmask |= NBIT;	break;
				default:	usage();
					/* NOTREACHED */
			}
		}
		if (tsize == 0 && tfmt == 0) {
			if (snmask) {
				if (strlen(*argv++) != 2 || argc-- < 2) {
					usage();
				}
				if (snmask == SBIT) {
					startaddr = atolm(*argv);
				} else {
					endaddr = atolm(*argv);
				}
			}
			goto nextarg;
		}
		    /*
		     * If any of -cblwa, and none of -xdo,
		     * default to -o for od, else all of -xdo
		     */
		if (tfmt == 0) {
			tfmt = (*Prog == 'o')? FOCT : (FHEX | FDEC | FOCT);
		}
		    /*
		     * If none of -cbwla were specified and we were invoked
		     * as od, default to -w.
		     */
		if (tsize == 0 && *Prog == 'o') {
			fmtmask[IWORD] |= tfmt;
			goto nextarg;
		}
		for (i = 0; i < NSIZES; i++) {
		    /*
		     * Set tfmt bits for this size if:
		     * 1) none of -cblwa were specified (and not od), OR
		     * 2) this particular one of -cblwa was specified.
		     */
			if (tsize == 0 || (tsize & (1 << i))) {
				fmtmask[i] |= tfmt;
			}
		}
nextarg:	;
	}
	if ((fmtmask[ICHAR]|fmtmask[IBYTE]|fmtmask[IWORD]|fmtmask[ILONG]) == 0){
		if (*Prog == 'o') {
			fmtmask[IWORD] = FOCT;		/* od default: -wo */
		} else {
			fmtmask[IBYTE] = FHEX;		/* default: -bx -A */
			Aflag = 1;
		}
	} else if (tflag) {
		fprintf(stderr, "%s: -t flag overrides other flags\n", Prog);
	}
	width = 0;
	for (tsize = 0; tsize < NOSIZES; tsize++) {
		tfmt = fmtmask[tsize];
		for (i = 0; i < NFMTS; i++) {
			if (tfmt & (1 << i) &&
			    (w = finfo[tsize]->f_cwidths[i]) > width) {
				width = w;
			}
		}
	}
	if (fmtmask[IADDR] == 0) {	/* default: -ao for od, else -ax */
		fmtmask[IADDR] = (*Prog == 'o')? FOCT : FHEX;
	}
	tfmt = fmtmask[IADDR];
	for (i = 0; (tfmt & (1 << i)) == 0; i++)
		;				/* find first set bit */
	addrfs = addrfmtstr[i];
	if (fmtmask[IBYTE]) {
		width = (width + 3) & ~3;	/* make divisible by 4 */
	} else if (fmtmask[IWORD]) {
		width = (width + 1) & ~1;	/* make divisible by 2 */
	}
	if (endaddr) {
		endaddr += startaddr;
	}
	if (argc > 1) {
		printname = (argc > 2);
		while (argc-- > 1) {
			if (access(*++argv, 4) < 0) {
				printf("%s: cannot access %s\n", Prog, *argv);
				continue;
			}
			if (freopen(*argv, "r", stdin) == NULL) {
				printf("%s: open of %s failed\n", Prog, *argv);
				exit(3);  /* once freopen failed, always will */
			}
			if (printname) {
				printf("%s:\n", *argv);
			}
			if (tflag) {
				td();			/* text dump 1 file */
			} else {
				hd();			/* hex dump 1 file */
			}
			if (printname && argc > 1) {
				printf("\n");
			}
		}
	} else {
		if (tflag) {
			td();			/* text dump stdin */
		} else {
			hd();			/* hex dump stdin */
		}
	}
}




td()
{
	register int c;
	register int ncols = 0;
	register long addr = startaddr;
	register int c1;

	if (doseek()) {
		goto eof;
	}
	while ((addr < endaddr || endaddr == 0L) && (c1=c=getchar()) != EOF) {
		if (ncols++ == 0) {
			printf(addrfs, addr);
			putchar('\t');
		}
		addr++;
		if ((c & 0x7f) == 0x7f) {
			printf("\\%o", c);
			ncols += 3;
		} else {
			if (c & 0x80) {
				c &= ~0x80;
				putchar('~');
				ncols++;
			}
			if (c < 0x20) {
				c += 0x40;
				putchar('^');
				ncols++;
			}
			switch (c) {
				case '\\':
				case '^':
				case '~':
					putchar('\\');
					ncols++;
			}
			putchar(c);
		}
		if (c1 == '\n' || ncols > 68) {
			putchar('\n');
			ncols = 0;
		}
	}
	if (ncols) {
		putchar('\n');
	}
eof:
	printf(addrfs, addr);
	putchar('\n');
}




hd()
{
	register n, i, same;
	register long addr = startaddr;
	
	if (doseek()) {
		goto eof;
	}
	same = -1;
	for ( ; addr < endaddr || endaddr == 0L; addr += n) {
		n = 16;
		if (endaddr && endaddr - addr < 16) {
			n = (int) (endaddr - addr);
		}
		if ((n = fread(cbuf, sizeof(char), n, stdin)) <= 0) {
			break;
		}
		if (same >= 0 && n == 16) {
			for (i = 0; i < 16; i++) {
				if (lastcbuf[i] != cbuf[i]) {
					goto notsame;
				}
			}
			if (same == 0) {
				printf("*\n");
				same = 1;
			}
			continue;
		}
	notsame:
		printf(addrfs, addr);
		dumpline(cbuf, n);
		same = 0;
		for (i = 0; i < 16; i++) {
			lastcbuf[i] = cbuf[i];
		}
	}
eof:
	printf(addrfs, addr);
	putchar('\n');
}




dumpline(buf, n)
char buf[];
int n;
{
	register int findex;
	register int fmask;
	register int sindex;
	register int cols;
	register int asciitoo;

	asciitoo = Aflag;
	for (sindex = 0; sindex < NOSIZES; sindex++) {
		cols = width >> finfo[sindex]->f_wshift;
		fmask = fmtmask[sindex];
		for (findex = 0; findex < NFMTS; findex++) {
			if (fmask & (1 << findex)) {
				dofmt(buf, n, sindex, cols, findex, asciitoo);
				asciitoo = 0;
			}
		}
	}
}




dofmt(buf, n, sindex, cols, findex, asciitoo)
char buf[];
int n;
int sindex;
int cols;
int findex;
int asciitoo;
{
	register int i;
	register int size;
	register int c;
	register long value;
	register int fill;
	register int delta;
	register int rawwidth;
	register char *fs;
	int ncols = 0;

	putchar('\t');
	fs = fmtstr[findex];
	i = 0;
newsize:
	size = finfo[sindex]->f_size;
	rawwidth = finfo[sindex]->f_rwidths[findex];
	fill = cols - rawwidth;
	for ( ; i < n; i += size) {
		delta = 0;
		if (i == 0) {
			delta = -1;	/* skip leading space */
		} else if (i == 8) {
			delta = 1;	/* separate left & right 8 bytes */
		}
		if (size > n - i) {	/* if not enough bytes left: */
			sindex--;		/* cut size in half */
			cols = (cols + 1) / 2;	/* and field width, too */
			goto newsize;
		}
		switch (size) {
			case 1:
				value = buf[i] & 0xffL;
				break;
			case 2:
				value = ( *((short *) &buf[i])) & 0xffffL;
				break;
			case 4:
				value = *((long *) &buf[i]);
				break;
		}
		if (sindex == ICHAR) {
			cput(fs, rawwidth, value, cols + delta);
		} else {
			printf("%*s", fill + delta, "");
			printf(fs, rawwidth, value);
		}
		ncols += cols + delta;
	}
	if (asciitoo) {
	    /*printf("%*s", (((16 - n) * width) / 4) + ((n < 8)? 4 : 3), "");*/
		printf("%*s", (4 * width) - ncols + 3, "");
		for (i = 0; i < n; i++) {
			if ((c = buf[i] & 0xff) < 0x20 || c > 0x7e) {
				c = '.';
			}
			putchar(c);
		}
	}
	putchar('\n');
}




cput(fs, rawwidth, value, cols)
char *fs;
int rawwidth;
long value;
int cols;
{
	register int c1 = ' ';		/* assume c is printable */
	register int c2 = (int) value;

	if (c2 < 0x20 || c2 > 0x7e) {
		c1 = '\\';		/* oops, can we print it as C escape? */
		switch (c2) {
			case '\0':  c2 = '0';  break;
			case '\b':  c2 = 'b';  break;
			case '\f':  c2 = 'f';  break;
			case '\n':  c2 = 'n';  break;
			case '\r':  c2 = 'r';  break;
			case '\t':  c2 = 't';  break;
			default:	/* nope, print in numeric format */
				printf("%*s", cols - rawwidth, "");
				printf(fs, rawwidth, value);
				return;
		}
	}
	printf("%*s%c%c", cols - 2, "", c1, c2);
}




doseek()
{
	register int c;
	register long count;
	extern int errno;

	if (isatty(fileno(stdin)) == 0) {
		if (fseek(stdin, startaddr, 0) < 0) {
			if (errno == ESPIPE) {
				goto pipe;
			}
			return(1);		/* failure */
		}
		return(0);
	}
pipe:
	count = startaddr;
	c = 0;
	while (count-- > 0 && (c = getchar()) != EOF)
		;
	return(c == EOF);
}




#define BHEX	16		/* base multipliers */
#define BDEC	10
#define BOCT	8


/*
 * atolm - atol with variable base and optional multiplier
 */

long
atolm(s)
register char *s;
{
	register int digit;
	register int base = BDEC;
	register long r;

	if (*s == '0' && s[1] == 'x') {
		s += 2;
		base = BHEX;
	} else if (*s == '0') {
		base = BOCT;
	}
	for (r = 0; isxdigit(digit = *s); s++) {
		if (isdigit(digit)) {
			if (digit > '7' && base == BOCT) {
				break;
			}
			r = (r * base) + digit - '0';
		} else if (base == BHEX) {
			r = (r * base) + LOWER(digit) + 10 - 'a';
		} else {
			break;
		}
	}
	if (*s == '*') {
		s++;
	}
	switch (LOWER(*s)) {
		case 'w':	r *= 2;		break;
		case 'l':	r *= 4;		break;
		case 'b':	r *= 512;	break;
		case 'k':	r *= 1024;	break;
		case '\0':	break;
		default:
			fprintf(stderr, "%s: bad count/offset value\n", Prog);
			exit(3);
	}
	return(r);
}




usage()
{
	fprintf(stderr,
    "usage: %s [-acbwlAxdo] [-t] [-s offset[*][wlbk]] [-n count[*][wlbk]] [file] ...\n",Prog);
	exit(2);
}
