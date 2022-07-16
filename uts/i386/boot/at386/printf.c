/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/printf.c	1.1.2.1"

#include "../sys/boot.h"

/* Stolen from the kernel */

#define	CR

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %lu %ld (==%lu)
 * %lo %lx are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 */

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register char *adx;
	char *s;

	adx = (char *)&x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0') {
			return;
		}
#ifdef	CR
		if (c == '\n')
			putchar('\r');
#endif /* CR */
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x') {
		printn((long)*(unsigned int *)adx, c=='o'? 8: (c=='x'? 16:10));
		adx += sizeof (int);
	}
	else if(c == 's') {
		s = *(char **)adx;
		while(c = *s++) {
#ifdef	CR
			if (c == '\n')
				putchar('\r');
#endif /* CR */
			putchar(c);
		}
		adx += sizeof (char *);
	} else if (c == 'l') {
		c = *fmt++;
		if(c == 'd' || c == 'u' || c == 'o' || c == 'x') {
			printn(*(long *)adx, c=='o'? 8: (c=='x'? 16:10));
			adx += sizeof (long);
		}
	}
	goto loop;
}

printn(n, b)
long n;
register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		putchar('-');
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		putchar("0123456789ABCDEF"[d[i]]);
	}
}
