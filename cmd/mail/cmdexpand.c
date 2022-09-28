/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:cmdexpand.c	1.4.3.1"
#include "mail.h"
/*
    NAME
	cmdexpand - expand mail surrogate command string

    SYNOPSIS
	void cmdexpand(int letnum, string *instr, string *outstr,
	               char **lbraslist, char **lbraelist);

    DESCRIPTION
	 cmdexpand() will expand the mail surrogate command.
	 It will make the following changes to the string:


		%D	->	local domain name
		%L	->	local system name
		%U	->	local uname
		%X	->	smarter host
		\1 - \9	->	pointers into orig and recip
		%l	->	content length
		%n	->	recipient name
		%R	->	return path to originator
		%C	->	content type
		%c	->	content type
		%S	->	subject
		%[a-z]	->	Xgetenv(%x)
		\x	->	x
		%%	->	%
*/

#define BS '\\'

void cmdexpand(letnum, instr, outstr, lbraslist, lbraelist)
int	letnum;
string	*instr, *outstr;
char	**lbraslist, **lbraelist;
{
    static char	pn[] = "cmdexpand";
    register char *ip = s_to_c(instr);
    register char *brap;
    struct hdrs	*hptr;
    register int i;

    Dout(pn, 7, "instr = '%s'\n", s_to_c(instr));
    for ( ; *ip; ip++)
	{
	switch (*ip)
	    {
	    case '%':
		switch (*++ip)
		    {
		    case 'C':
			s_append(outstr, let[letnum].text ? "text" : "binary");
			break;

		    case 'c':
			if ((hptr = hdrlines[H_CTYPE].head) !=
					(struct hdrs *)NULL) {
				s_append(outstr, hptr->value);
			}
			break;

		    case 'l':
			if ((hptr = hdrlines[H_CLEN].head) !=
					(struct hdrs *)NULL) {
				s_append(outstr, hptr->value);
			} else
				s_append(outstr, "0");
			break;

		    case 'D':
			s_append(outstr, maildomain());
			break;

		    case 'L':
			s_append(outstr, thissys);
			break;

		    case 'n':
			s_append(outstr, recipname);
			break;

		    case 'R':
			s_append(outstr, Rpath);
			break;

		    case 'U':
			s_append(outstr, utsn.nodename);
			break;

		    case 'X':
			s_append(outstr, Xgetenv("SMARTERHOST"));
			break;

		    case 'S':
			if ((hptr = hdrlines[H_SUBJ].head) !=
					(struct hdrs *)NULL) {
				s_append(outstr, hptr->value);
			}
			break;

		    default:
			if (islower(*ip))
			    {
			    char x[3];
			    x[0] = '%';
			    x[1] = *ip;
			    x[2] = '\0';
			    s_append(outstr, Xgetenv(x));
			    }

			else
			    s_putc(outstr, *ip);
			break;
		    }
		break;

	    case BS:
		switch (*++ip)
		    {
		    default:	/* \x -> \x */
			s_putc(outstr, BS);
			s_putc(outstr, *ip);
			break;

		    /* \1 - \0 becomes braslist[0] - braslist[9] */
		    case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9': case '0':
			i = (*ip == '0') ? 9 : (*ip - '1');
			for (brap = lbraslist[i]; brap < lbraelist[i]; brap++)
			    s_putc(outstr, *brap);
			break;
		    }
		break;

	    default:
		s_putc(outstr, *ip);
	    }
	}

    s_terminate(outstr);
    Dout(pn, 7, "outstr = '%s'\n", s_to_c(outstr));
}
