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

#ident	"@(#)crash:i386.c	1.1.2.1"

/*
 * This file contains code for the crash functions: idt, ldt, gdt
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/immu.h>
#include <sys/tss.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/var.h>
#include <sys/acct.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/lock.h>
#include <sys/reg.h>
#include "crash.h"

#define HEAD1 "SLOT     BASE/SEL LIM/OFF  TYPE       DPL  ACESSBITS\n"
#define HEAD2 "SLOT     SELECTOR OFFSET   TYPE       DPL  ACESSBITS\n"
#define UREG(x) 	((long *)(&ubp->u_stack[save_r0ptr-UVUBLK]))[x]

extern	struct user *ubp;
extern  struct proc procbuf;

extern char *malloc();

/* get arguments for ldt function */
int getldt()
{
	int slot = Procslot;
	int all = 0;
	int c;
	int first=0;
	int last=MINLDTSZ-1;
	optind = 1;
	while((c = getopt(argcnt,args,"ew:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((slot = strcon(args[optind],'d')) == -1)
			error("\n");
		if((slot < 0) || (slot >= vbuf.v_proc)) 
			error("proc %d is out of range\n",slot);
		optind++;
	}
	if(args[optind]) {
		if((first= strcon(args[optind],'d')) == -1)
			error("\n");
		if((first < 0) || (first >= MINLDTSZ))
			error("entry %d is out of range\n",slot);
		last=first;
		optind++;
	}
	if(args[optind]) {
		if((last = strcon(args[optind],'d')) == -1)
			error("\n");
		last=first+last-1;
		if (last>=MINLDTSZ) last=MINLDTSZ-1;
	}
	if (first==last) all=1;
	prldt(all,slot,first,last);
}

/* print local descriptor table */
/* The linear address for the ldt is assumed to be identical for all processes
   so, reading the address of the current process's ldt will result in a value
   which can be used for all processes
 */
int prldt(all,slot,first,last)
int all,slot;
int first,last;
{
	int i;
	struct dscr gdtent;
	struct dscr ldt[MINLDTSZ];
	struct syment *sp;
	if(sp = symsrch("gdt")) 
		readmem((long)sp->n_value + LDTSEL ,1,-1,
			&gdtent,sizeof gdtent,"gdtentry for ldt");
	readmem(
	    (gdtent.a_base2431<<24)|(gdtent.a_base1623<<16)|(gdtent.a_base0015),
	    1,slot,ldt,sizeof ldt,"LDT");
	fprintf(fp,"iAPX386 LDT for process %d\n",slot);
	fprintf(fp,HEAD1);
	for (i=first;i<=last;i++) prdescr(i,&ldt[i],all);
}

/* get arguments for idt function */
int getidt() {
	int all = 0;
	int c;
	int first=0;
	int last=IDTSZ-1;
	optind = 1;
	while((c = getopt(argcnt,args,"ew:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((first= strcon(args[optind],'d')) == -1)
			error("\n");
		if((first < 0) || (first >= IDTSZ))
			error("entry %d is out of range\n",first);
		last=first;
		optind++;
	}
	if(args[optind]) {
		if((last = strcon(args[optind],'d')) == -1)
			error("\n");
		last=first+last-1;
		if (last>=IDTSZ) last=IDTSZ-1;
	}
	if (first==last) all=1;
	pridt(all,first,last);
}

/* print interrupt descriptor table */
int pridt(all,first,last)
int all;
int first,last;
{
	int i;
	struct dscr idt[IDTSZ];
	struct syment *sp;
	if(sp = symsrch("idt")) 
		readmem((long)sp->n_value,1,-1,
			(char *) idt,sizeof idt,"IDT");
	else error("idt not found in symbol table\n");
	fprintf(fp,"iAPX386 IDT\n");
	fprintf(fp,HEAD2);
	for (i=first;i<=last;i++) prdescr(i,&idt[i],all);
}

/* get arguments for gdt function */
int getgdt() {
	int all = 0;
	int c;
	int first=0;
	int last=GDTSZ-1;
	optind = 1;
	while((c = getopt(argcnt,args,"ew:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((first= strcon(args[optind],'d')) == -1)
			error("\n");
		if((first < 0) || (first >= GDTSZ))
			error("entry %d is out of range\n",first);
		last=first;
		optind++;
	}
	if(args[optind]) {
		if((last = strcon(args[optind],'d')) == -1)
			error("\n");
		last=first+last-1;
		if (last>=GDTSZ) last=GDTSZ-1;
	}
	if (first==last) all=1;
	prgdt(all,first,last);
}

/* print global descriptor table */
int prgdt(all,first,last)
int all;
int first,last;
{
	int i;
	struct dscr gdt[GDTSZ];
	struct syment *sp;
	if(sp = symsrch("gdt")) 
		readmem((long)sp->n_value,1,-1,
			(char *) gdt,sizeof gdt,"GDT");
	else error("gdt not found in symbol table\n");
	fprintf(fp,"iAPX386 GDT\n");
	fprintf(fp,HEAD1);
	for (i=first;i<=last;i++) prdescr(i,&gdt[i],all);
}

/* dumb version
prdescr(i,t,all)
int i;
struct dscr *t;
int all;
{
	if (!(all | (t->a_acc0007 & 0x80))) return(0);
	fprintf(fp,"%4d %02x%02x%04x %01x%04x %01x%02x\n",i,
		t->a_base2431, t->a_base1623, t->a_base0015,
		t->a_lim1619,  t->a_lim0015,
		t->a_acc0811,  t->a_acc0007);
}
*/

prdescr(i,t,all)
int i;
struct dscr *t;
int all;
{
	int	selec=0;	/* true if selector, false if base */
	int	gran4;		/* true if granularity = 4K */
	char	acess[100];	/* Description of Accessbytes */
	long	base;		/* Base or Selector */
	long	offset;		/* Offset or Limit */
	char	*typ;		/* Type */

	if (!(all | (t->a_acc0007 & 0x80))) return(0);
	base = (t->a_base2431 << 24)|(t->a_base1623 << 16) |
	       (t->a_base0015);
	offset=(t->a_lim1619 << 16) | (t->a_lim0015);
	gran4= (t->a_acc0811) & 0x8;
	sprintf(acess,"  %d ",(t->a_acc0007 >> 5) & 3);

	if (!(t->a_acc0007 & 0x10)) {
	/* Segment Descriptor */
		selec=1;
		switch (t->a_acc0007&0xf) {
		case 0:	typ="SYS 0 ?  "; selec=0; break;
		case 3:
		case 1:	typ="TSS286   "; selec=0; break;
		case 2: typ="LDT      "; selec=0; break;
		case 4: typ="CGATE    ";
			sprintf(acess,"%s CNT=%d",acess,t->a_base1623);
			break;
		case 5: typ="TASK GATE";
			sprintf(acess,"%s CNT=%d",acess,t->a_base1623);
			break;
		case 6: typ="IGATE286 ";
			sprintf(acess,"%s CNT=%d",acess,t->a_base1623);
			break;
		case 7: typ="TGATE286 ";
			sprintf(acess,"%s CNT=%d",acess,t->a_base1623);
			break;
		case 9:
		case 11:typ="TSS386   "; selec=0; break;
		case 12:typ="CGATE386 ";
gate386:		offset = (t->a_lim1619 <<16) | (t->a_acc0811 << 20) |
				 (t->a_base2431<<24) | (t->a_lim0015);
			gran4=0;
			sprintf(acess,"%s CNT=%d",acess,t->a_base1623);
			break;
		case 14:typ="IGATE386 ";
			goto gate386;
		case 15:typ="TGATE386 ";
			goto gate386;
		default:typ="SYS???   ";
		}
	} else if (t->a_acc0007 & 0x8) {
	/* executable Segment */
		typ="XSEG     ";
		sprintf(acess,"%s%s%s%s%s",acess,
			t->a_acc0007 & 1 ? " ACCS'D":"",
			t->a_acc0007 & 2 ? " R&X":" XONLY",
			t->a_acc0007 & 4 ? " CONF":"",
			t->a_acc0811 & 4 ? " DFLT":"");
	} else {
	/* Data Segment */
		typ="DSEG     ";
		sprintf(acess,"%s%s%s%s%s",acess,
			t->a_acc0007 & 1 ? " ACCS'D":"",
			t->a_acc0007 & 2 ? " R&W":" RONLY",
			t->a_acc0007 & 4 ? " EXDOWN":"",
			t->a_acc0811 & 4 ? " BIG ":"");
	}

	fprintf(fp,"%4d     ",i);
	if (selec) fprintf(fp,"    %04x",base&0xffff);
	else fprintf(fp,"%08x",base);
	if (gran4) fprintf(fp," %08x ",offset<<12 );
	else fprintf(fp," %08x ",offset);
/*	fprintf(stderr," %01x%02x ",t->a_acc0811,t->a_acc0007); */
	fprintf(fp,"%s  %s%s%s%s\n",typ,acess,gran4 ? " G4096":"",
			t->a_acc0811 & 1 ? " AVL":"",
			t->a_acc0007 & 0x80 ? "":"** nonpres **");
}


/* Test command */
int debugmode=0;

int gettest()
{
	int p1;
	char c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((p1 = strcon(args[optind],'d')) == -1) error("\n");
		debugmode=p1;
	}
	fprintf(fp,"Debug Mode %d\n",debugmode);
}

/* get arguments for panic function */
int getpanic()
{
	int slot = Procslot;
	char c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((slot = strcon(args[optind],'d')) == -1)
			error("\n");
		if((slot < 0) || (slot >= vbuf.v_proc)) 
			error("proc %d is out of range\n",slot);
		optind++;
	}
	prpanic(slot);
}

prpanic(slot)
int	slot;
{
	extern  struct syment *Panic;
	struct	syment *Putbuf,*Putbufsz,*Save_r0ptr;
	long	panicstr;
	char	panicbuf[200];
	char	*putbuf;
	int	putbufsz;
	unsigned long save_r0ptr;
	unsigned long stkhi,stklo;

	if (getublock(slot) == -1)
		return;
	if (!(Putbuf=symsrch("putbuf")))
		error("symbol putbuf not found\n");
	if (!(Putbufsz=symsrch("putbufsz")))
		error("symbol putbufsz not found\n");
	if (!(Save_r0ptr=symsrch("save_r0ptr")))
		error("symbol save_r0ptr not found\n");
	readmem((long)Putbufsz->n_value,1,-1,&putbufsz,sizeof(int),"putbufsz");
	if ((putbuf = malloc(putbufsz+1)) == NULL)
		prerrmes("Insufficient memory for putbuf");
	else {
		readmem((long)Putbuf->n_value,1,-1,putbuf,putbufsz,"putbuf");
		putbuf[putbufsz] = '\0';
		fprintf(fp,"System Messages:\n\n%s\n\n",putbuf);
		free(putbuf);
	}
	readmem((long)Panic->n_value,1,-1,&panicstr,sizeof(long),"panicstr");
	if (!(panicstr)) {
		fprintf(fp,"No Panic\n");
		return(0);
	}
	readmem(panicstr,1,-1,panicbuf,sizeof(panicbuf),"panicstr");
	panicbuf[sizeof panicbuf - 1] = '\0';
	fprintf(fp, "Panic String: %s\n\n", panicbuf);
	readmem((long)Save_r0ptr->n_value,1,-1,&save_r0ptr,sizeof(int),"save_r0ptr");
	if (!(save_r0ptr)) return(0);
	fprintf(fp,"Kernel Trap. Kernel Registers saved at %08x\n",save_r0ptr);
	fprintf(fp,"ERR=%d, TRAPNO=%d\n",
		UREG(ERR),UREG(TRAPNO));
	printreg(UREG(EAX),UREG(EBX), UREG(ECX), UREG(EDX),
		UREG( ESP),UREG(EBP), UREG(ESI), UREG(EDI),
		UREG(EFL), UREG(EIP), UREG( CS), -1,
		UREG( DS), UREG( ES), UREG( FS), UREG( GS));

	fprintf(fp,"\nKernel Stack before Trap:\n");
	stklo = UVUBLK;
	/* u_ar0 points to location in kernel stack, where user registers are
	   saved. Stop stack trace there. */
	if (!(procbuf.p_flag & SSYS))
		stkhi = (unsigned long) ubp->u_ar0;
	else	stkhi = UVUBLK+KSTKSZ;
	if (stkhi < stklo) error ("upper bound < lower bound\n");
	stklo = stkhi - setbf(stkhi,stklo,slot);

	puttrace(stklo,UREG(EBP),UREG(ESP),0,UREG(EIP),0,stkhi);
}

