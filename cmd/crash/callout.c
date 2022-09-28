/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:callout.c	1.8.4.1"

/*
 * This file contains code for the crash function callout.
 */

#include <stdio.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/callo.h>
#include "crash.h"


struct syment *Callout;			/* namelist symbol pointer */
extern char *strtbl;			/* pointer to string table */
extern short N_TEXT;			/* used in symbol table search */

/* get arguments for callout function */
int
getcallout()
{
	int c;

	if(!Callout)
		if(!(Callout = symsrch("callout")))
			error("callout not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"FUNCTION        ARGUMENT    TIME     ID\n");
	if(args[optind]) 
		longjmp(syn,0);
	else prcallout();
}

/* print callout table */
int
prcallout()
{
	struct syment *sp;
	extern struct syment *findsym();
	struct callo callbuf;
	static char tname[SYMNMLEN+1];
	ulong addr;
	char *name;

	addr = (ulong)Callout->n_value;
	for(;;) {
		readmem(addr,1,-1,&callbuf,sizeof callbuf,"callout table");
		addr += sizeof(callbuf);
		if(!callbuf.c_func)	
			return;
		if(!(sp = findsym((unsigned long)callbuf.c_func)))
			error("%8x does not match in symbol table\n",
				callbuf.c_func);
		if(sp->n_zeroes){
			strncpy(tname,sp->n_name,SYMNMLEN);
			name = tname;
		}
		else name = strtbl + sp->n_offset;
		fprintf(fp,"%-15s",name);
		fprintf(fp," %08lx  %5u  %5u\n", 
			callbuf.c_arg,
			callbuf.c_time,
			callbuf.c_id);
	}
}

