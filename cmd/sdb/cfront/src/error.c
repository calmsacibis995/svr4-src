/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/error.c	1.1"
/*ident	"@(#)cfront:src/error.c	1.8" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


error.c :

	write error messages

	Until scan_started != 0 no context can be assumed

***************************************************************************/

#include "cfront.h"
#include "size.h"

int error_count;
static int no_of_warnings;
char scan_started;

#define ERRTRACE    20

static char* abbrev_tbl[] = {
	" argument",
	" base",
	" class",
	" declaration",
	" expression",
	" function",
	"G",
	"H",
	" initialize",
	"J",
	" constructor",	// 'K' !
	" list",
	" member",
	" name",
	" object",
	" pointer",
	" qualifie",
	" reference",
	" statement",
	" type",
	" undefined",
	" variable",
	" with",
	" expected", // 'X'
	"Y",
	"Z",
};

ea* ea0;

void error_init()
{
	static char errbuf[BUFSIZ];
	setbuf(stderr,errbuf);
	ea0 = new ea;
}

#define INTERNAL 127

void ext(int/* n*/ )
{
//	if (n == INTERNAL) abort();
	if (error_count==0) error_count = 1;
	exit(error_count);
}

static void print_loc()
{
	loc* sl = (Cstmt) ? &Cstmt->where : 0;
	loc* dl = (Cdcl) ? &Cdcl->where : 0;

	if (sl && dl && sl->file==dl->file)	// Cstmt and Cdcl in same file
		if (sl->line<=dl->line)
			dl->put(out_file);
		else
			sl->put(out_file);
	else if (sl && sl->file == curr_file)	// Cstmt in current file
		sl->put(out_file);
	else if (dl && dl->file== curr_file)	// Cdcl in current file
		dl->put(out_file);
	else
		curloc.put(out_file);
}

static void print_context()
{
	putc('\n',out_file);
}

static char in_error = 0;
loc dummy_loc;

void yyerror(char* s)
{
	error(0,&dummy_loc,s);
}

int error(const char* s, ea& a0, ea& a1, ea& a2, ea& a3)
{
	return error(0,&dummy_loc,s,a0,a1,a2,a3);
}

int error(loc* lc, const char* s, ea& a0, ea& a1, ea& a2, ea& a3)
{
	return error(0,lc,s,a0,a1,a2,a3);
}

int error(int t, const char* s, ea& a0, ea& a1, ea& a2, ea& a3)
{
	return error(t,&dummy_loc,s,a0,a1,a2,a3);
}

int suppress_error;

int error(int t, loc* lc, const char* s, ea& a0, ea& a1, ea& a2, ea& a3)
/*
	"int" not "void" because of "pch" in lex.c

	legal error types are:
		'w'		warning	 (not counted in error count)
		'd'		debug
		's'		"not implemented" message
    		0		error 
    		'i'		internal error (causes abort)
		't'		error while printing error message
*/
{
	if (suppress_error) return 0;

	if (in_error++)
		if (t!='t' || 4<in_error) {
			fprintf(stderr,"\nOops!, error while handling error\n");
			ext(13);
		}
	else if (t == 't')
		t = 'i';

	FILE * of = out_file;
	out_file = stderr;

	if (!scan_started || t=='t')
		putch('\n');
	else if (lc != &dummy_loc)
		lc->put(out_file);
	else
		print_loc();

	switch (t) {
    	case 0:
		putstring("error: ");
		break;
        case 'w':
		no_of_warnings++;
		putstring("warning: ");
		break;
        case 's':
		putstring("sorry, not implemented: ");
		break;
        case 'i':
		if (error_count++) {
			fprintf(out_file,"sorry, %s cannot recover from earlier errors\n",prog_name);
			ext(INTERNAL);
		}
		else
			fprintf(out_file,"internal %s error: ",prog_name);
        }

	ea argv[4];
	ea* a = argv;
	argv[0] = a0;
	argv[1] = a1;
	argv[2] = a2;
	argv[3] = a3;

	int c;

	while (c = *s++) {
		if ('A'<=c && c<='Z')
			putstring(abbrev_tbl[c-'A']);
		else if (c == '%') {
			switch (c = *s++) {
			case 'k':	// TOK assumed passed as an int
			{	TOK x = a->i;
				a++;
				if (0<x && x<=MAXTOK && keys[x])
					fprintf(out_file," %s",keys[x]);
				else
					fprintf(out_file," token(%d)",x);
				break;
			}
			case 't':	// Ptype 
			{	Ptype tt = Ptype(a->p);
				a++;
				if (tt) {
					TOK pm = emode;
					extern int ntok;
					int nt = ntok;
					emode = ERROR;
					putch(' ');
					tt->dcl_print(0);
					emode = pm;
					ntok = nt;
				}
				break;
			}
			case 'n':	// Pname
			{	Pname nn = Pname(a->p);
				a++;
				if (nn) {
					// suppress generated names:
					if (nn->string[0]=='_' && nn->string[1]=='C') break;
					TOK pm = emode;
					emode = ERROR;
					putch(' ');
					nn->print();
					emode = pm;
				}
				else
					putstring(" ?");
				break;
			}
			case 'p':	// pointer
			{	void* p = a->p;
				a++;
				char* f = sizeof(char*)==sizeof(int)?" %d":" %ld";
				fprintf(out_file,f,p);
				break;
			}
			case 'c':	// char assumed passed as an int
			{	char x = a->i;
				a++;
				putch(x);
				break;
			}
			case 'd':	// int
			{	int i = a->i;
				a++;
				fprintf(out_file," %d",i);
				break;
			}
			case 's':	// char*
			{	char* st = (char*)a->p;
				a++;
				putst(st);
				break;
			}
			}
		}
		else
			putch(c);
	}

	if (!scan_started) ext(4);

	switch (t) {
	case 'd':
	case 't':
	case 'w':
		putch('\n');
		break;
	default:
		print_context();
	}
	fflush(stderr);
    /* now we may want to carry on */

	out_file = of;

	switch (t) {
	case 't':
		if (--in_error) return 0;
	case 'i': 
		ext(INTERNAL);
	case 0:
	case 's':
		if (MAXERR<++error_count) {
			fprintf(stderr,"Sorry, too many errors\n");
			ext(7);
		}
	}

	in_error = 0;
	return 0;
}
