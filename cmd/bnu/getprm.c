/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:getprm.c	2.9.3.1"

#include "uucp.h"

#define LQUOTE	'('
#define RQUOTE ')'

static char *bal();

/*
 * get next parameter from s
 *	s	-> string to scan
 *	whsp	-> pointer to use to return leading whitespace
 *	prm	-> pointer to use to return token
 * return:
 *	 s	-> pointer to next character 
 *		NULL at end
 */
char *
getprm(s, whsp, prm)
register char *s, *whsp, *prm;
{
	register char *c;
	char rightq;		/* the right quote character */
	char *beginning;

	beginning = prm;

	while ((*s == ' ') || (*s == '\t') || (*s == '\n')) {
		if (whsp != (char *) NULL)
			*whsp++ = *s;
		s++;
	}

	if ( whsp != (char *) NULL )
		*whsp = '\0';

	while (*s) {
		switch (*s) {
		case '\0':
		case ' ':
		case '\t':
		case '\n':
			*prm = '\0';
			return(prm == beginning ? NULL : s);
			/* NOTREACHED */
			break;
		case '>':
			if ((prm == beginning + 1) && (*beginning == '2'))
				*prm++ = *s++;
			if ((prm == beginning + 1) && (*beginning == '1'))
				*beginning = *s++;
			if (prm == beginning)
				*prm++ = *s++;
			*prm = '\0';
			return(s);
			/* NOTREACHED */
			break;
		case '<':
			if ((prm == beginning + 1) && (*beginning == '0'))
				*beginning = *s++;
			/* FALLTHRU */
		case '|':
		case ';':
		case '&':
		case '^':
		case '\\':
			if (prm == beginning)
				*prm++ = *s++;
			*prm = '\0';
			return(s);
			/* NOTREACHED */
			break;
		case '\'':
		case '(':
		case '`':
		case '"':
			if (prm == beginning) {
				rightq = ( *s == '(' ? ')' : *s );
				c = bal(s, rightq);
				(void) strncpy(prm, s, c-s+1);
				prm += c - s + 1;
				if ( *(s=c) == rightq)
					s++;
			}
			*prm = '\0';
			return(s);
			/* NOTREACHED */
			break;
		default:
			*prm++ = *s++;
		}
	}

	*prm = '\0';
	return(prm == beginning ? NULL : s);
}

/*
 * bal - get balanced quoted string
 *
 * s - input string
 * r - right quote
 * Note: *s is the left quote
 * return:
 *  pointer to the end of the quoted string
 * Note:
 *	If the string is not balanced, it returns a pointer to the
 *	end of the string.
 */

static char *
bal(s, r)
register char *s;
char r;
{
	short count = 1;
	char l;		/* left quote character */

	for (l = *s++; *s; s++) {
	    if (*s == r) {
		if (--count == 0)
		    break;	/* this is the balanced end */
	    }
	    else if (*s == l)
		count++;
	}
	return(s);
}

/*
 * split - split the name into parts:
 *	arg  - original string
 *	sys  - leading system name
 *	fwd  - intermediate destinations, if not NULL, otherwise
 *		only split into two parts.
 *	file - filename part
 */

int
split(arg, sys, fwd, file)
char *arg, *sys, *fwd, *file;
{
    register char *cl, *cr, *n;
    int retval = 0;

    *sys = *file = NULLCHAR;
    if ( fwd != (char *) NULL )
	*fwd = NULLCHAR;

    /* uux can use parentheses for output file names */
    /* we'll check here until  we can move it to uux */
    if (EQUALS(Progname,"uux") && (*arg == LQUOTE)) {
	char *c;
	c = bal(arg++, RQUOTE);
	(void) strncpy(file, arg, c-arg);
	file[c-arg] = NULLCHAR;
	return(retval);
	}
	
    for (n=arg ;; n=cl+1) {
	cl = strchr(n, '!');
	if ( cl == NULL) {
	    /* no ! in n */
	    (void) strcpy(file, n);
	    return(retval);
	}

	retval = 1;
	if (cl == n)	/* leading ! */
	    continue;
	if (EQUALSN(Myname, n, cl - n) && Myname[cl - n] == NULLCHAR)
	    continue;

	(void) strncpy(sys, n, cl-n);
	sys[cl-n] = NULLCHAR;

	if (fwd != (char *) NULL) {
	    if ( cl != (cr = strrchr(n, '!')) ) {
		/*  more than one ! */
		(void) strncpy(fwd, cl+1, cr-cl-1);
		fwd[cr-cl-1] = NULLCHAR;
	    }
	} else {
	    cr = cl;
	}

	(void) strcpy(file, cr+1);
	return(retval);
    }
    /*NOTREACHED*/
}

