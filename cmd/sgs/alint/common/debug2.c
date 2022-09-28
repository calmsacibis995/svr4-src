/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/debug2.c	1.5"
#ifndef NODBG
#include <stdio.h>
#include <varargs.h>
#include "p1.h"
#include "lnstuff.h"
#include "lpass2.h"
#include "tables2.h"
#include "xl_byte.h"

/* 
** diagnostic output tools
**	the following programs are used to print out internal information
**	for diagnostic purposes
**	Current options:
**	-2i
**		turns on printing of intermediate file on entry to lint2
**			(input information)
**
** GLOBAL ENTRY POINTS:
**	pif()
**	dprintf()
*/

static void ptb();
static void tprint();

struct tb { 
	int m; 
	char * nm; 
};

static struct tb dfs[] = {
	LDI, "LDI",
	LIB, "LIB",
	LDC, "LDC",
	LDX, "LDX",
	LRV, "LRV",
	LUV, "LUV",
	LUE, "LUE",
	LUM, "LUM",
	LDS, "LDS",
	LFN, "LFN",
	LSU, "LSU",
	LPR, "LPR",
	LND, "LND",
	LPF, "LPF",
	LSF, "LSF",
	0, "" };

/* 
** ptb - print a value from the table 
*/
static void
ptb(v, tp) 
struct tb *tp; 
{
    int flag = 0;

    for ( ; tp->m; ++tp)
	if (v&tp->m) {
	    if (flag++) 
		fputc('|',stderr);
	    fprintf(stderr,"%s", tp->nm);
	}
}


/* 
**  pif - print intermediate file
**	prints file written out by first pass of lint
**  printing turned on by the debug option -Xi
**/
void
pif()
{
    int start, pass;
    FLENS lout;
    fprintf(stderr,"\n\tintermediate file printout:\n");
    fprintf(stderr,"\t===========================\n");
    start = 1;
    pass = 1;
    for (;;) {
	if (start) {
	    curpos = ftell(stdin);
	    if (fread((char *)&lout, sizeof(FLENS), 1, stdin) != 1) {
		rewind(stdin);
		return;
	    }
	    lout = *(xl_t_flens(&lout));
	    fprintf(stderr,"\n\nDefinitions:\n\n");
	    start = 0;
	}

	if (fread((char *)&r, sizeof(r), 1, stdin) < 1)
	    return;

	r = *(xl_t_rec(&r, XL_LINE|XL_TY));
	if (r.l.decflag & LND) {
	    fprintf(stderr,"\nEnd Pass %d\n", pass);
	    switch (pass) {
		case 1:
		    pass=2;
		    fprintf(stderr,"\n\nTentative Definitions:\n\n");
		    continue;
		case 2:
		    pass=3;
		    fprintf(stderr,"\n\nUses:\n\n");
		    continue;
		case 3:
		    start = 1;
		    pass = 1;
		    (void)fseek(stdin, curpos+lout.f1+lout.f2+lout.f3+lout.f4,0);
		    continue;
	    }
	}
		    
	if (r.l.decflag & LFN) {
	    r.f.fn = getstr();
	    fprintf(stderr,"\nFILE NAME: %-15s\n", r.f.fn);
	} else {
	    curname = getstr();
#if 0
	    r.l.name = getstr();
#endif
	    fprintf(stderr,"\t%s  (", curname);
	    ptb(r.l.decflag, dfs);
	    fprintf(stderr,")\t line= %d", r.l.fline);
	    if (LN_ISFTN(r.l.type.dcl_mod) || (r.l.decflag&LSU)) 
		fprintf(stderr,"\tnargs= %d", r.l.nargs);
	    else fprintf(stderr,"\t\t\t");
	    fprintf(stderr,"\t type= ");
	    tprint(r.l.type);
	    fprintf(stderr,"\n");
	    if (r.l.nargs) {
		int n = 0;
		ATYPE *arg;
		if (r.l.nargs < 0) 
		    r.l.nargs = -r.l.nargs - 1;
		if (r.l.nargs > args_alloc)
		    arg_expand(r.l.nargs);
		if (r.l.decflag & LSU) {
		    for (n=0;n<r.l.nargs;n++) {
			arg = &getarg(n);
			fread((char *) arg, sizeof(ATYPE),1,stdin);
			*arg = *(xl_t_atype(arg, XL_TY));
			argname(n) = getstr();
		    }
		} else if (r.l.nargs) {
		    int i;
		    arg = &getarg(0);
		    fread((char *)arg,sizeof(ATYPE),r.l.nargs,stdin);
		    for (i=r.l.nargs; i!=0; --i, ++arg)
		        *arg = *(xl_t_atype(arg, XL_TY));
		}
		n = 0;
		while (++n <= r.l.nargs) {
		    fprintf(stderr,"\t\targ(%d)= ",n);
		    tprint(getarg(n-1));
		    if (r.l.decflag & LSU)
			fprintf(stderr,"\t\tname: %s\n",argname(n-1));
		    fprintf(stderr,"\t\textra: %ld\n",getarg(n-1).extra.ty);
		    fprintf(stderr,"\n");
		}
	    }
	}
    } /* for(;;) */
} /*end pif*/



static void
tprint(t)
ATYPE t; 
{
    char str[MAXPRNT];
    ptype(str, t, cmno);
    fprintf(stderr,"%s\n", str);
}



/*VARARGS0*/
void
dprintf(va_alist)
va_dcl
{
    va_list args;
    char * fmt;

    va_start(args);

    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    return;
}
#endif
