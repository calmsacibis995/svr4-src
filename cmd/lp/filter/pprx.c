/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/pprx.c	1.1.3.1"

#include	<stdio.h>
#include	<signal.h>


#define	LINEMAX	132

char buf1[LINEMAX], buf2[LINEMAX];
char line[2*BUFSIZ];
char block[BUFSIZ];
int n1 = 0, n2 = -1;

main(argc, argv)
int argc;
char *argv[];
{
	int i, bs, spaces;
	char c,	*p, *fgets();

	setbuf(stdout, block);
	for(i =	0; i < LINEMAX;	i++) {
		buf2[i]	= ' ';
		buf1[i]	= '\0';
	}

	while(fgets(line, 2*BUFSIZ, stdin) != NULL) {
		p = line;
		bs = 0;
		if(*p == '\b') {
			if(puts(line) < 0)
				exit(1);
		}
		else {
			while((c = *(p++)) != '\0' && c	!= '\n') {
				switch(c) {
				case '\b':
					if(n1) {
						bs++;
						n1--;
					}
					break;
				case '\t':
					spaces = 8 - (n1 % 8);
					for(i =	1; i <=	spaces;	i++)
						buf1[n1++] = ' ';
					break;
				default:
					if(bs) {
						if(c ==	'_' ||
						   c ==	buf1[n1]) {
							n2 = n1++;
							buf2[n2] = '_';
						}
						else if(buf1[n1] == '_') {
							n2 = n1;
							buf2[n2] = '_';
							buf1[n1++] = c;
						}
						else {
							buf1[n1++] = c;
						}
						bs--;
					}
					else
						buf1[n1++] = c;
					if(n1 >= LINEMAX)
						n1--;
				}
			}
			flushbuf();
		}
	}
	exit(0);
}

flushbuf()
{
	int i;
	if(n1) {
		if(n2 >= 0) {
			buf1[n1++] = '\r';
			buf2[++n2] = '\0';
		}
		else
			buf1[n1++] = '\n';
		buf1[n1] = '\0';
		if(printf("%s", buf1) < 0)
			exit(1);
		for(i =	0; i < n1; i++)
			buf1[i]	= '\0';
		n1 = 0;
		if(n2 >= 0) {
			if(printf("%s\n", buf2) < 0)
				exit(1);
			for(i =	0; i <=	n2; i++)
				buf2[i]	= ' ';
			n2 = -1;
		}
	}
	else
		if(printf("\n") < 0)
			exit(1);
}

