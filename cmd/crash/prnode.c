/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:prnode.c	1.3.9.1"

/*
 * This file contains code for the crash function:  prnode.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/immu.h>
#include <sys/var.h>
#include <sys/proc.h>
#ifndef i386
#include <sys/psw.h>
#include <sys/pcb.h>
#include <sys/sbd.h>
#endif
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/procfs.h>		/* needs to be after user.h on 386 */
#include <sys/vfs.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/cred.h>
#include <fs/proc/prdata.h>
#include "crash.h"

extern char *vnode_header;

static struct syment *Prrootnode;	/* namelist symbol pointers */

/* get arguments for prnode function */
int
getprnode()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	struct prnode *pptr;
	char *heading = "SLOT   PROC     MODE   WRITER FLAGS\n";


	if(!Prrootnode)
		if(!(Prrootnode= symsrch("prrootnode")))
			error("prrootnode not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
					break;
		}
	}

	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_proc,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prprnode(all,full,slot,phys,addr,pptr,
						heading);
			else {
				if(arg1 < vbuf.v_proc)
					slot = arg1;
				else addr = arg1;
				prprnode(all,full,slot,phys,addr,pptr,heading);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_proc; slot++)
		prprnode(all,full,slot,phys,addr,pptr,heading);
}



/* print prnode */
int
prprnode(all,full,slot,phys,addr,pptr,heading)
int all,full,slot,phys;
long addr;
struct prnode *pptr;
char *heading;
{

	char ch;
	struct proc pbuf;
	struct vnode vnbuf;
	struct prnode prnbuf;
	long vp, pnp;
	extern long lseek();
	proc_t *procaddr;
	proc_t *slot_to_proc();

	if(addr > -1)
	{
		readmem(addr,1,-1,(char *)&prnbuf,sizeof prnbuf,"prnode");
	}
	else
	{
		procaddr = slot_to_proc(slot);
		if (procaddr)
			readmem((long)procaddr,1,-1,(char *)&pbuf,
				sizeof pbuf,"proc table");
		else
			return;
		if(pbuf.p_trace == 0)
			return;
		readmem((long)pbuf.p_trace,1,-1,(char *)&vnbuf,sizeof vnbuf,"vnode");
		readmem((long)vnbuf.v_data,1,-1,(char *)&prnbuf,sizeof prnbuf,"prnode");
	}

	if(full)
		fprintf(fp,"%s",heading);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);

	fprintf(fp,"  %8x    %s  %4d ",
		prnbuf.pr_proc,
		(prnbuf.pr_mode & 0600) ? "rw" : "  ",
		prnbuf.pr_writers);

	fprintf(fp,"%s%s\n",
		prnbuf.pr_flags & PREXCL ? " excl" : "",
		prnbuf.pr_flags & PRINVAL ? " inval" : "");
	if(!full)
		return;
	/* print vnode info */
	fprintf(fp,"\nVNODE :\n");
	fprintf(fp, vnode_header);
	prvnode(&prnbuf.pr_vnode);
	fprintf(fp,"\n");
}
