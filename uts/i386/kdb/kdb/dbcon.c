/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbcon.c	1.3"

/*
 * kernel debugger console interface routines
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/kdebugger.h"

#define EOT     0x04    /* ascii eot character */

#define XOFF    0x13
#define XON     0x11

extern int db_msg_pending;

char dbhold;    /* dbprintf hold flag set by XOFF in dbwait()*/

/*
 * dbgets reads a line from the debugging console using polled i/o
 */

char *
dbgets (buf, count)
    char *buf;
    short count;
{
    short c;
    short i;

    count--;
    for (i = 0; i < count; ) {
	while ((c = DBG_GETCHAR()) == -1) ;
	if (c == '\r')
	    c = '\n';
	DBG_PUTCHAR(c);
	if (c == '\b') {                /* backspace */
	    DBG_PUTCHAR(' ');
	    DBG_PUTCHAR('\b');
	    if (i > 0)
		    i--;
	    continue;
	}
	if (c == EOT && i == 0)         /* ctrl-D */
	    return NULL;
	buf[i++] = c;
	if (c == '\n')
	    break;
    }
    buf[i] = '\0';
    return (buf);
}


/*
 * dbwait - called by dbprintf to wait if XOFF has been entered.
 * If so, turns off interrupts
 * and polls keyboard until XON is entered.
 */
dbwait()
{
    if (! dbhold) {
	if (DBG_GETCHAR() == XOFF) {
	    dbhold++;
	}
    }
    if (dbhold) {
	while (DBG_GETCHAR() != XON) ;
	dbhold = 0;
    }
}

dbpause(seconds)
    short seconds;
{
    /*
     * Use 'spinwait' since it compensates for processor clock speed.
     */
    spinwait( (int) (1000*seconds));
}


dbprintf(fmt, x1)
	register char *fmt;
	unsigned x1;
{
	register int	c;
	register uint	*adx;
	register char	*s;
	int prec, width, ljust;

	if (db_msg_pending)
		db_brk_msg(1);

	dbwait();      /* wait for XON/XOFF */

	adx = &x1;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			return;
		}
		DBG_PUTCHAR(c);
	}
	if (*fmt == '%') {
		DBG_PUTCHAR('%');
		fmt++;
		goto loop;
	}
	prec = width = ljust = 0;
	if (*fmt == '-') {
		ljust = 1;  ++fmt;
	}
	if (*fmt == '*') {
		width = *adx++;
		++fmt;
	} else {
		while ('0' <= *fmt && *fmt <= '9')
			width = width * 10 + *fmt++ - '0';
	}
	if ( *fmt == '.' ) {
		if (*++fmt == '*') {
			prec = *adx++;
			++fmt;
		} else {
			while ( '0' <= *fmt && *fmt <= '9' )
				prec = prec * 10 + *fmt++ - '0';
		}
	}
	if ((c = *fmt++) == 'l') c = *fmt++;
	if (c <= 'Z' && c >= 'A') c += 'a' - 'A';
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		dbprintn((long)*adx, c=='o'? 8: (c=='x'? 16:10), width, ljust);
	else if (c == 's') {
		s = (char *)*adx;
		if (prec == 0 || prec > strlen(s))
			prec = strlen(s);
		width -= prec;
		if (!ljust)
			while (width-- > 0)
				DBG_PUTCHAR(' ');
		while (prec-- > 0)
			DBG_PUTCHAR(*s++);
		if (ljust)
			while (width-- > 0)
				DBG_PUTCHAR(' ');
	} else if (c == 'c') {
		if (!ljust)
			while (width-- > 1)
				DBG_PUTCHAR(' ');
		DBG_PUTCHAR(*adx);
		if (ljust)
			while (width-- > 1)
				DBG_PUTCHAR(' ');
	}
	adx++;
	goto loop;
}

dbprintn(n, b, width, ljust)
	long n;
	register b;
	int width, ljust;
{
	register i, nd, c;
	int	flag, minus;
	int	plmax;
	char d[12];

	c = 1;
	minus = 0;
	if ((flag = n < 0))
		n = -n;
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		minus = 1;
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

	width -= (i + 1) + minus;

	if (!ljust)
		while (width-- > 0)
			DBG_PUTCHAR(' ');

	if (minus)
		DBG_PUTCHAR('-');
	for (;i>=0;i--) {
		DBG_PUTCHAR("0123456789ABCDEF"[d[i]]);
	}

	if (ljust)
		while (width-- > 0)
			DBG_PUTCHAR(' ');
}
