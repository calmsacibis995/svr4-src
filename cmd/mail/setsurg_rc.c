/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:setsurg_rc.c	1.8.3.1"
#include "mail.h"

/*
    NAME
	setsurg_rc - interpret the option list for surrogate entries

    SYNOPSIS
	char *setsurg_rc(string *st, int defreal, int *pbatchsize)

    DESCRIPTION
	The string st is of the form
		S=n;C=n;F=n;B=n;
	where the options can be in any order. Each "n" is of the form
		errorcode,errorcode,...
	Each errorcode is of the form
		number
	or
		number-number
	or
		*

	If st is null, then it defaults to "S=0;C=*".

	Each list is a list of status codes which are to be considered
	as a Success, Continuation or Failure. The "*" means "all", and
	an explicit entry overrides the "*". The precedence is Success,
	Continue and Failure.

	Defreal is either REAL or DEFAULT, determining whether a static
	default array is to be set, or whether memory is to be allocated
	and returned.

	The value for B= is returned in pbatchsize and will be set to -1
	if a B= entry is not found.

    RETURNS
	A pointer to the buffer used is returned.
*/

/* Look for range n-m, single number, or '*' */
static void setrange(ret, st, setting)
char *ret;
string *st;
int setting;
{
    char *pn = "setrange";
    int r1;

    /* check for '*' first */
    s_restart(st);
    Dout(pn, 18, "\tlooking at '%s'\n", s_ptr_to_c(st));
    if (s_ptr_to_c(st)[0] == '*')
	{
	Dout(pn, 18, "\tfound *\n");
	return;
	}

    r1 = atoi(s_ptr_to_c(st));

    /* Check for dash in range */
    if (strchr(s_ptr_to_c(st), '-') != 0)
	{
	/* split apart at dash */
	int r2;
	s_free(s_tok(st, "-"));
	r2 = atoi(s_ptr_to_c(st));

	Dout(pn, 18, "\trange: %d-%d\n", r1, r2);
	if (r2 < r1)
	    Tout(pn, "Invalid status range %d-%d\n", r1, r2);

	else
	    {
	    /* Check boundaries and set the values */
	    if (r1 < 0) r1 = 0;
	    if (r2 > 255) r2 = 255;
	    memset(ret+r1, setting, (r2 - r1 + 1));
	    }
	}

    /* Single number. If it's in the range, use it. */
    else
	{
	Dout(pn, 18, "\tsingle number: %d\n", r1);
	if ((r1 >= 0) && (r1 < 256))
	    ret[r1] = setting;
	}
}

/* Split a status up on the commas. */
static void setlist(ret, st, setting)
char *ret;
string *st;
int setting;
{
    char *pn = "setlist";
    string *tok;

    if (!st)
	return;
    Dout(pn, 18, "\tlooking at '%s'\n", s_ptr_to_c(st));

    /* skip past the X= */
    s_restart(st);
    s_skipc(st);
    s_skipc(st);
    Dout(pn, 18, "\tnow looking at '%s'\n", s_ptr_to_c(st));

    /* split apart at the ,'s */
    for ( ; ((tok = s_tok(st, ",")) != 0); s_free(tok))
	setrange(ret, tok, setting);
}

static void dumpstatlist(s)
char *s;
{
    register int i, iend = 256;
#define NPERLINE 70
    fprintf(dbgfp, "The status list is:\n");
    for (i = 0; i < iend; i += NPERLINE)
	{
	register int j, jend = ((i+NPERLINE) >= iend) ? iend : (i+NPERLINE);
	for (j = i; j < jend; j += 10)
	    {
	    register int k, kend = ((j+10) >= iend) ? iend : (j+10);
	    (void) fprintf(dbgfp, "'%3d .", j);
	    for (k = j+6; k < kend; k++)
		putc(' ', dbgfp);
	    }
	putc('\n', dbgfp);
	for (j = i; j < jend; j++)
	    putc((s[j] == SUCCESS) ? 's':
		    (s[j] == CONTINUE) ? 'c':
					'f', dbgfp);
	putc('\n', dbgfp);
	putc('\n', dbgfp);
	}
}

char *setsurg_rc(st, realdef, pbatchsize)
string *st;
int realdef;
int *pbatchsize;
{
    char *pn = "setsurg_rc";
    string *B = 0;
    static char defrc[256];
    char *ret = realdef ? malloc(sizeof(char) * 256) : defrc;

    Tout(pn, "Looking at status list for the %s list\n", (realdef ? "real" : "default"));

    if (!ret)
	{
	Tout(pn, "no space!");
	return 0;
	}

    if (st)
	{
	string *F = 0, *S = 0, *C = 0, *tok;

	/* split off the S=;F=;C=;B=; option list */
	Tout("", "\tThe status list is '%s'\n", s_to_c(st));

	/* split apart at the ;'s */
	while ((tok = s_tok(st, ";")) != 0)
	    {
	    switch (s_ptr_to_c(tok)[0])
		{
		case 'F': case 'f':
		    F = tokdef(F, tok, "F");
		    break;

		case 'S': case 's':
		    S = tokdef(S, tok, "S");
		    break;

		case 'C': case 'c':
		    C = tokdef(C, tok, "C");
		    break;

		case 'B': case 'b':
		    B = tokdef(B, tok, "B");
		    break;

		default:
		    Tout(pn, "Unknown status list field: %s\n",
			s_to_c(tok));
		    s_free(tok);
		    break;
		}
	    }

	/* Copy in the default. Use any *'s as a new */
	/* default, or else use the default array. */
	if (S && (strchr(s_to_c(S), '*') != 0))
	    {
	    memset(ret, SUCCESS, 256);
	    Dout(pn, 15, "\tS has *, setting default to SUCCESS\n");
	    }
	else if (C && (strchr(s_to_c(C), '*') != 0))
	    {
	    memset(ret, CONTINUE, 256);
	    Dout(pn, 15, "\tC has *, setting default to CONTINUE\n");
	    }
	else if (F && (strchr(s_to_c(F), '*') != 0))
	    {
	    memset(ret, FAILURE, 256);
	    Dout(pn, 15, "\tF has *, setting default to FAILURE\n");
	    }
	else if (ret != defrc)
	    {
	    memcpy(ret, defrc, 256);
	    Dout(pn, 15, "\tno *'s, using default settings\n");
	    }

	/* Make the explicit assignments. */
	setlist(ret, F, FAILURE);
	setlist(ret, C, CONTINUE);
	setlist(ret, S, SUCCESS);
	s_free(F);
	s_free(C);
	s_free(S);
	}

    /* No string passed in. Use "S=0;C=*;". */
    else
	{
	Tout("", "\tThe status list defaults to 'S=0;C=*'\n");
	ret[0] = SUCCESS;
	memset(&ret[1], CONTINUE, 255);
	}

    if (debug > 15)
	dumpstatlist(ret);

    /* Now look at the B= info. */
    if (pbatchsize)
	*pbatchsize = B ? atoi(s_to_c(B) + 2) : -1;
    s_free(B);

    return ret;
}
