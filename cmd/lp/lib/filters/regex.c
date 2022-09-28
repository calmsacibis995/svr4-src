/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/regex.c	1.2.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "regexpr.h"

#include "regex.h"

/**
 ** match() - TEST MATCH OF TEMPLATE/PATTERN WITH PARAMETER
 **/

int
#if	defined(__STDC__)
match (
	char *			re,
	char *			value
)
#else
match (re, value)
	register char *		re;
	register char *		value;
#endif
{
	int			ret;

	/*
	 * We want exact matches, just as if the regular expression
	 * was ^...$, to explicitly match the beginning and end of line.
	 * Using "advance" instead of "step" takes care of the ^ and
	 * checking where the match left off takes care of the $.
	 * We don't do something silly like add the ^ and $ ourselves,
	 * because the user may have done that already.
	 */
	ret = advance(value, re);
	if (ret && *loc2)
		ret = 0;
	return (ret);
}

/**
 ** replace() - REPLACE TEMPLATE WITH EXPANDED REGULAR EXPRESSION MATCH
 **/

size_t
#if	defined(__STDC__)
replace (
	char **			pp,
	char *			result,
	char *			value,
	int			nbra
)
#else
replace (pp, result, value)
	char **			pp;
	char *			result;
	char *			value;
	int			nbra;
#endif
{
	register char *		p;
	register char *		q;

	register size_t		ncount	= 0;


/*
 * Count and perhaps copy a single character:
 */
#define	CCPY(SRC)	if ((ncount++, pp)) \
				*p++ = SRC

/*
 * Count and perhaps copy a string:
 */
#define	SCPY(SRC)	if (pp) { \
				register char *	r; \
				for (r = (SRC); *r; ncount++) \
					*p++ = *r++; \
			} else \
				ncount += strlen(SRC)


	if (pp)   
		p = *pp;

	for (q = result; *q; q++)  switch (*q) {

	case '*':
	case '&':
		SCPY (value);
		break;

	case '\\':
		switch (*++q) {
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			register int		n = *q-'1';

			if (n < nbra) {
				register char		c = *(braelist[n]);

				*(braelist[n]) = 0;
				SCPY (braslist[n]);
				*(braelist[n]) = c;
			}
			break;
		}

		default:
			CCPY (*q);
			break;
		}
		break;

	default:
		CCPY (*q);
		break;
	}

	if (pp)
		*pp = p;

	return (ncount);
}
