/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:disp.c	1.1.5.1"
/*
 * This file contains code for the crash functions:  dispq
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/var.h>
#include <sys/priocntl.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include "crash.h"


struct syment	*Dispq;		/* namelist symbol */
dispq_t		*dispqbuf;

/* get arguments for dispq function */
int
getdispq()
{
	int slot = -1;
	int c, i;
	long arg1 = -1;
	long arg2 = -1;
	dispq_t	*dispqaddr;

	char *dispqhdg = "SLOT     DQ_FIRST     DQ_LAST     RUNNABLE COUNT\n\n";

	if(!Dispq)
		if(!(Dispq = symsrch("dispq")))
			error("dispq not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}


	/* Allocate enough space to read in the whole table at once */

	dispqbuf = (dispq_t *)malloc(vbuf.v_nglobpris * sizeof(dispq_t));

	/* Read the dispq table address */

	readmem((long)Dispq->n_value, 1, -1, (char *)&dispqaddr, 
		sizeof(dispq_t *), "dispq address");

	/* Read in the entire table of dispq headers */

	readmem((long)dispqaddr, 1, -1, (char *)dispqbuf,
	    vbuf.v_nglobpris * sizeof(dispq_t), "dispq header table");

	fprintf(fp, "%s", dispqhdg);

	if(args[optind]){
		do {
			getargs(vbuf.v_nglobpris,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prdispq(slot);
			else {
				if(arg1 < vbuf.v_nglobpris)
					slot = arg1;
				prdispq(slot);
			}
			slot = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_nglobpris; slot++)
		prdispq(slot);

	free(dispqbuf);
}

/* print dispq header table  */
int
prdispq(slot)
int slot;
{
	fprintf(fp, "%4d     %8x    %8x          %4d\n",
		slot, dispqbuf[slot].dq_first,
		dispqbuf[slot].dq_last, dispqbuf[slot].dq_sruncnt);
}
