/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:page.c	1.10.11.1"

/*
 * This file contains code for the crash functions: page, as, and ptbl.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/tss.h>
#include <sys/immu.h>
#include <sys/vnode.h>
#include <vm/vm_hat.h>
#include <vm/hat.h>
#include <vm/seg.h>
#include <vm/as.h>
#include <vm/page.h>
#include <sys/user.h>
#include <sys/var.h>
#include <sys/proc.h>
#include "crash.h"

/* namelist symbol pointers */
extern struct syment *V;		/* ptr to var structure */
static struct syment *Pageahead;	/* ptr to page accounting table */
static struct syment *Pagepoolsize;	/* ptr to page pool size */

static struct pageac *pageahead;
static u_long pagepoolsize;

extern u_long vtop();
extern pte_t *kpd_start;

void prsegs();

#define MAX_PFN	0xFFFFF


static void
load_pageac()
{
	u_long	pap_addr;
	struct pageac	**ppap = &pageahead;
	struct pageac	*pap;

	if(!(Pageahead = symsrch("pageahead")))
		error("pageahead not found in symbol table\n");
	readmem((long)Pageahead->n_value, 1, -1,
		(char *)&pap_addr, sizeof pageahead,
		"pageahead: page pool accounting table");
	while (pap_addr != 0) {
		pap = (struct pageac *)malloc(sizeof(struct pageac));
		readmem((long)pap_addr, 1, -1, (char *)pap,
			sizeof(struct pageac),
			"page pool accounting structure");
		*ppap = pap;
		pap_addr = (u_long)*(ppap = &pap->panext);
	}
}

u_int
page_pptonum(pp)
	page_t	*pp;
{
	struct pageac	*pap;

	if(!Pageahead)
		load_pageac();

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pp >= pap->firstpp && pp < pap->endpp)
			return(pap->firstpfn + (pp - pap->firstpp)
						* (PAGESIZE/MMU_PAGESIZE));
	}

	return((u_int)-1);
}

u_long
page_numtopp(pfn)
	u_int	pfn;
{
	struct pageac	*pap;

	if(!Pageahead)
		load_pageac();

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pfn >= pap->firstpfn && pfn < pap->endpfn)
			return (u_long)(&pap->firstpp[(pfn - pap->firstpfn)
						/ (PAGESIZE/MMU_PAGESIZE)]);
	}

	return 0;
}

/* get arguments for page function */
int
getpage()
{
	u_int pfn = (u_int)-1;
	u_int all = 0;
	u_int phys = 0;
	u_long addr = -1;
	u_long arg1 = -1;
	u_long arg2 = -1;
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}

	if(!Pagepoolsize) {
		if(!(Pagepoolsize = symsrch("pagepoolsize")))
			error("pagepoolsize not found in symbol table\n");
		readmem((long)Pagepoolsize->n_value, 1, -1,
			(char *)&pagepoolsize, sizeof pagepoolsize,
			"pagepoolsize: page pool accounting table");
	}

	fprintf(fp,"PAGE POOL SIZE: %d\n\n", pagepoolsize);
	fprintf(fp,"       PFN  KEEP       VNODE        HASH        PREV      VPPREV  FLAGS\n");
	fprintf(fp,"        PP   NIO      OFFSET                    NEXT      VPNEXT\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs((int)MAX_PFN+1,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(pfn = arg1; pfn <= (u_int)arg2; pfn++)
					prpage(all,pfn,phys,addr);
			else {
				if(arg1 <= MAX_PFN)
					pfn = arg1;
				else
					addr = arg1;
				prpage(all,pfn,phys,addr);
			}
			pfn = addr = arg1 = arg2 = -1;
		} while(args[++optind]);
	} else
		for(pfn = 0; pfn <= MAX_PFN; pfn++)
			prpage(all,pfn,phys,addr);
}

/* print page structure table */
int
prpage(all,pfn,phys,addr)
u_int all,pfn,phys;
u_long addr;
{
	struct page pagebuf;
	u_int next, prev;
	u_int vpnext, vpprev;
	u_int hash;

	if (!Virtmode)
		phys = 1;
	if (addr == (u_long)-1) {
		if ((addr = page_numtopp(pfn)) == 0)
			return;
		phys = 0;
	} else if (pfn == (u_int)-1 && !phys)
		pfn = page_pptonum((page_t *)addr);

	readmem(addr,!phys,-1,
		(char *)&pagebuf,sizeof pagebuf,"page structure table");

	/* check page flags */
	if ((*((ushort *)&pagebuf) == 0) && !all)
		return;

	if (pfn == (u_int)-1)
		fprintf(fp,"         -");
	else
		fprintf(fp,"     %5x",pfn);

	fprintf(fp,"  %4d  0x%08x  ",
		pagebuf.p_keepcnt,
		pagebuf.p_vnode);

	/* calculate page structure entry number of pointers */

	hash = page_pptonum(pagebuf.p_hash);
	if (hash == (u_int)-1)
		fprintf(fp,"0x%08x  ", pagebuf.p_hash);
	else
		fprintf(fp,"     %5x  ",hash);

	prev = page_pptonum(pagebuf.p_prev);
	if (prev == (u_int)-1)
		fprintf(fp,"0x%08x  ", pagebuf.p_prev);
	else
		fprintf(fp,"     %5x  ",prev);

	vpprev = page_pptonum(pagebuf.p_vpprev);
	if (vpprev == (u_int)-1)
		fprintf(fp,"0x%08x  ", pagebuf.p_vpprev);
	else
		fprintf(fp,"     %5x  ",vpprev);

	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s\n",
		pagebuf.p_lock    ? "lock "    : "",
		pagebuf.p_want    ? "want "    : "",
		pagebuf.p_free    ? "free "    : "",
		pagebuf.p_intrans ? "intrans " : "",
		pagebuf.p_gone    ? "gone "    : "",
		pagebuf.p_mod     ? "mod "     : "",
		pagebuf.p_ref     ? "ref "     : "",
		pagebuf.p_pagein  ? "pagein "  : "",
		pagebuf.p_nc      ? "nc "      : "",
		pagebuf.p_age     ? "age "     : "");

	/* second line */

	fprintf(fp,"0x%08x  %4d    %8d              ",
		addr,
		pagebuf.p_nio,
		pagebuf.p_offset);

	next = page_pptonum(pagebuf.p_next);
	if (next == (u_int)-1)
		fprintf(fp,"0x%08x  ", pagebuf.p_next);
	else
		fprintf(fp,"     %5x  ",next);

	vpnext = page_pptonum(pagebuf.p_vpnext);
	if (vpnext == (u_int)-1)
		fprintf(fp,"0x%08x  \n", pagebuf.p_vpnext);
	else
		fprintf(fp,"     %5x  \n",vpnext);
}

/* get arguments for ptbl function */
int
getptbl()
{
	int proc = Procslot;
	int all = 0;
	int phys = 0;
	u_long addr = -1;
	int c;
	struct proc prbuf;
	struct as asbuf;
	int count = 1;

	optind = 1;
	while((c = getopt(argcnt,args,"epw:s:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 's' :	proc = setproc();
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	procntry(proc,&prbuf);
	readmem((long)prbuf.p_as, 1, -1, (char *)&asbuf, sizeof asbuf, "as");
	if (args[optind]) {
		if ((addr = (u_long)strcon(args[optind++],'h')) == (u_long)-1)
			error("\n");
		if (args[optind]) 
			if ((count = strcon(args[optind],'d')) == -1)
				error("\n");
		prptbl(all,phys,addr,count,(u_long)-1,proc);
	}
	else
		prptbls(all,proc,prbuf.p_as,&asbuf.a_hat);
}

/* print all of a proc's page tables */
int
prptbls(all,proc,as,hatp)
int all;
u_int proc;
struct as *as;
struct hat *hatp;
{
	u_long	ptapaddr;
	hatpt_t	ptapbuf;
	u_long	pt_addr, base;

	fprintf(fp, "Page Tables for Process %d\n", proc);

	if ((ptapaddr = (u_long)hatp->hat_pts) == 0)
		return;

	do {
		readmem(ptapaddr, 1, proc, (char *)&ptapbuf, sizeof(ptapbuf),
			"hatpt structure");

		fprintf(fp,
		"\nHATPT 0x%08x: virt 0x%08x pde 0x%08x aec %d locks %d\n\n",
			ptapaddr,
			base = (ptapbuf.hatpt_pdtep - kpd_start) << PTNUMSHFT,
			ptapbuf.hatpt_pde.pg_pte,
			ptapbuf.hatpt_aec,
			ptapbuf.hatpt_locks);

		if (ptapbuf.hatpt_as != as) {
			fprintf(fp, "WARNING - hatpt was not pointing to the correct as struct: 0x%8x\n",
				ptapbuf.hatpt_as);
			fprintf(fp, "          hatpt list traversal aborted.\n");
			break;
		}

		/* locate page table */
		pt_addr = pfntophys(ptapbuf.hatpt_pde.pgm.pg_pfn);
		prptbl(all, 1, pt_addr, NPGPT, base, proc);
	} while ((ptapaddr = (u_long)ptapbuf.hatpt_forw) != (u_long)hatp->hat_pts);
}

/* print page table */
int
prptbl(all,phys,addr,count,base,proc)
int all,phys;
u_int count,proc;
u_long addr,base;
{
	pte_t	ptebuf;
	u_int	i;

	if (count > NPGPT)
		count = NPGPT;

	if (base != (u_long)-1)
		fprintf(fp, "SLOT     VADDR    PFN   TAG   FLAGS\n");
	else
		fprintf(fp, "SLOT    PFN   TAG   FLAGS\n");

	seekmem(addr, (!phys && Virtmode), proc);
	for (i = 0; i < count; i++) {
		if (read(mem, (char *)&ptebuf, sizeof ptebuf) != sizeof ptebuf) 
			error("read error on page table\n");
		if (ptebuf.pgm.pg_pfn == 0 && !all)
			continue;
		fprintf(fp, "%4u", i);
		if (base != (u_long)-1)
			fprintf(fp, "  %08x", base + pfntophys(i));
		fprintf(fp, " %6x   %3x   %s%s%s%s%s\n",
			ptebuf.pgm.pg_pfn,
			ptebuf.pgm.pg_tag,
			ptebuf.pgm.pg_ref   ? "ref "   : "",	
			ptebuf.pgm.pg_rw    ? "w "     : "",	
			ptebuf.pgm.pg_us    ? "us "    : "",	
			ptebuf.pgm.pg_mod   ? "mod "   : "",	
			ptebuf.pgm.pg_v     ? "v "     : "");	
	}
}

/* get arguments for as function */
int
getas()
{
	struct var varbuf;
	int slot = -1;
	int proc = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	u_long addr = -1;
	u_long arg1 = -1;
	u_long arg2 = -1;
	int c;
	char *heading = "PROC  KEEP        SEGS    SEGLAST  MEM_CLAIM     HAT_PTS HAT_PTLAST    FLAGS\n";

	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}

	if (!full)
		fprintf(fp,"%s",heading);

	if(args[optind]) {
		do {
			getargs(vbuf.v_proc,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(proc = arg1; proc <= arg2; proc++)
					pras(all,full,proc,phys,addr,heading);
			else
				pras(all,full,arg1,phys,addr,heading);
			proc = arg1 = arg2 = -1;
		} while(args[++optind]);
	} else if (all) {
		readmem((long)V->n_value,1,-1,(char *)&vbuf,
			sizeof vbuf,"var structure");
		for(proc = 0; proc < vbuf.v_proc; proc++) 
			pras(all,full,proc,phys,addr,heading);
	} else
		pras(all,full,proc,phys,addr,heading);
}


/* print address space structure */
int
pras(all,full,slot,phys,addr,heading)
int all,full,slot,phys;
char *heading;
{
	struct proc prbuf, *procaddr;
	struct as asbuf;
	proc_t *slot_to_proc();

	procaddr = slot_to_proc(slot);

	if (procaddr) {
		readmem((long)procaddr,1, -1,(char *)&prbuf,sizeof prbuf,
		    "proc table");
	} else {
		return;
	}

	if (full)
		fprintf(fp,"\n%s",heading);

	fprintf(fp, "%4d  ", slot);

	if (prbuf.p_as == NULL) {
		fprintf(fp, "- no address space.\n");
		return;
	}

	readbuf(addr,(long)(prbuf.p_as),phys,-1,
		(char *)&asbuf,sizeof asbuf,"as structure");

	fprintf(fp,"%4d  0x%08x 0x%08x  %9d  0x%08x 0x%08x  %s%s\n",
		asbuf.a_keepcnt,
		asbuf.a_segs,
		asbuf.a_seglast,
		asbuf.a_rss,
		asbuf.a_hat.hat_pts,
		asbuf.a_hat.hat_ptlast,
		(asbuf.a_lock == 0) ? "" : "lock " ,
		(asbuf.a_want == 0) ? "" : "want " );

	if (full) { 
		prsegs(prbuf.p_as, (struct as *)&asbuf, phys, addr);
	}
}


/* print list of seg structures */
void
prsegs(as, asbuf, phys, addr)
	struct as *as, *asbuf;
	u_long phys, addr;
{
	struct seg *seg, *sseg;
	struct seg  segbuf;
	struct syment *sp;
	extern char * strtbl;
	extern struct syment *findsym();

	sseg = seg = asbuf->a_segs;

	if (seg == NULL)
		return;

	fprintf(fp, "      LOCK        BASE     SIZE        NEXT       PREV          OPS        DATA\n");

	do {
		readbuf(addr, seg, phys, -1, (char *)&segbuf, sizeof segbuf,
			"seg structure");
		fprintf(fp, "      %4d  0x%08x %8d  0x%08x 0x%08x ",
			segbuf.s_lock,
			segbuf.s_base,
			segbuf.s_size,
			segbuf.s_next,
			segbuf.s_prev);

		/* Try to find a symbolic name for the sops vector. If
		 * can't find one print the hex address.
		 */
		sp = findsym((unsigned long)segbuf.s_ops);
		if ((!sp) || ((unsigned long)segbuf.s_ops != sp->n_value))
			fprintf(fp,"0x%08x  ", segbuf.s_ops);
		else if (sp->n_zeroes) 
			fprintf(fp, "  %8.8s  ", sp->n_name);
		else
			fprintf(fp, "%12.12s  ", strtbl+sp->n_offset);

		fprintf(fp,"0x%08x\n", segbuf.s_data);

		if (segbuf.s_as != as) {
			fprintf(fp, "WARNING - seg was not pointing to the correct as struct: 0x%8x\n",
				segbuf.s_as);
			fprintf(fp, "          seg list traversal aborted.\n");
			return;
		}
	} while((seg = segbuf.s_next) != sseg);
}
