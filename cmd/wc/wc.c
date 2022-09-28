/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)wc:wc.c	1.5.1.4"
/*
**	wc -- word and line count
*/

#include	<stdio.h>
#include	<ctype.h>
#include	<locale.h>

unsigned char	b[BUFSIZ];

FILE *fptr = stdin;
long	wordct;
long	twordct;
long	linect;
long	tlinect;
long	charct;
long	tcharct;

main(argc,argv)
char **argv;
{
	register unsigned char *p1, *p2;
	register unsigned int c;
	int	i, token;
	int	status = 0;
	char	*wd;

	(void)setlocale(LC_ALL, "");

	wd = "lwc";
	if(argc > 1 && *argv[1] == '-') {
		wd = ++argv[1];
		argc--;
		argv++;
	}

	i = 1;
	do {
		if(argc>1 && (fptr=fopen(argv[i], "r")) == NULL) {
			fprintf(stderr, "wc: cannot open %s\n", argv[i]);
			status = 2;
			continue;
		}
		p1 = p2 = b;
		linect = 0;
		wordct = 0;
		charct = 0;
		token = 0;
		for(;;) {
			if(p1 >= p2) {
				p1 = b;
				c = fread(p1, 1, BUFSIZ, fptr);
				if(c <= 0)
					break;
				charct += c;
				p2 = p1+c;
			}
			c = *p1++;
			if(isgraph(c)) {
				if(!token) {
					wordct++;
					token++;
				}
				continue;
			}
			if(c=='\n')
				linect++;
			else if(c!=' '&&c!='\t')
				continue;
			token = 0;
		}

		/* print lines, words, chars */
		wcp(wd, charct, wordct, linect);
		if(argc>1) {
			printf(" %s\n", argv[i]);
		}
		else
			printf("\n");
		fclose(fptr);
		tlinect += linect;
		twordct += wordct;
		tcharct += charct;
	} while(++i<argc);
	if(argc > 2) {
		wcp(wd, tcharct, twordct, tlinect);
		printf(" total\n");
	}
	exit(status);
}

wcp(wd, charct, wordct, linect)
char *wd;
long charct; long wordct; long linect;
{
	register char *wdp=wd;

	while(*wdp) {
	switch(*wdp++) {
		case 'l':
			printf("%7ld ", linect);
			break;

		case 'w':
			printf("%7ld ", wordct);
			break;

		case 'c':
			printf("%7ld ", charct);
			break;

		default:
			fprintf(stderr, "usage: wc [-clw] [name ...]\n");
			exit(2);
		}
	}
}
