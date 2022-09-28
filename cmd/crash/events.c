/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:events.c	1.1.4.1"
/*
* This file contains code for the crash functions: evactive, evmm
*/

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>
#include <sys/priocntl.h>
#include <sys/procset.h>
#include <sys/events.h>
#include <sys/evsyscall.h>
#include <sys/vnode.h>
#include <sys/cred.h>
#include <sys/evsys.h>
#include "crash.h"

struct syment *Evactive, *Evcinfo, *Evmm;
struct evactq evactivebuf;
struct evcinfo evcbuf;
struct evmminfo evmmbuf;
char *evhdg = "	NAME		UID	GID	MODE	OPENS	NEVENTS	MAXEV	MAXDPE\n\n";

int
getevactive()
{
	int c;
	int evqueue = 0;
	int full = 0;
	char ename[EV_NMSZ + 1];

	if(!Evactive)
		if(!(Evactive = symsrch("ev_actqp")))
			error("ev_actqp not found in symbol table\n");

	if(!Evcinfo)
		if(!(Evcinfo = symsrch("evcinfo")))
			error("evcinfo not found in symbol table\n");

	readmem ((long)Evcinfo->n_value, 1, -1, (char *)&evcbuf,
		sizeof evcbuf, "events global limits");

	optind=1;
	while((c=getopt(argcnt,args,"fw:")) != EOF){
		switch(c) {
			case 'f':	full=1;
					break;
			case 'w':	redirect();
					break;
			default :	longjmp(syn,0);
		}
	}

	if(args[optind]) {
		strcpy(ename, args[optind]);
		evqueue=1;
	}


	if(!full)
		fprintf(fp, "%s", evhdg);
	prevactive(evqueue, ename, full);
}


int
prevactive(evqueue,ename,full)
int evqueue,full;
char *ename;
{

	uint i;
	struct evqueue evbuf;
	struct evactq *evptr;

	readmem((long)Evactive->n_value, 1, -1, (char *)&evptr,
		sizeof (struct evactq *), "active queue pointer");
	for(i=0; i < evcbuf.evci_mevqueues; i++, evptr++) {
		readmem((long)evptr, 1, -1, (char *)&evactivebuf,
			sizeof evactivebuf, "active event queue");
		if(evactivebuf.eaq_qp == NULL)
			continue;
		readmem((long)evactivebuf.eaq_qp, 1, -1, (char *)&evbuf,
			sizeof evbuf, "event queue");
		if(evqueue){
			if((strcmp(evbuf.evq_name, ename)== 0)){
				prevname(evbuf,full);
				break;
			}
		} else
			prevname(evbuf,full);
	}
}

int
prevname(evbuf,full)
struct evqueue evbuf;
int full;
{

	if(full)
		fprintf(fp, "%s", evhdg);
	fprintf(fp, "%-16s	%d	%d	%x	%d	%d	%d	%d\n",
		evbuf.evq_name,
		evbuf.evq_uid,
		evbuf.evq_gid,
		evbuf.evq_mode,
		evbuf.evq_opencnt,
		evbuf.evq_nevents,
		evbuf.evq_maxev,
		evbuf.evq_maxdpe);
	if(!full)
		return;

	fprintf(fp,"\nnext: %x,  prev: %x\n", evbuf.evq_next, evbuf.evq_prev);
	fprintf(fp,"events(first): %x,  events(last): %x\n",
		evbuf.evq_events.lh_first, evbuf.evq_events.lh_last);
	fprintf(fp, "exrefs(first): %x,  exrefs(last):  %x\n",
		evbuf.evq_exrefs.lh_first, evbuf.evq_exrefs.lh_last);
	fprintf(fp, "memsize:  %d,  shmsize:  %d\n", 
		evbuf.evq_memsize, evbuf.evq_shmsize);
	fprintf(fp, "closemd:  %u, wspace:  %d,  wevent:  %d\n",
		evbuf.evq_closemd, evbuf.evq_wspace, evbuf.evq_wevent);
	fprintf(fp,"locked:  %d,  wanted:  %d\n", evbuf.evq_locked, evbuf.evq_wanted);
	fprintf(fp, "maxmem:  %d, vnode:  %x,  aqp:  %x\n",
		evbuf.evq_maxmem, evbuf.evq_vnode, evbuf.evq_aqp);
	fprintf(fp,"atime:  %d,  mtime:  %d,  ctime:  %d\n\n\n",
		evbuf.evq_atime, evbuf.evq_mtime, evbuf.evq_ctime);
}

/* get arguments for evmm function */
int
getevmm()
{

	int c, i;
	struct evmminfo *mmptr;
	char *evmmhdg = "	NAME      SIZE   WANT   NALL   MAX	 FP           LP           FRP\n\n";

	if(!Evmm)
		if(!(Evmm = symsrch("ev_mminfo")))
			error("ev_mminfo not found in symbol table\n");

	while((c=getopt(argcnt, args,"w:")) != EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	mmptr=(struct evmminfo *)Evmm->n_value;
	
	fprintf(fp, "%s", evmmhdg);
	for(i=0; i < EV_MT_NBR; i++, mmptr++) {
		readmem((long)mmptr, 1, -1, (char *)&evmmbuf,
			sizeof evmmbuf, "events memory management");
		prevmm();
	}
}

int
prevmm()
{



	fprintf(fp, "%-16s %4d    %4d  %4d  %6d %10x   %10x   %10x\n",
		evmmbuf.emmi_name,
		evmmbuf.emmi_size,
		evmmbuf.emmi_wanted,
		evmmbuf.emmi_nalloced,
		evmmbuf.emmi_maxalloc,
		evmmbuf.emmi_firstp,
		evmmbuf.emmi_lastp,
		evmmbuf.emmi_freep);
}
