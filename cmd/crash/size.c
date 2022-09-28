/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:size.c	1.14.7.1"

/*
 * This file contains code for the crash functions:  size, findslot, and
 * findaddr.  The size table for RFS and Streams structures is located in
 * sizenet.c
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#ifndef i386
#include <sys/psw.h>
#include <sys/pcb.h>
#endif
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/var.h>
#include <sys/buf.h>
#include <sys/callo.h>
#include <sys/conf.h>
#include <sys/fstyp.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include <sys/immu.h>
#include <vm/vm_hat.h>
#include <vm/hat.h>
#include <vm/seg.h>
#include <vm/as.h>
#include <vm/page.h>
#include <sys/proc.h>
#include <sys/termios.h>
#define _KERNEL
#include <sys/stream.h>
#undef _KERNEL
#include <sys/strtty.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/fs/s5inode.h>
#include <sys/fs/snode.h>
#include <sys/fs/fifonode.h>
#include <sys/procfs.h>
#include <fs/proc/prdata.h>
#include "crash.h"

struct sizetable {
	char *name;
	char *symbol;
	unsigned size;
	int indirect;
};

struct sizetable siztab[] = {
	"buf","buf",sizeof (struct buf),0,
	"callo","callout",sizeof (struct callo),0,
	"callout","callout",sizeof (struct callo),0,
	"file","file",sizeof (struct file),0,
	"flckinfo","flckinfo",sizeof (struct flckinfo),0,
	"fifonode","fifonode",sizeof (struct fifonode),0,
	"filock","flox",sizeof (struct filock),0,
	"flox","flox",sizeof (struct filock),0,
	"inode","inode",sizeof (struct inode),0,
	"pp","pp",sizeof (struct page),0,
	"prnode","prnode",sizeof (struct prnode),0,
	"proc","proc",sizeof (struct proc),0,
	"snode","snode",sizeof (struct snode),0,
	"tty","tty",sizeof (struct strtty),0,
	"vfs","vfs",sizeof (struct vfs),0,
	"vfssw","vfssw",sizeof (struct vfssw),0,
	"vnode","vnode",sizeof (struct vnode),0,
	NULL,NULL,0,0
};	

extern struct sizetable	siznettab[];


/* get entry from size tables */
struct sizetable *
getsizetab(name)
char *name;
{
	struct sizetable *st;

	for(st = siztab; st->name; st++) {
		if(!(strcmp(st->name,name)))
			return st;
	}
	for(st = siznettab; st->name; st++) {
		if(!(strcmp(st->name,name)))
			return st;
	}
	error("no match for %s in sizetable\n",name);
}

/* get arguments for size function */
int
getsize()
{
	int c;
	char *all = "";
	int hex = 0;

	optind = 1;
	while((c = getopt(argcnt,args,"xw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'x' : 	hex = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do{
			prtsize(args[optind++],hex);
		}while(args[optind]);
	}
	else prtsize(all,hex);
}

/* print size */
int
prtsize(name,hex)
char *name;
int hex;
{
	unsigned size;
	struct sizetable *st;
	int i;

	if(!strcmp("",name)) {
		for(st = siztab,i = 0; st->name; st++,i++) {
			if(!(i & 3))
				fprintf(fp,"\n");
			fprintf(fp,"%-15s",st->name);
		}
		for(st = siznettab,i = 0; st->name; st++,i++) {
			if(!(i & 3))
				fprintf(fp,"\n");
			fprintf(fp,"%-15s",st->name);
		}
		fprintf(fp,"\n");
	}
	else {
		size = getsizetab(name)->size;
		if(hex)
			fprintf(fp,"0x%x\n",size);
		else fprintf(fp,"%d\n",size);
	}
}
	

/* get arguments for findaddr function */
int
getfindaddr()
{
	int c;
	int slot;
	char *name;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		name = args[optind++];
		if(args[optind]) {
			if((slot = (int)strcon(args[optind],'d')) == -1)
				error("\n");
			prfindaddr(name,slot);
		}
		else longjmp(syn,0);
	}
	else longjmp(syn,0);
}

/* print address */
int
prfindaddr(name,slot)
char *name;
int slot;
{
	struct syment *sp;
	struct sizetable *st;
	char symbol[10];
	unsigned long base;

	strcpy(symbol, (st = getsizetab(name))->symbol);
	if(!(sp = symsrch(symbol)))
		error("no match for %s in symbol table\n",name);
	if (st->indirect)
		readmem(sp->n_value,1,-1,(char *)&base,sizeof base,name);
	else
		base = sp->n_value;
	fprintf(fp,"%8x\n",base + st->size * slot);
}

/* get arguments for findslot function */
int
getfindslot()
{
	int c;
	unsigned long addr;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do{
			if((addr = strcon(args[optind++],'h')) == -1)
				continue;
			prfindslot(addr);
		}while(args[optind]);
	}
	else longjmp(syn,0);
}

/* print table and slot */
int
prfindslot(addr)
unsigned long addr;
{
	struct syment *sp, *indsrch();
	unsigned long base;
	int slot,offset;
	unsigned size;
	extern char *strtbl;
	extern struct syment *findsym();
	static char tname[SYMNMLEN+1];
	char *name;

	if(!(sp = findsym(addr)))
		error("no symbol match for %8x\n",addr);
	base = sp->n_value;

	/* See if an indirect table is a better choice */
	sp = indsrch(siztab,addr,sp,&base);
	sp = indsrch(siznettab,addr,sp,&base);

	if(sp->n_zeroes){
		strncpy(tname,sp->n_name,SYMNMLEN);
		name = tname;
	}
	else name = strtbl + sp->n_offset;
	size = getsizetab(name)->size;
	slot = (addr - base)/size;
	offset = (addr - base)%size;
	fprintf(fp,"%s",name);
	fprintf(fp,", slot %d, offset %d\n",slot,offset);
}


struct syment *indsrch(st, addr, sp, base)
	struct sizetable *st;
	unsigned long addr, *base;
	struct syment *sp;
{
	struct syment *sp2;
	unsigned long indaddr;

	for (; st->name; st++) {
		if (!st->indirect || !(sp2 = symsrch(st->symbol)))
			continue;
		readmem(sp2->n_value,1,-1,(char *)&indaddr,sizeof indaddr,st->symbol);
		if ((unsigned long)sp->n_value <= indaddr && indaddr <= addr) {
			sp = sp2;  *base = indaddr;
			break;
		}
	}
	return sp;
}
