/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass1.c	1.14"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <paths.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"
#include "section.h"

/*
 *
 *	"pass1.c" is a file containing the main routine for the first
 *	pass of the assembler.
 *
 *	The following things are done by this function:
 *
 *	1. Initialization. This consists of calling signal to catch
 *	   interrupt signals for hang-up, break, and terminate. Then
 *	   the argument list is processed by "getargs" followed by the
 *	   initialization of the symbol table with mnemonics for
 *	   instructions and pseudos-ops.
 *
 *	2. "yyparse" is called to do the actual first pass processing.
 *	   This is followed by a call to "cgsect(1)". Normally this
 *	   function is used to change the section into which code
 *	   is generated. In this case, it is only called to make
 *	   sure that the section program counters are up to date.
 *
 *	3. The section name symbols are then defined. Each section
 *	   name symbol has the type of the section and a value of zero.
 *	   The symbol is used to label the beginning of the section
 *	   and later as a reference for relocation entries that are
 *	   relative to the section.
 *
 *
 */

int txtsec = -1;
int bsssec = -1;
extern short passnbr;

extern char *filenames[];

extern short anyerrs;

extern void aerror();
extern void cgsect();
extern void strtabinit();
extern void creasyms();
extern void yyparse();
extern void correctallsets();
extern void correctallsizes();
extern void sectabinit();
extern void initdot();

extern int mksect();
#if M32
extern char	dbg;
#endif

extern void delexit();
extern void fixsyms();
extern unsigned short	line;
extern void	mk_hashtab();
extern void onintr();
extern symbol *lookup();

extern void dovstamp();
extern short vstamp;
extern FILE *fderr;


#if M32RSTFIX
short rstflag = YES;
#endif


FILE	*fdin;

#if DEBUG
FILE	*perfile;
#endif

#if DEBUG
/*
 *	Performance data structure
 */
	long	ttime;
	struct	tbuffer {
		long	proc_user_time;
		long	proc_system_time;
		long	child_user_time;
		long	child_system_time;
		} ptimes;
	extern	long	times();

#endif

void
aspass1()
{

#if i386
	line = 1;
#else
	line = 0;
#endif
	passnbr = 1;
	if (signal(SIGHUP,SIG_IGN) == SIG_DFL)
		(void) signal(SIGHUP,onintr);
	if (signal(SIGINT,SIG_IGN) == SIG_DFL)
		(void) signal(SIGINT,onintr);
	if (signal(SIGTERM,SIG_IGN) == SIG_DFL)
		(void) signal(SIGTERM,onintr);
	fderr = stderr;

#if DEBUG
	ttime = times(&ptimes);		/* Performance data collected */
#endif

#if M32
	if (dbg)
		fdin = stdin;
	else 
#endif
		if ((fdin = fopen(filenames[0], "r")) == NULL)
			aerror("Unable to open input file");
	strtabinit();
	creasyms();
	sectabinit();
	initdot();
	mk_hashtab();
	txtsec =(mksect(lookup(_TEXT, INSTALL), (SHF_ALLOC | SHF_EXECINSTR),
		(long) SHT_PROGBITS));	/* first scn */
	cgsect(txtsec);
	yyparse();		/* pass 1 */
	(void) fclose(fdin);
	if (vstamp)
		dovstamp();

	fixsyms();		/* span dependent optimization */
	correctallsets();	/* fix any symbol table entries left
				unfinished by ".set" directive processing. */
	correctallsizes();

#if DEBUG
/*
 *	Performance data collected and written out here
 */
	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		(void) fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			(void) fprintf(perfile,
			   "as1\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 1\n",
			    ttime, ptimes);
			(void) fclose(perfile);
		}
	}
#endif

	if (anyerrs)
		delexit();
	return;
}
