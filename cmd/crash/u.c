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

#ident	"@(#)crash-3b2:u.c	1.20.19.1"

/*
 * This file contains code for the crash functions:  user, pcb, stack,
 * trace, and kfp.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/immu.h>
#ifdef i386
#include <sys/tss.h>
#include <sys/seg.h>
#include <sys/reg.h>
#else
#include <sys/psw.h>
#include <sys/sbd.h> 
#include <sys/nvram.h> 
#include <sys/pcb.h>
#endif
#include <sys/user.h>
#include <sys/var.h>
#include <sys/acct.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/lock.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include "crash.h"
#ifdef i386
#include <sys/sysi86.h>
#else
#include <sys/sys3b.h>
#endif

#define DECR	4
#ifdef i386
#define UBADDR UVUBLK
#define UREG(x) ((long*)(ubp->u_stack))[((long)(&ubp->u_ar0[x]) - UBADDR)/sizeof(long)]
long tmp1;
#else
#define FP	1
#define AP	0
#define USTKADDR 0xc0020000
#define UBADDR 0xc0000000
#define UPCBADDR 0xc000000c
#define UKPCBADDR 0xc000007c
#endif

#define min(a,b) (a>b ? b:a)

#define	DATE_FMT	"%a %b %e %H:%M:%S %Y\n"
/*
 *	%a	abbreviated weekday name
 *	%b	abbreviated month name
 *	%e	day of month
 *	%H	hour
 *	%M	minute
 *	%S	second
 *	%Y	year
 */

extern struct user *ubp;		/* ublock pointer */
extern int active;			/* active system flag */
struct proc procbuf;			/* proc entry buffer */
static unsigned long Kfp = 0;		/* kernel frame pointer */
static char	time_buf[50];		/* holds date and time string */
extern	char	*strtbl ;		/* pointer to string table */
unsigned long *stk_bptr;		/* stack pointer */
#ifndef i386
extern struct xtra_nvr xtranvram;	/* xtra nvram buffer */
#endif
extern	struct	syment	*File,
	*Vnode, *Curproc, *Panic, *V;	/* namelist symbol pointers */
extern struct	syment	*findsym();
extern char *malloc();
void free();
unsigned long *temp;

char *rlimits[] = {
	"cpu time",
	"file size",
	"swap size",
	"stack size",
	"coredump size",
	"file descriptors",
	"address space"
};

/* read ublock into buffer */
int
getublock(slot)
int slot;
{
	return _getublock(slot,USIZE*NBPC,ubp);
}

int
_getublock(slot, size, buf)
int slot;
long size;
struct user *buf;
{
	int 	i,cnt;
	struct proc *procp;
	pte_t	ubptbl[MAXUSIZE];
	long length;
	proc_t *slot_to_proc();

	if(slot == -1) 
		slot = getcurproc();
	if(slot >= vbuf.v_proc || slot < 0) {
		prerrmes("%d out of range\n",slot);
		return(-1);
	}

	procp = slot_to_proc(slot);
	if (procp == NULL) {
		prerrmes("%d is not a valid process\n",slot);
		return(-1);
	}
	readmem((unsigned long)procp,1,slot,(char *)&procbuf,sizeof procbuf,
		"process table");
	if (procbuf.p_stat == SZOMB) {
		prerrmes("%d is a zombie process\n",slot);
		return(-1);
	}
	if(active)
	{
#ifdef i386
		if(sysi86(RDUBLK, slot_to_pid(slot), (char *)buf, (USIZE*NBPC))==-1)
#else
		if(sys3b(RDUBLK, slot_to_pid(slot), (char *)buf, (USIZE*NBPC))==-1)
#endif
			return(-1);
		else
			return(0);
	}
	/* examine sysdump and U-Block was swapped-out */
	else if(!(procbuf.p_flag & SULOAD)) {
		prerrmes("%d was swapped-out\n", slot);
		return(-1);
	} else {
#ifdef i386
	if (procbuf.p_ubptbl == 0) {
		prerrmes("proc %d ubptbl is 0, but not swapped\n", slot);
		return(-1);
	}
	readmem(procbuf.p_ubptbl,1,-1,ubptbl,sizeof(ubptbl),
			"ublock page table entries");
	i=0;
	for(cnt=0; cnt < size; cnt += NBPP, i++) {
		/* seek from begining of memory to ith page of uarea */
		if (!ubptbl[i].pgm.pg_v) error("ublock not present\n");
		if(_seekmem(ctob(ubptbl[i].pgm.pg_pfn),0,0) == -1) {
			prerrmes("seek error on ublock address\n");
			return(-1);
		}
#else /* not i386 */
	i=((char*)ubptbl(procp) - (char*)procp -
		((char*)procbuf.p_ubptbl - (char*)&procbuf)) >> 2;
	for(cnt=0; cnt < (USIZE * NBPC); cnt += NBPC, i++) {
		/* seek from beginning of memory to ith page of uarea */
		if(lseek(mem,(long)(procbuf.p_ubptbl[i].pgm.pg_pfn<<11)-
			MAINSTORE,0) == -1) {
			prerrmes("seek error on ublock address\n");
			return(-1);
		}
#endif /* not i386 */
		length=min(NBPP,size-cnt);  /* UGH - joeh */
		if((read(mem,(char *)buf+cnt,length)) != length) {
			prerrmes("read error on ublock\n");
			return(-1);
		}
	}
	}
	return(0);
}

/* allocate buffer for stack */
unsigned
setbf(top, bottom, slot)
unsigned long top;
unsigned long bottom;
int slot;
{
	unsigned range;
	char *bptr;
	long remainder;
	long nbyte;
	unsigned long paddr;


	if (bottom > top) 
		error("Top of stack value less than bottom of stack\n");
	range = (unsigned)(top - bottom);
	if((stk_bptr = (unsigned long *)malloc(range)) == NULL)
#ifndef i386
	{
		prerrmes("Insufficient memory available for stack buffering.\n"); 
		prerrmes("Only the most recently pushed 4K bytes will be dumped from the stack.\n");
		prerrmes("New stack lower bound: %8.8x\n",top - range);
		range = 4096;
		if((stk_bptr = (unsigned long *)malloc(range)) == 0)
			error("Second attempt to allocate memory for stack buffering failed, try again later\n");

	}
#else /* not i386 */
		error("Insufficient memory available for stack buffering.\n");
#endif
	
	bottom = top - range;
	bptr = (char *)stk_bptr;
	do {
		remainder = ((bottom + NBPP) & ~((long)NBPP -1)) - bottom;
		nbyte = min(remainder, top-bottom);
		if((paddr = vtop(bottom,slot)) == -1) {
			free((char *)stk_bptr);
			stk_bptr = NULL;
			error("The stack lower bound, %x, is an invalid address\nThe saved stack frame pointer is %x\n",bottom,top);
		}
		if(_seekmem(paddr,0,0) == -1) {
			free((char *)stk_bptr);
			error("seek error on stack\n");
		}
		if(read(mem,bptr,(unsigned)nbyte) != (unsigned)nbyte) {
			free((char *)stk_bptr);
			stk_bptr = NULL;
			error("read error on stack\n");
		}
		bptr += nbyte;
		bottom += nbyte;
	} while (bottom < top);
	return(range);
}

/* get arguments for user function */
int
getuser()
{
	int slot = Procslot;
	int full = 0;
	int all = 0;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"efw:")) !=EOF) {
		switch(c) {
			case 'f' :	full = 1;
					break;
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do {
			getargs(vbuf.v_proc,&arg1,&arg2);
			if(arg1 == -1)
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					pruser(full,slot);
			else pruser(full,arg1);
			slot = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else if (all) {
		readmem(V->n_value,1,-1,(char *)&vbuf,
			sizeof vbuf,"var structure");
		for(slot =0; slot < vbuf.v_proc; slot++)
			pruser(full,slot);
	}
	else pruser(full,slot);
}

/* print ublock */
int
pruser(full,slot)
int full,slot;
{
	register  int  i,j;
	unsigned offset;

	if(getublock(slot) == -1)
		return;
	if(slot == -1)
		slot = getcurproc();
	fprintf(fp,"PER PROCESS USER AREA FOR PROCESS %d\n",slot);

	fprintf(fp,"PROCESS MISC:\n");
	fprintf(fp,"\tcommand: %s,", ubp->u_comm);
	fprintf(fp," psargs: %s\n", ubp->u_psargs);
	fprintf(fp,"\tproc slot: %d", proc_to_slot(ubp->u_procp));
	cftime(time_buf, DATE_FMT, &ubp->u_start);
	fprintf(fp,"\tstart: %s", time_buf);
	fprintf(fp,"\tmem: %x, type: %s%s\n",
		ubp->u_mem,
		ubp->u_acflag & AFORK ? "fork" : "exec",
		ubp->u_acflag & ASU ? " su-user" : "");
#ifndef i386
	fprintf(fp,"\t%s", ubp->u_dmm ? "double mapped, " : ""); 
#endif
	fprintf(fp,"proc/text lock:%s%s%s%s\n",
		ubp->u_lock & TXTLOCK ? " txtlock" : "",
		ubp->u_lock & DATLOCK ? " datlock" : "",
		ubp->u_lock & PROCLOCK ? " proclock" : "",
		ubp->u_lock & (PROCLOCK|TXTLOCK|DATLOCK) ? "" : " none");
#ifndef i386	/* stack is part of userarea at UNIX V/386 */
	fprintf(fp,"\tstack: %8x,", ubp->u_stack);
#endif
	if(ubp->u_cdir)
		fprintf(fp,"\tvnode of current directory: %8x",ubp->u_cdir);
	else fprintf(fp," - ,");
	if(ubp->u_rdir)
		fprintf(fp,", vnode of root directory: %8x,",ubp->u_rdir);
	fprintf(fp,"\nOPEN FILES AND POFILE FLAGS:\n");
	for(i = 0, j = 0; i < ubp->u_nofiles; i++){
		struct ufchunk uf;
		struct ufchunk *ufp;

		if ((i % NFPCHUNK) == 0) {
			if (i == 0) {
				ufp = &ubp->u_flist;
			} else {
				readmem((long)ufp->uf_next,1,-1,(char *)&uf,
					sizeof uf,"user file array");
				ufp = &uf;
			}
		}
		if(ufp->uf_ofile[i % NFPCHUNK] != 0) {
			if ((j++ % 2) == 0)
				fprintf(fp,"\n");
			fprintf(fp,"\t[%d]: F %#.8x, %x\t",i,
			    ufp->uf_ofile[i%NFPCHUNK],ufp->uf_pofile[i%NFPCHUNK]);
		}
	}
	fprintf(fp,"\n");
	fprintf(fp,"FILE I/O:\n\tu_base: %8x,",ubp->u_base);
	fprintf(fp," file offset: %d, bytes: %d,\n",
		ubp->u_offset,
		ubp->u_count);
	fprintf(fp,"\tsegment: %s,", ubp->u_segflg == 0 ? "data" :
		(ubp->u_segflg == 1 ? "sys" : "text"));
	fprintf(fp," cmask: %4.4o\n", ubp->u_cmask);
	fprintf(fp,"RESOURCE LIMITS:\n");
	for (i = 0; i < RLIM_NLIMITS; i++) {
		if (rlimits[i] == 0)
			continue;
		fprintf(fp,"\t%s: ", rlimits[i]);
		if (ubp->u_rlimit[i].rlim_cur == RLIM_INFINITY)
			fprintf(fp,"unlimited/");
		else
			fprintf(fp,"%d/", ubp->u_rlimit[i].rlim_cur);
		if (ubp->u_rlimit[i].rlim_max == RLIM_INFINITY)
			fprintf(fp,"unlimited\n");
		else
			fprintf(fp,"%d\n", ubp->u_rlimit[i].rlim_max);
	}
	fprintf(fp,"\tfile mode(s):");	
	fprintf(fp,"%s%s%s%s%s%s%s%s\n",
		ubp->u_fmode & FREAD ? " read" : "",
		ubp->u_fmode & FWRITE ? " write" : "",
		ubp->u_fmode & FAPPEND ? " append" : "",
		ubp->u_fmode & FSYNC ? " sync" : "",
		ubp->u_fmode & FCREAT ? " creat" : "",
		ubp->u_fmode & FTRUNC ? " trunc" : "",
		ubp->u_fmode & FEXCL ? " excl" : "",
		ubp->u_fmode & FNDELAY ? " ndelay" : "");
	fprintf(fp,"SIGNAL DISPOSITION:");
	for (i = 0; i < MAXSIG; i++) {
		if(!(i & 3))
			fprintf(fp,"\n\t");
		fprintf(fp,"%4d: ", i+1);
		if((int)ubp->u_signal[i] == 0 || (int)ubp->u_signal[i] == 1)
			fprintf(fp,"%8s",(int)ubp->u_signal[i] ? "ignore" : "default");
		else fprintf(fp,"%-8x",(int)ubp->u_signal[i]);
	}
	if(full) {
#ifndef i386
		fprintf(fp,"\n\tpcbp: %x, r0tmp: %x, spop: %x\n",
			ubp->u_pcbp,
			ubp->u_r0tmp,
			ubp->u_spop);
		fprintf(fp,"\tmau asr: %x, mau dr[0]: %x, mau dr[1]: %x, mau dr[2]",
			ubp->u_mau.asr,
			ubp->u_mau.dr[0],
			ubp->u_mau.dr[1],
			ubp->u_mau.dr[2]);
		for(i = 0; i < 4; i++) {
			fprintf(fp,"\n\t");
			for(j = 0; j < 3; j++)
				fprintf(fp,"fpregs[%d][%d]: %x  ",
					i,j,ubp->u_mau.fpregs[i][j]);
		}
		fprintf(fp,"\n");
#endif
		fprintf(fp,"\tfc_flags: %x, fc_func: %x, nshmseg: %d\n",
			ubp->u_fault_catch.fc_flags,
			ubp->u_fault_catch.fc_func,
			ubp->u_nshmseg);
		fprintf(fp,"\tbsize: %d, qsav: %x, error: %d\n",
			ubp->u_bsize,
			ubp->u_qsav,
			ubp->u_error);
		fprintf(fp,"\tap: %x, u_r: %x, pbsize: %d\n",
			ubp->u_ap,
			ubp->u_rval1,
			ubp->u_pbsize);
		fprintf(fp,"\tpboff: %d,",ubp->u_pboff);
		fprintf(fp," rablock: %x, errcnt: %d\n",
			ubp->u_rablock,
			ubp->u_errcnt);
		fprintf(fp," tsize: %x, dsize: %x, ssize: %x\n",
			ubp->u_tsize,
			ubp->u_dsize,
			ubp->u_ssize);
		fprintf(fp,"\targ[0]: %x, arg[1]: %x, arg[2]: %x\n",
			ubp->u_arg[0],
			ubp->u_arg[1],
			ubp->u_arg[2]);
		fprintf(fp,"\targ[3]: %x, arg[4]: %x, arg[5]: %x\n",
			ubp->u_arg[3],
			ubp->u_arg[4],
			ubp->u_arg[5]);	
#ifndef i386
		fprintf(fp,"\tiop: %x, ar0: %x, ticks: %x\n",
			ubp->u_iop,
#else
		fprintf(fp,"\tar0: %x, ticks: %x\n",
#endif
			ubp->u_ar0,
			ubp->u_ticks);

		fprintf(fp,"\tpr_base: %x, pr_size: %d, pr_off: %x, pr_scale: %d\n",
			ubp->u_prof.pr_base,
			ubp->u_prof.pr_size,
			ubp->u_prof.pr_off,
			ubp->u_prof.pr_scale);
		fprintf(fp,"\tior: %x, iow: %x, iosw: %x, ioch: %x\n",
			ubp->u_ior,
			ubp->u_iow,
			ubp->u_iosw,
			ubp->u_ioch);
		fprintf(fp, "\tsysabort: %d, systrap: %d\n",
			ubp->u_sysabort,
			ubp->u_systrap);
		fprintf(fp, "\tentrymask:");
		for (i = 0; i < sizeof(k_sysset_t)/sizeof(long); i++)
			fprintf(fp, " %08x", ubp->u_entrymask.word[i]);
		fprintf(fp, "\n");
		fprintf(fp, "\texitmask:");
		for (i = 0; i < sizeof(k_sysset_t)/sizeof(long); i++)
			fprintf(fp, " %08x", ubp->u_exitmask.word[i]);
		fprintf(fp, "\n");
		fprintf(fp,"\n\tEXDATA:\n");
		fprintf(fp,"\tvp: ");
		if(ubp->u_exdata.vp)
			fprintf(fp," %8x,",ubp->u_exdata.vp);
		else fprintf(fp," - , ");
		fprintf(fp,"tsize: %x, dsize: %x, bsize: %x, lsize: %x\n",
			ubp->u_exdata.ux_tsize,
			ubp->u_exdata.ux_dsize,
			ubp->u_exdata.ux_bsize,
			ubp->u_exdata.ux_lsize);
		fprintf(fp,"\tmagic#: %o, toffset: %x, doffset: %x, loffset: %x\n",
			ubp->u_exdata.ux_mag,
			ubp->u_exdata.ux_toffset,
			ubp->u_exdata.ux_doffset,
			ubp->u_exdata.ux_loffset);
		fprintf(fp,"\ttxtorg: %x, datorg: %x, entloc: %x, nshlibs: %d\n",
			ubp->u_exdata.ux_txtorg,
			ubp->u_exdata.ux_datorg,
			ubp->u_exdata.ux_entloc,
			ubp->u_exdata.ux_nshlibs);
		fprintf(fp,"\texecsz: %x\n",ubp->u_execsz);
#ifndef i386
		fprintf(fp,"\ttracepc: %x\n",ubp->u_tracepc);
#endif
		fprintf(fp,"\n\tRFS:\n");
		fprintf(fp,"\tsyscall: %d\n",ubp->u_syscall);
		fprintf(fp,"\n\tSIGNAL MASK:");
		for (i = 0; i < MAXSIG; i++) {
			if(!(i & 3))
				fprintf(fp,"\n\t");
			fprintf(fp,"%4d: %-8x ",i+1, (int)ubp->u_sigmask[i]);
		}
		fprintf(fp,"\n\tsigonstack: %x, sigflag: %x, oldmask: %x\n",
			ubp->u_sigonstack,
			ubp->u_sigflag,
			ubp->u_sigoldmask);
		fprintf(fp,"\taltflags: %s %s, altsp: %x, altsize: %x\n",
			ubp->u_sigaltstack.ss_flags&SS_DISABLE ? "disabl" : "",
			ubp->u_sigaltstack.ss_flags&SS_ONSTACK ? "onstak" : "",
			ubp->u_sigaltstack.ss_sp,
			ubp->u_sigaltstack.ss_size);
	}
	fprintf(fp,"\n");
}

/* get arguments for pcb function */
int
getpcb()
{
	int proc = Procslot;
	int phys = 0;
	char type = 'n';
	unsigned long addr = -1;
	int c;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"iukpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'i' :	type = 'i';
					break;
			case 'u' :	type = 'u';
					break;
			case 'k' :	type = 'k';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(')  {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = (unsigned long)sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
		else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pripcb(phys,addr);
		}
	else {
		if(args[optind]) {
			if((proc = strcon(args[optind],'d')) == -1)
				error("\n");
			if((proc > vbuf.v_proc) || (proc < 0))
				error("%d out of range\n",proc);
			prpcb(proc,type);
		}
		else prpcb(proc,type);
	}
}


/* print user, kernel, or active pcb */
#ifdef i386
/* The kernel pcb is the 386 task state segment (pointed to by u.u_tss)
   There is no user pcb in the 386. The user task state is saved on the
   kernel stack, instead. This area is taken as "user pcb". */
   
int prpcb(proc,type)
int proc;
char type;
{
	struct tss386 tss;
	struct syment *sym;

	if(getublock(proc) == -1)
		return;
	switch(type) {
		case 'n' :
		case 'k' :
			if (active && ((proc== -1) || (proc == getcurproc())))
				error ("This is current process on active system\n");
			readmem(ubp->u_tss,1,proc,&tss,sizeof(tss),"TSS");
			test_tss(tss.t_esp0,UVUBLK+KSTKSZ,"esp0");
			test_tss(tss.t_esp1,UVUBLK+KSTKSZ,"esp1");
			test_tss(tss.t_esp2,UVUBLK+KSTKSZ,"esp2");
			test_tss(tss.t_ss0,KDSSEL,"ss0");
			test_tss(tss.t_ss1,KDSSEL,"ss1");
			test_tss(tss.t_ss2,KDSSEL,"ss2");
			if(!(sym = symsrch("kpd0"))) 
				error("kpd0 not found in symbol table\n");
			else if (tss.t_cr3 != ((sym->n_value-KVSBASE)|0x80000000))
				test_tss(tss.t_cr3,sym->n_value-KVSBASE,"cr3");
			test_tss(tss.t_ldt,LDTSEL,"ldt");
			printreg(tss.t_eax,tss.t_ebx,tss.t_ecx,tss.t_edx,
				 tss.t_esp,tss.t_ebp,tss.t_esi,tss.t_edi,
				 tss.t_eflags,tss.t_eip,tss.t_cs ,tss.t_ss,
				 tss.t_ds ,tss.t_es ,tss.t_fs ,tss.t_gs);
			break;
		case 'u' :
			if (procbuf.p_flag & SSYS)
				error ("This is a system process\n");
			/* u_ar0 points to location in kernel stack */
			fprintf(fp,"ERR=%d, TRAPNO=%d\n",
				UREG(ERR),UREG(TRAPNO));
			printreg(UREG(EAX),UREG(EBX), UREG(ECX), UREG(EDX),
				UREG(UESP),UREG(EBP), UREG(ESI), UREG(EDI),
				UREG(EFL), UREG(EIP), UREG( CS), UREG( SS),
				UREG( DS), UREG( ES), UREG( FS), UREG( GS));
			break;
		default  : longjmp(syn,0);
			   break;
	}
}

test_tss(actual,expected,regname)
unsigned long int actual,expected;
char *regname;
{
	if (actual != expected) fprintf(fp,
		"Field u_t%s in tss has strange value %08x, expected %08x\n",
		regname,actual,expected);
}

printreg(eax,ebx,ecx,edx,esp,ebp,esi,edi,efl,eip,cs,ss,ds,es,fs,gs)
unsigned int eax,ebx,ecx,edx,esp,ebp,esi,edi,efl,eip,cs,ss,ds,es,fs,gs;
{
	fprintf(fp,"cs:eip=%04x:%08x Flags=%03x\n",cs&0xffff,eip,efl&0x3ffff);
	fprintf(fp,
		"ds = %04x   es = %04x   fs = %04x   gs = %04x",
		ds&0xffff,es&0xffff,fs&0xffff,gs&0xffff);
	if (ss != -1) fprintf(fp,"   ss = %04x",ss&0xffff);
	fprintf(fp,"\nesi= %08x   edi= %08x   ebp= %08x   esp= %08x\n",
		esi,edi,ebp,esp);
	fprintf(fp,"eax= %08x   ebx= %08x   ecx= %08x   edx= %08x\n",
		eax,ebx,ecx,edx);
}

#else /* not i386 */

int prpcb(proc,type)
int proc;
char type;
{
	int	i, j;
	struct kpcb *kpcbp;
	struct pcb *pcbp;

	if(getublock(proc) == -1)
		return;
	switch(type) {
		case 'n' : kpcbp = (struct kpcb *)(((long)ubp->u_pcbp - 
				sizeof (struct ipcb) - UBADDR) + (long)ubp);
			   if((kpcbp != (struct kpcb*)(long)&ubp->u_kpcb) &&
				(kpcbp != (struct kpcb*)&ubp->u_pcb))
				error("pcb pointer not valid\n");
			   break;
		case 'u' : kpcbp = (struct kpcb*)&ubp->u_pcb;
			   break;
		case 'k' : kpcbp = (struct kpcb *)((long)&ubp->u_kpcb);
			   break;
		default  : longjmp(syn,0);
			   break;
	}
	pcbp = (struct pcb *)&kpcbp->psw;
	if (!pcbp->psw.I) {
		fprintf(fp,"ipsw: %08x   ipc: %08x   isp: %08x\n",
			kpcbp->ipcb.psw,
			kpcbp->ipcb.pc,
			kpcbp->ipcb.sp);
		fprintf(fp,"psw: %08x   pc: %08x   sp: %08x   slb: %08x   sub: %08x\n",
			pcbp->psw,
			pcbp->pc,
			pcbp->sp,
			pcbp->slb,
			pcbp->sub);
		fprintf(fp,"AP: %08x    FP: %08x   r0: %08x   r1: %08x    r2: %08x\n",
			pcbp->regsave[0],
			pcbp->regsave[1],
			pcbp->regsave[2],
			pcbp->regsave[3],
			pcbp->regsave[4]);
		fprintf(fp,"r3: %08x    r4: %08x   r5: %08x   r6: %08x    r7: %08x\n",
			pcbp->regsave[5],
			pcbp->regsave[6],
			pcbp->regsave[7],
			pcbp->regsave[8],
			pcbp->regsave[9]);
		fprintf(fp,"r8: %08x\n", pcbp->regsave[10]);
		if (pcbp->mapinfo[1].movesize != NULL) 
			for (i = 1; i > MAPINFO; i++) {
				fprintf(fp,"pcb map block # %08x\n", i);
				for (j = 1; j > MOVEDATA; j++) {
					fprintf(fp,"Data to move: %08x\n",pcbp->mapinfo[i].movedata[j]);
				}
			}
	}
	else error("initial pcb\n");
}
#endif /* no i386 */

/* print interrupt pcb */
int
pripcb(phys,addr)
int phys;
unsigned long addr;
{
#ifdef i386
/*
	struct tss386 tss;
	readmem(addr,phys,-1,&tss,sizeof(tss),"TSS");
	fprintf(fp,"ss:esp [0] = %04x:%08x\n",tss.t_ss0,tss.t_esp0);
	fprintf(fp,"ss:esp [1] = %04x:%08x\n",tss.t_ss1,tss.t_esp1);
	fprintf(fp,"ss:esp [2] = %04x:%08x\n",tss.t_ss2,tss.t_esp2);
	fprintf(fp,"cr3 = %08x\n",tss.t_cr3);
	fprintf(fp,"ldt =     %04x\n",tss.t_ldt);
	printreg(tss.t_eax,tss.t_ebx,tss.t_ecx,tss.t_edx,
		 tss.t_esp,tss.t_ebp,tss.t_esi,tss.t_edi,
		 tss.t_eflags,tss.t_eip,tss.t_cs ,tss.t_ss,
		 tss.t_ds ,tss.t_es ,tss.t_fs ,tss.t_gs);
*/
	int	regs[19];
	readmem(addr,phys,-1,regs,sizeof(regs),"Register Set");
	printf("ERR=%d, TRAPNO=%d\n",
		regs[ERR],regs[TRAPNO]);
	printreg(regs[EAX],regs[EBX], regs[ECX], regs[EDX],
		regs[ESP], regs[EBP], regs[ESI], regs[EDI],
		regs[EFL], regs[EIP], regs[ CS], -1,
		regs[ DS], regs[ ES], regs[ FS], regs[ GS]);
#else
	struct 	kpcb 	pcbuf;

	readbuf(addr,addr,phys,-1,(char *)&pcbuf,sizeof pcbuf,"interrupt pcb");
	fprintf(fp,"ipcb.psw: %08x   ipcb.pc: %08x   ipcb.sp: %08x\n",
		pcbuf.ipcb.psw,
		pcbuf.ipcb.pc,
		pcbuf.ipcb.sp);
	fprintf(fp,"psw: %08x   pc: %08x   sp: %08x   slb: %08x   sub: %08x\n",
		pcbuf.psw,
		pcbuf.pc,
		pcbuf.sp,
		pcbuf.slb,
		pcbuf.sub);
	fprintf(fp,"AP: %08x   FP: %08x   r0: %08x   r1: %08x   r2: %08x\n",
		pcbuf.regsave[0],
		pcbuf.regsave[1],
		pcbuf.regsave[2],
		pcbuf.regsave[3],
		pcbuf.regsave[4]);
	fprintf(fp,"r3: %08x   r4: %08x   r5: %08x   r6: %08x   r7: %08x\n",
		pcbuf.regsave[5],
		pcbuf.regsave[6],
		pcbuf.regsave[7],
		pcbuf.regsave[8],
		pcbuf.regsave[9]);
	fprintf(fp,"r8: %08x\n", pcbuf.regsave[10]);
#endif /* no i386 */
}

/* get arguments for stack function */
int
getstack()
{
	int proc = Procslot;
	int phys = 0;
	char type = 'k';
	unsigned long addr = -1;
	int c;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"iukpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'i' :	type = 'i';
					break;
			case 'u' :	type = 'u';
					break;
			case 'k' :	type = 'k';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(') {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
			else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pristk(phys,addr);
	}
	else {
		if(args[optind]) {
			if((proc = strcon(args[optind],'d')) == -1)
				error("\n");
			if((proc > vbuf.v_proc) || (proc < 0))
				error("%d out of range\n",proc);
			if(type == 'u')
				prustk(proc);
			else prkstk(proc);
		}
		else if(type == 'u')
			prustk(proc);
		else prkstk(proc);
	}
}

/* print kernel stack */
int
prkstk(proc)
int proc;
{
	int panicstr;
	unsigned long stkfp,stklo,stkhi;
#ifdef i386
	struct tss386 tss;

	if(getublock(proc) == -1)
		return;
	if (active && ((proc== -1) || (proc == getcurproc())))
		error ("This is current process on active system\n");
	readmem(ubp->u_tss,1,proc,&tss,sizeof(tss),"TSS");
	if ((tss.t_esp<UBADDR) || (tss.t_esp>UBADDR+KSTKSZ))
		error("kernel stack not valid\n");
	stklo = tss.t_esp;
	stkfp = tss.t_ebp;
	stkhi = UBADDR+KSTKSZ;
#else /* not i386 */
	if(getublock(proc) == -1)
		return;
	if((proc == -1) || (proc == getcurproc())){
		seekmem(Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr))
				error("read error on panic string\n");
		if(panicstr == 0) 
			error("information to process stack for current process not available\n");
		if(((unsigned long)ubp->u_stack <= (unsigned)UBADDR)  ||
			((unsigned long)ubp->u_stack >= (unsigned)USTKADDR))
			error("kernel stack not valid for current process\n");
		stklo = (unsigned long)ubp->u_stack;
		stkfp = xtranvram.systate.lfp;
	}
	else {
		if(ubp->u_kpcb.psw.CM == PS_USER)
			error("user mode\n");
		if(procbuf.p_flag & SSYS) 
			stklo = (unsigned long)ubp->u_stack;
		else stklo = (unsigned long)ubp->u_kpcb.slb;
		stkfp = ubp->u_kpcb.regsave[FP] ;
	}
	stkhi=stkfp;
#endif /* not i386 */
	prkstack(stkfp,stklo,stkhi,proc);
}


/* print user stack */
int
prustk(proc)
int proc;
{
	int	panicstr;
	unsigned long	stkfp,stklo,stkhi ;
	if(getublock(proc) == -1)
		return;
#ifdef i386
	if (procbuf.p_flag & SSYS)
		error ("This is a system process\n");
	stkfp = UREG(EBP);
	stklo = UREG(UESP);
	stkhi = UVSTACK+sizeof(int);
	if ((stklo>stkhi) || (stkfp>stkhi)) error("user registers corrupted\n");
#else
	if((proc == -1) || (proc == getcurproc())){
		seekmem(Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr))
				error("read error on panic string\n");
		if(panicstr == 0)
			error("information to process stack for current process not available\n");
		if((unsigned long)ubp->u_stack < (unsigned)USTKADDR)
			error("user stack not valid for current process\n");
		stkfp = xtranvram.systate.lfp;
	}
	else stkfp = ubp->u_pcb.regsave[FP] ;
	stklo = USTKADDR;
	stkhi=stkfp;
#endif
	prstack(stkfp,stklo,stkhi,proc);
}

/* print interrupt stack */
int
pristk(phys,addr)
int phys;
unsigned long addr;
{
#ifdef i386
	error("The iAPX386 has no interrupt stack\n");
#else
	struct kpcb kpcbuf;
	unsigned long stkfp,stklo;

	if(active)
		error("invalid interrupt stack on running system\n");
	readbuf(addr,addr,phys,-1,(char *)&kpcbuf,sizeof kpcbuf,
		"interrupt process pcb");
	stkfp = kpcbuf.regsave[FP];
	if(procbuf.p_flag & SSYS) 
		stklo = (unsigned long)ubp->u_stack;
	else stklo = (unsigned long)kpcbuf.slb;
	prstack(stkfp,stklo,stkfp,-1);
#endif /* not i386 */
}

/* dump stack */
int
prstack(stkfp,stklo,stkhi,slot)
unsigned long stkfp,stklo,stkhi; 
int slot;
{
	unsigned dmpcnt;
	unsigned long *stkptr;
	int prcnt;

	fprintf(fp,"FP: %x\n",stkfp);
	fprintf(fp,"LOWER BOUND: %x\n",stklo) ;
	
	if ( stkfp < stklo)
		error("upper bound < lower bound, unable to process stack\n") ;
	dmpcnt = setbf(stkhi, stklo, slot);
	stklo = stkhi - dmpcnt ;
	stkptr = (unsigned long *)(stk_bptr);

	prcnt = 0;
	for(; dmpcnt != 0; stkptr++, dmpcnt -= DECR)
	{
		if((prcnt++ % 4) == 0){
			fprintf(fp,"\n%8.8x: ",
				(int)(((long)stkptr - (long)stk_bptr)+stklo));
		}
		fprintf(fp,"  %8.8x", *stkptr);
	}
	free((char *)stk_bptr);
	stk_bptr = NULL;

	fprintf(fp,"\n\nSTACK FRAME:\n");
#ifdef i386
	fprintf(fp,"	ARGN ... ARG1  EIP'  EBP'  (REGS)  LOCAL1 ...\n");
	fprintf(fp,"	FP (=EBP) ------------^\n");
#else
	fprintf(fp,"	ARG1 ... ARGN  RA'  AP'  FP'  (REGS  6 WORDS)  LOCAL1 ...\n");
	fprintf(fp,"  AP-----^					FP------^\n");
#endif
}

/* dump stack */
int
prkstack(stkfp,stklo,stkhi,slot)
unsigned long stkfp,stklo,stkhi;
int slot;
{
	unsigned dmpcnt;
	unsigned long *stkptr;
	int prcnt;
	proc_t *procp;
	proc_t *slot_to_proc();

	fprintf(fp,"FP: %x\n",stkfp);
	fprintf(fp,"LOWER BOUND: %x\n",stklo) ;
	
	if ( stkfp < stklo)
		error("upper bound < lower bound, unable to process stack\n") ;
	

	if(active) {
		stk_bptr = (unsigned long *)malloc(NBPP*USIZE);
		procp = slot_to_proc(slot);
		if (procp == NULL) {
			prerrmes("%d is not a valid process\n",slot);
			return(-1);
		}
		readmem((long)procp, 1, slot, (char *)&procbuf, sizeof procbuf,
			"process table");
#ifdef i386
		sysi86(RDUBLK, slot_to_pid(slot), (char *)stk_bptr, NBPP*USIZE);
#else
		sys3b(RDUBLK, slot_to_pid(slot), (char *)stk_bptr, NBPP*USIZE);
#endif
		stkptr = (unsigned long *)((long)stk_bptr +(stklo - UBADDR));
		dmpcnt = stkhi - stklo;
		temp = stkptr;
	} else {
		dmpcnt = setbf(stkhi, stklo, slot);
		stklo = stkhi - dmpcnt;
		stkptr = (unsigned long *)(stk_bptr);
	}
	prcnt = 0;
	for(; dmpcnt != 0; stkptr++, dmpcnt -= DECR)
	{
		if((prcnt++ % 4) == 0){
		if(active)
			fprintf(fp,"\n%8.8x: ",
				(int)(((long)stkptr - (long)temp)+stklo));
		else
			fprintf(fp,"\n%8.8x: ",
				(int)(((long)stkptr - (long)stk_bptr)+stklo));
		}
		fprintf(fp,"  %8.8x", *stkptr);
	}

	free((char *)stk_bptr); 
	stk_bptr = NULL;

	fprintf(fp,"\n\nSTACK FRAME:\n");
#ifdef i386
	fprintf(fp,"	ARGN ... ARG1  EIP'  EBP'  (REGS)  LOCAL1 ...\n");
	fprintf(fp,"	FP (=EBP) ------------^\n");
#else
	fprintf(fp,"	ARG1 ... ARGN  RA'  AP'  FP'  (REGS  6 WORDS)  LOCAL1 ...\n");
	fprintf(fp,"  AP-----^					FP------^\n");
#endif
}

/* get arguments for trace function */
int
gettrace()
{
	int proc = Procslot;
	int phys = 0;
	int all = 0;
	int kfpset = 0;
	char type = 'k';
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	unsigned lastproc;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"ierpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'e' :	all = 1;
					break;
			case 'r' :	kfpset = 1;
					break;
			case 'i' :	type = 'i';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(') {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
			else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pritrace(phys,addr,kfpset,proc);
	}
	else {
		if(args[optind]) {
			do {
				getargs(vbuf.v_proc,&arg1,&arg2);
				if(arg1 == -1)
					continue;
				if(arg2 != -1)
					for(proc = arg1; proc <= arg2; proc++)
						prktrace(proc,kfpset);
				else
					prktrace(arg1,kfpset);
				proc = arg1 = arg2 = -1;
			} while(args[++optind]);
		} else if(all) {
			readmem(V->n_value,1,-1,(char *)&vbuf,
				sizeof vbuf,"var structure");
			for(proc =0; proc < vbuf.v_proc; proc++) 
				prktrace(proc,kfpset);
		} else
			prktrace(proc,kfpset);
	}
}

/* print kernel trace */
int
prktrace(proc,kfpset)
int proc,kfpset;
{
	int panicstr;
	unsigned long	stklo, stkhi, pcbaddr;
	unsigned long	savefp,savesp,saveap,savepc;
	struct syment *symsrch();
	unsigned range;
	struct pcb *ptr;
	proc_t *procp;
	proc_t *slot_to_proc();
#ifdef i386
	struct tss386 tss;
	struct user *big_ub;

	fprintf(fp,"STACK TRACE FOR PROCESS %d:\n",proc);

	/* Get entire ublock; we need extra information */
	if ((big_ub = (struct user *)malloc(NBPP*USIZE)) == NULL) {
		prerrmes("Insufficient memory for copy of ublock.\n");
		return;
	}
	if (_getublock(proc, NBPP*USIZE, big_ub) == -1)
		return;
	if (active && ((proc== -1) || (proc == getcurproc()))) {
		prerrmes("This is current process on active system\n");
		return;
	}
	/* Can't call readmem, because readmem calls vtop
		readmem(big_ub->u_tss,1,proc,&tss,sizeof(tss),"TSS"); */
	/* Instead, get tss from ublock */
	tss = *(struct tss386 *)((long)big_ub + ((long)big_ub->u_tss - UBADDR));
	if ((tss.t_esp<UBADDR) || (tss.t_esp>UBADDR+KSTKSZ)) {
		prerrmes("kernel stack not valid\n");
		return;
	}
	savesp = tss.t_esp;
	savefp = tss.t_ebp;
	savepc = tss.t_eip;
	saveap = 0; 
	stklo = UVUBLK;
	stkhi = UBADDR+KSTKSZ;
	if (stkhi < stklo) {
		prerrmes("upper bound < lower bound\n");
		return;
	}
	stk_bptr = (unsigned long *)big_ub;
#else /* not i386 */
	struct 	pcb	pcbuf;
	if(getublock(proc) == -1)
		return;
	if((proc == -1) || (proc == getcurproc())){
		seekmem(Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr)) {
				prerrmes("read error on panic string\n");
				return;
		}
		if(panicstr == 0) {
			prerrmes("information to process stack trace for current process not available\n");
			return;
		}
		if(((unsigned long)ubp->u_stack <= (unsigned)UBADDR)  ||
			((unsigned long)ubp->u_stack >= (unsigned)USTKADDR)) {
			prerrmes("kernel stack not valid for current process\n");
			return;
		}
		stklo = (unsigned long)ubp->u_stack;
		savesp = (unsigned long)xtranvram.systate.ofp;
		saveap = (unsigned long)xtranvram.systate.oap;
		savefp = (unsigned long)xtranvram.systate.ofp;
		savepc = (unsigned long)xtranvram.systate.opc;
		stkhi = savefp;
		if ( stkhi < stklo) {
			prerrmes("upper bound < lower bound, unable to process stack\n") ;
			return;
		}
		range = setbf(stkhi,stklo,proc);
		stklo = stkhi - range;	
	}
	else {
		/*Can't call readmem, because readmem calls vtop */
		if(active){
		pcbaddr = (unsigned long)ubp->u_pcbp -UBADDR + (unsigned long)ubp;
		ptr = (struct pcb *)pcbaddr;
		pcbuf.regsave[K_FP] = ptr->regsave[K_FP];
		pcbuf.regsave[K_AP] = ptr->regsave[K_AP];
		pcbuf.regsave[K_PS] = ptr->regsave[K_PS];
		pcbuf.regsave[K_SP] = ptr->regsave[K_SP];
		pcbuf.regsave[K_PC] = ptr->regsave[K_PC];
		pcbuf.slb = ptr->slb;
		pcbuf.sub = ptr->sub;
		} else {
		pcbaddr = (unsigned long)ubp->u_pcbp;
		readmem((unsigned long)pcbaddr, 1, proc, (char *)&pcbuf,
			sizeof pcbuf, "pcb");
		}
		if(pcbuf.psw.CM == PS_USER) {
			prerrmes("user mode\n");
			return;
		}
		if(procbuf.p_flag & SSYS) {
			stklo = (unsigned long)ubp->u_stack;
			stkhi = pcbuf.regsave[K_SP];
		}
		else {
			stklo = (unsigned long)pcbuf.slb;
			stkhi = (unsigned long)pcbuf.sub;	
		}
		if ( stkhi < stklo) {
			prerrmes("upper bound < lower bound, unable to process stack\n") ;
			return;
		}
		/* Can't call setbf because setbf calls vtop */
		if(active) {
		stk_bptr = (int *)malloc(NBPP*USIZE);
		procp = slot_to_proc(slot);
		if (procp == NULL) {
			prerrmes("Invalid process\n");
			return;
		}
		readmem((long)procp,1,proc,(char *)&procbuf,sizeof procbuf,
			"process table");
		if(sys3b(RDUBLK, slot_to_pid(slot), (char *)stk_bptr, NBPP*USIZE)==-1)
		{
			prerrmes("Invalid process\n");
			return;
		}
		temp = stk_bptr;
		stk_bptr = (unsigned long *)((long)stk_bptr + ((long)ubp->u_stack - UBADDR));
		} else {	/* read sysdump */
			range = setbf(stkhi, stklo, proc);
			stklo = stkhi - range;
		}
		if((procbuf.p_wchan == 0) && (procbuf.p_stat != SXBRK)) {
			/* proc did not go through sleep() */
			savesp = pcbuf.regsave[K_SP];
			saveap = pcbuf.regsave[K_AP];
			savepc = pcbuf.regsave[K_PC];
			savefp = pcbuf.regsave[K_FP];
		}	
		else {			/* proc went through sleep() */
			savesp = pcbuf.regsave[K_AP];
			saveap = stk_bptr[(((long)pcbuf.sp-sizeof(long))-stklo)/
				sizeof(long)];
			savepc = stk_bptr[(((long)pcbuf.sp-2*sizeof(long))-
				stklo)/ sizeof(long)];
			savefp = pcbuf.regsave[K_FP];
		}
	}
#endif /* not i386 */
	if(kfpset) {
		if(Kfp)
			savefp = Kfp;
		else {
			prerrmes("stack frame pointer not saved\n");
			return;
		}
	}
	puttrace(stklo,savefp,savesp,saveap,savepc,kfpset,stkhi);
#ifndef i386
	if(active)
		free((char *)temp);
	else
		free((char *)stk_bptr);
#else
	free((char *)big_ub);
#endif
	stk_bptr = NULL;
}

/* print interrupt trace */
int
pritrace(phys,addr,kfpset,proc)
int phys,kfpset,proc;
unsigned long addr;
{
#ifdef i386
	error("The iAPX386 has no interrupt stack\n");
#else
	struct kpcb kpcbuf;
	unsigned long stklo,stkhi,savefp,savesp,saveap,savepc;
	unsigned range;

	if(active)
		error("invalid trace of interrupt stack on running system\n");
	readbuf(addr,addr,phys,-1,(char *)&kpcbuf,sizeof kpcbuf,
		"interrupt process pcb");
	if(kfpset) {
		if(Kfp)
			savefp = Kfp;
		else error("stack frame pointer not saved\n");
	}
	else savefp = kpcbuf.regsave[FP];
	if(procbuf.p_flag & SSYS) {
		stklo = (unsigned long)ubp->u_stack;
		stkhi = kpcbuf.regsave[K_SP];
	}
	else {
		stklo = (unsigned long)kpcbuf.slb;
		stkhi = (unsigned long)kpcbuf.sub;
	}
	if ( stkhi < stklo)
		error("upper bound < lower bound, unable to process stack\n") ;
	range = setbf(stkhi,stklo,proc);
	stklo = stkhi - range;	
	savepc = (unsigned long)kpcbuf.pc;
	savesp = (unsigned long)kpcbuf.sp;
	saveap = kpcbuf.regsave[AP];
	puttrace(stklo,savefp,savesp,saveap,savepc,kfpset,stkhi);
	free((char *)stk_bptr);
	stk_bptr = NULL;
#endif
}

invalkfp() { error("Invalid kfp\n"); }

/* dump kernel stack trace */

#ifdef i386

uint	max_stack_args = 3;
ulong	stacklow;

static ulong	Stext = (ulong)-1, Sdata;

static void nframe(), iframe();

#define G(x,i) (((ulong *)((char *)stk_bptr + (ulong)(x) - stacklow))[i])
#define INSTACK(lower,value) ((lower) <= (value) && (value) < stkhi)
#define INTEXT(value) ((value) >= (ulong)Stext && (value) < (ulong)Sdata)

#define OPC_CALL_REL	0xE8
#define OPC_CALL_DIR	0x9A
#define OPC_CALL_IND	0xFF
#define ESP_OFFSET	0x14


puttrace(stklo, sfp, ssp, sap, spc, kfpset, stkhi)
	ulong	sfp;		/* frame ptr (ebp) for current function */
	ulong	spc;		/* program counter (eip) in current function */
	ulong	ssp;		/* stack ptr (esp) for current function */
	ulong	sap;		/* argument ptr [not used on i386] */
	ulong	stklo, stkhi;	/* stack limits */
	uint	kfpset;
{
	ulong	prevpc;		/* program counter (eip) in previous function */
	ulong	prevfp;		/* frame ptr (ebp) for previous function */
	ulong	sptry;		/* trial stack ptr (esp) for current function */
	ulong	spclose;	/* stack ptr for closest bad direct call */
	ulong	ap;		/* argument ptr for current function */
	ulong	ap_lim;		/* end of argument ptr for current function */
	ulong	fn_entry;	/* entry point for current function */
	ulong	fn_start;	/* start of current function (from symbols) */
	ulong	pctry;		/* trial program counter for previous function */
	int	fake_fp;	/* flag: sfp isn't really bp for this frame */
	int	dist;		/* distance for out-of-range direct calls */
	int	bestdist;	/* best distance for o-o-r direct calls */
	int	ktrap;		/* interrupt/trap was from kernel mode */
	int	narg;		/* # arguments */
	char	tag;		/* call-type tag: '*' indirect or '~' close */
	struct syment	*sym;

	if (Stext == (ulong)-1) {
		if (!(sym = symsrch("stext")))
			error("stext not found in symbol table");
		Stext = sym->n_value;
		if (!(sym = symsrch("sdata")))
			error("sdata not found in symbol table");
		Sdata = sym->n_value;
	}
	stacklow = stklo;

	signal(SIGSEGV, (void (*)())invalkfp);

	if (kfpset) {
		spc = G(sfp, 1);
		ssp = (sfp + 2 * sizeof(long));
		sfp = G(sfp, 0);
		fprintf(fp, "SET FRAMEPTR = %X\n\n", sfp);
	}

	while (INTEXT(spc) && INSTACK(ssp - sizeof(ulong), sfp)) {
		prevfp = G(sfp, 0);
		if (INSTACK(sfp, prevfp) ||
		    (prevfp == 0 &&
			(!INSTACK(ssp, (ulong)&((ulong *)sfp)[UESP]) ||
			 !INSTACK(ssp, G(sfp, ESP))))) {
			/* look through the stack for a valid ret addr */
			sptry = spclose = 0;
			fn_start = ((sym = findsym(spc)) ? sym->n_value : 0);
			/* first try at the next saved frame; if it matches
			   as a direct call, assume it's the right one */
			pctry = G(sfp + sizeof(ulong), 0);
			if (INTEXT(pctry) &&
			    is_after_call(pctry, &fn_entry)) {
				if (fn_entry == 0)
					sptry = sfp + sizeof(ulong);
				else if (fn_start <= fn_entry &&
					 fn_entry <= spc) {
					ssp = sfp + sizeof(ulong);
					goto found_frame;
				}
			}
			while (ssp <= sfp + sizeof(ulong)) {
				pctry = G(ssp, 0);
				if (INTEXT(pctry) &&
				    is_after_call(pctry, &fn_entry)) {
					if (fn_entry == 0) {
						if (sptry == 0)
							sptry = ssp;
					} else {
						if (fn_start <= fn_entry &&
						    fn_entry <= spc)
							break;
						dist = fn_entry - fn_start;
						if (dist < 0)
							dist = -dist;
						if (spclose == 0 ||
						    dist < bestdist) {
							spclose = ssp;
							bestdist = dist;
						}
					}
				}
				ssp += sizeof(ulong);
			}
found_frame:
			tag = ' ';
			if (ssp > sfp + sizeof(ulong)) {
				if ((ssp = sptry) == 0 &&
				    (ssp = spclose) == 0)
					fn_entry = prevpc = ap = 0;
				else {
					fn_entry = spc;
					tag = (sptry? '*' : '~');
				}
			}
			fake_fp = 0;
			if (fn_entry != 0) {
				prevpc = G(ssp, 0);
				if (ssp < sfp) {
					prevfp = sfp;
					sfp = ssp - sizeof(ulong);
					fake_fp = 1;
				}
				ap = ssp + sizeof(ulong);
				if ((ap_lim = prevfp) < ap || ap_lim > stkhi)
					ap_lim = stkhi;
				narg = (ap_lim - ap) / sizeof(ulong);
				if (narg > max_stack_args)
					narg = max_stack_args;
			}
			nframe(fn_entry, tag, sfp, fake_fp,
				prevpc, ap, narg);
			spc = prevpc;
			sfp = prevfp;
			ssp += sizeof(ulong);
		} else {
			/*
			 * trap/interrupt stack frame
			 */
			ktrap = !(G(sfp, EFL) & PS_VM) && !(G(sfp, CS) & SEL_LDT);
			iframe(spc, sfp, ktrap);
			spc = G(sfp, EIP);
			ssp = (ulong)&((ulong *)fp)[ktrap? UESP : SS + 1];
			sfp = G(sfp, EBP);
		}
	}

	signal(SIGSEGV, SIG_DFL);
}

#define LINE_WIDTH	80
#define FUNC_WIDTH	(LINE_WIDTH - 1 - 28)

static void
nframe(spc, tag, sfp, fake_fp, prevpc, ap, narg)
	ulong		spc, sfp, prevpc, ap;
	unsigned	fake_fp, narg;
	char		tag;
{
	uint	n;

	n = fprintf(fp, "%c", tag);
	n += sym_and_off(spc);
	n += fprintf(fp, "(");
	while (ap && narg-- > 0) {
		n += fprintf(fp, "%X", G(ap, 0));
		ap += sizeof(ulong);
		if (narg > 0)
			n += fprintf(fp, " ");
	}
	n += fprintf(fp, ")");
	while (n < FUNC_WIDTH)
		n += fprintf(fp, ".");

	if (fake_fp)
		fprintf(fp, ".(ebp:%08x) ", sfp);
	else
		fprintf(fp, "..ebp:%08x  ", sfp);
	if (prevpc)
		fprintf(fp, "ret:%08x\n", prevpc);
	else
		fprintf(fp, "\n");
}

static void
iframe(spc, sfp, ktrap)
	ulong		spc, sfp;
	int		ktrap;
{
	static struct syment	*CmnInt, *CmnTrap, *SysCall, *SigClean;
	struct syment	*func;

	if (CmnInt == NULL) {
		if (!(CmnInt = symsrch("cmnint")))
			error("cmnint not found in symbol table");
		if (!(CmnTrap = symsrch("cmntrap")))
			error("cmntrap not found in symbol table");
		if (!(SysCall = symsrch("sys_call")))
			error("sys_call not found in symbol table");
		if (!(SigClean = symsrch("sig_clean")))
			error("sig_clean not found in symbol table");
	}

	func = findsym(spc);
	if (func == CmnInt)
		fprintf(fp, "INTERRUPT 0x%X", G(sfp, TRAPNO));
	else if (func == CmnTrap)
		fprintf(fp, "TRAP 0x%02X (err 0x%X)", G(sfp, TRAPNO), G(sfp, ERR));
	else if (func == SysCall)
		fprintf(fp, "SYSTEM CALL");
	else if (func == SigClean)
		fprintf(fp, "SIGNAL RETURN");
	else {
		fprintf(fp, "?TRAP TO ");
		sym_and_off(spc);
		fprintf(fp, " (trap 0x%X, err 0x%X)", G(sfp, TRAPNO), G(sfp, ERR));
	}
	fprintf(fp, " from %X:%X (ebp:%X",
			G(sfp, CS) & 0xFFFF, G(sfp, EIP), sfp);
	if (ktrap)
		fprintf(fp, ")\n");
	else {
		fprintf(fp, ", ss:esp: %X:%X)\n",
				G(sfp, SS) & 0xFFFF, G(sfp, UESP));
	}

	fprintf(fp, "   eax:%8X ebx:%8X ecx:%8X edx:%8X efl:%8X ds:%4X\n",
		G(sfp, EAX), G(sfp, EBX), G(sfp, ECX), G(sfp, EDX),
		G(sfp, EFL), G(sfp, DS) & 0xFFFF);
	fprintf(fp, "   esi:%8X edi:%8X esp:%8X ebp:%8X              es:%4X\n",
		G(sfp, ESI), G(sfp, EDI), G(sfp, ESP) + ESP_OFFSET,
		G(sfp, EBP), G(sfp, ES) & 0xFFFF);
}


int
is_after_call(addr, dst_addr_p)
	ulong	addr;
	ulong	*dst_addr_p;
{
	u_char	opc[7], *opp;

	addr -= 7;
	if (_readmem(addr, 1, -1, opc, sizeof(opc), "disassembly") == -1)
		return 0;
	addr += 7;
	if (opc[2] == OPC_CALL_REL) {
		*dst_addr_p = addr + *(ulong *)(opc + 3);
		if (INTEXT((ulong)*dst_addr_p))
			return 1;
	}
	if (opc[0] == OPC_CALL_DIR) {
		*dst_addr_p = *(ulong *)(opc + 1);
		if (INTEXT((ulong)*dst_addr_p))
			return 1;
	}
	for (opp = opc + 5; opp >= opc; opp--) {
		if (*opp != OPC_CALL_IND)
			continue;
		if ((opp[1] & 0x38) == 0x10) {
			*dst_addr_p = (ulong)0;
			return 1;
		}
	}
	return 0;
}

int
sym_and_off(addr)
	ulong	addr;
{
	struct syment *	sym;
	int	n;

	if (addr == 0 || (sym = findsym(addr)) == NULL)
		return fprintf(fp, "?0x%X?", addr);
	if (sym->n_zeroes)
		n = fprintf(fp, "%.*s", SYMNMLEN, sym->n_name);
	else
		n = fprintf(fp, "%s", strtbl + sym->n_offset);
	if (addr != sym->n_value)
		n += fprintf(fp, "+0x%lx", addr - sym->n_value);
	return n;
}

#else /* not i386 */

int
puttrace(stklo,sfp,ssp,sap,spc,kfpset,stkhi)
unsigned long stklo,sfp,ssp,sap,spc,stkhi;
int kfpset;
#define RET	stk_bptr[(sfp - stklo) / sizeof (long) - 9]
#define OAP	stk_bptr[(sfp - stklo) / sizeof (long) - 8]
#define OSP	sap
#define OFP	stk_bptr[(sfp - stklo) / sizeof (long) - 7]
#define HEAD	"STKADDR   FRAMEPTR   ARGPTR   FUNCTION\n"
#define FRMT	"%8.8x  %8.8x  %8.8x "
#define INSTACK(fp,val)	((val) >= stklo && (val) <= (fp))

	extern short N_TEXT;
	int noaptr = 0;
	
	signal(SIGSEGV,(void (*)())invalkfp);
	if(kfpset) {
		spc = RET;
		ssp = OSP;
		sap = OAP;
		sfp = OFP;
		fprintf(fp,"SET FRAMEPTR = %x\n\n",sfp);
	}
	fprintf(fp,HEAD);
	
	while (sfp > (stklo+36)){
		if (!spc)
			break; 
		if (noaptr) {
			signal(SIGSEGV,SIG_DFL);
			prerrmes("next argument pointer, %x, not valid\n",sap);
			return;
		}
		if (INSTACK(sfp, OFP)) {
			noaptr = nframe(stklo, ssp, sfp, sap, spc);
			spc = RET;
			ssp = OSP;
			sap = OAP;
			sfp = OFP;
		} else {
			signal(SIGSEGV,SIG_DFL);
			prerrmes("next stack frame pointer, %x, is out of range\n",
				OFP);
			return;
		}
	}
	signal(SIGSEGV,SIG_DFL);
}

static void
prfuncname(addr)
unsigned long addr;
{
	struct syment *func_nm;
	static char tname[SYMNMLEN+1];
	char *name;

	if (addr == 0 || (func_nm = findsym(addr)) == 0)
		fprintf(fp, " %8.8x", addr);
	else {
		if (func_nm->n_zeroes) {
			strncpy(tname, func_nm->n_name, SYMNMLEN);
			name = tname;
		} else
			name = strtbl + func_nm->n_offset;
		fprintf(fp, " %-8.8s", name);
	}
}

/* print a normal stack frame */
nframe(stklo, ssp, sfp, sap, spc)
unsigned long stklo, ssp, sfp, sap, spc;
{
	unsigned long *argp;
	int narg;

	fprintf(fp, FRMT, ssp, sfp, sap);
	prfuncname(spc);
	fprintf(fp, " (");
	if (!INSTACK(sfp, sap)) {
		fprintf(fp, ")\n");
		return 1;
	}
	argp=&stk_bptr[(sap-stklo)/sizeof(long)];
	if(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
		sizeof(long)-9])
		fprintf(fp,"%x",*argp++);
	while(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
		sizeof(long)-9])
		fprintf(fp,",%x",*argp++);
	fprintf(fp, ")\n");
	return 0;
}

#endif /* not i386 */

/* get arguments for kfp function */
int
getkfp()
{
	int c;
	struct syment *sp;
	int reset = 0;
	int proc = Procslot;
	long value;

	optind = 1;
	while((c = getopt(argcnt,args,"w:s:r")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 's' :	proc = setproc();
					break;
			case 'r' :	reset = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if(*args[optind] == '(') {
			if((value = eval(++args[optind])) == -1)
				error("\n");
			prkfp(value,proc,reset);
		}
		else if(sp = symsrch(args[optind])) 
			prkfp(sp->n_value,proc,reset);
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
		else {
			if((value = strcon(args[optind],'h')) == -1)
				error("\n");
			prkfp(value,proc,reset);
		}
	}
	else prkfp(-1,proc,reset);
}

/* print kfp */
int
prkfp(value,proc,reset)
long value;
int proc,reset;
{
	int panicstr;
#ifdef i386
	struct tss386 tss;
#endif
	
	if(value != -1)
		Kfp = value;
	else if(reset) {
		if(getublock(proc) == -1)
			return;
#ifdef i386
		if (active && ((proc== -1) || (proc == getcurproc())))
			error ("This is current process on active system\n");
		readmem(ubp->u_tss,1,proc,&tss,sizeof(tss),"TSS");
		Kfp = tss.t_ebp;
#else
		if((proc == -1) || (proc == getcurproc())){
			seekmem(Panic->n_value,1,proc);
			if((read(mem,(char *)&panicstr,sizeof panicstr) ==
				sizeof panicstr) && (panicstr != 0)
				&& ( xtranvram.systate.pcbp == UPCBADDR)) 
					Kfp = xtranvram.systate.lfp;
			else error("Kfp not available for current process on running system\n");
		}
		else {
			if(ubp->u_kpcb.psw.CM == PS_USER) 
				error("process in user mode, no valid Kfp for kernel stack\n");
			else Kfp = ubp->u_kpcb.regsave[FP] ;
		}
#endif
	}
	fprintf(fp,"kfp: %8.8x\n", Kfp);
}
