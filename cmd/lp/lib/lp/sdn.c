/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/sdn.c	1.5.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"

#define N_COMPRESSED	9999

/**
 ** printsdn() - PRINT A SCALED DECIMAL NUMBER NICELY
 **/

#define	DFLT_PREFIX	0
#define	DFLT_SUFFIX	0
#define	DFLT_NEWLINE	"\n"

static char		*print_prefix	= DFLT_PREFIX,
			*print_suffix	= DFLT_SUFFIX,
			*print_newline	= DFLT_NEWLINE;

void
#if	defined(__STDC__)
printsdn_setup (
	char *			prefix,
	char *			suffix,
	char *			newline
)
#else
printsdn_setup (prefix, suffix, newline)
	char			*prefix,
				*suffix,
				*newline;
#endif
{
	if (prefix)
		print_prefix = prefix;
	if (suffix)
		print_suffix = suffix;
	if (newline)
		print_newline = newline;
	return;
}

void
#if	defined(__STDC__)
printsdn_unsetup (
	void
)
#else
printsdn_unsetup ()
#endif
{
	print_prefix = DFLT_PREFIX;
	print_suffix = DFLT_SUFFIX;
	print_newline = DFLT_NEWLINE;
	return;
}

void
#if	defined(__STDC__)
printsdn (
	FILE *			fp,
	SCALED			sdn
)
#else
printsdn (fp, sdn)
	FILE			*fp;
	SCALED			sdn;
#endif
{
	register char		*dec = "9999.999",
				*z;

	if (sdn.val <= 0)
		return;

	(void)fprintf (fp, "%s", NB(print_prefix));

	/*
	 * Let's try to be a bit clever in dealing with decimal
	 * numbers. If the number is an integer, don't print
	 * a decimal point. If it isn't an integer, strip trailing
	 * zeros from the fraction part, and don't print more
	 * than the thousandths place.
	 */
	if (-1000. < sdn.val && sdn.val < 10000.) {

		/*
		 * Printing 0 will give us 0.000.
		 */
		sprintf (dec, "%.3f", sdn.val);

		/*
		 * Skip zeroes from the end until we hit
		 * '.' or not-0. If we hit '.', clobber it;
		 * if we hit not-0, it has to be in fraction
		 * part, so leave it.
		 */
		z = dec + strlen(dec) - 1;
		while (*z == '0' && *z != '.')
			z--;
		if (*z == '.')
			*z = '\0';
		else
			*++z = '\0';

		(void)fprintf (fp, "%s", dec);

	} else
		(void)fprintf (fp, "%.3f", sdn.val);

	if (sdn.sc == 'i' || sdn.sc == 'c')
		putc (sdn.sc, fp);

	(void)fprintf (fp, "%s%s", NB(print_suffix), NB(print_newline));
	return;
}


/**
 ** _getsdn() - PARSE SCALED DECIMAL NUMBER
 **/

SCALED
#if	defined(__STDC__)
_getsdn (
	char *			str,
	char **			p_after,
	int			is_cpi
)
#else
_getsdn (str, p_after, is_cpi)
	char *			str;
	char **			p_after;
	int			is_cpi;
#endif
{
	static SCALED		sdn	= { 0.0 , 0 };

	char *			rest;


	/*
	 * A nonzero "errno" is our only method of indicating error.
	 */
	errno = 0;

	if (is_cpi && STREQU(str, NAME_PICA)) {
		sdn.val = 10;
		sdn.sc = 0;
		if (p_after)
			*p_after = str + strlen(NAME_PICA);

	} else if (is_cpi && STREQU(str, NAME_ELITE)) {
		sdn.val = 12;
		sdn.sc = 0;
		if (p_after)
			*p_after = str + strlen(NAME_ELITE);

	} else if (is_cpi && STREQU(str, NAME_COMPRESSED)) {
		sdn.val = N_COMPRESSED;
		sdn.sc = 0;
		if (p_after)
			*p_after = str + strlen(NAME_COMPRESSED);

	} else {
		sdn.val = strtod(str, &rest);
		if (sdn.val <= 0) {
			lp_errno = LP_EBADSDN;
			errno = EINVAL;
			return (sdn);
		}

		while (*rest && *rest == ' ')
			rest++;

		switch (*rest) {
		case 0:
			sdn.sc = 0;
			if (p_after)
				*p_after = rest;
			break;
		case 'i':
		case 'c':
			sdn.sc = *rest++;
			if (p_after)
				*p_after = rest;
			break;
		default:
			lp_errno = LP_EBADSDN;
			errno = EINVAL;
			sdn.sc = 0;
			break;
		}
	}

	return (sdn);
}
