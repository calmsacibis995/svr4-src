/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mb1:uts/i386/boot/mb1/printf.c	1.3"

#include "../sys/boot.h"
#include "../sys/error.h"
#include "../sys/farcall.h"

#define LINE_LEN 80
char msg[LINE_LEN];	/* Cannot take address of a local */
int msgindx = 0;
output(c)
register int c;
{
	i8251co(c);
}

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 *
 * MODIFICATION HISTORY:
 *	26 Feb 1988	greggo
 *		added support for %b and %c options.
 */
int prec;
int hadprec;

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register int	c;
	POINTER	adx;
	register char	*s;
	register int nb;	/* number of bytes to print.  used for %b support */

	adx.offset = (char *)&x1;
	adx.sel = get_ss();
loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			return;
		}
		output(c);
	}
	hadprec = 0;
	if ( *fmt == '.' ) {
		prec = 0;
		hadprec = 1;
		while ( '0' <= *++fmt && *fmt <= '9' )
			prec = prec * 10 + *fmt - '0';
	}
	if ((c = *fmt++) == 'l') c = *fmt++;
	if (c <= 'Z' && c >= 'A') c += 'a' - 'A';
	nb = (c == 'b') ? 1 : 4;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x' || c == 'b')
	    printn(far_long(adx.sel, adx.offset),
				c=='o'? 8: (c=='x'? 16: (c=='b'? 16:10)),
				nb);
	else if (c == 's') {
		s = (char *)far_long(adx.sel, adx.offset);
		while (c = *s++) {
			output(c);
		}
	}
	else if (c == 'c') {
		output( far_byte(adx.sel, adx.offset) );
	}
	adx.offset += sizeof(long);
	goto loop;
}

char d[12];
printn(n, b, nbytes)
long n;
register b;
{
	register i, nd, c;
	register int	flag;
	register int	plmax;

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
	if (nbytes == 1)
		plmax = 2;
	else
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		output('-');
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
		if ((n==0) && (b==10) && (flag==0))  /* zero fill hex, octal */
			break;
	}
	if (i==plmax)
		i--;
	if ( hadprec ) {
	    int npad;

	    npad = prec - i - 1;
	    while ( npad-- > 0 ) {
		output( ' ' );
	    }
	}
	for (;i>=0;i--) {
		if (d[i] > 9)
			output('A'+(d[i]-10));
		else
			output('0'+d[i]);
	}
}
