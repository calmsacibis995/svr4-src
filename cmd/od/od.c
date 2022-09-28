/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)od:od.c	1.11.1.5"
/*
 * od -- octal (also hex, decimal, and character) dump
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>

#define BSIZE	512	/* standard buffer size */
#define YES	1
#define NO	0
#define DBUFSIZE 16

struct dfmt {
	char	df_option;	/* command line option */
	int	df_field;	/* # of chars in a field */
	int	df_size;	/* # of bytes displayed in a field */
	int	df_radix;	/* conversion radix */
} *conv_vec[32];

/* The names of the structures are chosen for the options action
 *	e.g. u_s_oct - unsigned octal
 *	     u_l_hex - unsigned long hex
*/

struct dfmt	u_s_oct	= {'o',  6, sizeof (short),	 8};
struct dfmt	u_s_dec	= {'d',  5, sizeof (short),	10};
struct dfmt	s_s_dec	= {'s',  6, sizeof (short),	10};
struct dfmt	u_s_hex	= {'x',  4, sizeof (short),	16};
struct dfmt	byte	= {'b',  3, sizeof (char),	 8};
struct dfmt	cchar	= {'c',  3, sizeof (char),	 8};
struct dfmt	u_l_oct	= {'O', 11, sizeof (long),	 8};
struct dfmt	u_l_dec	= {'D', 10, sizeof (long),	10};
struct dfmt	s_l_dec	= {'S', 11, sizeof (long),	10};
struct dfmt	u_l_hex	= {'X',  8, sizeof (long),	16};
struct dfmt	flt	= {'f', 14, sizeof (float),	10};
struct dfmt	dble	= {'F', 21, sizeof (double),	10};

char	usage[] = "usage: od [-bcdDfFoOsSvxX] [file] [[+]offset[.][b]]";
char	word[DBUFSIZE];
char	lastword[DBUFSIZE];
int	base =	010;
int	max;
long	addr;

main(argc, argv)
int argc;
char **argv;
{
	register char *p;
	register n, f, same;
	struct dfmt *d;
	struct dfmt **cv = conv_vec;
	int showall = NO;

	(void)setlocale(LC_ALL, "");

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("od", &argv, 0);
#endif

	argv++;
	f = 0;
	if(argc > 1) {
		p = *argv;
		if(*p == '-') {
			while(*++p != '\0') {
				switch(*p) {
				case 'o':
					d = &u_s_oct;
					break;
				case 'd':
					d = &u_s_dec;
					break;
				case 's':

					d = &s_s_dec;
					break;
				case 'x':
				case 'h':
					d = &u_s_hex;
					break;
				case 'c':
					d = &cchar;
					break;
				case 'b':
					d = &byte;
					break;
				case 'D':
					d = &u_l_dec;
					break;
				case 'f':
					d = &flt;
					break;
				case 'F':
					d = &dble;
					break;
				case 'O':
					d = &u_l_oct;
					break;
				case 'S':
					d = &s_l_dec;
					break;
				case 'X':
					d = &u_l_hex;
					break;
				case 'v':
					showall = YES;
					continue;
				default :
					printf("od: bad flag -%c\n", *p);
					puts(usage);
					exit (1);
				}
				*(cv++) = d;
			}
			argc--;
			argv++;
		}
	}
	if (cv == conv_vec) {
		*(cv++) = &u_s_oct;
	}

	*cv = (struct dfmt *)0; 

	/* Calucate max number chars in line */
	max = 0;
	for (cv = conv_vec; d = *cv; cv++) {
		f = (d->df_field +1) * (DBUFSIZE / d->df_size);
		if (f > max)
			max = f;
	}

	if(argc > 1)
	if(**argv != '+') {
		if (freopen(*argv, "r", stdin) == NULL) {
			fprintf(stderr, "od: cannot open %s\n", *argv);
			exit(2);
		}
		argv++;
		argc--;
	}
	if(argc > 1)
		offset(*argv);

	same = -1;
	for ( ; (n = fread(word, 1, sizeof(word), stdin)) > 0; addr += n) {
		if (same>=0 && !showall) {
			for (f=0; f<DBUFSIZE; f++)
				if (lastword[f] != word[f])
					goto notsame;
			if (same==0) {
				printf("*\n");
				same = 1;
			}
			continue;
		}
	notsame:
		line(addr, word, (n+sizeof(word[0])-1)/sizeof(word[0]));
		same = 0;
		for (f=0; f<DBUFSIZE; f++)
			lastword[f] = word[f];
		for (f=0; f<DBUFSIZE; f++)
			word[f] = 0;
	}
	putn(addr, base, 7);
	putchar('\n');
	exit(0);
}

line(a, w, n)
long a;
char w[];
int n;
{
	register i, f;
	register struct dfmt *c;
	register struct dfmt **cv = conv_vec;

	f = 1;
	while (c = *cv++) {
		if(f) {
			putn(a, base, 7);
			f = 0;
		} else
			putchar('\t');

		i = 0;
		while (i<n) {
			i += putx(w+i, c);
		}
		putchar('\n');
	}
}

putx(n, c)
char n[];
struct dfmt *c;
{
	register ret = 0;
	
	switch(c->df_option) {
	case 'o':
	case 'd':
	case 'x':
		{
			unsigned short *sn = (unsigned short *)n;
			pre(c->df_field, c->df_size);
			putn((long)*sn, c->df_radix, c->df_field);
			ret = c->df_size;
			break;
		}
	case 's':
		{
			unsigned short *sn = (unsigned short *)n;
			putchar(' ');
			if(*sn > 32767){pre(c->df_field, c->df_size);
				putchar('-'); *sn = (~(*sn) + 1) & 0177777;}
			else pre(c->df_field-1, c->df_size);
			putn((long)*sn, c->df_radix, c->df_field-1);
			ret = c->df_size;
			break;
		}
	case 'c':
		pre(c->df_field, c->df_size);
		cput(*n);
		ret = c->df_size;
		break;
	case 'b':
		pre(c->df_field, c->df_size);
		putn((long)(*n)&0377, c->df_radix, c->df_field);
		ret = c->df_size;
		break;
	case 'D':
	case 'O':
	case 'X':
		pre(c->df_field, c->df_size);
		{
			unsigned long *ln = (unsigned long *)n;
			putn((long)*ln, c->df_radix, c->df_field);
			ret = c->df_size;
			break;
		}
	case 'f':
		pre(c->df_field, c->df_size);
		{
			float *fn = (float *)n;
			printf("%14.7e",*fn);
			ret = c->df_size;
			break;
		}
	case 'F':
		pre(c->df_field, c->df_size);
		{
			double *dn = (double *)n;
			printf("%21.14e",*dn);
			ret = c->df_size;
			break;
		}
	case 'S':
		{
			unsigned long *ln = (unsigned long *)n;
			if(*ln > 2147483647){pre(c->df_field, c->df_size);
				putchar('-'); *ln = (~(*ln)+1) & 037777777777;}
			else pre(c->df_field-1, c->df_size);
			putn((long)*ln, c->df_radix, c->df_field-1);
			ret = c->df_size;
			break;
		}
	}
	return(ret);
}

cput(c)
int c;
{
	c &= 0377;
	if(c >= 0200 && MB_CUR_MAX > 1)
		putn((long)c,8,3); /* Multi Envir. with Multi byte char */	
	else{
		if(isprint(c)) {
			printf("  ");
			putchar(c);
			return;
			}
		switch(c) {
		case '\0':
			printf(" \\0");
			break;
		case '\b':
			printf(" \\b");
			break;
		case '\f':
			printf(" \\f");
			break;
		case '\n':
			printf(" \\n");
			break;
		case '\r':
			printf(" \\r");
			break;
		case '\t':
			printf(" \\t");
			break;
		default:
			putn((long)c, 8, 3);
		}
	}
}

putn(n, b, c)
long n;
int b, c;
{
	register d;

	if(!c)
		return;
	putn(n/b, b, c-1);
	d = n%b;
	if (d > 9)
		putchar(d-10+'a');
	else
		putchar(d+'0');
}

pre(f,n)
int f, n;
{
	int i,m;

	m = (max/(DBUFSIZE/n)) - f;
	for(i=0; i<m; i++)
		putchar(' ');
}

offset(s)
register char *s;
{
	register char *p;
	long a;
	register int d;

	if (*s=='+')
		s++;
	if (*s=='x') {
		s++;
		base = 16;
	} else if (*s=='0' && s[1]=='x') {
		s += 2;
		base = 16;
	} else if (*s == '0')
		base = 8;
	p = s;
	while(*p) {
		if (*p++=='.')
			base = 10;
	}
	for (a=0; *s; s++) {
		d = *s;
		if(d>='0' && d<='9')
			a = a*base + d - '0';
		else if (d>='a' && d<='f' && base==16)
			a = a*base + d + 10 - 'a';
		else
			break;
	}
	if (*s == '.')
		s++;
	if(*s=='b' || *s=='B')
		a *= BSIZE;
	fseek(stdin, a, 0);
	addr = a;
}
