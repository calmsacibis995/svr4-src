/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:rt.c	1.1.6.1"
/*
 * This file contains code for the crash functions:  rtproc, rtdptbl
 */

#include <stdio.h>
#include <sys/types.h>
#include <a.out.h>
#include <sys/rt.h>
#include "crash.h"


struct syment  *Rtproc;	/* namelist symbol */
struct syment	*Rtdptbl, *Rtmaxpri;
rtdpent_t *rtdptbl;
struct rtproc rtbuf;
struct rtproc *rtp;

int
getrtproc()
{
	int c;
	char *rtprochdg = "PQUANT  TMLFT  PRI  FLAGS   PROCP   PSTATP    PPRIP   PFLAGP    NEXT     PREV\n";

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	if(!Rtproc)
		if(!(Rtproc = symsrch("rt_plisthead"))) 
			error("rt_plisthead not found in symbol table\n");

	readmem((long)Rtproc->n_value, 1, -1, (char *)&rtbuf,
		sizeof rtbuf, "rtproc table");

	fprintf(fp, "%s", rtprochdg);
	rtp = rtbuf.rt_next;

	for(; rtp != (rtproc_t *)Rtproc->n_value; rtp = rtbuf.rt_next) {
		readmem((long)rtp, 1, -1, (char *)&rtbuf,
			sizeof rtbuf, "rtproc table");
		prrtproc();
	}
}




/* print the real time process table */
int
prrtproc()
{


	fprintf(fp, "%4d    %4d %4d %5x %10x %8x %8x %8x %8x %8x\n",  rtbuf.rt_pquantum,
		rtbuf.rt_timeleft, rtbuf.rt_pri, rtbuf.rt_flags,
		rtbuf.rt_procp, rtbuf.rt_pstatp, rtbuf.rt_pprip,
		rtbuf.rt_pflagp, rtbuf.rt_next, rtbuf.rt_prev);
}


/* get arguments for rtdptbl function */

int
getrtdptbl()
{
	int slot = -1;
	int all = 0;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	long addr = -1;
	short rtmaxpri;

	char *rtdptblhdg = "SLOT     GLOBAL PRIORITY     TIME QUANTUM\n\n";

	if(!Rtdptbl)
		if(!(Rtdptbl=symsrch("rt_dptbl")))
			error("rt_dptbl not found in symbol table\n");

	if(!Rtmaxpri)
		if(!(Rtmaxpri=symsrch("rt_maxpri")))
			error("rt_maxpri not found in symbol table\n");

	optind=1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	readmem((long)Rtmaxpri->n_value, 1, -1, (char *)&rtmaxpri, sizeof(short),
		"rt_maxpri");

	rtdptbl = (rtdpent_t *)malloc((rtmaxpri + 1) * sizeof(rtdpent_t));

	readmem((long)Rtdptbl->n_value, 1, -1, (char *)rtdptbl,
	    (rtmaxpri + 1) * sizeof(rtdpent_t), "rt_dptbl");

	fprintf(fp,"%s",rtdptblhdg);


	if(args[optind]) {
		all = 1;
		do {
			getargs(rtmaxpri + 1,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prrtdptbl(slot);
			else {
				if(arg1 < rtmaxpri + 1)
					slot = arg1;
				prrtdptbl(slot);
			}
			slot = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else 
		for(slot = 0; slot < rtmaxpri + 1; slot++)
			prrtdptbl(slot);

	free(rtdptbl);
}

/* print the real time dispatcher parameter table */
int
prrtdptbl(slot)
int  slot;
{
	fprintf(fp, "%3d           %4d           %10ld\n",
	    slot, rtdptbl[slot].rt_globpri, rtdptbl[slot].rt_quantum);	
}
