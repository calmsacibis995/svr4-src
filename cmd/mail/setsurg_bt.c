/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:setsurg_bt.c	1.3.3.1"
#include "mail.h"

/*
    NAME
	setsurg_bt - interpret the option list for translation entries

    SYNOPSIS
	void setsurg_bt(string *st, int *pbatchsize, int *presolved)

    DESCRIPTION
	The string st is of the form
		B=n;T=n;
	where the options can be in any order. Each "n" is of the form
		number

	The value of B= is returned in pbatchsize.
	The value of T= is returned in presolved.
	The value -1 is returned for values not set.
*/

void setsurg_bt(st, pbatchsize, presolved)
string *st;
int *pbatchsize;
int *presolved;
{
    char *pn = "setsurg_bt";
    string *B = 0, *T = 0, *tok;

    Tout(pn, "Looking at status list\n");
    Tout("", "\tThe status list is '%s'\n", s_to_c(st));

    /* split apart at the ;'s */
    while ((tok = s_tok(st, ";")) != 0)
	{
	switch (s_ptr_to_c(tok)[0])
	    {
	    case 'B': case 'b':
		B = tokdef(B, tok, "B");
		break;

	    case 'T': case 't':
		T = tokdef(T, tok, "T");
		break;

	    default:
		Tout(pn, "Unknown option list field: %s\n",
		    s_to_c(tok));
		s_free(tok);
		break;
	    }
	}

    if (presolved)
	*presolved = T ? atoi(s_to_c(T)+2) : -1;

    if (pbatchsize)
	*pbatchsize = B ? atoi(s_to_c(B)+2) : -1;

    s_free(T);
    s_free(B);
}

