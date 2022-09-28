/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:mailcomp.c	1.3.3.1"
#include "mail.h"
/*
    NAME
	mailcompile - compile mail surrogate regular expressions

    SYNOPSIS
	char *mailcompile(string *pattern, int *retlen, int *retnbra)

    DESCRIPTION
	 mailcompile() will compile the regular expression and
	 return the compiled version. Before passing the regular
	 expression on to compile(), it will make the following
	 changes to the pattern (because compile() expects the
	 magic characters to be in this format):

		(, )	->	\(, \)
		?	->	\{0,1\}
		+	->	\{1,\}
		\(, \)	->	(, )
		\?, \+	->	?, +

		%D	->	local domain name
		%L	->	local system name
		%U	->	local uname
		%X	->	smarter host
		%[lnRCcS]	disallowed
		%[a-z]	->	Xgetenv(%x)
		%%	->	%

    RETURNS
	A pointer to the compiled expression will be returned.
	The length of the compiled expression will be stored
	into retlen. The number of bracketed expressions will be
	stored into retnbra;

	If the compilation failed, NULL will be returned and
	regerrno will be left set to the error.
*/

#define BS '\\'

static void nomagic();

char *mailcompile(inpattern, retlen, retnbra)
string *inpattern;
int *retlen, *retnbra;
{
    static char	pn[] = "mailcompile";
    string *outpattern = s_new();
    register char *ip = s_to_c(inpattern);
    char *ret, *regex;
    int insize;
    unsigned int compsize;

    Dout(pn, 5, "inpattern = '%s'\n", s_to_c(inpattern));
    for ( ; *ip; ip++)
	switch (*ip)
	    {
	    case '%':
		switch (*++ip)
		    {
		    case 'D':	/* local domain name */
			nomagic(outpattern, maildomain());
			break;

		    case 'L':	/* local system name */
			nomagic(outpattern, thissys);
			break;

		    case 'U':	/* uname */
			nomagic(outpattern, utsn.nodename);
			break;

		    case 'X':
			nomagic(outpattern, Xgetenv("SMARTERHOST"));
			break;

		    case 'C': 	/* ignore */
		    case 'c': 	/* ignore */
		    case 'l':	/* ignore */
		    case 'n':	/* ignore */
		    case 'R': 	/* ignore */
		    case 'S':	/* ignore */
			break;

		    default:
			if (islower(*ip))
			    {
			    char x[3];
			    x[0] = '%';
			    x[1] = *ip;
			    x[2] = '\0';
			    nomagic(outpattern, Xgetenv(x));
			    }

			else
			    s_putc(outpattern, *ip);
			break;
		    }
		break;

	    case '(':	/* -> \( and \) */
	    case ')':
		s_putc(outpattern, BS);
		s_putc(outpattern, *ip);
		break;

	    case '?':	/* -> \{0,1\} */
		s_putc(outpattern, BS);
		s_putc(outpattern, '{');
		s_putc(outpattern, '0');
		s_putc(outpattern, ',');
		s_putc(outpattern, '1');
		s_putc(outpattern, BS);
		s_putc(outpattern, '}');
		break;

	    case '+':	/* -> \{1,\} */
		s_putc(outpattern, BS);
		s_putc(outpattern, '{');
		s_putc(outpattern, '1');
		s_putc(outpattern, ',');
		s_putc(outpattern, BS);
		s_putc(outpattern, '}');
		break;

	    case BS:
		switch (*++ip)
		    {
		    default:		/* \x -> \x */
			s_putc(outpattern, BS);
			/* FALLTHROUGH */

		    case '(': case ')':	/* \(,\),\?,\+ -> (,),?,+ */
		    case '?': case '+':
			s_putc(outpattern, *ip);
			break;
		    }
		break;

	    default:	/* regular character */
		s_putc(outpattern, *ip);
	    }

    s_terminate(outpattern);
    Dout(pn, 5, "outpattern = '%s'\n", s_to_c(outpattern));

    /* Now convert everything to lower case */
    for (ip = s_to_c(outpattern); *ip; ip++)
	if (isupper(*ip))
	    *ip = tolower(*ip);

    Dout(pn, 5, "lowered is = '%s'\n", s_to_c(outpattern));

    /* Let's malloc space to hold the compiled string, */
    /* using an estimate of twice the pattern size. */
    insize = s_curlen(outpattern);
    if (insize == 0)
	insize = 2;
    compsize = insize;

    /* regerrno == 50 means that the compiled string is */
    /* too large for the buffer or the buffer cannot be allocated. */
    regerrno = 50;
    ret = 0;
    regex = 0;
    loc1 = 0;

    /* Loop until we succeed with a buffer large enough */
    /* or the compilation fails for some other reason. */
    while (!ret && regerrno == 50)
	{
	compsize += insize;
	regex = regex ? realloc(regex, compsize) : malloc(compsize);
	if (!regex)
	    break;
	ret = compile(s_to_c(outpattern), regex, regex+compsize);
	}

    s_free(outpattern);
    if (retlen)
	*retlen = loc1 - regex;
    if (retnbra)
	*retnbra = nbra;
    return regex;
}

/*
    append a string to given pattern, making certain that all
    magic characters become non-magic.

	\	->	\\
	.	->	\.
	*	->	\*
	[	->	\[
*/
static void nomagic(outpattern, str)
register string *outpattern;
register char *str;
{
    for ( ; *str; str++)
	switch (*str)
	    {
	    case BS:
	    case '.':
	    case '*':
	    case '[':
		s_putc(outpattern, BS);
		/* FALLTHROUGH */
	    default:
		s_putc(outpattern, *str);
		break;
	    }
}
