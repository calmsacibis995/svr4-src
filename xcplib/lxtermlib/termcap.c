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

#ident	"@(#)xcplxterm:termcap.c	1.1"

/*
 *	@(#) termcap.c 1.1 90/04/06 lxtermlib:termcap.c
 */
/* Copyright (c) 1979 Regents of the University of California */
#define	BUFSIZ	1024
#define MAXHOP	32	/* max number of tc= indirections */

#include <ctype.h>
/* M000 begin */
/*#include "local/uparm.h"*/
#define libpath(file) "/usr/lib/file"
#define loclibpath(file) "/usr/local/lib/file"
#define binpath(file) "/usr/bin/file"
#define usrpath(file) "/usr/file"
#define E_TERMCAP	"/etc/termcap"
#define B_CSH		"/bin/csh"
/* M000 end */

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 *
 *	M000	08 Aug 83	andyp	3.0 upgrade
 *	- removed Berkeley include file dependencies.
 *	M001	09 Aug 84	sco!andy
 *	- removed recursion in tgetent() & tnchktc() to reduce
 *	  stack usage.
 *	  Tgetent() was changed to tget1ent(), and it now returns
 *	  without calling tnchktc().  A new tgetent() was added
 *	  that first calls tget1ent() and then tnchktc().
 *	  Tnchktc() was modified to add all of the tc='s instead
 *	  of just one.
 *	  I also declared tnchktc() and tnamatch() as static, and
 *	  made a minor bug fix in tnchktc().
 */

static	char *tbuf;
static	int hopcount;	/* detect infinite loops in termcap, init 0 */
static	char	*tskip();
	char	*tgetstr();
static	char	*tdecode();
static	int	 tget1ent();
static	int	 tnamatch();
static	int	 tnchktc();
extern	char	*getenv();

/* M001 begin */
/*
 * Get the full entry for terminal name in buffer bp.
 * This routine calls tget1ent() to get the primary
 * entry for the terminal, and then calls tnchktc()
 * to get all of the tc= entries.
 */
tgetent(bp, name)
	char *bp, *name;
{
	register ret;

	if( (ret = tget1ent(bp,name)) <= 0 )
		return(ret);	/* failure */
	return(tnchktc());
}
/* M001 end */

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
static int
tget1ent(bp, name)		/* M001: changed tgetent() to tget1ent() */
	char *bp, *name;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[BUFSIZ];
	char *cp2;
	int tf;

	tbuf = bp;
	tf = 0;
#ifndef V6
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.
	 */
	if (cp && *cp) {
		if (*cp!='/') {
			cp2 = getenv("TERM");
			if (cp2==(char *) 0 || strcmp(name,cp2)==0) {
				strcpy(bp,cp);
				/* return(tnchktc());      M001 */
				return(1);		/* M001 */
			} else {
				tf = open(E_TERMCAP, 0);
			}
		} else
			tf = open(cp, 0);
	}
	if (tf==0)
		tf = open(E_TERMCAP, 0);
#else
	tf = open(E_TERMCAP, 0);
#endif
	if (tf < 0)
		return (-1);
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZ);
				if (cnt <= 0) {
					close(tf);
					return (0);
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+BUFSIZ) {
				write(2,"Termcap entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (tnamatch(name)) {
			close(tf);
			/* return(tnchktc());	   M001 */
			return(1);		/* M001 */
		}
	}
}

/*
 * tnchktc: check the last entry, see if it's tc=xxx. If so,
 * call tget1ent() to find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
static int						/* M001 */
tnchktc()
{
	register char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[BUFSIZ];
	char *holdtbuf = tbuf;
	int l;

	for (;;) {				/* M001: added for loop  */
		p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
		while (*--p != ':')
			if (p<tbuf) {
				write(2, "Bad termcap entry\n", 18);
				return (0);
			}
		p++;
		/* p now points to beginning of last field */
		if (p[0] != 't' || p[1] != 'c')
			return(1);
		strcpy(tcname,p+3);
		q = tcname;
		while (q && *q != ':')
			q++;
		*q = 0;
		if (++hopcount > MAXHOP) {
			write(2, "Infinite tc= loop\n", 18);
			return (0);
		}
		if (tget1ent(tcbuf, tcname) != 1)	/* M001 */
			return(0);
		for (q=tcbuf; *q != ':'; q++)
			;
		l = p - holdtbuf + strlen(q);
		if (l > BUFSIZ) {
			write(2, "Termcap entry too long\n", 23);
			/* q[BUFSIZ - (p-tbuf)] = 0;	   M001: bug fix */
			q[BUFSIZ - (p-holdtbuf)] = 0;	/* M001 */
			return (1);			/* M001 */
		}
		strcpy(p, q+1);
		tbuf = holdtbuf;
	}					/* M001: added for loop */
}

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
static int					/* M001 */
tnamatch(np)
	char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
tskip(bp)
	register char *bp;
{

	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (bp);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
tgetnum(id)
	char *id;
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
tgetflag(id)
	char *id;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return(0);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
tgetstr(id, area)
	char *id, **area;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(0);
		if (*bp != '=')
			continue;
		bp++;
		return (tdecode(bp, area));
	}
}

/*
 * Tdecode does the grung work to decode the
 * string capability escapes.
 */
static char *
tdecode(str, area)
	register char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}

