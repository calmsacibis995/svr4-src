/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)crash-3b2:vtop.c	1.5.8.1"

/*
 * This file contains code for the crash functions:  vtop and mode, as well as
 * the virtual to physical offset conversion routine vtop.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef i386
#define MAINSTORE 0
#include <sys/sysmacros.h>
#else
#include <sys/sbd.h>
#endif
#include <sys/fs/s5dir.h>
#include <sys/immu.h>
#include <sys/vnode.h>
#include <vm/hat.h>
#include <vm/seg.h>
#include <vm/as.h>
#include <vm/page.h>
#include <sys/proc.h>
#include "crash.h"

extern struct syment *Curproc;
#ifndef i386
char * sramapt[4];			/* srama: initialized in init() */
SRAMB srambpt[4];			/* sramb: initialized in init() */
#endif

int abortflag = 0;			/* flag to abort or continue */
					/* used in vtop function */
struct proc prbuf;			/* process entry buffer */
struct as   asbuf;			/* address space buffer */
int prsid;				/* temporary variables to hold */
long prssl,prsram,prpsl;		/* values to be printed by vtop */
					/* function */
pte_t *kpd_start;

/* virtual to physical offset address translation */
paddr_t
vtop(vaddr,slot)
unsigned long vaddr;
int slot;
{
#ifdef i386
#define errtx(tx) {if(!abortflag)return(-1);abortflag=0;error(tx);}
	static pte_t *KPD;
	pte_t *kpd_entry;
	pte_t pte, *pt;
	struct as c_as;
	struct as *ppas;
	struct hat *hatp;
	struct hat c_hat;
	struct hatpt c_ptap, *start_ptap, *ptap;

	/* Get kernel page directory */
	if (!KPD) {
		struct syment *sym;
		if (!(sym = symsrch("kpd0")))
			error("kpd0 not found in symbol table\n");
		
		kpd_start = (pte_t *)(sym->n_value);
		KPD = (pte_t *)(sym->n_value - KVSBASE);
	}

	pt = KPD + ptnum(vaddr);
	if (debugmode>1) fprintf(stderr,"vtop(%x,%d): ",vaddr,slot);
	if (KADDR(vaddr) && \
	(!(vaddr >= UVUBLK && PFNUM(vaddr - UVUBLK) < MAXUSIZE))) {
		if (debugmode>1) fprintf(stderr,"kvirt  ");
	} else {
		if (debugmode>1) fprintf(stderr,"must be transformed.\n");
		procntry(slot,&prbuf);
		if (!(prbuf.p_flag&SLOAD)) error("proc is swapped out\n");
		if (vaddr >= UVUBLK && PFNUM(vaddr - UVUBLK) < MAXUSIZE) {
			if (debugmode>1) fprintf(stderr,"ublock address.\n");
			readmem(prbuf.p_ubptbl + PFNUM(vaddr - UVUBLK),1,-1,
				&pte,sizeof(pte),"ublock page table entry");
			goto got_pte;
		} else if (vaddr < MAXUVADR) {
		    kpd_entry = (pte_t *)kpd_start + ptnum(vaddr);
		    /* Read in address space structure */
		    ppas = prbuf.p_as;
		    readmem(ppas, 1, slot, &c_as, sizeof(c_as), "vm as");
		    /* Get start of HAT list structures */
		    hatp = &c_as.a_hat;
		    start_ptap = ptap = hatp->hat_pts;
		    /* Scan each HAT structure */
		    do {
			if (ptap == (struct hatpt *) NULL)
				error("Page table address pointer = NULL\n");
			/* Read in Page Table aligned HAT structure */
			readmem(ptap, 1, slot, &c_ptap, sizeof(c_ptap), "hat ptap");
			/* ASSERT: HAT pointer must be in kpd0 range */
			if ((c_ptap.hatpt_pdtep < kpd_start) || ((char *)c_ptap.hatpt_pdtep > (char *)kpd_start + NBPP))
				error("Kernel hat pointer invalid\n");
			if (c_ptap.hatpt_pdtep == kpd_entry){
				/* Get physical byte address of Page Table */
				pt = (pte_t *)ctob(c_ptap.hatpt_pde.pgm.pg_pfn);
				pt += pnum(vaddr); /* offset into Page Table */
				/* Physical address read of Page Table Entry */
				readmem(pt,0,slot,&pte, sizeof(pte), "Page table entry");
				goto got_pte;
			}
			else
				/* Next Page Table aligned HAT structure */
				ptap = c_ptap.hatpt_forw;
		    } while (ptap != start_ptap);
		}
	}

	readmem(pt,0,slot,&pte,sizeof(pte),"page directory entry");
	if (!(pte.pgm.pg_v)) errtx("Page Table not in core\n");
	readmem((pte_t *)ctob(pte.pgm.pg_pfn) + pnum(vaddr),
		0,slot,&pte,sizeof(pte),"page table entry");
got_pte:
	if (!(pte.pgm.pg_v)) errtx("Page not in core\n");
	return(ctob(pte.pgm.pg_pfn)+PAGOFF(vaddr));
#else

	union {
		long intvar;
		struct _VAR vaddr;
	} virtaddr;
	sde_t *sdte;			/* segment descriptor table entry */
	sde_t sdtbuf;
	long  sde_paddr;		/* physical address of sde in crash file */
	pte_t pte;			/* page table entry */
        long pte_paddr;                 /* physical address of pte in crash file */
	long cntgseg;			/* phys address of contiguous segment */
	long paddr;			/* physical address to return */
	int len;			/* length of segment table */

	virtaddr.intvar = vaddr;
	/* 
	 * Use the section number in the addr to get the 
	 * address and the length of the segment table.
	 */
	switch (virtaddr.vaddr.v_sid) {
		case 0:			/* kernel section 0 */
			sde_paddr = (long) (sramapt[0] 
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			len = srambpt[0].SDTlen;
			break;
		case 1:			/* kernel section 1 */
			sde_paddr = (long) (sramapt[1] 
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			len = srambpt[1].SDTlen;
			break;
		case 2:			/* text, data and bss section */
			/* get a proc entry for given process */
			procntry(slot, &prbuf);
			readmem((long)prbuf.p_as, 1, -1, (char *)&asbuf,
				sizeof asbuf, "as structure");
			sde_paddr = (long) (asbuf.a_hat.hat_srama[0]
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			/* maximum offset in SDT */
			len = asbuf.a_hat.hat_sramb[0].SDTlen;            
			break;
		case 3:			/* shared memory, stack and ublock */
			/* get a proc entry for given process */
			procntry(slot, &prbuf);
			readmem((long)prbuf.p_as, 1, -1, (char *)&asbuf,
				sizeof asbuf, "as structure");
			sde_paddr = (long) (asbuf.a_hat.hat_srama[1]
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			/* maximum offset in SDT */
			len = asbuf.a_hat.hat_sramb[1].SDTlen;            
			break;
		}
	/* sde_paddr now contains the physical address of 
	   the proper segment descriptor */
		prsid = virtaddr.vaddr.v_sid;
		prssl = virtaddr.vaddr.v_ssl;
		if(virtaddr.vaddr.v_sid > 1) 
			prsram = asbuf.a_hat.hat_srama[virtaddr.vaddr.v_sid-2];
		else
			prsram = -1;
	
	/*
	 * Check the flag bits for errors and get the address
	 * of the segment number specified in addr.
	 */
	if(virtaddr.vaddr.v_ssl > (uint)len)  {
		if(abortflag) {
			abortflag = 0;
			error("%d out of range for segment table\n",len);
		}
		return(-1);
	}

	/* read in the segment descriptor from the crash file */
	readmem((sde_paddr&~MAINSTORE),0,slot,(char *)&sdtbuf,
		sizeof (sde_t),"segment descriptor table");
	sdte = &sdtbuf;

	if(!(isvalid(sdte))) {
		if(abortflag) {
			abortflag = 0;
			error("segment descriptor table entry is invalid\n");
		}
		return(-1);
	}

	/* indirect if shared memory */
	if(indirect(sdte))
	{
		sde_paddr = sdte->wd2.address & ~7;
		
		readmem((sde_paddr&~MAINSTORE),0,slot,(char *)&sdtbuf,
			sizeof (sde_t),"segment descriptor table");

		if(!(isvalid(sdte))) {
			if(abortflag) {
				abortflag = 0;
				error("indirect segment descriptor table entry is invalid\n");
			}
			return(-1);
		}
	}
	
	/*
	 * Check the "c" bit in the sdte to tell whether this
	 * is a "contiguous" or "paged" segment.
	 */
	if(iscontig(sdte)) {
		cntgseg=(long) sdte->wd2.address & 0xfffffff8;
		/* add in segment offset */
		paddr = virtaddr.intvar & 0x1ffffL;
		paddr += cntgseg;
		/* turn the address into an offset and return */
		return(paddr & ~MAINSTORE);
	}
	else {	/* get page descriptor table entry */
                pte_paddr = (sdte->wd2.address & ~0x1f) 
				+ virtaddr.vaddr.v_psl * sizeof (pte_t);

		readmem((pte_paddr&~MAINSTORE),0,slot,(char *)&pte,
			sizeof (pte_t),"page descriptor table");

                if (!pte.pgm.pg_v) {
			if(abortflag) {
				abortflag = 0;
                    		error("page not valid in memory\n");
			}
			return(-1);
		}
		prpsl = virtaddr.vaddr.v_psl;
                /* calculate physical address and return */
                paddr = (pte.pg_pte & PG_ADDR) +
                       (virtaddr.vaddr.v_byte & POFFMASK);
                return(paddr & ~MAINSTORE);

	}
#endif
}

/* get arguments for vtop function */
int
getvtop()
{
	int proc = Procslot;
	struct syment *sp;
	unsigned long addr;
	int c;


	optind = 1;
	while((c = getopt(argcnt,args,"w:s:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 's' :	proc = setproc();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		fprintf(fp,"VIRTUAL  PHYSICAL SECT SDT   SRAM   PDT\n");
		do {
			if(*args[optind] == '(') {
				if((addr = eval(++args[optind])) == -1)
					continue;
				prvtop(addr,proc);
			}
			else if(sp = symsrch(args[optind])) 
				prvtop((long)sp->n_value,proc);
			else if(isasymbol(args[optind]))
				error("%s not found in symbol table\n",
					args[optind]);
			else {
				if((addr = strcon(args[optind],'h')) == -1)
					continue;
				prvtop(addr,proc);
			}
		}while(args[++optind]);
	}
	else longjmp(syn,0);
}

/* print vtop information */
int
prvtop(addr,proc)
unsigned long addr;
int proc;
{
	paddr_t paddr;

	abortflag = 1;
	paddr = vtop(addr,proc) + MAINSTORE;
	fprintf(fp,"%8x %8x %4d %3d",
		addr,
		paddr,
		prsid,
		prssl);
	if(prsram == -1)
		fprintf(fp,"         ");
	else fprintf(fp," %8x",prsram);
	fprintf(fp," %3d\n",
		prpsl);
	abortflag = 0;
}


/* get arguments for mode function */
int
getmode()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		prmode(args[optind]);
	else prmode("s");
}

/* print mode information */
int
prmode(mode)
char *mode;
{

	switch(*mode) {
		case 'p' :  Virtmode = 0;
			    break;
		case 'v' :  Virtmode = 1;
			    break;
		case 's' :  break;
		default  :  longjmp(syn,0);
	}
	if(Virtmode)
		fprintf(fp,"Mode = virtual\n");
	else fprintf(fp,"Mode = physical\n");
}
